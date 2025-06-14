#include "AimbotMelee.h"

#include "../AimbotHelper.h"

// SDK
#include "../../../SDK/class/Source Entity.h"
#include "../../../SDK/class/BaseWeapon.h"
#include "../../../SDK/class/IVDebugOverlay.h"
#include "../../../SDK/class/FileWeaponInfo.h"
#include "../../../SDK/class/CCollisionProperty.h"
#include "../../../SDK/class/Basic Structures.h"
#include "../../../SDK/class/CUserCmd.h"
#include "../../../SDK/class/IEngineTrace.h"
#include "../../../SDK/TF object manager/TFOjectManager.h"
#include "../../MovementSimulation/MovementSimulation.h"
#include "../../../Extra/math.h"

/*
PROBLEM :
-> angle setting timing if fucked up!
*/

#define DEBUG_HULL_SIZE        false
#define DEBUG_MELEE_SWING_HULL true

constexpr float SWING_RANGE_MULTIPLIER = 1.2f;

//=========================================================================
//                     PUBLIC METHODS
//=========================================================================
void AimbotMelee_t::RunV2(BaseEntity* pLocalPlayer, baseWeapon* pActiveWeapon, CUserCmd* pCmd, bool* pSendPacket)
{
    if (Features::Aimbot::Melee_Aimbot::MeleeAimbot.IsActive() == false)
        return;

    // NOTE : m_flSmackTime is -1.0f when not in a swing, else holds the time of hit registeration

    // Scan & retrieve the best target only if we are not already in the swing.
    // Cause now the target is already locked!
    bool bSwingActive = pActiveWeapon->m_flSmackTime() > 0.0f;
    if (bSwingActive == false)
    {
        float flSmackDelay  = pActiveWeapon->GetTFWeaponInfo()->GetWeaponData(0)->m_flSmackDelay;
        float flSwingRange  = _GetSwingHullRange(pLocalPlayer, pActiveWeapon);
        BaseEntity* pTarget = _ChooseTarget(pLocalPlayer, flSmackDelay, flSwingRange);
        
        if (pTarget != nullptr)
            m_pBestTarget = pTarget;
    }
    else if(Features::Aimbot::Melee_Aimbot::MeleeAimbot_DebugPrediction.IsActive() == true && m_pBestTarget != nullptr)
    {
        _DrawPredictionDebugInfo(pActiveWeapon, pActiveWeapon, m_pBestTarget);
    }

    // No target found
    if (m_pBestTarget == nullptr)
        return;

    // if auto fire is active & we have a valid target, FIRE!!!
    if (Features::Aimbot::Melee_Aimbot::MeleeAimbot_AutoFire.IsActive() == true)
        pCmd->buttons |= IN_ATTACK;

    float flCurTime              = tfObject.pGlobalVar->interval_per_tick * pLocalPlayer->m_nTickBase();    
    bool  bHitRegisteredThisTick = flCurTime >= pActiveWeapon->m_flSmackTime() && bSwingActive == true;

    if (bHitRegisteredThisTick == true)
    {
        vec vTargetBestPos = _GetClosestPointOnEntity(pLocalPlayer, m_pBestTarget);
        qangle qTargetBestAngles;
        Maths::VectorAnglesFromSDK(vTargetBestPos - pLocalPlayer->GetEyePos(), qTargetBestAngles);

        // Silent aim
        pCmd->viewangles = qTargetBestAngles;
        *pSendPacket = false; // Setting silent aim

        // Release best target once swing is done, Now we are free to get another target.
        m_pBestTarget  = nullptr;
    }
}


void AimbotMelee_t::Reset()
{
    m_vAttackerFuturePos.Init();
    m_vBestTargetFuturePos.Init();

    m_pBestTarget = nullptr;
}


