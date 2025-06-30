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
#include "../../Projectile Engine/ProjectileEngine.h"
#include "../../../SDK/TF object manager/TFOjectManager.h"

#include "../AimbotHelper.h"
#include "../../../Extra/math.h"

//=========================================================================
//                     PUBLIC METHODS
//=========================================================================
void AimbotProjectile_t::Run(BaseEntity* pLocalPlayer, baseWeapon* pActiveWeapon, CUserCmd* pCmd, bool* pCreatemoveResult)
{
    // Return if disabled.
    if (Features::Aimbot::Aimbot_Projectile::ProjAimbot_Enable.IsActive() == false)
        return;

    // Initialize CVars.
    _InitliazeCVars();

    // Get Projectile info ( like gravity, speed n stuff )
    auto& projInfo = FeatureObj::projectileEngine.SetupProjectile(pActiveWeapon, pLocalPlayer, pCmd->viewangles);

    // Scanning for target
    if (SDK::CanAttack(pLocalPlayer, pActiveWeapon, pCmd) == true)
    {
        m_pBestTarget = _GetBestTarget(projInfo, pLocalPlayer, pActiveWeapon, pCmd);
    }

    // no target found
    if (m_pBestTarget == nullptr)
        return;

    // Aim at calculated position
    if (_ShouldAim(pLocalPlayer, pActiveWeapon, pCmd, projInfo) == true)
    {
        // Aimbot-ing.
        pCmd->viewangles    = _GetTargetAngles(projInfo, pLocalPlayer, pActiveWeapon, pCmd->viewangles);
        *pCreatemoveResult  = false;

        _ResetTargetData();
    }
}


