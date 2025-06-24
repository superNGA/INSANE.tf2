#include "AimbotProjectile.h"

// SDK
#include "../../../SDK/class/BaseEntity.h"
#include "../../../SDK/class/BaseWeapon.h"
#include "../../../SDK/class/ETFWeaponType.h"
#include "../../../SDK/class/CUserCmd.h"
#include "../../../SDK/class/CommonFns.h"
#include "../../../SDK/class/IVDebugOverlay.h"
#include "../../../SDK/class/FileWeaponInfo.h"
#include "../../../SDK/class/IEngineTrace.h"
#include "../../../SDK/class/CVar.h"
#include "../../MovementSimulation/MovementSimulation.h"
#include "../../ProjectileSimulation/ProjectileSimulation.h"
#include "../../../SDK/TF object manager/TFOjectManager.h"

#include "../AimbotHelper.h"
#include "../../../Extra/math.h"

#define DEBUG_WEAPON_STATS false
#define DRAW_PROJECTILE_PATH false

//=========================================================================
//                     PUBLIC METHODS
//=========================================================================
void AimbotProjectile_t::Run(BaseEntity* pLocalPlayer, baseWeapon* pActiveWeapon, CUserCmd* pCmd, bool* pCreatemoveResult)
{
    // Simulating projectile
#if (DRAW_PROJECTILE_PATH == true)
    if(m_weaponInfo.IsUpdated(pActiveWeapon) == true)
    {
        vec vProjectileOrigin = m_weaponInfo.GetProjectileOrigin(pLocalPlayer, m_weaponInfo.GetShootPosOffset(pLocalPlayer, m_bFlipViewModels), pCmd->viewangles);
        
        /*vec vViewAngles;
        Maths::AngleVectors(pCmd->viewangles, &vViewAngles);
        vViewAngles.NormalizeInPlace();
        I::IDebugOverlay->AddLineOverlay(vProjectileOrigin, vProjectileOrigin + (vViewAngles * 100.0f), 255, 255, 255, true, 1.0f);*/

        FeatureObj::projectileSimulator.Initialize(
            vProjectileOrigin, pCmd->viewangles, 
            m_weaponInfo.GetProjectileSpeed(pLocalPlayer),
            m_weaponInfo.GetProjectileGravity(pLocalPlayer, m_flGravity), 
            m_weaponInfo.m_flUpwardVelOffset,
            m_weaponInfo.m_vHullSize, pLocalPlayer,
            MASK_SHOT
        );
        uint32_t nTicksToSimulate = TIME_TO_TICK(Features::Aimbot::Aimbot_Projectile::ProjAimbot_MaxSimulationTime.GetData().m_flVal);
        for (int i = 0; i < nTicksToSimulate; i++)
            FeatureObj::projectileSimulator.RunTick(true, true, 1.0f);
    }
#endif

    // Draw the history, even when turned off.
    _DrawPredictionHistory();
    _DrawProjectilePathPred(pLocalPlayer, pActiveWeapon);

    // Return if disabled.
    if (Features::Aimbot::Aimbot_Projectile::ProjAimbot_Enable.IsActive() == false)
        return;

    // Initialize CVars.
    _InitliazeCVars();

    // Refreshing weapon's stats
    m_weaponInfo.UpdateWpnInfo(pActiveWeapon);

    // Scanning for target
    if (SDK::CanAttack(pLocalPlayer, pActiveWeapon, pCmd) == true)
    {
        m_pBestTarget = _GetBestTarget(m_weaponInfo, pLocalPlayer, pCmd);
    }

    // no target found
    if (m_pBestTarget == nullptr)
        return;

    // Aim at calculated position
    if (_ShouldAim(pLocalPlayer, pActiveWeapon, pCmd) == true)
    {
        m_flDrawStartTime          = CUR_TIME;
        m_bProjectilePathPredicted = false; 

        pCmd->viewangles    = _GetTargetAngles(pLocalPlayer, pCmd->viewangles);
        m_qLastAimbotAngles = pCmd->viewangles;
        *pCreatemoveResult  = false;

        _ResetTargetData();
    }
}


void AimbotProjectile_t::Reset()
{
    // CVars...
    m_bInitializedCVars = false;
    m_bFlipViewModels   = 0;
    m_flGravity         = 0.0f;

    m_pBestTarget       = nullptr;
    m_vBestTargetFuturePos.Init();
    m_weaponInfo.Reset();
}

