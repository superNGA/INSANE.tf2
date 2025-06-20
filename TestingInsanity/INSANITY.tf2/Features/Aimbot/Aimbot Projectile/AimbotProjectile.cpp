#include "AimbotProjectile.h"

// SDK
#include "../../../SDK/class/BaseEntity.h"
#include "../../../SDK/class/BaseWeapon.h"
#include "../../../SDK/class/CUserCmd.h"
#include "../../../SDK/class/CommonFns.h"
#include "../../../SDK/class/IVDebugOverlay.h"
#include "../../../SDK/class/ETFWeaponType.h"
#include "../../../SDK/class/IEngineTrace.h"
#include "../../MovementSimulation/MovementSimulation.h"
#include "../../../SDK/TF object manager/TFOjectManager.h"

#include "../AimbotHelper.h"
#include "../../../Extra/math.h"

constexpr vec vRocketHullMin(-1.0f, -1.0f, -1.0f);
constexpr vec vRocketHullMax( 1.0f,  1.0f,  1.0f);

//=========================================================================
//                     PUBLIC METHODS
//=========================================================================
void AimbotProjectile_t::Run(BaseEntity* pLocalPlayer, baseWeapon* pActiveWeapon, CUserCmd* pCmd, bool* pCreatemoveResult)
{
    // Not working for all weapons, but its working for some.
    //float flProjSpeed = (*(float(__fastcall*)(void*))(util.GetVirtualTable(pActiveWeapon)[0xEC0 / 8]))(pActiveWeapon);
    //printf("flProjectile speed : %.2f\n", flProjSpeed);

    // Fuck off
    if (Features::Aimbot::Aimbot_Projectile::ProjAimbot_Enable.IsActive() == false)
        return;

    if (SDK::CanAttack(pLocalPlayer, pActiveWeapon, pCmd) == true)
    {
        BaseEntity* pBestTarget = _ComputeBestTarget(pLocalPlayer, pActiveWeapon);

        if (pBestTarget != nullptr)
            m_pBestTarget = pBestTarget;
    }

    float flCurTime  = TICK_TO_TIME(pLocalPlayer->m_nTickBase());
    bool  bShouldAim = flCurTime >= pActiveWeapon->m_flNextPrimaryAttack() && (pCmd->buttons & IN_ATTACK);
    if (bShouldAim == true && m_pBestTarget != nullptr)
    {
        // simulate & aim
        float flProjectileSpeed = _GetProjectileSpeed(pActiveWeapon);

        //vec vFutureAttackerToTarget = _GetBestTickToAim(pLocalPlayer, pActiveWeapon, m_pBestTarget, flProjectileSpeed);
        qangle qBestTargetAngles;
        Maths::VectorAnglesFromSDK(m_vBestTargetFuturePos - pLocalPlayer->GetEyePos(), qBestTargetAngles);
        pCmd->viewangles = qBestTargetAngles;
        *pCreatemoveResult = false;

        Reset();
    }
}

void AimbotProjectile_t::Reset()
{
    m_pBestTarget = nullptr;
    m_vBestTargetFuturePos.Init();
}