//=========================================================================
//                     PRIVATE METHODS
//=========================================================================
const BaseEntity* AimbotMelee_t::_ChooseTarget(BaseEntity* pLocalPlayer, float flSwingRange) const
{
    const auto& targetData = FeatureObj::aimbotHelper.GetAimbotTargetData();
    
    const vec& vEyePos = pLocalPlayer->GetEyePos();
    BaseEntity* pBestTarget    = nullptr;
    float       flBestDistance = std::numeric_limits<float>::infinity();
    
    for (BaseEntity* pTarget : targetData.m_vecEnemyPlayers)
    {
        if (Features::Aimbot::MovementSim::Debug_MovementSim.IsActive() == true)
        {
            // Initialize Movement Sim
            FeatureObj::movementSimulation.Initialize(pTarget);

            // Run ticks
            int nTicks = 1.0f / tfObject.pGlobalVar->interval_per_tick;
            printf("SImulating %d ticks\n", nTicks);
            for (int i = 0; i < nTicks; i++)
                FeatureObj::movementSimulation.RunTick();

            // Restore to original
            FeatureObj::movementSimulation.Restore();
        }

        const vec vClosestPoint = _GetClosestPointOnEntity(pLocalPlayer, pTarget); // TODO : just use one fn nigga!
        float flDist            = vEyePos.DistTo(vClosestPoint);

        if (flDist < flBestDistance)
        {
            flBestDistance = flDist;
            pBestTarget    = pTarget;
        }
    }

    return pBestTarget;
}

BaseEntity* AimbotMelee_t::_ChooseTarget(BaseEntity* pAttacker, float flSmackDelay, float flSwingRange)
{
    uint32_t nTicksToSimulate = flSmackDelay / tfObject.pGlobalVar->interval_per_tick;

    // ATTACKERS FUTURE POSITION
    FeatureObj::movementSimulation.Initialize(pAttacker);
    for (int i = 0; i < nTicksToSimulate; i++)
        FeatureObj::movementSimulation.RunTick();

    m_vAttackerFuturePos = FeatureObj::movementSimulation.m_moveData.m_vecAbsOrigin + pAttacker->m_vecViewOffset();
    FeatureObj::movementSimulation.Restore();

    BaseEntity* pBestTarget     = nullptr;
    float       flBestDistance  = std::numeric_limits<float>::infinity();
    vec         vBestTargetFuturePos;
    const auto& targetData      = FeatureObj::aimbotHelper.GetAimbotTargetData();

    for (BaseEntity* pTarget : targetData.m_vecEnemyPlayers)
    {
        FeatureObj::movementSimulation.Initialize(pTarget);

        for (int i = 0; i < nTicksToSimulate; i++)
            FeatureObj::movementSimulation.RunTick();
        
        vec vTargetFuturePos = FeatureObj::movementSimulation.GetSimulationPos();

        FeatureObj::movementSimulation.Restore();

        const vec vClosestPoint = _GetClosestPointOnEntity(pAttacker, m_vAttackerFuturePos, pTarget, vTargetFuturePos);
        float flDist            = m_vAttackerFuturePos.DistTo(vClosestPoint);

        if (flDist < flBestDistance)
        {
            flBestDistance = flDist;
            vBestTargetFuturePos = vTargetFuturePos;
            pBestTarget    = pTarget;
        }
    }

    if (flBestDistance > flSwingRange)
        return nullptr;

    m_vBestTargetFuturePos = vBestTargetFuturePos;
    return pBestTarget;
}


const vec AimbotMelee_t::_GetClosestPointOnEntity(BaseEntity* pLocalPlayer, const BaseEntity* pEnt) const
{
    auto* pCollidable  = pEnt->GetCollideable();

    // Hull Min & Max in World-Space
    const vec vHullMin = pCollidable->GetCollisionOrigin() + pCollidable->OBBMins();
    const vec vHullMax = pCollidable->GetCollisionOrigin() + pCollidable->OBBMaxs();

    // Shooting origin
    const vec vEyePos  = pLocalPlayer->GetEyePos();

    vec vClosestPoint;
    vClosestPoint.x = std::clamp(vEyePos.x, vHullMin.x, vHullMax.x);
    vClosestPoint.y = std::clamp(vEyePos.y, vHullMin.y, vHullMax.y);
    vClosestPoint.z = std::clamp(vEyePos.z, vHullMin.z, vHullMax.z);

    return vClosestPoint;
}