//=========================================================================
//                     PRIVATE METHODS
//=========================================================================
BaseEntity* AimbotProjectile_t::_GetBestTarget(const ProjectileWeaponInfo_t& weaponInfo, BaseEntity* pAttacker, CUserCmd* pCmd)
{
    std::vector<BaseEntity*> vecTargets = FeatureObj::aimbotHelper.GetAimbotTargetData().m_vecEnemyPlayers;

    float flAngBestDistance = std::numeric_limits<float>::infinity();
    BaseEntity* pBestTarget = nullptr;

    // Our Eyepos 
    vec vAttackerEyePos = pAttacker->GetEyePos();

    // Projectile's Gravity, Speed & origin
    float flProjGravity     = weaponInfo.GetProjectileGravity(pAttacker, m_flGravity);
    float flProjVelocity    = weaponInfo.GetProjectileSpeed(pAttacker);
    vec   vShootPosOffset   = weaponInfo.GetShootPosOffset(pAttacker, m_bFlipViewModels);
    vec   vProjectileOrigin = weaponInfo.GetProjectileOrigin(pAttacker, vShootPosOffset, pCmd->viewangles);

    uint32_t nTickToSimulate = TIME_TO_TICK(Features::Aimbot::Aimbot_Projectile::ProjAimbot_MaxSimulationTime.GetData().m_flVal);
    
    // Will store prediction history here
    m_vecTargetPredictionHistory.resize(nTickToSimulate + 1U);

    // Looping though all targets and finding the most "killable"
    for (BaseEntity* pTarget : vecTargets)
    {
        // This stores the predicted positions for this entity, which we can use to draw :)
        std::vector<vec> vecTargetPredictionHistory(nTickToSimulate + 1U);

        // Checking if in FOV or not
        vec vTargetsInitialPos = pTarget->GetAbsOrigin();
        vec vTargetFuturePos;
        float flAngDistance = _GetAngleFromCrosshair(vTargetsInitialPos, vAttackerEyePos, pCmd->viewangles);
        if (flAngDistance > Features::Aimbot::Aimbot_Projectile::ProjAimbot_FOV.GetData().m_flVal)
            continue;

        // Constructing Projectile Origin for each entity.
        qangle qAttackerToTarget;
        Maths::VectorAnglesFromSDK(vTargetsInitialPos - vAttackerEyePos, qAttackerToTarget);
        vProjectileOrigin = weaponInfo.GetProjectileOrigin(pAttacker, vShootPosOffset, qAttackerToTarget);

        // Simulating Target
        FeatureObj::movementSimulation.Initialize(pTarget);
        for (int iTick = 0; iTick < nTickToSimulate; iTick++)
        {
            // I absolutely didn't forgot this
            FeatureObj::movementSimulation.RunTick();

            float flTargetsTimeToReach          = TICK_TO_TIME(iTick);
            float flProjectilesTimeToReach      = 0.0f;
            float flProjLaunchAngle             = 0.0f;
            vecTargetPredictionHistory[iTick]   = FeatureObj::movementSimulation.GetSimulationPos();

            // Calculaing our projectile's time to reach to the center of the target
            bool bCanReach = _SolveProjectileMotion(
                vProjectileOrigin,
                vecTargetPredictionHistory[iTick],
                flProjVelocity,
                flProjGravity,
                flProjLaunchAngle,
                flProjectilesTimeToReach);

            // Exit on the first tick when projectile can reach target faster
            if (bCanReach == true && flProjectilesTimeToReach < flTargetsTimeToReach)
            {
                vTargetFuturePos         = vecTargetPredictionHistory[iTick];
                m_nValidPredictionRecord = iTick;
                break;
            }
        }
        FeatureObj::movementSimulation.Restore();

        // if we didn't found any hitable tick for this enitity than skip
        if (vTargetFuturePos.IsEmpty() == true)
            continue;

        // Getting best point on target's hull to hit
        vec vBestHitPoint;
        bool bCanHit = _GetBestHitPointOnTargetHull(
            pTarget,            vTargetFuturePos,
            weaponInfo,         vBestHitPoint,
            vProjectileOrigin,  flProjVelocity,
            flProjGravity,      pAttacker
        );
        if (bCanHit == false)
            continue;

        // compare against best target
        if (flAngDistance < flAngBestDistance)
        {
            pBestTarget            = pTarget;
            flAngBestDistance      = flAngDistance;
            m_vBestTargetFuturePos = vBestHitPoint;
            m_vecTargetPredictionHistory = std::move(vecTargetPredictionHistory);
        }
    }

    return pBestTarget;
}

float AimbotProjectile_t::_GetAngleFromCrosshair(const vec& vTargetPos, const vec& vOrigin, const qangle& qViewAngles)
{
    vec vViewAngles;
    Maths::AngleVectors(qViewAngles, &vViewAngles);
    vViewAngles.NormalizeInPlace();

    vec vAttackerToTarget = vTargetPos - vOrigin;
    vAttackerToTarget.NormalizeInPlace();

    float flDot = vViewAngles.Dot(vAttackerToTarget);
    return RAD2DEG(acosf(flDot));
}


bool AimbotProjectile_t::_ShouldAim(BaseEntity* pLocalPlayer, baseWeapon* pActiveWeapon, CUserCmd* pCmd)
{
    // can we even attack this tick?
    if (SDK::CanAttack(pLocalPlayer, pActiveWeapon, pCmd) == false)
        return false;

    float flCurTime  = TICK_TO_TIME(pLocalPlayer->m_nTickBase());
    bool  bShouldAim = flCurTime >= pActiveWeapon->m_flNextPrimaryAttack() && (pCmd->buttons & IN_ATTACK);

    bool bRequiresCharging = 
        m_weaponInfo.m_pWeaponFileInfo->m_iProjectile == TF_PROJECTILE_ARROW ||
        m_weaponInfo.m_pWeaponFileInfo->m_iProjectile == TF_PROJECTILE_FESTIVE_ARROW ||
        m_weaponInfo.m_pWeaponFileInfo->m_iProjectile == TF_PROJECTILE_PIPEBOMB_REMOTE ||
        m_weaponInfo.m_pWeaponFileInfo->m_iProjectile == TF_PROJECTILE_PIPEBOMB_PRACTICE;

    // if doesn't require charing them aim this will work flawlessly.
    if (bRequiresCharging == false)
        return bShouldAim;

    // NOTE : if we are using charging weapons, then we need to aim on the tick we lift our attack button,
    //          that is the first tick where "bShouldAim" is false.
    if (m_bLastShouldAim == true && bShouldAim == false)
        return true;

    m_bLastShouldAim = bShouldAim;
    return false;
}


