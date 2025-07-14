//=========================================================================
//                      MELEE AIMBOT
//=========================================================================
// by      : INSANE
// created : 14/06/2025
// 
// purpose : Hits enemy perfectly with melee weapons ( doesn't contain auto backstab )
//-------------------------------------------------------------------------

#include "AimbotMelee.h"
#include "../AimbotHelper.h"
#include "../../CritHack/CritHack.h"

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
#include "../../../SDK/class/CommonFns.h"

// UTILITY
#include "../../../Utility/Insane Profiler/InsaneProfiler.h"
#include "../../MovementSimulation/MovementSimulation.h"
#include "../../../Extra/math.h"
#include "../../ImGui/InfoWindow/InfoWindow_t.h"

#define DEBUG_BACKSTAB_CHECK false

constexpr float PREDICTION_DEBUG_DRAWING_LIFE = 3.0f;

// NOTE : WE ARE ASSUMING THAT ALL MELEE'S HAVE SMACK DELAY, AND SPY KNIFE ALSO HAS SMACK DELAY ( but not backstabs )
// NOTE : m_flSmackTime for Spy is messed up, so we can't relie on it to set our angles on the perfect tick

//=========================================================================
//                     PUBLIC METHODS
//=========================================================================
void AimbotMelee_t::RunV3(BaseEntity* pLocalPlayer, baseWeapon* pActiveWeapon, CUserCmd* pCmd, bool* pCreatemoveResult)
{
    PROFILE_FUNCTION("Melee Aimbot");

    if (Features::Aimbot::Melee_Aimbot::MeleeAimbot.IsActive() == false)
        return;

    bool bInAttack = SDK::InAttack(pLocalPlayer, pActiveWeapon, pCmd);
    
    // FINDING TARGET
    if (bInAttack == false)
    {
        BaseEntity* pTarget = _ChooseTarget(pLocalPlayer, pActiveWeapon);

        if (pTarget != nullptr)
            m_pBestTarget = pTarget;
    }
    else if (Features::Aimbot::Melee_Aimbot::MeleeAimbot_DebugPrediction.IsActive() == true && m_pBestTarget != nullptr)
    {
        // Storing position at the time of "Locking"
        if (m_vBestTargetPosAtLock.IsEmpty() == true)
            m_vBestTargetPosAtLock = m_pBestTarget->GetAbsOrigin();
        
        // Drawing Current & Predicted position.
        _DrawPredictionDebugInfo(pActiveWeapon, pActiveWeapon, m_pBestTarget);
    }

    if (m_pBestTarget == nullptr)
        return;

    // HANDLING AUTO FIRE
    if (m_pBestTarget != nullptr && SDK::CanAttack(pLocalPlayer, pActiveWeapon, pCmd) == true &&
        Features::Aimbot::Melee_Aimbot::MeleeAimbot_AutoFire.IsActive() == true)
    {
        pCmd->buttons |= IN_ATTACK;
    }

    // SETTING AIMBOT ANGLES
    if(_ShouldAim(pLocalPlayer, pActiveWeapon, pCmd) == true)
    {
        // Calculating target's angles
        vec vTargetBestPos = _GetClosestPointOnEntity(pLocalPlayer, m_pBestTarget);
        qangle qTargetBestAngles;
        Maths::VectorAnglesFromSDK(vTargetBestPos - pLocalPlayer->GetEyePos(), qTargetBestAngles);

        // Silent aim
        pCmd->viewangles = qTargetBestAngles;
        *pCreatemoveResult = false; // Setting silent aim

        // Reset everything once swing is done.
        Reset();
    }
}