//=========================================================================
//                     PRIVATE METHODS
//=========================================================================
BaseEntity* AimbotProjectile_t::_ComputeBestTarget(BaseEntity* pLocalPLayer, baseWeapon* pActiveWeapon)
{
    const std::vector<BaseEntity*>& vecEnemyPlayers = FeatureObj::aimbotHelper.GetAimbotTargetData().m_vecEnemyPlayers;
    const vec vLocalPlayerEyePos = pLocalPLayer->GetEyePos();
    const float flProjectileSpeed = _GetProjectileSpeed(pActiveWeapon);

    BaseEntity* pBestTarget = nullptr;
    float       flBestDist  = std::numeric_limits<float>::infinity();

    for (BaseEntity* pTarget : vecEnemyPlayers)
    {   
        constexpr float flMaxTimeToSimulateInSec = 2.0f;
        uint32_t        nTicksToSimulate         = TIME_TO_TICK(flMaxTimeToSimulateInSec);
        uint32_t        iEntFlags                = 0;
        const vec&      vTargetInitialOrigin     = pTarget->GetAbsOrigin();
        vec             vTargetFutureOrigin;
        
        // Checking if target is in FOV range or not
        float flDist = _GetAngleFromCrosshair(pLocalPLayer, vTargetInitialOrigin);
        if (flDist > Features::Aimbot::Aimbot_Projectile::ProjAimbot_FOV.GetData().m_flVal)
            continue;

        FeatureObj::movementSimulation.Initialize(pTarget);
        for (int iTick = 0; iTick < nTicksToSimulate; iTick++)
        {
            FeatureObj::movementSimulation.RunTick();

            const vec& vTargetOrigin      = FeatureObj::movementSimulation.GetSimulationPos();
            float flTimeToReachPlayer     = TICK_TO_TIME(iTick);
            float flTimeToReachProjectile = vLocalPlayerEyePos.DistTo(vTargetOrigin) / flProjectileSpeed;

            // projectile's time to reach player must be smaller than 
            // target's time to reach, else we can't possibly hit them.
            if (flTimeToReachProjectile < flTimeToReachPlayer)
            {
                vTargetFutureOrigin = vTargetOrigin;
                iEntFlags           = FeatureObj::movementSimulation.GetSimulationFlags();
                break;
            }
        }
        FeatureObj::movementSimulation.Restore();

        // if no hittable position found 
        if (vTargetFutureOrigin.IsEmpty() == true)
            continue;
        
        // Ray Tracing and finding best point on enemy hull.
        vec vBestPointOnTargetHull;
        if (_FindBestVisibleHullPoint(pLocalPLayer, pTarget, iEntFlags, vTargetFutureOrigin, vBestPointOnTargetHull) == false)
            continue;

        // Debug Drawing
        if(Features::Aimbot::Aimbot_Projectile::ProjAimbot_DebugPrediction.IsActive() == true)
        {
            I::IDebugOverlay->AddLineOverlay(vTargetInitialOrigin, vTargetFutureOrigin, 255, 255, 255, true, 1.0f);
            
            // Delete this
            I::IDebugOverlay->AddTextOverlay(vTargetInitialOrigin, 1.0f, "FOV : %.2f", flDist);
        }

        if (flDist < flBestDist)
        {
            pBestTarget            = pTarget;
            flBestDist             = flDist;
            m_vBestTargetFuturePos = vBestPointOnTargetHull;
        }

    }

    return pBestTarget;
}

float AimbotProjectile_t::_GetAngleFromCrosshair(BaseEntity* pLocalPlayer, const vec& vTargetPos)
{
    vec vAttackerAngles;
    qangle qAttackerAngles = pLocalPlayer->m_angEyeAngles();
    Maths::AngleVectors(qAttackerAngles, &vAttackerAngles);
    vAttackerAngles.NormalizeInPlace();

    vec vAttackerToTarget = vTargetPos - pLocalPlayer->GetEyePos();
    vAttackerToTarget.NormalizeInPlace();

    float flDot = vAttackerAngles.Dot(vAttackerToTarget);
    return RAD2DEG(acosf(flDot));
}