const vec AimbotMelee_t::_GetClosestPointOnEntity(BaseEntity* pAttacker, const vec& vAttackerOrigin, const BaseEntity* pTarget, const vec& vTargetOrigin) const
{
    auto* pCollidable = pTarget->GetCollideable();

    // Hull Min & Max in World-Space
    const vec vHullMin = vTargetOrigin + pCollidable->OBBMins();
    const vec vHullMax = vTargetOrigin + pCollidable->OBBMaxs();

    // Shooting origin
    const vec vEyePos = vAttackerOrigin + pAttacker->m_vecViewOffset();

    vec vClosestPoint;
    vClosestPoint.x = std::clamp(vEyePos.x, vHullMin.x, vHullMax.x);
    vClosestPoint.y = std::clamp(vEyePos.y, vHullMin.y, vHullMax.y);
    vClosestPoint.z = std::clamp(vEyePos.z, vHullMin.z, vHullMax.z);

    return vClosestPoint;
}

void AimbotMelee_t::_DrawPredictionDebugInfo(BaseEntity* pAttacker, baseWeapon* pActiveWeapon, BaseEntity* pTarget)
{
    float flSwingRange = _GetSwingHullRange(pAttacker, pActiveWeapon);
    
    // eye pos, swing range line, enemy collision hull
    vec vClosestPointOnEnemyHull = _GetClosestPointOnEntity(pAttacker, m_vAttackerFuturePos, pTarget, m_vBestTargetFuturePos);
    vec vSwingEndPoint           = m_vAttackerFuturePos + ((vClosestPointOnEnemyHull - m_vAttackerFuturePos).NormalizeInPlace() * flSwingRange);

    constexpr float PREDICTION_DEBUG_DRAWING_LIFE = 3.0f;
    // Visualizing swing range using line
    I::IDebugOverlay->AddLineOverlay(m_vAttackerFuturePos, vSwingEndPoint, 255, 255, 255, false, PREDICTION_DEBUG_DRAWING_LIFE);
    
    // Visualizing eye pos using box
    constexpr vec EYE_POS_BOX_MAX(3.0f, 3.0f, 3.0f);
    I::IDebugOverlay->AddBoxOverlay(m_vAttackerFuturePos, 
        EYE_POS_BOX_MAX, EYE_POS_BOX_MAX * -1.0f, 
        qangle(0.0f, 0.0f, 0.0f),
        255, 0, 0, 50.0f, PREDICTION_DEBUG_DRAWING_LIFE);

    // Visualizing Target's future collision hull using box
    auto* pCollidable = pTarget->GetCollideable();
    I::IDebugOverlay->AddBoxOverlay(
        m_vBestTargetFuturePos,             
        pCollidable->OBBMins(), pCollidable->OBBMaxs(),
        pCollidable->GetCollisionAngles(),             
        255, 255, 255, 10.0f, PREDICTION_DEBUG_DRAWING_LIFE);
}


float AimbotMelee_t::_GetLooseSwingRange(BaseEntity* pLocalPlayer, baseWeapon* pActiveWeapon)
{
    float flSwingRange = 0.0f;

    if (pLocalPlayer->InCond(TF_COND_SHIELD_CHARGE) == true)
    {
        flSwingRange = 128.0f;
    }
    else
    {
        int iIsSword = 0;
        pActiveWeapon->CALL_ATRIB_HOOK_INT(iIsSword, "is_a_sword");
        
        if (iIsSword == 0)
            flSwingRange = 48.0f;
        else
            flSwingRange = 72.0f;
    }

    return flSwingRange;
}



float AimbotMelee_t::_GetSwingHullRange(BaseEntity* pLocalPlayer, baseWeapon* pActiveWeapon)
{
    // Compensating for Model Scale
    float flSwingRange = _GetLooseSwingRange(pLocalPlayer, pActiveWeapon);
    float flModelScale = pLocalPlayer->m_flModelScale();
    if (flModelScale > 1.0f)
        flSwingRange *= flModelScale;

    // Adding Atribute Melee range
    pActiveWeapon->CALL_ATRIB_HOOK_FLOAT(flSwingRange, "melee_range_multiplier");
    return flSwingRange;
}