bool AimbotMelee_t::_ShouldAim(BaseEntity* pAttacker, baseWeapon* pActiveWeapon, CUserCmd* pCmd)
{
    float flCurTime = TICK_TO_TIME(pAttacker->m_nTickBase());
    
    // If not spy, we can just use the netvar :)
    if (pAttacker->m_iClass() != TF_SPY)
        return SDK::InAttack(pAttacker, pActiveWeapon, pCmd) == true && flCurTime >= pActiveWeapon->m_flSmackTime();

    // If we are spy & trying to backstab, Aiming & Firing must be done on the same tick.
    // You will get a backstab as long as the angles are right.
    return SDK::CanAttack(pAttacker, pActiveWeapon, pCmd);
}


void AimbotMelee_t::Reset()
{
    m_vAttackerFutureEyePos.Init();
    m_vBestTargetFuturePos.Init();
    m_vBestTargetPosAtLock.Init();

    m_pBestTarget     = nullptr;
}


//=========================================================================
//                     PRIVATE METHODS
//=========================================================================
BaseEntity* AimbotMelee_t::_ChooseTarget(BaseEntity* pAttacker, baseWeapon* pActiveWeapon)
{
    const AimbotHelper_t::AimbotTargetData_t& aimbotTargetData = F::aimbotHelper.GetAimbotTargetData();
    float flSmackDelay = pActiveWeapon->GetTFWeaponInfo()->GetWeaponData(0)->m_flSmackDelay;
    float flSwingRange = _GetSwingHullRange(pAttacker, pActiveWeapon);

    bool bShouldSimulateForPlayers = (pActiveWeapon->m_iClass() != TF_SPY);
    BaseEntity* pBestTarget = nullptr;

    // Trying to find enemy players first
    pBestTarget = _ChooseTargetFromList(pAttacker, aimbotTargetData.m_vecEnemyPlayers, flSmackDelay, flSwingRange, bShouldSimulateForPlayers);
    if (pBestTarget != nullptr)
        return pBestTarget;

    // if no enemy player found, try to find building next
    // SENTRY GUNS
    pBestTarget = _ChooseTargetFromList(pAttacker, aimbotTargetData.m_vecEnemySentry, flSmackDelay, flSwingRange, false);
    if (pBestTarget != nullptr)
        return pBestTarget;

    // DISPENSERS
    pBestTarget = _ChooseTargetFromList(pAttacker, aimbotTargetData.m_vecEnemyDispensers, flSmackDelay, flSwingRange, false);
    if (pBestTarget != nullptr)
        return pBestTarget;

    // TELEPORTERS
    pBestTarget = _ChooseTargetFromList(pAttacker, aimbotTargetData.m_vecEnemyTeleporters, flSmackDelay, flSwingRange, false);
    if (pBestTarget != nullptr)
        return pBestTarget;

    // TODO : Make this dynamic
    bool bIsDisciplinaryAction = (pActiveWeapon->GetWeaponDefinitionID() == 447);
    if (bIsDisciplinaryAction == true && Features::Aimbot::Melee_Aimbot::MeleeAimbot_HitTeammates_when_benificial.IsActive() == true) // Find valid teammate if we can hit them for advantage
    {
        pBestTarget = _ChooseTargetFromList(pAttacker, aimbotTargetData.m_vecFriendlyPlayers, flSmackDelay, flSwingRange, true);
        if (pBestTarget != nullptr)
            return pBestTarget;
    }

    // Add building support too
    return nullptr;
}