void AimbotProjectile_t::_DrawPredictionHistory()
{
    if (CUR_TIME - m_flDrawStartTime < flPredictionHistoryDrawingLife)
    {
        uint32_t nMaxRecords = m_vecTargetPredictionHistory.size();
        for (int i = 1; i < m_nValidPredictionRecord - 1 && i < nMaxRecords; i++)
        {
            if (m_vecTargetPredictionHistory[i].IsEmpty() == true)
                break;

            I::IDebugOverlay->AddLineOverlay(m_vecTargetPredictionHistory[i - 1], m_vecTargetPredictionHistory[i], 255, 255, 255, true, 5.0f);
        }
    }
    else
    {
        m_flDrawStartTime = 0.0f;
        _ClearPredictionHistory();
    }
}

void AimbotProjectile_t::_DrawProjectilePathPred(BaseEntity* pLocalPlayer, baseWeapon* pActiveWeapon)
{
    // We need updated weapon info
    if (m_weaponInfo.IsUpdated(pActiveWeapon) == false)
        return;

    // Simulating only before starting drawing
    if (m_flDrawStartTime > 1.0f && m_bProjectilePathPredicted == false)
    {
        vec   vShootOffset      = m_weaponInfo.GetShootPosOffset(pLocalPlayer, m_bFlipViewModels);
        vec   vProjectileOrigin = m_weaponInfo.GetProjectileOrigin(pLocalPlayer, vShootOffset, m_qLastAimbotAngles);
        float flProjVel         = m_weaponInfo.GetProjectileSpeed(pLocalPlayer);
        float flProjGravity     = m_weaponInfo.GetProjectileGravity(pLocalPlayer, m_flGravity);

        // Initializing projectile simulator
        FeatureObj::projectileSimulator.Initialize(
            vProjectileOrigin,  m_qLastAimbotAngles,
            flProjVel,          flProjGravity,
            m_weaponInfo.m_flUpwardVelOffset,
            m_weaponInfo.m_vHullSize, pLocalPlayer);

        vec vEndPos = m_vecTargetPredictionHistory[m_nValidPredictionRecord];

        // Simulating projectile at aimbot angles and recording ticks
        uint32_t nTicksToSimulate = TIME_TO_TICK(Features::Aimbot::Aimbot_Projectile::ProjAimbot_MaxSimulationTime.GetData().m_flVal);
        m_vecProjectilePathPred.resize(nTicksToSimulate + 1u);

        for (int iTick = 0; iTick < nTicksToSimulate; iTick++)
        {
            // Not tracing while simulating cause the trace is colliding with 
            // the projectile itself and caausing trouble.
            FeatureObj::projectileSimulator.RunTick(false);
            m_vecProjectilePathPred[iTick] = FeatureObj::projectileSimulator.m_projectileInfo.m_vLauchPos;
            m_nValidProjectilePathRecord   = iTick;

            // if projectile prediction hits the target then break, else draw missed projectile path
            if (m_vecProjectilePathPred[iTick].DistTo(vEndPos) < 15.0f)
                break;
        }

        m_bProjectilePathPredicted = true;
    }

    if (CUR_TIME - m_flDrawStartTime < flPredictionHistoryDrawingLife)
    {
        uint32_t nTicks = Maths::MIN<uint32_t>(m_nValidProjectilePathRecord, m_vecProjectilePathPred.size());
        for (int iTick = 1; iTick < nTicks; iTick++)
        {
            constexpr vec vHullSize(1.0f, 1.0f, 1.0f);
            if(iTick == 1)
            {
                I::IDebugOverlay->AddBoxOverlay(
                    m_vecProjectilePathPred[iTick], vHullSize * -1.5f, vHullSize * 1.5f,
                    m_qLastAimbotAngles, 255, 255, 255, 40.0f, 1.0f);
            }
            else
            {
                I::IDebugOverlay->AddBoxOverlay(
                    m_vecProjectilePathPred[iTick], vHullSize * -1.0f, vHullSize,
                    m_qLastAimbotAngles, 145, 145, 145, 40.0f, 1.0f);
            }

            I::IDebugOverlay->AddLineOverlay(
                m_vecProjectilePathPred[iTick - 1],
                m_vecProjectilePathPred[iTick],
                255, 255, 255, true, 1.0f
            );
        }
    }
    else
    {
        m_bProjectilePathPredicted = false;
        m_flDrawStartTime = 0.0f;
        m_nValidProjectilePathRecord = 0;
        m_qLastAimbotAngles.Init();
    }
}

bool AimbotProjectile_t::_SolveProjectileMotion(
    const vec&  vLauchPos,      const vec&  vTargetPos, 
    const float flProjVelocity, const float flGravity, 
    float&      flAngleOut,     float&      flTimeToReach)
{
    float x = vLauchPos.Dist2Dto(vTargetPos);
    float y = vTargetPos.z - vLauchPos.z;

    // Handling no gravity case like Rockets
    if (flGravity <= 1.0f)
    {
        flAngleOut    = RAD2DEG(atanf(y / x));
        flTimeToReach = vLauchPos.DistTo(vTargetPos) / flProjVelocity;

        // we can always hit a enemy with a rocket ( without any walls )
        return true;
    }

    float flDiscriminant = powf(flProjVelocity, 4.0f) - flGravity * (((x * x) * flGravity) - (2.0f * flProjVelocity * flProjVelocity * y));
    
    // Projectile can't reach this position.
    if (flDiscriminant < 0.0f)
        return false;

    // Calculating both solutions
    float flSolution1 = (-(flProjVelocity * flProjVelocity) + sqrtf(flDiscriminant)) / (x * flGravity);
    float flSolution2 = (-(flProjVelocity * flProjVelocity) - sqrtf(flDiscriminant)) / (x * flGravity);

    float flAngle1InDeg = RAD2DEG(atanf(flSolution1));
    float flAngle2InDeg = RAD2DEG(atanf(flSolution2));

    float flTimeToReach1 = x / (flProjVelocity * cos(atanf(flSolution1)));
    float flTimeToReach2 = x / (flProjVelocity * cos(atanf(flSolution2)));

    flAngleOut    = flTimeToReach1 < flTimeToReach2 ? flAngle1InDeg  : flAngle2InDeg;
    flTimeToReach = flTimeToReach1 < flTimeToReach2 ? flTimeToReach1 : flTimeToReach2;

    return true;
}