bool AimbotProjectile_t::_FindBestVisibleHullPoint(BaseEntity* pLocalPlayer, BaseEntity* pTarget, uint32_t iFlags, const vec& vTargetPos, vec& vBestPointOut)
{
    ICollideable_t* pCollidable = pTarget->GetCollideable();
    const vec&      vHullMins   = pCollidable->OBBMins();
    const vec&      vHullMaxs   = pCollidable->OBBMaxs();

    float flHeight = vHullMins.z > vHullMaxs.z ? vHullMins.z : vHullMaxs.z;

    constexpr float flSafeTraceOffset = 2.0f;
    const vec vHead(  0.0f, 0.0f, flHeight - flSafeTraceOffset);
    const vec vFeet(  0.0f, 0.0f, flSafeTraceOffset);
    const vec vChest( 0.0f, 0.0f, flHeight * 0.66);
    const vec vPelvis(0.0f, 0.0f, flHeight * 0.33);

    const vec vHullBoundary1(vHullMins.x, vHullMins.y, flHeight * 0.5f);
    const vec vHullBoundary2(vHullMaxs.x, vHullMaxs.y, flHeight * 0.5f);
    const vec vHullBoundary3(vHullMaxs.x, vHullMins.y, flHeight * 0.5f);
    const vec vHullBoundary4(vHullMins.x, vHullMaxs.y, flHeight * 0.5f);

    bool bOnGround = (iFlags & FL_ONGROUND);
    std::vector<const vec*> vecHitPointPriorityList = {};

    if (bOnGround == true)
    {
        vecHitPointPriorityList = { &vFeet, &vChest, &vPelvis, &vHead, &vHullBoundary1, &vHullBoundary2, &vHullBoundary3, &vHullBoundary4 };
    }
    else
    {
        vecHitPointPriorityList = { &vChest, &vPelvis, &vHead, &vFeet, &vHullBoundary1, &vHullBoundary2, &vHullBoundary3, &vHullBoundary4 };
    }

    const vec&     vLocalPlayerEyePos = pLocalPlayer->GetEyePos();
    i_trace_filter filter(pLocalPlayer);
    trace_t        trace;
    for (const vec* vHitPoint : vecHitPointPriorityList)
    {
        constexpr vec vDebugMultiPointHullBase(2.0f, 2.0f, 2.0f);

        I::EngineTrace->UTIL_TraceRay(vLocalPlayerEyePos, vTargetPos + *vHitPoint, MASK_SHOT, &filter, &trace);

        // Drawing MultiPoint boxes,
        // Green = Choosen, white = can't hit
        if (Features::Aimbot::Aimbot_Projectile::ProjAimbot_DebugMultiPoint.IsActive() == true)
        {
            bool bHit = (trace.m_fraction >= 0.99f);
            I::IDebugOverlay->AddBoxOverlay(vTargetPos + *vHitPoint, vDebugMultiPointHullBase, vDebugMultiPointHullBase * -1.0f,
                qangle(0.0f, 0.0f, 0.0f), (bHit == true ? 0 : 255), 255, (bHit == true ? 0 : 255), 20.0f, 1.0f);
        }

        if (trace.m_fraction >= 0.99f)
        {
            vBestPointOut = vTargetPos + *vHitPoint;
            return true;
        }
    }
    
    return false;
}


BaseEntity* AimbotProjectile_t::_ChooseTarget(BaseEntity* pLocalPlayer)
{
    const AimbotHelper_t::AimbotTargetData_t aimbotTargetData = FeatureObj::aimbotHelper.GetAimbotTargetData();

    BaseEntity* pBestTarget     = nullptr;
    float       flBestDistance  = std::numeric_limits<float>::infinity();
    const vec&  vAttackersPos   = pLocalPlayer->GetEyePos();

    for (BaseEntity* pTarget : aimbotTargetData.m_vecEnemyPlayers)
    {
        float flDist = 0.0f;

        // Getting closest enemy to local player's crosshair
        if (Features::Aimbot::Aimbot_Projectile::ProjAimbot_HitClosestEnemy.IsActive() == false)
        {
            vec vAttackerAngles;
            qangle qAttackerAngles = pLocalPlayer->m_angEyeAngles();
            Maths::AngleVectors(qAttackerAngles, &vAttackerAngles);
            vAttackerAngles.NormalizeInPlace();

            vec vAttackerToTarget = pTarget->GetAbsOrigin() - vAttackersPos;
            vAttackerToTarget.NormalizeInPlace();
            float flDot = vAttackerAngles.Dot(vAttackerToTarget);

            flDist = RAD2DEG(acosf(flDot));

            // Skipping entitiy if out of FOV range
            if (flDist > Features::Aimbot::Aimbot_Projectile::ProjAimbot_FOV.GetData().m_flVal)
                continue;
        }
        else // closest enemy to local player's position.
        {
            flDist = vAttackersPos.DistTo(pTarget->GetAbsOrigin());
        }

        if (flDist < flBestDistance)
        {
            pBestTarget    = pTarget;
            flBestDistance = flDist;
        }
    }

    return pBestTarget;
}