BaseEntity* AimbotMelee_t::_ChooseTargetFromList( BaseEntity* pAttacker, 
    const std::vector<BaseEntity*>& vecTargets, 
    float flSmackDelay, float flSwingRange, bool bShouldSimulate)
{
    uint32_t nTicksToSimulate = flSmackDelay / TICK_INTERVAL;

    if (bShouldSimulate == false)
        nTicksToSimulate = 0;

    // Attacker's future position
    if (bShouldSimulate == true)
    {
        F::movementSimulation.Initialize(pAttacker);
        for (int i = 0; i < nTicksToSimulate; i++)
            F::movementSimulation.RunTick();

        m_vAttackerFutureEyePos = F::movementSimulation.GetSimulationPos() + pAttacker->m_vecViewOffset();
        F::movementSimulation.Restore();
    }
    else
    {
        m_vAttackerFutureEyePos = pAttacker->GetEyePos();
    }

    BaseEntity* pBestTarget     = nullptr;
    float       flBestDistance  = std::numeric_limits<float>::infinity();
    vec         vBestTargetFuturePos;

    for (BaseEntity* pTarget : vecTargets)
    {
        vec vTargetFuturePos;
        if (bShouldSimulate == true)
        {
            F::movementSimulation.Initialize(pTarget);
            for (int i = 0; i < nTicksToSimulate; i++)
                F::movementSimulation.RunTick();

            vTargetFuturePos = F::movementSimulation.GetSimulationPos();

            F::movementSimulation.Restore();
        }
        else
        {
            vTargetFuturePos = pTarget->GetAbsOrigin();
        }

        // FOV check
        if (_IsInFOV(pAttacker, m_vAttackerFutureEyePos, vTargetFuturePos, Features::Aimbot::Melee_Aimbot::MeleeAimbot_FOV.GetData().m_flVal) == false)
            continue;

        if (pAttacker->m_iClass() == TF_SPY && Features::Aimbot::Melee_Aimbot::MeleeAimbot_OnlyDoBackStabs_Spy.IsActive() == true)
            if (_CanBackStab(pAttacker, pTarget) == false)
                continue;

        const vec vClosestPoint = _GetClosestPointOnEntity(pAttacker, m_vAttackerFutureEyePos, pTarget, vTargetFuturePos);
        float flDist = m_vAttackerFutureEyePos.DistTo(vClosestPoint);

        if (flDist < flBestDistance)
        {
            flBestDistance = flDist;
            vBestTargetFuturePos = vTargetFuturePos;
            pBestTarget = pTarget;
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


const vec AimbotMelee_t::_GetClosestPointOnEntity(BaseEntity* pAttacker, const vec& vAttackerEyePos, const BaseEntity* pTarget, const vec& vTargetOrigin) const
{
    auto* pCollidable = pTarget->GetCollideable();

    // Hull Min & Max in World-Space
    const vec vHullMin = vTargetOrigin + pCollidable->OBBMins();
    const vec vHullMax = vTargetOrigin + pCollidable->OBBMaxs();

    // Shooting origin
    const vec vEyePos = vAttackerEyePos;

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
    vec vClosestPointOnEnemyHull = _GetClosestPointOnEntity(pAttacker, m_vAttackerFutureEyePos, pTarget, m_vBestTargetFuturePos);
    vec vSwingEndPoint           = m_vAttackerFutureEyePos + ((vClosestPointOnEnemyHull - m_vAttackerFutureEyePos).NormalizeInPlace() * flSwingRange);

    // Visualizing swing range using line
    I::IDebugOverlay->AddLineOverlay(m_vAttackerFutureEyePos, vSwingEndPoint, 255, 255, 255, false, PREDICTION_DEBUG_DRAWING_LIFE);
    
    // Visualizing eye pos using box
    constexpr vec EYE_POS_BOX_MAX(3.0f, 3.0f, 3.0f);
    I::IDebugOverlay->AddBoxOverlay(m_vAttackerFutureEyePos, 
        EYE_POS_BOX_MAX, EYE_POS_BOX_MAX * -1.0f, 
        qangle(0.0f, 0.0f, 0.0f),
        255, 0, 0, 50.0f, PREDICTION_DEBUG_DRAWING_LIFE);

    // Target's Collision hull mins, maxs, angles
    auto* pCollidable                  = pTarget->GetCollideable();
    const vec& vOBBMin                 = pCollidable->OBBMins();
    const vec& vOBBMax                 = pCollidable->OBBMaxs();
    const qangle& qCollisionHullAngles = pCollidable->GetCollisionAngles();

    // Visualizing Target's future collision hull using box
    I::IDebugOverlay->AddBoxOverlay(
        m_vBestTargetFuturePos,             
        vOBBMin, vOBBMax,
        qCollisionHullAngles,
        134, 173, 153, 10.0f, PREDICTION_DEBUG_DRAWING_LIFE); // LIGHT GREEN

    // Visualizing Target's CURRENT collision hull using box
    I::IDebugOverlay->AddBoxOverlay(
        m_vBestTargetPosAtLock,
        vOBBMin, vOBBMax,
        qCollisionHullAngles,
        168, 126, 137, 10.0f, PREDICTION_DEBUG_DRAWING_LIFE); // LIGHT RED
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
    constexpr float flSwingMaxBase = 18.0f;
    float flBoundScale = 1.0f;
    pActiveWeapon->CALL_ATRIB_HOOK_FLOAT(flBoundScale, "melee_bounds_multiplier");
    float flSwingMax = flSwingMaxBase * flBoundScale;

    // Compensating for Model Scale
    float flSwingRange = _GetLooseSwingRange(pLocalPlayer, pActiveWeapon);
    float flModelScale = pLocalPlayer->m_flModelScale();
    if (flModelScale > 1.0f)
        flSwingRange *= flModelScale;

    // Adding Atribute Melee range
    pActiveWeapon->CALL_ATRIB_HOOK_FLOAT(flSwingRange, "melee_range_multiplier");
    
    // Since melee swing is traced using a hull, we gonna have to add this too.
    // Imagine, flSwingRange as distance centers of 2 boxes, ( 2 boxes means the min & max of the hull )
    // and flSwingMax is the distance between the center of the box to the surface.
    flSwingRange += flSwingMax;
    return flSwingRange;
}

bool AimbotMelee_t::_CanBackStab(BaseEntity* pAttacker, BaseEntity* pTarget)
{
    auto* pAttackerCollidable = pAttacker->GetCollideable();
    auto* pTargetCollidable   = pTarget->GetCollideable();
    
    vec vAttackerOrigin = pAttackerCollidable->GetCollisionOrigin();
    vec vTargetOrigin   = pTargetCollidable->GetCollisionOrigin();

    // Vector connecting Attacker to Taret
    vec vAttackerToTarget = vTargetOrigin - vAttackerOrigin;
    vAttackerToTarget.z = 0.0f;
    vAttackerToTarget.NormalizeInPlace();

    // Target's view angle vector
    vec vTargetViewAngles;
    qangle qTargetAngles = pTarget->m_angEyeAngles();
    Maths::AngleVectors(qTargetAngles, &vTargetViewAngles);
    vTargetViewAngles.z = 0.0f;
    vTargetViewAngles.NormalizeInPlace();
    
#if (DEBUG_BACKSTAB_CHECK == true)
    I::IDebugOverlay->AddLineOverlay(vAttackerOrigin, vAttackerOrigin + (vAttackerToTarget * 100.0f), 255, 0, 0, false, 3.0f);
    I::IDebugOverlay->AddLineOverlay(vTargetOrigin,   vTargetOrigin + (vTargetViewAngles * 100.0f),   0, 255, 0, false, 3.0f);
#endif

    float flDotProduct = vAttackerToTarget.x * vTargetViewAngles.x + vAttackerToTarget.y * vTargetViewAngles.y;

    return flDotProduct > 0.0f;
}

bool AimbotMelee_t::_IsInFOV(BaseEntity* pAttacker, const vec& vAttackerPos, const vec& vTargetPos, float FOV)
{
    vec vAttackerToTarget = vTargetPos - vAttackerPos;
    vAttackerToTarget.NormalizeInPlace();

    vec vAttackerAngles;
    qangle qAttackerAngles = pAttacker->m_angEyeAngles();
    Maths::AngleVectors(qAttackerAngles, &vAttackerAngles);
    vAttackerAngles.NormalizeInPlace();

    float flDot       = vAttackerAngles.Dot(vAttackerToTarget);
    float flTargetFOV = RAD2DEG(acosf(flDot));

    return FOV > flTargetFOV;
}