bool AimbotProjectile_t::_GetBestHitPointOnTargetHull(
    BaseEntity* pTarget,                        const vec& vTargetOrigin, 
    const ProjectileWeaponInfo_t& weaponInfo,   vec& vBestPointOut, 
    const vec& vProjectileOrigin,               const float flProjVelocity,
    const float flProjGravity,                  BaseEntity* pProjectileOwner)
{
    ICollideable_t* pCollidable = pTarget->GetCollideable();
    const vec&      vHullMins   = pCollidable->OBBMins();
    const vec&      vHullMaxs   = pCollidable->OBBMaxs();

    float flHeight = Maths::MAX<float>(vHullMaxs.z, vHullMins.z);

    // Positions on the hull to check ( HitPoints )
    constexpr float flSafeTraceOffset = 2.0f;
    const vec vHead(  0.0f, 0.0f, flHeight - flSafeTraceOffset);
    const vec vFeet(  0.0f, 0.0f, flSafeTraceOffset);
    const vec vChest( 0.0f, 0.0f, flHeight * 0.66);
    const vec vPelvis(0.0f, 0.0f, flHeight * 0.33);

    const vec vHeadLoose1(vHullMins.x, vHullMaxs.y, flHeight - flSafeTraceOffset);
    const vec vHeadLoose2(vHullMaxs.x, vHullMins.y, flHeight - flSafeTraceOffset);

    const vec vHullBoundary1(vHullMins.x, vHullMins.y, flHeight * 0.5f);
    const vec vHullBoundary2(vHullMaxs.x, vHullMaxs.y, flHeight * 0.5f);
    const vec vHullBoundary3(vHullMaxs.x, vHullMins.y, flHeight * 0.5f);
    const vec vHullBoundary4(vHullMins.x, vHullMaxs.y, flHeight * 0.5f);

    // List of HitPoints ( Depending on current weapon )
    const std::vector<const vec*> vecHeadPriorityHitpointList = {
        &vHead, &vHeadLoose1, &vHeadLoose2, &vChest, &vPelvis, &vHullBoundary1, &vHullBoundary2,&vHullBoundary3,&vHullBoundary4, &vFeet 
    };
    const std::vector<const vec*> vecChestPriorityHitpointList = {
        &vChest, &vPelvis, &vHead, &vFeet, &vHullBoundary1, &vHullBoundary2, &vHullBoundary3, &vHullBoundary4
    };
    const std::vector<const vec*> vecFeetPriorityHitpointList = {
        &vFeet, &vChest, &vPelvis, &vHullBoundary1, &vHullBoundary2, &vHullBoundary3, &vHullBoundary4, &vHead
    };

    // Choosing HitPoint Priority list 
    const std::vector<const vec*>* vecHitPointPriorityList = &vecChestPriorityHitpointList;
    if (weaponInfo.m_pWeaponFileInfo->m_iProjectile == TF_PROJECTILE_ARROW || 
        weaponInfo.m_pWeaponFileInfo->m_iProjectile == TF_PROJECTILE_FESTIVE_ARROW)
    {
        vecHitPointPriorityList = &vecHeadPriorityHitpointList;
    }
    else if (weaponInfo.m_pWeaponFileInfo->m_iProjectile == TF_PROJECTILE_ROCKET && (pTarget->m_fFlags() & FL_ONGROUND))
    {
        vecHitPointPriorityList = &vecFeetPriorityHitpointList;
    }

    // Choosing most hitable HitPoint
    vec vAttackerEyePos = pProjectileOwner->GetEyePos();
    vec vShootPosOffset = weaponInfo.GetShootPosOffset(pProjectileOwner, m_bFlipViewModels);
    vec vBestHitpoint;
    for (const vec* vHitPointOffset : *vecHitPointPriorityList)
    {
        vec vTargetHitPoint = vTargetOrigin + *vHitPointOffset;

        // Constructing projectile origin for each HitPoint
        qangle qAttackerToHitPoint;
        Maths::VectorAnglesFromSDK(vTargetHitPoint - vAttackerEyePos, qAttackerToHitPoint);
        vec vProjectileOriginCustom = weaponInfo.GetProjectileOrigin(pProjectileOwner, vShootPosOffset, qAttackerToHitPoint);

        float flProjLaunchAngle = 0.0f;
        float flTimeToReach     = 0.0f;
        _SolveProjectileMotion(
            vProjectileOriginCustom,
            vTargetHitPoint,
            flProjVelocity, flProjGravity, 
            flProjLaunchAngle, flTimeToReach
        );

        // Calculaing aimbot view angles
        qangle qProjLaunchAngles;
        Maths::VectorAnglesFromSDK(vTargetHitPoint - vProjectileOriginCustom, qProjLaunchAngles);
        qProjLaunchAngles.pitch = flProjLaunchAngle * -1.0f; // setting pitch in valve format ( down -ve & up +ve )

        uint32_t nTicksToSimulate = TIME_TO_TICK(flTimeToReach);
        FeatureObj::projectileSimulator.Initialize(
            vProjectileOriginCustom, qProjLaunchAngles, flProjVelocity,
            flProjGravity, weaponInfo.m_flUpwardVelOffset,
            weaponInfo.m_vHullSize, pProjectileOwner
        );

        for (int iTick = 0; iTick < nTicksToSimulate; iTick++)
            FeatureObj::projectileSimulator.RunTick();

        constexpr vec vMultiPointBoxMins(3.0f, 3.0f, 3.0f);

        // we can hit this hitpoint.
        constexpr float flProjectileHitTolerance = 3.0f;
        const ProjectileInfo_t& projSimInfo      = FeatureObj::projectileSimulator.m_projectileInfo;
        if( projSimInfo.m_bDidHit == false || projSimInfo.m_pEntHit == pTarget ||
            FeatureObj::projectileSimulator.m_projectileInfo.m_vEndPos.DistTo(vTargetHitPoint) < flProjectileHitTolerance)
        {
            vBestHitpoint = vTargetHitPoint;

            if (Features::Aimbot::Aimbot_Projectile::ProjAimbot_DebugMultiPoint.IsActive() == true)
                I::IDebugOverlay->AddBoxOverlay(vTargetHitPoint, vMultiPointBoxMins * -1.0f, vMultiPointBoxMins, qangle(0.0f, 0.0f, 0.0f), 0, 255, 0, 40, 1.0f);

            break;
        }

        if (Features::Aimbot::Aimbot_Projectile::ProjAimbot_DebugMultiPoint.IsActive() == true)
            I::IDebugOverlay->AddBoxOverlay(vTargetHitPoint, vMultiPointBoxMins * -1.0f, vMultiPointBoxMins, qangle(0.0f, 0.0f, 0.0f), 255, 255, 255, 40, 1.0f);
    }

    // if not empty then return true
    vBestPointOut = vBestHitpoint;
    return (vBestPointOut.IsEmpty() == false);
}


