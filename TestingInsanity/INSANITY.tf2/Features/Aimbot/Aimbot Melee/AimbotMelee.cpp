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
#include "../../../Extra/math.h"

/*
TODO : 
->  FOV Support
->  Predict target position

->  Debug only feature support
->  NetvarHandler support for everything
*/

#define DEBUG_HULL_SIZE        false
#define DEBUG_MELEE_SWING_HULL true

constexpr float SWING_RANGE_MULTIPLIER = 1.2f;

//=========================================================================
//                     PUBLIC METHODS
//=========================================================================
void AimbotMelee_t::Run(BaseEntity* pLocalPlayer, baseWeapon* pActiveWeapon, CUserCmd* pCmd, bool* pSendPacket)
{
    // Is Active ?
    if (TempFeatureHelper::MeleeAimbot.IsActive() == false)
        return;

    // TODO : Move this to CreateMove, and only call this if anything is drawn.
    //      Also, Gotta move the drawing to ImGui, and ditch this bullshit. 
    //      Cause for one, this is maybe not optmized and secondly, its updated every tick and not every frame.
    I::IDebugOverlay->ClearAllOverlays();


#if (DEBUG_MELEE_SWING_HULL == true)
    // Drawing Melee Range Circle
    if (TempFeatureHelper::MeleeRange_Circle.IsActive() == true)
        _DrawMeleeSwingRadius(pLocalPlayer, pActiveWeapon);

    // Drawing EyePos ( Bullet origin )
    if (TempFeatureHelper::MeleeEyePos.IsActive() == true)
        _DrawEyePos(pLocalPlayer, pActiveWeapon);

    // Drawing Hull ( Effective Range )
    if (TempFeatureHelper::MeleeRange_HULL.IsActive() == true)
        _DrawMeleeHull(pLocalPlayer, pActiveWeapon, pCmd);

    // Swing range Demostration using Ray ( Not Hull )
    if(TempFeatureHelper::Melee_Swing_Range_Ray.IsActive() == true)
        _DrawSwingRangeRay(pLocalPlayer, pActiveWeapon);
#endif

    const BaseEntity* pTarget = _ChooseTarget(pLocalPlayer);
    
    // No target found :(
    if (pTarget == nullptr)
        return;

    float flSwingRange        = _GetSwingHullRange(pLocalPlayer, pActiveWeapon);
    const vec vTargetWorldPos = _GetClosestPointOnEntity(pLocalPlayer, pTarget);
    const vec vEyePos         = pLocalPlayer->getLocalEyePos();

    //LOG("Swing Range from fn : %.2f", flSwingRange);

    // Target is way out of range.
    if (vEyePos.DistTo(vTargetWorldPos) > flSwingRange)
        return;

    // Drawing Targets collision hull
    if (TempFeatureHelper::MeleeDrawCollisionHull.IsActive() == true)
        _DrawEntityCollisionHull(pTarget);

    // Is the path clear OR we got a fucking wall or shit like that?
    /*if (_IsPathObstructed(vEyePos, vTargetWorldPos, pLocalPlayer) == true)
    {
        FAIL_LOG("Paths is not clear, there is something in between!");
        return;
    }*/
    
    /*
    qangle qTargetAngles;
    Maths::VectorAnglesFromSDK(vTargetWorldPos - vEyePos, qTargetAngles);
    */

    float flNextFireTime = pActiveWeapon->GetNextPrimaryAttackTime();
    float flCurTime      = static_cast<float>(pLocalPlayer->GetTickBase()) * tfObject.pGlobalVar->interval_per_tick;

    bool bShotFiredThisTick = (pCmd->buttons & IN_ATTACK) && flCurTime >= flNextFireTime && flNextFireTime > m_flLastAttackTime;

    if (bShotFiredThisTick == true)
    {
        LOG("Swing Started...");
        m_bSwingActive     = true;
        m_flLastAttackTime = flNextFireTime;
        m_iSwingTick       = pCmd->tick_count;

        if (TempFeatureHelper::MeleeAimbot_AutoFire.IsActive() == true)
            pCmd->buttons |= IN_ATTACK;
    }

    float flSmackTime    = pActiveWeapon->GetSmackTime();
    bool bShouldSetAngle = flCurTime >= flSmackTime && flSmackTime > 0.0f;
    if (bShouldSetAngle == true)
    {
        qangle qTargetAngles;
        Maths::VectorAnglesFromSDK(vTargetWorldPos - vEyePos, qTargetAngles);
        pCmd->viewangles = qTargetAngles;
        *pSendPacket   = false;
        m_bSwingActive = false;
    }

}


void AimbotMelee_t::Reset()
{
    m_flLastAttackTime = 0.0f;
    m_bSwingActive = false;
    m_iSwingTick = 0;
}