bool AimbotProjectile_t::_SolveProjectileMotion(BaseEntity* pLocalPlayer, baseWeapon* pActiveWeapon, ProjectileInfo_tV2& projInfo, const vec& vTarget, float& flAngleOut, float& flTimeToReachOut)
{
    float flGravity = projInfo.m_flGravityMult * m_flGravity * -1.0f;

    float x = projInfo.m_vOrigin.Dist2Dto(vTarget);
    float y = vTarget.z - projInfo.m_vOrigin.z;

    if (projInfo.m_flGravityMult == 0.0f)
    {
        flAngleOut       = RAD2DEG(atanf(y / x));
        flTimeToReachOut = projInfo.m_vStart.DistTo(vTarget) / projInfo.m_flSpeed;
        return true;
    }

    // Quadratic shit here
    float flSpeed        = sqrtf((projInfo.m_flSpeed * projInfo.m_flSpeed) + (projInfo.m_flUpwardVelOffset * projInfo.m_flUpwardVelOffset));
    float flDiscriminant = powf(flSpeed, 4.0f) - flGravity * (((x * x) * flGravity) - (2.0f * flSpeed * flSpeed * y));

    // if even quadratic can't reach this point
    if (flDiscriminant < 0.0f)
    {
        //I::IDebugOverlay->AddTextOverlayRGB(vTarget, 0, 1.0f, 255, 0, 0, 255, "Can't reach this point");
        return false;
    }
    else // Drawing target
    {
        //I::IDebugOverlay->AddBoxOverlay(vTarget, vec(4.0f), vec(-4.0f), qangle(0.0f), 255, 0, 0, 20, 1.0f);
    }

    // Getting best angles & speed
    float flSolution1       = (-(flSpeed * flSpeed) + sqrtf(flDiscriminant)) / (x * flGravity);
    float flSolution2       = (-(flSpeed * flSpeed) - sqrtf(flDiscriminant)) / (x * flGravity);

    float flAngle1InDeg     = RAD2DEG(atanf(flSolution1));
    float flAngle2InDeg     = RAD2DEG(atanf(flSolution2));

    float flTimeToReach1    = x / (flSpeed * cos(atanf(flSolution1)));
    float flTimeToReach2    = x / (flSpeed * cos(atanf(flSolution2)));

    float flQuadraticAngles = flTimeToReach1 < flTimeToReach2 ? flAngle1InDeg : flAngle2InDeg;
    float flQuadraticTimeToReach = flTimeToReach1 < flTimeToReach2 ? flTimeToReach1 : flTimeToReach2;

    //printf("Simple quadratic says we can reach with angle [ %.2f ] & time [ %.2f ]\n", flBestAngle, flBestTimeToReach);

    float flBestAngle = 0.0f;
    float flBestTimeToReach = 0.0f;

    constexpr float flProjLife   = 3.0f;
    constexpr float flTolerance  = 20.0f;
    float           flDeltaAngle = 0.0f;
    float           flStepSize   = 5.0f;
    float           flLastEndPosDelta = 0.0f;
    vec             vFirstAttemptHitPos;
    qangle          qSimAngle;
    bool            bDidSucceed = false;
    for (int iAttempt = 0; iAttempt < 6; iAttempt++)
    {
        // Constructing simulation angle
        {
            Maths::VectorAnglesFromSDK(vTarget - projInfo.m_vStart, qSimAngle);
            qSimAngle.pitch = flQuadraticAngles - RAD2DEG(atanf(projInfo.m_flUpwardVelOffset / projInfo.m_flSpeed)) + flDeltaAngle;
            qSimAngle.pitch *= -1.0f;

            //printf("Attempting with angle :  %.2f,  %.2f,  %.2f\n", qSimAngle.pitch, qSimAngle.yaw, qSimAngle.roll);
        }
        FeatureObj::projectileEngine.Initialize(pLocalPlayer, pActiveWeapon, qSimAngle);

        float flLastDistance = 1000000.0f;
        vec   vEnd;

        vec   vLastPos; // This is for drawing
        for(int iTick = 0; iTick < TIME_TO_TICK(flProjLife); iTick++)
        {
            FeatureObj::projectileEngine.RunTick(false);

            vec   vOrigin = FeatureObj::projectileEngine.GetPos();
            float flDist  = vOrigin.DistTo(vTarget);

            // Invalidate this angle if distance is more than last distance.
            if (flDist > flLastDistance || flDist <= flTolerance)
            {
                vEnd              = vOrigin;
                flBestAngle       = qSimAngle.pitch * -1.0f;
                flBestTimeToReach = TICK_TO_TIME(iTick);
                break;
            }

            vLastPos       = vOrigin;
            flLastDistance = flDist;
        }

        // Did projectile even reach that point
        if (vEnd.IsEmpty() == true)
        {
            //FAIL_LOG("Projectile can't reach this place");
            continue;
        }

        // Draw end pos
        bDidSucceed = vEnd.DistTo(vTarget) <= flTolerance;
        //I::IDebugOverlay->AddTextOverlay(vEnd, 1.0f, "[ %d ]", iAttempt + 1);
        //I::IDebugOverlay->AddBoxOverlay(vEnd, vec(4.0f), vec(-4.0f), qangle(0.0f), 0, bDidSucceed == true ? 255 : 0, bDidSucceed == true ? 0 : 255, 255, 1.0f);
        if (bDidSucceed == true)
        {
            break;
        }

        if (iAttempt == 0)
            vFirstAttemptHitPos = vEnd;

        // Adjusting angle
        float flEndPosDelta = vFirstAttemptHitPos.DistTo(vEnd) - vFirstAttemptHitPos.DistTo(vTarget);
        if (iAttempt == 0)
        {
            flLastEndPosDelta = flEndPosDelta;
        }

        bool bDidGoPastTarget = (flLastEndPosDelta < 0.0f) != (flEndPosDelta < 0.0f);
        if (bDidGoPastTarget == true)
        {
            flStepSize *= 0.5f;
            //LOG("We went past the target @ %d attempt, new step size is [ %.2f ]", iAttempt, flStepSize);
        }
        if (flEndPosDelta > 0.0f)
        {
            flDeltaAngle -= flStepSize;
        }
        else
        {
            flDeltaAngle += flStepSize;
        }
        flLastEndPosDelta = flEndPosDelta;

    }

    flAngleOut       = flBestAngle;
    flTimeToReachOut = flBestTimeToReach;
    return bDidSucceed;
}