const qangle AimbotProjectile_t::_GetTargetAngles(BaseEntity* pAttacker, const qangle& qViewAngles)
{
    qangle qAttackerToTarget;
    Maths::VectorAnglesFromSDK(m_vBestTargetFuturePos - pAttacker->GetEyePos(), qAttackerToTarget);
    vec   vShootOffset      = m_weaponInfo.GetShootPosOffset(pAttacker, m_bFlipViewModels);
    vec   vProjectileOrigin = m_weaponInfo.GetProjectileOrigin(pAttacker, vShootOffset, qAttackerToTarget);
    float flProjVel         = m_weaponInfo.GetProjectileSpeed(pAttacker);
    float flProjGravity     = m_weaponInfo.GetProjectileGravity(pAttacker, m_flGravity);

    qangle qBestAngles;
    Maths::VectorAnglesFromSDK(m_vBestTargetFuturePos - vProjectileOrigin, qBestAngles);
    
    float flProjLaunchAngle = 0.0f;
    float flTimeToReach     = 0.0f;
    _SolveProjectileMotion(
        vProjectileOrigin, m_vBestTargetFuturePos,
        flProjVel, flProjGravity,
        flProjLaunchAngle,
        flTimeToReach
    );
    qBestAngles.pitch = flProjLaunchAngle * -1.0f; // Gotta invert the pitch.

    return qBestAngles;
}


void AimbotProjectile_t::_InitliazeCVars()
{
    if (m_bInitializedCVars == true)
        return;

    // Getting all necessary CVars
    m_bFlipViewModels = I::iCvar->FindVar("cl_flipviewmodels")->GetInt();
    m_flGravity = I::iCvar->FindVar("sv_gravity")->GetFloat();
    m_flGravity *= -1.0f; // "Fixxing Gravity"

    WIN_LOG("Initializd CVars");

    // Initialized CVars
    m_bInitializedCVars = true;
}

//=========================================================================
//                     ProjectileWeaponInfo_t METHODS
//=========================================================================
void ProjectileWeaponInfo_t::Reset()
{
    m_vShootPosOffset.Init();
    m_vHullSize.Init();
    m_pWeapon                     = nullptr;
    m_flProjectileBaseSpeed       = 0.0f;
    m_flUpwardVelOffset           = 0.0f;
    m_flProjectileBaseGravityMult = 0.0f;
    m_pWeaponFileInfo             = nullptr;
}

void ProjectileWeaponInfo_t::UpdateWpnInfo(baseWeapon* pActiveWeapon)
{
    // Weapon didn't change, no need to update all stats
    if (pActiveWeapon == m_pWeapon)
        return;

    // if weapon did change from last tick, update stats
    m_pWeapon                     = pActiveWeapon;
    m_pWeaponFileInfo             = m_pWeapon->GetTFWeaponInfo()->GetWeaponData(0);
    m_vShootPosOffset             = _GetWpnBaseShootPosOffset(m_pWeapon, m_pWeaponFileInfo->m_iProjectile);
    m_vHullSize                   = _GetHullSize(m_pWeaponFileInfo->m_iProjectile);
    m_flProjectileBaseSpeed       = _GetBaseProjectileSpeed(m_pWeaponFileInfo, m_pWeapon);
    m_flProjectileBaseGravityMult = _GetBaseProjectileGravityMult(m_pWeaponFileInfo->m_iProjectile);
    m_flUpwardVelOffset           = _GetUpwardVelocityOffset(m_pWeaponFileInfo->m_iProjectile);

    // Notifying user of weapon stat refresh
#if (DEBUG_WEAPON_STATS == true)
    FAIL_LOG("Weapon change detected, Refreshing weapon's stats");
    LOG("WPN info updated to the following : ");
    printf("Base Projectile Speed : %.2f\n", m_flProjectileBaseSpeed);
    printf("Base Gravity mult     : %.2f\n", m_flProjectileBaseGravityMult);
    printf("Upward velocty offset : %.2f\n", m_flUpwardVelOffset);
    LOG_VEC3(m_vShootPosOffset);
#endif
}