//=========================================================================
//                     PRIVATE METHODS
//=========================================================================
const BaseEntity* AimbotMelee_t::_ChooseTarget(BaseEntity* pLocalPlayer) const
{
    const auto& targetData = FeatureObj::aimbotHelper.GetAimbotTargetData();
    
    const vec&  vEyePos        = pLocalPlayer->getLocalEyePos();
    BaseEntity* pBestTarget    = nullptr;
    float       flBestDistance = std::numeric_limits<float>::infinity();
    
    for (BaseEntity* pTarget : targetData.m_vecEnemyPlayers)
    {
        const vec vClosestPoint = _GetClosestPointOnEntity(pLocalPlayer, pTarget);
        float flDist            = vEyePos.DistTo(vClosestPoint);

        if (flDist < flBestDistance)
        {
            flBestDistance = flDist;
            pBestTarget    = pTarget;
        }
    }

    return pBestTarget;
}


const vec AimbotMelee_t::_GetClosestPointOnEntity(BaseEntity* pLocalPlayer, const BaseEntity* pEnt) const
{
    auto* pCollidable  = pEnt->GetCollideable();

    // Hull Min & Max in World-Space
    const vec vHullMin = pCollidable->GetCollisionOrigin() + pCollidable->OBBMins();
    const vec vHullMax = pCollidable->GetCollisionOrigin() + pCollidable->OBBMaxs();

    // Shooting origin
    const vec vEyePos  = pLocalPlayer->getLocalEyePos();

    vec vClosestPoint;
    vClosestPoint.x = std::clamp(vEyePos.x, vHullMin.x, vHullMax.x);
    vClosestPoint.y = std::clamp(vEyePos.y, vHullMin.y, vHullMax.y);
    vClosestPoint.z = std::clamp(vEyePos.z, vHullMin.z, vHullMax.z);

    return vClosestPoint;
}



bool AimbotMelee_t::_ShouldSwing(BaseEntity* pLocalPlayer, baseWeapon* pActiveWeapon)
{
    // Did next attack time got updated?
    float flNextAttackTime = pActiveWeapon->GetNextPrimaryAttackTime();
    if (flNextAttackTime <= m_flLastAttackTime)
        return false;

    // Can we swing now?
    float flCurTime = tfObject.pGlobalVar->interval_per_tick * static_cast<float>(pLocalPlayer->GetTickBase());
    if (flNextAttackTime > flCurTime)
        return false;

    m_flLastAttackTime = flNextAttackTime;
    return true;
}



bool AimbotMelee_t::_ShouldSetAngle(BaseEntity* pLocalPlayer, baseWeapon* pActiveWeapon)
{
    if (m_flLastAttackTime == 0.0f)
        return false;

    if (m_bSwingActive == false)
        return false;

    float flCurTime = tfObject.pGlobalVar->interval_per_tick * static_cast<float>(pLocalPlayer->GetTickBase());
    float flSmackDelay = pActiveWeapon->GetTFWeaponInfo()->GetWeaponData(0)->m_flSmackDelay;
    
    if (flCurTime >= m_flLastAttackTime + flSmackDelay)
    {
        LOG("SmackDelay : %.2f\n", flSmackDelay);
        m_flLastAttackTime = 0.0f;
        return true;
    }

    return false;
}



float AimbotMelee_t::_GetSwingRange(BaseEntity* pLocalPlayer, baseWeapon* pActiveWeapon)
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
    float flSwingRange = _GetSwingRange(pLocalPlayer, pActiveWeapon);
    float flModelScale = pLocalPlayer->GetModelScale();
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
    float flSwingRange = _GetSwingRange(pLocalPlayer, pActiveWeapon);
    float flModelScale = pLocalPlayer->GetModelScale();
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
    const vec    vEyePos = pLocalPlayer->getLocalEyePos();
    
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
    float flSwingRange = _GetSwingRange(pLocalPlayer, pActiveWeapon);
    float flModelScale = pLocalPlayer->GetModelScale();
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
    const vec vSwingStart = pLocalPlayer->getLocalEyePos();
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
        _GetSwingRange(pLocalPlayer, pActiveWeapon) * SWING_RANGE_MULTIPLIER, // This is not the real range.
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
        pLocalPlayer->getLocalEyePos(), // <- Eye Pos
        BOX_SIZE_EYE_POS,               // Box mins
        BOX_SIZE_EYE_POS * -1.0f,       // Box maxs
        pLocalPlayer->GetAbsAngles(),   // Box's orientation
        255, 255, 255, 50, 10.0f);       // Color, alpha and duration
}


void AimbotMelee_t::_DrawEntityCollisionHull(const BaseEntity* pEnt) const
{
    auto* pCollidable = pEnt->GetCollideable();

    I::IDebugOverlay->AddBoxOverlay(
        pCollidable->GetCollisionOrigin(),              // Collision Origin
        pCollidable->OBBMins(), pCollidable->OBBMaxs(), // Collision Origin Mins & Maxs
        pCollidable->GetCollisionAngles(),              // Angle of collision Hull
        255, 255, 255, 40, 3.0f); // Color n shit
}