void AimbotProjectile_t::Reset()
{
    // CVars...
    m_bInitializedCVars = false;
    m_bFlipViewModels   = 0;
    m_flGravity         = 0.0f;

    m_pBestTarget       = nullptr;
    m_vBestTargetFuturePos.Init();
}

//=========================================================================
//                     PRIVATE METHODS
//=========================================================================
BaseEntity* AimbotProjectile_t::_GetBestTarget(ProjectileInfo_tV2& projInfo, BaseEntity* pAttacker, baseWeapon* pWeapon, CUserCmd* pCmd)
{
    std::vector<BaseEntity*> vecTargets = FeatureObj::aimbotHelper.GetAimbotTargetData().m_vecEnemyPlayers;

    float flAngBestDistance = std::numeric_limits<float>::infinity();
    BaseEntity* pBestTarget = nullptr;

    // Our Eyepos 
    vec vAttackerEyePos = pAttacker->GetEyePos();

    // Projectile's Gravity, Speed & origin
    float flProjGravity = projInfo.m_flGravityMult * m_flGravity * -1.0f;

    uint32_t nTickToSimulate = TIME_TO_TICK(Features::Aimbot::Aimbot_Projectile::ProjAimbot_MaxSimulationTime.GetData().m_flVal);

    // Looping though all targets and finding the most "killable"
    for (BaseEntity* pTarget : vecTargets)
    {
        // Checking if in FOV or not
        // TODO : ABS origin ain't a good place to check time against, Take the center of
        //      face in the direction of target's movement. That should be a good enough place.
        vec vTargetsInitialPos = pTarget->GetAbsOrigin();
        vec vTargetFuturePos;
        float flAngDistance = _GetAngleFromCrosshair(vTargetsInitialPos, vAttackerEyePos, pCmd->viewangles);
        if (flAngDistance > Features::Aimbot::Aimbot_Projectile::ProjAimbot_FOV.GetData().m_flVal)
            continue;

        // Constructing Projectile Origin for each entity.
        qangle qAttackerToTarget;
        Maths::VectorAnglesFromSDK(vTargetsInitialPos - vAttackerEyePos, qAttackerToTarget);
        projInfo.SetProjectileAngle(vAttackerEyePos, qAttackerToTarget);

        // Simulating Target
        FeatureObj::movementSimulation.Initialize(pTarget);
        for (int iTick = 0; iTick < nTickToSimulate; iTick++)
        {
            // I absolutely didn't forgot this
            FeatureObj::movementSimulation.RunTick();

            float flTargetsTimeToReach          = TICK_TO_TIME(iTick);
            float flProjectilesTimeToReach      = 0.0f;
            float flProjLaunchAngle             = 0.0f;
            vec   vFuturePos                    = FeatureObj::movementSimulation.GetSimulationPos();

            // Calculaing our projectile's time to reach to the center of the target
            bool bCanReach = _SolveProjectileMotion(
                pAttacker, pWeapon, projInfo, vFuturePos, flProjLaunchAngle, flProjectilesTimeToReach
            );

            // Exit on the first tick when projectile can reach target faster
            if (bCanReach == true && flProjectilesTimeToReach < flTargetsTimeToReach)
            {
                vTargetFuturePos = vFuturePos;
                break;
            }
        }
        FeatureObj::movementSimulation.Restore();

        // if we didn't found any hitable tick for this enitity than skip
        if (vTargetFuturePos.IsEmpty() == true)
            continue;

        // Max distance check for drag weapons
        if (projInfo.m_bUsesDrag == true && projInfo.m_vStart.DistTo(vTargetFuturePos) > Features::Aimbot::Aimbot_Projectile::ProjAimbot_MaxDistance.GetData().m_flVal)
            continue;

        // Getting best point on target's hull to hit
        vec vBestHitPoint;
        bool bCanHit = _GetBestHitPointOnTargetHull(
            pTarget, vTargetFuturePos,
            projInfo, vBestHitPoint,
            projInfo.m_vOrigin, projInfo.m_flSpeed,
            flProjGravity, pAttacker, pWeapon);
        if (bCanHit == false)
            continue;

        // compare against best target
        if (flAngDistance < flAngBestDistance)
        {
            pBestTarget            = pTarget;
            flAngBestDistance      = flAngDistance;
            m_vBestTargetFuturePos = vBestHitPoint;
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


bool AimbotProjectile_t::_ShouldAim(BaseEntity* pLocalPlayer, baseWeapon* pActiveWeapon, CUserCmd* pCmd, ProjectileInfo_tV2& projInfo)
{
    // can we even attack this tick?
    if (SDK::CanAttack(pLocalPlayer, pActiveWeapon, pCmd) == false)
        return false;

    float flCurTime = TICK_TO_TIME(pLocalPlayer->m_nTickBase());
    bool  bShouldAim = flCurTime >= pActiveWeapon->m_flNextPrimaryAttack() && (pCmd->buttons & IN_ATTACK);

    bool bRequiresCharging = 
        projInfo.m_pTFWpnFileInfo->m_iProjectile == TF_PROJECTILE_ARROW ||
        projInfo.m_pTFWpnFileInfo->m_iProjectile == TF_PROJECTILE_FESTIVE_ARROW ||
        projInfo.m_pTFWpnFileInfo->m_iProjectile == TF_PROJECTILE_PIPEBOMB_REMOTE ||
        projInfo.m_pTFWpnFileInfo->m_iProjectile == TF_PROJECTILE_PIPEBOMB_PRACTICE;

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


bool AimbotProjectile_t::_GetBestHitPointOnTargetHull(BaseEntity* pTarget, const vec& vTargetOrigin,
    ProjectileInfo_tV2& projInfo, vec& vBestPointOut,
    const vec& vProjectileOrigin, const float flProjVelocity,
    const float flProjGravity, BaseEntity* pProjectileOwner, baseWeapon* pWeapon)
{
    ICollideable_t* pCollidable = pTarget->GetCollideable();
    const vec& vHullMins = pCollidable->OBBMins();
    const vec& vHullMaxs = pCollidable->OBBMaxs();

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
    if (projInfo.m_pTFWpnFileInfo->m_iProjectile == TF_PROJECTILE_ARROW || 
        projInfo.m_pTFWpnFileInfo->m_iProjectile == TF_PROJECTILE_FESTIVE_ARROW)
    {
        vecHitPointPriorityList = &vecHeadPriorityHitpointList;
    }
    else if (projInfo.m_pTFWpnFileInfo->m_iProjectile == TF_PROJECTILE_ROCKET && (pTarget->m_fFlags() & FL_ONGROUND))
    {
        vecHitPointPriorityList = &vecFeetPriorityHitpointList;
    }

    // Looping & Choosing most hitable HitPoint
    vec vAttackerEyePos = pProjectileOwner->GetEyePos();
    vec vBestHitpoint;
    for (const vec* vHitPointOffset : *vecHitPointPriorityList)
    {
        vec vTargetHitPoint = vTargetOrigin + *vHitPointOffset;

        // Constructing projectile origin for each HitPoint
        qangle qAttackerToHitPoint;
        Maths::VectorAnglesFromSDK(vTargetHitPoint - vAttackerEyePos, qAttackerToHitPoint);
        projInfo.SetProjectileAngle(vAttackerEyePos, qAttackerToHitPoint);

        float flProjLaunchAngle = 0.0f;
        float flTimeToReach     = 0.0f;
        _SolveProjectileMotion(
            pProjectileOwner, pWeapon, projInfo, vTargetHitPoint, flProjLaunchAngle, flTimeToReach
        );

        // Calculaing aimbot view angles
        qangle qProjLaunchAngles;
        Maths::VectorAnglesFromSDK(vTargetHitPoint - projInfo.m_vOrigin, qProjLaunchAngles);
        //qProjLaunchAngles.pitch = flProjLaunchAngle * -1.0f; // setting pitch in valve format ( down -ve & up +ve )

        uint32_t nTicksToSimulate = TIME_TO_TICK(flTimeToReach);
        FeatureObj::projectileEngine.Initialize(
            pProjectileOwner, pWeapon, qProjLaunchAngles
        );

        // Simulating projectile & check if we can reach this hitpoint or not
        constexpr vec vMultiPointBoxMins(3.0f, 3.0f, 3.0f);
        bool bCanReach = true;
        for (int iTick = 0; iTick < nTicksToSimulate; iTick++)
        {
            FeatureObj::projectileEngine.RunTick(true);

            constexpr float flProjHitRangeTolerance = 3.0f;

            // Checking if we hit something mid way
            if (projInfo.m_vEnd.IsEmpty() == false && projInfo.m_vEnd.DistTo(vTargetHitPoint) > flProjHitRangeTolerance)
            {
                bCanReach = false;
                break;
            }
        }

        // exit if hittable hitpoint found
        if (bCanReach == true)
        {
            vBestHitpoint = vTargetHitPoint;
            if (Features::Aimbot::Aimbot_Projectile::ProjAimbot_DebugMultiPoint.IsActive() == true && vBestHitpoint.IsEmpty() == false)
                I::IDebugOverlay->AddBoxOverlay(vBestHitpoint, vMultiPointBoxMins * -1.0f, vMultiPointBoxMins, qangle(0.0f, 0.0f, 0.0f), 0, 255, 0, 40, 1.0f);
            break;
        }

        if (Features::Aimbot::Aimbot_Projectile::ProjAimbot_DebugMultiPoint.IsActive() == true)
            I::IDebugOverlay->AddBoxOverlay(vTargetHitPoint, vMultiPointBoxMins * -1.0f, vMultiPointBoxMins, qangle(0.0f, 0.0f, 0.0f), 255, 255, 255, 40, 1.0f);
    }

    // if not empty then return true
    vBestPointOut = vBestHitpoint;
    return (vBestPointOut.IsEmpty() == false);
}


const qangle AimbotProjectile_t::_GetTargetAngles(ProjectileInfo_tV2& projInfo, BaseEntity* pAttacker, baseWeapon* pWeapon, const qangle& qViewAngles)
{
    // Rotating projectile info so the projectile origin is positioned properly
    qangle qAttackerToTarget;
    Maths::VectorAnglesFromSDK(m_vBestTargetFuturePos - pAttacker->GetEyePos(), qAttackerToTarget);
    projInfo.SetProjectileAngle(pAttacker->GetEyePos(), qAttackerToTarget);

    // Getting angle to hit target
    float flProjLaunchAngle = 0.0f;
    float flTimeToReach     = 0.0f;
    _SolveProjectileMotion(pAttacker, pWeapon, projInfo, m_vBestTargetFuturePos, flProjLaunchAngle, flTimeToReach);
    
    // Constructing aimbot angles
    qangle qBestAngles;
    Maths::VectorAnglesFromSDK(m_vBestTargetFuturePos - projInfo.m_vStart, qBestAngles);
    qBestAngles.pitch = flProjLaunchAngle * -1.0f; // Converting pitch to source engine format

    return qBestAngles;
}



void AimbotProjectile_t::_InitliazeCVars()
{
    if (m_bInitializedCVars == true)
        return;

    // Getting all necessary CVars
    m_bFlipViewModels = I::iCvar->FindVar("cl_flipviewmodels")->GetInt();
    m_flGravity = I::iCvar->FindVar("sv_gravity")->GetFloat();

    WIN_LOG("Initializd CVars");

    // Initialized CVars
    m_bInitializedCVars = true;
}