vec ProjectileWeaponInfo_t::GetProjectileOrigin(BaseEntity* pWeaponOwner, const vec& vShootPosOffset, const qangle& qViewAngles) const
{
    vec vForward, vRight, vUp;
    Maths::AngleVectors(qViewAngles, &vForward, &vRight, &vUp);
    vec vProjectileOrigin = pWeaponOwner->GetEyePos() + (vForward * vShootPosOffset.x) + (vRight * vShootPosOffset.y) + (vUp * vShootPosOffset.z);
    return vProjectileOrigin;
}

vec ProjectileWeaponInfo_t::GetShootPosOffset(BaseEntity* pWeaponOwner, int bFlipedViewModels) const
{
    vec vBaseShootPosOffst = m_vShootPosOffset;

    // flipping Y offset if view model is flipped.
    if (bFlipedViewModels == 1)
        vBaseShootPosOffst.y *= -1.0f;

    // Some weapons have a different Z offset when ducking, so we will 
    // compensate for that too.
    if (pWeaponOwner->m_fFlags() & IN_DUCK)
    {
        switch (m_pWeaponFileInfo->m_iProjectile)
        {
        case TF_PROJECTILE_ROCKET:
        case TF_PROJECTILE_FLARE:
        case TF_PROJECTILE_FLAME_ROCKET:
        case TF_PROJECTILE_ENERGY_BALL:
        case TF_PROJECTILE_ENERGY_RING:
            vBaseShootPosOffst.z = 8.0f;
            break;
        default:
            break;
        }
    }

    return vBaseShootPosOffst;
}


float ProjectileWeaponInfo_t::GetProjectileSpeed(BaseEntity* pWeaponOwner) const
{
    float flCurTime = TICK_TO_TIME(pWeaponOwner->m_nTickBase());

    // Calculating velocity for weapons that require charging.
    switch (m_pWeaponFileInfo->m_iProjectile)
    {
    case TF_PROJECTILE_ARROW:
    case TF_PROJECTILE_FESTIVE_ARROW:
    {
        float flCharge = 0.0f;
        
        if (m_pWeapon->m_flChargeBeginTime() >= 0.1f)
            flCharge = Maths::MIN<float>(flCurTime - m_pWeapon->m_flChargeBeginTime(), 1.0f);
        
        return Maths::RemapValClamped(flCharge, 0.0f, 1.f, 1800.0f, 2600.0f);
        break;
    }
    case TF_PROJECTILE_PIPEBOMB_REMOTE:
    case TF_PROJECTILE_PIPEBOMB_PRACTICE:
    {
        constexpr float TF_PIPEBOMB_MIN_CHARGE_VEL = 900.0f;
        constexpr float TF_PIPEBOMB_MAX_CHARGE_VEL = 2400.0f;
        constexpr float TF_PIPEBOMB_MAX_CHARGE_TIME = 4.0f;

        // Calculating current charge ammount.
        float flCharge = flCurTime - m_pWeapon->m_flChargeBeginTime();

        // Calculating max charge time for stickies
        // TODO : This can be done in advance, calculate & account for attributes ahead of time.
        float flMaxChargeTime = TF_PIPEBOMB_MAX_CHARGE_TIME;
        m_pWeapon->CALL_ATRIB_HOOK_FLOAT(flMaxChargeTime, "stickybomb_charge_rate");

        return Maths::RemapValClamped(flCharge, 0.0f, flMaxChargeTime, TF_PIPEBOMB_MIN_CHARGE_VEL, TF_PIPEBOMB_MAX_CHARGE_VEL);
        break;
    }
    default:
        break;
    }

    return m_flProjectileBaseSpeed;
}


float ProjectileWeaponInfo_t::GetProjectileGravity(BaseEntity* pWeaponOwner, const float flBaseGravity) const
{
    // Only arrow's gravity is influenced by charging.
    switch (m_pWeaponFileInfo->m_iProjectile)
    {
    case TF_PROJECTILE_ARROW:
    case TF_PROJECTILE_FESTIVE_ARROW:
    {
        float flCurTime = TICK_TO_TIME(pWeaponOwner->m_nTickBase());
        float flCharge = 0.0f;
        if (m_pWeapon->m_flChargeBeginTime() > 0.1f)
            flCharge = Maths::MIN<float>(flCurTime - m_pWeapon->m_flChargeBeginTime(), 1.0f);
        return flBaseGravity * Maths::RemapValClamped(flCharge, 0.0f, 1.f, 0.5, 0.1);
    }
    default:
        return flBaseGravity * m_flProjectileBaseGravityMult;
    }

    return flBaseGravity * m_flProjectileBaseGravityMult;
}

bool ProjectileWeaponInfo_t::IsUpdated(baseWeapon* pWeapon)
{
    return pWeapon == m_pWeapon;
}