bool AimbotMelee_t::_IsPathObstructed(const vec& vStart, const vec& vEnd, BaseEntity* pLocalPlayer)
{
    float flDistance = (vEnd - vStart).length();
    
    // Trace Setup
    ray_t ray;
    ray.Init(vStart, vEnd);
    trace_t trace;
    i_trace_filter filter(pLocalPlayer->GetCollideable()->GetEntityHandle());

    // Casting Ray from Start to End
    I::EngineTrace->TraceRay(ray, MASK_SOLID, &filter, &trace);

    // if Traced ray length less than our Original ray length, then Obstructed!
    return (trace.m_end - trace.m_start).length() < flDistance;
}



void AimbotMelee_t::_DrawSwingRangeRay(BaseEntity* pLocalPlayer, baseWeapon* pActiveWeapon)
{
    static vec vMeleeHullMinBase(-18.0f, -18.0f, -18.0f);
    static vec vMeleeHullMaxBase(18.0f, 18.0f, 18.0f);

    float flBoundScale = 1.0f;
    pActiveWeapon->CALL_ATRIB_HOOK_FLOAT(flBoundScale, "melee_bounds_multiplier");

    // Compensating for Bound Scale
    vec vMeleeHullMin = vMeleeHullMinBase * flBoundScale;
    vec vMeleeHullMax = vMeleeHullMaxBase * flBoundScale;

    // Compensating for Model Scale
    float flSwingRange = _GetLooseSwingRange(pLocalPlayer, pActiveWeapon);
    float flModelScale = pLocalPlayer->m_flModelScale();
    if (flModelScale > 1.0f)
    {
        flSwingRange *= flModelScale;
        vMeleeHullMin *= flModelScale;
        vMeleeHullMax *= flModelScale;
    }

    // Accounting for weapon attributes
    pActiveWeapon->CALL_ATRIB_HOOK_FLOAT(flSwingRange, "melee_range_multiplier");

    float flHullLength = vMeleeHullMax.x - vMeleeHullMin.x;
    float flHullBredth = vMeleeHullMax.y - vMeleeHullMin.y;
    float flHullHeight = vMeleeHullMax.z - vMeleeHullMin.z;

    //printf("Length : %.2f, Breadth : %.2f, Height : %.2f | SwingRange : %.2f\n", flHullLength, flHullBredth, flHullHeight, flSwingRange);

    // Calculating Start & End point.
    vec vForward; 
    Maths::AngleVectors(pLocalPlayer->GetAbsAngles(), &vForward);
    const qangle qViewAngles = pLocalPlayer->GetAbsAngles();
    const vec    vEyePos = pLocalPlayer->GetEyePos();
    
    // Hull End, start and swing range.
    const vec vInitialialHullEnd = vEyePos - vForward * (flHullLength / 2.0f);
    const vec vSwingRangeEnd     = vEyePos + vForward * (flSwingRange);
    const vec vFinalHullEnd      = vEyePos + vForward * (flSwingRange + flHullLength * 0.5f);

    // Drawing
    I::IDebugOverlay->AddLineOverlay(vEyePos,        vInitialialHullEnd, 255, 0,   0, false, 10.0f);
    I::IDebugOverlay->AddLineOverlay(vEyePos,        vSwingRangeEnd,     0,   255, 0, false, 10.0f);
    I::IDebugOverlay->AddLineOverlay(vSwingRangeEnd, vFinalHullEnd,      255, 255, 0, false, 10.0f);
}



