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

constexpr float PREDICTION_DEBUG_DRAWING_LIFE = 3.0f;

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
        if (m_vBestTargetPosAtLock.IsEmpty() == true)
            m_vBestTargetPosAtLock = m_pBestTarget->GetAbsOrigin();

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

        // Reset everything once swing is done.
        Reset();
    }
}


void AimbotMelee_t::Reset()
{
    m_vAttackerFuturePos.Init();
    m_vBestTargetFuturePos.Init();
    m_vBestTargetPosAtLock.Init();

    m_pBestTarget = nullptr;
}


//=========================================================================
//                     PRIVATE METHODS
//=========================================================================
BaseEntity* AimbotMelee_t::_ChooseTarget(BaseEntity* pAttacker, float flSmackDelay, float flSwingRange)
{
    uint32_t nTicksToSimulate = flSmackDelay / tfObject.pGlobalVar->interval_per_tick;

    // ATTACKERS FUTURE POSITION
    FeatureObj::movementSimulation.Initialize(pAttacker);
    for (int i = 0; i < nTicksToSimulate; i++)
        FeatureObj::movementSimulation.RunTick();

    m_vAttackerFuturePos = FeatureObj::movementSimulation.GetSimulationPos() + pAttacker->m_vecViewOffset();
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

    // Visualizing swing range using line
    I::IDebugOverlay->AddLineOverlay(m_vAttackerFuturePos, vSwingEndPoint, 255, 255, 255, false, PREDICTION_DEBUG_DRAWING_LIFE);
    
    // Visualizing eye pos using box
    constexpr vec EYE_POS_BOX_MAX(3.0f, 3.0f, 3.0f);
    I::IDebugOverlay->AddBoxOverlay(m_vAttackerFuturePos, 
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
    // Compensating for Model Scale
    float flSwingRange = _GetLooseSwingRange(pLocalPlayer, pActiveWeapon);
    float flModelScale = pLocalPlayer->m_flModelScale();
    if (flModelScale > 1.0f)
        flSwingRange *= flModelScale;

    // Adding Atribute Melee range
    pActiveWeapon->CALL_ATRIB_HOOK_FLOAT(flSwingRange, "melee_range_multiplier");
    return flSwingRange;
}