const vec ProjectileWeaponInfo_t::_GetWpnBaseShootPosOffset(const baseWeapon* pWeapon, const ProjectileType_t iProjectileType) const
{
    vec vShootPosOffset;
    switch (iProjectileType)
    {
    case TF_PROJECTILE_ROCKET:
    {
        vShootPosOffset = { 23.5f, 12.0f, -3.0f };
        break;
    }
    case TF_PROJECTILE_SYRINGE:
    {
        vShootPosOffset = { 16.0f, 6.0f, -8.0f };
        break;
    }
    case TF_PROJECTILE_FLARE:
    {
        vShootPosOffset = { 23.5f, 12.0f, -3.0f };
        break;
    }
    case TF_PROJECTILE_PIPEBOMB:
    case TF_PROJECTILE_PIPEBOMB_REMOTE:
    case TF_PROJECTILE_PIPEBOMB_PRACTICE:
    case TF_PROJECTILE_CANNONBALL:
    {
        vShootPosOffset = { 16.0f, 8.0f, -6.0f };
        break;
    }
    case TF_PROJECTILE_JAR:
    case TF_PROJECTILE_JAR_MILK:
    case TF_PROJECTILE_CLEAVER:
    case TF_PROJECTILE_THROWABLE:
    case TF_PROJECTILE_FESTIVE_JAR:
    case TF_PROJECTILE_BREADMONSTER_JARATE:
    case TF_PROJECTILE_BREADMONSTER_MADMILK:
    {
        vShootPosOffset = { 16.0f, 8.0f, -6.0f };
        break;
    }
    case TF_PROJECTILE_ARROW:
    case TF_PROJECTILE_HEALING_BOLT:
    case TF_PROJECTILE_BUILDING_REPAIR_BOLT:
    case TF_PROJECTILE_FESTIVE_ARROW:
    case TF_PROJECTILE_FESTIVE_HEALING_BOLT:
    case TF_PROJECTILE_GRAPPLINGHOOK:
    {
        vShootPosOffset = { 23.5f, 8.0f, -3.0f };
        break;
    }

    case TF_PROJECTILE_FLAME_ROCKET: // Drangon's furry uses this maybe
    {
        vShootPosOffset = { 23.5f, -8.0f, -3.0f };
        break;
    }

    case TF_PROJECTILE_ENERGY_BALL: // Cow-Mangler's projectile
    case TF_PROJECTILE_ENERGY_RING: // Rightous bison's projectile
    {
        vShootPosOffset = { 23.5f, -8.0f, -3.0f };
        break;
    }

    default:
        break;
    }

    // Adjusting for center fire weapons
    int iCenterFireProjectile = 0;
    pWeapon->CALL_ATRIB_HOOK_INT(iCenterFireProjectile, "centerfire_projectile");
    if (iCenterFireProjectile == 1)
        vShootPosOffset.y = 0.0f;

    return vShootPosOffset;
}

const float ProjectileWeaponInfo_t::_GetBaseProjectileSpeed(const WeaponData_t* pWeaponFileInfo, const baseWeapon* pWeapon) const
{
    auto  iProjectileType = pWeaponFileInfo->m_iProjectile;

    switch (iProjectileType)
    {
        // Base Rocket velocity is 0 and if you get it form weapon info then its 0.
        // so hard coding it is the best option.
    case TF_PROJECTILE_ROCKET:
    {
        constexpr float flBaseRocketVelocity = 1100.0f;
        float           flRocketVelocity     = flBaseRocketVelocity;
        pWeapon->CALL_ATRIB_HOOK_FLOAT(flRocketVelocity, "mult_projectile_speed");
        return flRocketVelocity;
        break;
    }
    case TF_PROJECTILE_SYRINGE:
    {
        constexpr float SYRINGE_GRAVITY_MULTIPLIER = 0.3f;
        constexpr float SYRINGE_VELOCITY = 1000.0f;

        return SYRINGE_VELOCITY;
    }
    case TF_PROJECTILE_FLARE:
    {
        constexpr float FLARE_GRAVITY_MULTIPLIER = 0.3f;
        constexpr float FLARE_SPEED   = 2000.0f;

        float flFlareSpeed = FLARE_SPEED;
        pWeapon->CALL_ATRIB_HOOK_FLOAT(flFlareSpeed, "mult_projectile_speed");
        return flFlareSpeed;
    }
    case TF_PROJECTILE_PIPEBOMB:
    case TF_PROJECTILE_CANNONBALL:
    {
        constexpr float TF_DEFAULT_PIPE_VELOCITY = 1200.0f;
        float flPipeBaseSpeed = TF_DEFAULT_PIPE_VELOCITY;
        pWeapon->CALL_ATRIB_HOOK_FLOAT(flPipeBaseSpeed, "mult_projectile_range");
        float flPipeSpeed = sqrtf((flPipeBaseSpeed * flPipeBaseSpeed) + (200.0f * 200.0f));
        return flPipeSpeed;
        break;
    }
    
    // just store the base velocity. This should get us through the 
    // checking phase atleast.
    case TF_PROJECTILE_PIPEBOMB_REMOTE:
    case TF_PROJECTILE_PIPEBOMB_PRACTICE:
    {
        constexpr float TF_PIPEBOMB_MIN_CHARGE_VEL = 900.0f;
        return TF_PIPEBOMB_MIN_CHARGE_VEL;
    }
    
    case TF_PROJECTILE_JAR:
    case TF_PROJECTILE_JAR_MILK:
    case TF_PROJECTILE_THROWABLE:
    case TF_PROJECTILE_FESTIVE_JAR:
    case TF_PROJECTILE_BREADMONSTER_JARATE:
    case TF_PROJECTILE_BREADMONSTER_MADMILK:
    {
        constexpr float TF_JAR_SPEED = 1000.0f;
        return TF_JAR_SPEED;
    }
    case TF_PROJECTILE_CLEAVER:
    {
        constexpr float TF_CLEAVER_SPEED = 7000.0f;
        return TF_CLEAVER_SPEED;
    }
    // The Arrow's velocity is influenced by charge level, but I have store 
    // the base velocity cause if we are able to hit someone with base velocity we 
    // should also be able to hit them with increased velocity too. :)
    case TF_PROJECTILE_ARROW:
    case TF_PROJECTILE_FESTIVE_ARROW:
    {
        constexpr float TF_ARROW_BASE_VELOCITY = 1800.0f;
        return TF_ARROW_BASE_VELOCITY;
    }
    //case TF_PROJECTILE_GRAPPLINGHOOK: I don't think this will benifit form being Projectile Aimbot-ed
    case TF_PROJECTILE_BUILDING_REPAIR_BOLT:
    case TF_PROJECTILE_FESTIVE_HEALING_BOLT:
    case TF_PROJECTILE_HEALING_BOLT: // According to TF official WIKI, this shit also has velocity of 2400.0f 
    {
        constexpr float TF_HEALING_BOLT_BASE_VELOCITY = 2400.0f;
        return TF_HEALING_BOLT_BASE_VELOCITY;
    }

    default:
        break;
    }

    return 0.0f;
}