void AimbotMelee_t::_DrawMeleeHull(BaseEntity* pLocalPlayer, baseWeapon* pActiveWeapon, CUserCmd* pCmd)
{
    static vec vMeleeHullMinBase(-18.0f, -18.0f, -18.0f);
    static vec vMeleeHullMaxBase( 18.0f,  18.0f,  18.0f);

    float flBoundScale = 1.0f;
    pActiveWeapon->CALL_ATRIB_HOOK_FLOAT(flBoundScale, "melee_bounds_multiplier");

    // Compensating for Bound Scale
    vec vMeleeHullMin = vMeleeHullMinBase * flBoundScale;
    vec vMeleeHullMax = vMeleeHullMaxBase * flBoundScale;

    // Compensating for Model Scale
    float flSwingRange = _GetLooseSwingRange(pLocalPlayer, pActiveWeapon);
    float flModelScale = pLocalPlayer->m_flModelScale();
    if (flModelScale > 1.0f)
    {
        flSwingRange  *= flModelScale;
        vMeleeHullMin *= flModelScale;
        vMeleeHullMax *= flModelScale;
    }

    // Adding Atribute Melee range
    pActiveWeapon->CALL_ATRIB_HOOK_FLOAT(flSwingRange, "melee_range_multiplier");
    
#if (DEBUG_HULL_SIZE == true)
    printf("Mins : %.2f %.2f %.2f\n", vMeleeHullMin.x, vMeleeHullMin.y, vMeleeHullMin.z);
    printf("vMeleeHullMaxBase : %.2f %.2f %.2f\n", vMeleeHullMinBase.x, vMeleeHullMinBase.y, vMeleeHullMinBase.z);
    printf("Maxs : %.2f %.2f %.2f\n", vMeleeHullMax.x, vMeleeHullMax.y, vMeleeHullMax.z);
    printf("vMeleeHullMaxBase : %.2f %.2f %.2f\n", vMeleeHullMaxBase.x, vMeleeHullMaxBase.y, vMeleeHullMaxBase.z);
    printf("ModelScale : %.2f | BoundScale : %.2f\n", flModelScale, flBoundScale);
#endif

    vec vForward;
    Maths::AngleVectors(pCmd->viewangles, &vForward);
    const vec vSwingStart = pLocalPlayer->GetEyePos();
    const vec vSwingEnd   = vSwingStart + (vForward * flSwingRange);

    bool bDidHullHit = [&]()->bool
        {
            trace_t trace;
            i_trace_filter filter(pLocalPlayer->GetCollideable()->GetEntityHandle()); // This is a make shift filter, make a proper one ( or maybe not? IDK )

            I::EngineTrace->UTIL_TraceHull(
                vSwingStart, vSwingEnd,         // Swing Start & End 
                vMeleeHullMin, vMeleeHullMax,   // Hull Mins & Max
                MASK_SOLID, &filter, &trace     // Filters n shit
            );

            return trace.did_hit();
        }();

    // Drawing Hull
    I::IDebugOverlay->AddSweptBoxOverlay(
        vSwingStart, vSwingEnd,         // Swing Start & End
        vMeleeHullMin, vMeleeHullMax,   // Hull Mins and Maxes
        pCmd->viewangles,               // Orientation
        bDidHullHit ? 0 : 255, bDidHullHit ? 255 : 0, 0, 50, 10.0f); // Color, Alpha & duration
}



void AimbotMelee_t::_DrawMeleeSwingRadius(BaseEntity* pLocalPlayer, baseWeapon* pActiveWeapon)
{
    I::IDebugOverlay->AddCircleOverlay(
        pLocalPlayer->GetAbsOrigin(),                                         // Circles base
        vec(0.0f, 0.0f, 1.0f),                                                // points straight up!
        _GetLooseSwingRange(pLocalPlayer, pActiveWeapon) * SWING_RANGE_MULTIPLIER, // This is not the real range.
        20,                 // Segments / Smoothness
        255, 255, 0, 255,   // Color
        false,
        10.0f,
        DEG2RAD(pLocalPlayer->GetAbsAngles().yaw));
}

void AimbotMelee_t::_DrawEyePos(BaseEntity* pLocalPlayer, baseWeapon* pActiveWeapon)
{
    vec BOX_SIZE_EYE_POS(5.0f, 5.0f, 5.0f);

    I::IDebugOverlay->AddBoxOverlay(
        pLocalPlayer->GetEyePos(), // <- Eye Pos
        BOX_SIZE_EYE_POS,               // Box mins
        BOX_SIZE_EYE_POS * -1.0f,       // Box maxs
        pLocalPlayer->GetAbsAngles(),   // Box's orientation
        255, 255, 255, 50, 10.0f);       // Color, alpha and duration
}


void AimbotMelee_t::_DrawEntityCollisionHull(const BaseEntity* pEnt, const vec& vOrigin) const
{
    auto* pCollidable = pEnt->GetCollideable();

    I::IDebugOverlay->AddBoxOverlay(
        pCollidable->GetCollisionOrigin(),              // Collision Origin
        pCollidable->OBBMins(), pCollidable->OBBMaxs(), // Collision Origin Mins & Maxs
        pCollidable->GetCollisionAngles(),              // Angle of collision Hull
        255, 255, 255, 40, 3.0f); // Color n shit
}