const vec AimbotProjectile_t::_GetBestTickToAim(BaseEntity* pAttacker, baseWeapon* pActiveWeapon, BaseEntity* pBestTaret, float flProjectileSpeed)
{
    vec vBestTargetPos(0.0f, 0.0f, 0.0f);

    constexpr float flMaxSimulationTimeInSec = 2.0f;

    // SIMULATION LOCAL PLAYER
    vec vAttackerEyePos = pAttacker->GetEyePos();
    uint32_t nTicksToSimulate = TIME_TO_TICK(flMaxSimulationTimeInSec);

    // SIMULATING TARGET
    if (FeatureObj::movementSimulation.Initialize(pBestTaret) == false)
        return vBestTargetPos;
    
    for (int iTick = 0; iTick < nTicksToSimulate; iTick++)
    {
        FeatureObj::movementSimulation.RunTick();

        const vec& vFutureTargetPos = FeatureObj::movementSimulation.GetSimulationPos();
        float      flTimeToReach    = vAttackerEyePos.DistTo(vFutureTargetPos) / flProjectileSpeed;
        float      flTimePast       = TICK_TO_TIME(iTick);

        // Projectile should reach quicker than target
        if (flTimeToReach < flTimePast)
        {
            vBestTargetPos = vFutureTargetPos;
            break;
        }
    }
    FeatureObj::movementSimulation.Restore();

    if (Features::Aimbot::Aimbot_Projectile::ProjAimbot_DebugPrediction.IsActive() == true)
    {
        _DrawEntityCollisionHull(pBestTaret, 255, 0, 0, 20, 2.0f);
        _DrawEntityCollisionHull(pBestTaret, vBestTargetPos, 0, 255, 0, 20, 2.0f);
    }

    // Returning vector pointing from our future eye pos to enemy future pos
    return vBestTargetPos - vAttackerEyePos;
}


float AimbotProjectile_t::_GetProjectileSpeed(baseWeapon* pWeapon)
{
    int iWeaponID = pWeapon->GetWeaponTypeID();
    
    if (iWeaponID == TF_WEAPON_ROCKETLAUNCHER || iWeaponID == TF_WEAPON_ROCKETLAUNCHER_DIRECTHIT)
    {
        float flRocketSpeedBase = 1100.0f;
        pWeapon->CALL_ATRIB_HOOK_FLOAT(flRocketSpeedBase, "mult_projectile_speed");
        return flRocketSpeedBase;
    }

    return 0.0f;
}


void AimbotProjectile_t::_DrawEntityCollisionHull(BaseEntity* pEnt, int r, int g, int b, int a, float flDuration)
{
    auto* pCollidable = pEnt->GetCollideable();

    I::IDebugOverlay->AddBoxOverlay(
        pCollidable->GetCollisionOrigin(),
        pCollidable->OBBMins(), pCollidable->OBBMaxs(),
        pCollidable->GetCollisionAngles(),
        r, g, b, a, flDuration);
}

void AimbotProjectile_t::_DrawEntityCollisionHull(BaseEntity* pEnt, const vec& vPosition, int r, int g, int b, int a, float flDuration)
{
    auto* pCollidable = pEnt->GetCollideable();

    I::IDebugOverlay->AddBoxOverlay(
        vPosition,
        pCollidable->OBBMins(), pCollidable->OBBMaxs(),
        pCollidable->GetCollisionAngles(),
        r, g, b, a, flDuration);
}