const float ProjectileWeaponInfo_t::_GetBaseProjectileGravityMult(const ProjectileType_t iProjectileType)
{
    constexpr float TF_DEFAULT_NADE_GRAVITY = 1.0f;

    switch (iProjectileType)
    {
    // Default gravity weapons
    case TF_PROJECTILE_ROCKET:
        return 0.0f;

    case TF_PROJECTILE_JAR:
    case TF_PROJECTILE_JAR_MILK:
    case TF_PROJECTILE_CLEAVER:
    case TF_PROJECTILE_THROWABLE:
    case TF_PROJECTILE_FESTIVE_JAR:
    case TF_PROJECTILE_BREADMONSTER_JARATE:
    case TF_PROJECTILE_BREADMONSTER_MADMILK:
        return TF_DEFAULT_NADE_GRAVITY;

    case TF_PROJECTILE_SYRINGE:
    {
        constexpr float SYRINGE_GRAVITY_MULTIPLIER = 0.3f;
        return SYRINGE_GRAVITY_MULTIPLIER;
    }
    case TF_PROJECTILE_FLARE:
    {
        constexpr float FLARE_GRAVITY_MULTIPLIER = 0.3f;
        return FLARE_GRAVITY_MULTIPLIER;
    }
    case TF_PROJECTILE_PIPEBOMB:
    case TF_PROJECTILE_CANNONBALL:
    case TF_PROJECTILE_PIPEBOMB_REMOTE:
    case TF_PROJECTILE_PIPEBOMB_PRACTICE:
    {
        constexpr float TF_WEAPON_PIPEBOMB_GRAVITY = 0.5f;
        return TF_WEAPON_PIPEBOMB_GRAVITY;
    }

    case TF_PROJECTILE_BUILDING_REPAIR_BOLT:
    case TF_PROJECTILE_FESTIVE_HEALING_BOLT:
    case TF_PROJECTILE_HEALING_BOLT:
    {
        constexpr float TF_HEALING_BOLT_GRAVITY_MULTIPLIER = 0.2f;
        return TF_HEALING_BOLT_GRAVITY_MULTIPLIER;
    }

    // Although the arrow gravity is depended on the charge level, 
    // I'm storing it as default here, cause if we can hit someone with
    // defult gravity, we shoudl be able to hit them with lowered gravity. :)
    case TF_PROJECTILE_ARROW:
    case TF_PROJECTILE_FESTIVE_ARROW:
    {
        constexpr float TF_DEFAULT_ARROW_GRAVITY = 0.5f;
        return TF_DEFAULT_ARROW_GRAVITY;
    }

    default: break;
    }

    return 0.0f;
}

const float ProjectileWeaponInfo_t::_GetUpwardVelocityOffset(ProjectileType_t iProjectileType)
{
    constexpr float TF_DEFAULT_UPWARD_VELOCITY_OFFSET = 200.0f;

    switch (iProjectileType)
    {
    case TF_PROJECTILE_JAR:
    case TF_PROJECTILE_JAR_MILK:
    case TF_PROJECTILE_CLEAVER:
    case TF_PROJECTILE_THROWABLE:
    case TF_PROJECTILE_FESTIVE_JAR:
    case TF_PROJECTILE_BREADMONSTER_JARATE:
    case TF_PROJECTILE_BREADMONSTER_MADMILK:
    case TF_PROJECTILE_PIPEBOMB:
    case TF_PROJECTILE_PIPEBOMB_REMOTE:
    case TF_PROJECTILE_PIPEBOMB_PRACTICE:
    case TF_PROJECTILE_CANNONBALL:
        return TF_DEFAULT_UPWARD_VELOCITY_OFFSET;
    default:
        return 0.0f;
    }

    return 0.0f;
}

const vec ProjectileWeaponInfo_t::_GetHullSize(ProjectileType_t iProjectileType)
{
    switch (iProjectileType)
    {
    case TF_PROJECTILE_JAR:
    case TF_PROJECTILE_JAR_MILK:
    case TF_PROJECTILE_CLEAVER:
    case TF_PROJECTILE_THROWABLE:
    case TF_PROJECTILE_FESTIVE_JAR:
    case TF_PROJECTILE_BREADMONSTER_JARATE:
    case TF_PROJECTILE_BREADMONSTER_MADMILK:
    case TF_PROJECTILE_PIPEBOMB:
    case TF_PROJECTILE_PIPEBOMB_REMOTE:
    case TF_PROJECTILE_PIPEBOMB_PRACTICE:
    case TF_PROJECTILE_CANNONBALL:
        return vec(8.0f, 8.0f, 8.0f);
    default:
        return vec(0.0f, 0.0f, 0.0f);
    }

    return vec(0.0f, 0.0f, 0.0f);
}