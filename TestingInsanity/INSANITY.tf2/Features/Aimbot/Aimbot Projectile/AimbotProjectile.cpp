#include "AimbotProjectile.h"

// SDK
#include "../../../SDK/class/BaseEntity.h"
#include "../../../SDK/class/BaseWeapon.h"
#include "../../../SDK/class/ClientClass.h"
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
#include "../../Graphics Engine/Graphics Engine/GraphicsEngine.h"

#include "../../../Utility/Insane Profiler/InsaneProfiler.h"
#include "../../../Utility/ClassIDHandler/ClassIDHandler.h"
#include "../AimbotHelper.h"
#include "../../../Extra/math.h"


/*
TODO : 
    -> Fix visibility check.
    -> Fix mask, getting stopped @ walkway invisible walls.
    -> Fix initial pos for time to reach.
    -> 
*/


//=========================================================================
//                     PUBLIC METHODS
//=========================================================================
void AimbotProjectile_t::Run(BaseEntity* pLocalPlayer, baseWeapon* pActiveWeapon, CUserCmd* pCmd, bool* pCreatemoveResult)
{
    PROFILE_FUNCTION("ProjAimbot::Run");

    // Constructing LUT ( only for pipes )
    if(pActiveWeapon->getSlot() == WPN_SLOT_PRIMARY && pLocalPlayer->m_iClass() == TF_DEMOMAN)
    {
        m_lutTrajactory.Initialize(pLocalPlayer, pActiveWeapon);
    }

    // Return if disabled.
    if (Features::Aimbot::Aimbot_Projectile::ProjAimbot_Enable.IsActive() == false)
        return;

    // Initialize CVars.
    _InitliazeCVars();

    // Get Projectile info ( like gravity, speed n stuff )
    auto& projInfo = F::projectileEngine.SetupProjectile(pActiveWeapon, pLocalPlayer, pCmd->viewangles);

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

        GraphicInfo_t graphicInfo(
            Features::Aimbot::Visuals::CLR1.GetData().GetAsBytes(),
            Features::Aimbot::Visuals::CLR2.GetData().GetAsBytes(),
            Features::Aimbot::Visuals::CLR3.GetData().GetAsBytes(),
            Features::Aimbot::Visuals::CLR4.GetData().GetAsBytes(),
            Features::Aimbot::Visuals::Thickness.GetData().m_flVal,
            Features::Aimbot::Visuals::Speed.GetData().m_flVal,
            Features::Aimbot::Visuals::GlowPower.GetData().m_flVal
        );

        // Draiwng
        _DrawProjectilePath(pLocalPlayer, pActiveWeapon, pCmd->viewangles, &graphicInfo);
        _DrawTargetFuturePos(&graphicInfo, pCmd->viewangles);
        _DrawTargetPath(pCmd->viewangles, &graphicInfo);

        _ResetTargetData();
    }
}


bool AimbotProjectile_t::_SolveProjectileMotion(BaseEntity* pLocalPlayer, baseWeapon* pActiveWeapon, ProjectileInfo_t& projInfo, const vec& vTarget, float& flAngleOut, float& flTimeToReachOut)
{
    PROFILE_FUNCTION("Proj::Solver");

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
    if(projInfo.m_bUsesDrag == false)
    {
        flAngleOut = flQuadraticAngles;
        flTimeToReachOut = flQuadraticTimeToReach;
        return true;
    }

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
        F::projectileEngine.Initialize(pLocalPlayer, pActiveWeapon, qSimAngle);

        float flLastDistance = 1000000.0f;
        vec   vEnd;

        vec   vLastPos; // This is for drawing
        for(int iTick = 0; iTick < TIME_TO_TICK(flProjLife); iTick++)
        {
            F::projectileEngine.RunTick(false, nullptr);

            vec   vOrigin = F::projectileEngine.GetPos();
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
    m_vFutureFootPos.Init();
}


//=========================================================================
//                     PRIVATE METHODS
//=========================================================================


void AimbotProjectile_t::_DrawTargetFuturePos(GraphicInfo_t* pGraphicInfo, const qangle& qNormal)
{
    auto* pDrawObj = F::graphicsEngine.DrawBox(
        "Target",
        m_vFutureFootPos + m_pBestTarget->GetCollideable()->OBBMins(),
        m_vFutureFootPos + m_pBestTarget->GetCollideable()->OBBMaxs(),
        qNormal, 2000.0f, pGraphicInfo);
}


void AimbotProjectile_t::_DrawTargetPath(const qangle& qNormal, GraphicInfo_t* pGraphicInfo)
{
    uint32_t nRecords = m_vecTargetPathRecord.size();
    if (nRecords <= 0)
        return;

    for (uint32_t iRecordIndex = 1; iRecordIndex < nRecords; iRecordIndex++)
    {
        F::graphicsEngine.DrawLine(std::format("PathRecord_{}", iRecordIndex),
            m_vecTargetPathRecord[iRecordIndex - 1],
            m_vecTargetPathRecord[iRecordIndex],
            qNormal, 2000.0f, pGraphicInfo);
    }
}


void AimbotProjectile_t::_DrawProjectilePath(BaseEntity* pLocalPlayer, baseWeapon* pActiveWeapon, const qangle& qProjLauchAngles, GraphicInfo_t* pGraphicInfo) const
{
    F::projectileEngine.Initialize(pLocalPlayer, pActiveWeapon, qProjLauchAngles);
    
    GraphicInfo_t graphicInfo1(
        Features::Aimbot::Visuals::CLR2.GetData().GetAsBytes(),
        Features::Aimbot::Visuals::CLR1.GetData().GetAsBytes(),
        Features::Aimbot::Visuals::CLR3.GetData().GetAsBytes(),
        Features::Aimbot::Visuals::CLR4.GetData().GetAsBytes(),
        pGraphicInfo->m_flThickness, pGraphicInfo->m_flSpeed, pGraphicInfo->m_flGlowPower
    );

    vec vLastProjPos(0.0f);
    for (int iTick = 0; iTick < TIME_TO_TICK(3.0f); iTick++)
    {
        F::projectileEngine.RunTick(true, pLocalPlayer);
        const vec vProjPos = F::projectileEngine.GetPos();

        if (vLastProjPos.IsEmpty() == false)
        {
            F::graphicsEngine.DrawLine(std::format("ProjAimbot_{}", iTick), vLastProjPos, vProjPos, qProjLauchAngles, 2000.0f + (static_cast<float>(iTick) * 50.0f), iTick % 2 == 0 ? pGraphicInfo : &graphicInfo1);
        }
        vLastProjPos = vProjPos;

        // projectile hit Target or some obsticle, so we end drawing here
        if (F::projectileEngine.m_projInfo.m_vEnd.isEmpty() == false ||
            vProjPos.DistTo(m_vBestTargetFuturePos) < 50.0f)
            return;
    }
}


BaseEntity* AimbotProjectile_t::_GetBestTarget(ProjectileInfo_t& projInfo, BaseEntity* pAttacker, baseWeapon* pWeapon, CUserCmd* pCmd)
{
    PROFILE_FUNCTION();

    std::vector<BaseEntity*> vecTargets = F::aimbotHelper.GetAimbotTargetData().m_vecEnemyPlayers;

    float flAngBestDistance = std::numeric_limits<float>::infinity();
    BaseEntity* pBestTarget = nullptr;

    // Our Eyepos 
    vec vAttackerEyePos = pAttacker->GetEyePos();

    // Projectile's Gravity, Speed & origin
    float flProjGravity = projInfo.m_flGravityMult * m_flGravity * -1.0f;

    uint32_t nTickToSimulate = TIME_TO_TICK(Features::Aimbot::Aimbot_Projectile::ProjAimbot_MaxSimulationTime.GetData().m_flVal);

    std::vector<vec> vecPlayerPathRecord(nTickToSimulate);

    // Looping though all targets and finding the most "killable"
    for (BaseEntity* pTarget : vecTargets)
    {
        vecPlayerPathRecord.clear();
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
        F::movementSimulation.Initialize(pTarget);
        for (int iTick = 0; iTick < nTickToSimulate; iTick++)
        {
            // I absolutely didn't forgot this
            F::movementSimulation.RunTick();

            float flTargetsTimeToReach          = TICK_TO_TIME(iTick);
            float flProjectilesTimeToReach      = 0.0f;
            float flProjLaunchAngle             = 0.0f;
            vec   vFuturePos                    = F::movementSimulation.GetSimulationPos();
            
            vecPlayerPathRecord.push_back(vFuturePos);
            
            bool  bCanReach = false;

            // Calculaing our projectile's time to reach to the center of the target
            if(pWeapon->getSlot() == WPN_SLOT_PRIMARY && pAttacker->m_iClass() == TF_DEMOMAN && projInfo.m_bUsesDrag == true)
            {
                flProjectilesTimeToReach = m_lutTrajactory.GetTravelTime(projInfo.m_vStart.Dist2Dto(vFuturePos), vFuturePos.z - projInfo.m_vStart.z, true);
                bCanReach = true;
            }
            else
            {
                bCanReach = _SolveProjectileMotion(
                    pAttacker, pWeapon, projInfo, vFuturePos, flProjLaunchAngle, flProjectilesTimeToReach
                );
            }

            // Exit on the first tick when projectile can reach target faster
            if (bCanReach == true && flProjectilesTimeToReach < flTargetsTimeToReach)
            {
                //LOG("Using time  ->  %.2f", flProjectilesTimeToReach);

                //m_lutTrajactory.DrawClosestPointAvialable(projInfo.m_vStart, vFuturePos);
                vTargetFuturePos = vFuturePos;
                break;
            }
        }
        F::movementSimulation.Restore();

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
            m_vFutureFootPos       = vTargetFuturePos;
            
            m_vecTargetPathRecord = std::move(vecPlayerPathRecord);
            vecPlayerPathRecord.clear();
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


bool AimbotProjectile_t::_ShouldAim(BaseEntity* pLocalPlayer, baseWeapon* pActiveWeapon, CUserCmd* pCmd, ProjectileInfo_t& projInfo)
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
    ProjectileInfo_t& projInfo, vec& vBestPointOut,
    const vec& vProjectileOrigin, const float flProjVelocity,
    const float flProjGravity, BaseEntity* pProjectileOwner, baseWeapon* pWeapon)
{
    PROFILE_FUNCTION();

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
        F::projectileEngine.Initialize(
            pProjectileOwner, pWeapon, qProjLaunchAngles
        );

        // Simulating projectile & check if we can reach this hitpoint or not
        constexpr vec vMultiPointBoxMins(3.0f, 3.0f, 3.0f);
        bool bCanReach = true;
        for (int iTick = 0; iTick < nTicksToSimulate; iTick++)
        {
            F::projectileEngine.RunTick(true, pProjectileOwner);

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


void AimbotProjectile_t::DeleteProjLUT()
{
    m_lutTrajactory.Delete();
}


const qangle AimbotProjectile_t::_GetTargetAngles(ProjectileInfo_t& projInfo, BaseEntity* pAttacker, baseWeapon* pWeapon, const qangle& qViewAngles)
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


//=========================================================================
//                     TRAJECTORY LOOK UP TABLE HANDLER
//=========================================================================

/*
    Max Height : 742.20 | Min Height : -4514.32 | Max Range : 2336.83
    Max Height : 742.20 | Min Height : -4514.32 | Max Range : 2336.83
    Max Height : 742.20 | Min Height : -4514.32 | Max Range : 2336.83
    Max Height : 742.20 | Min Height : -4514.32 | Max Range : 2336.83
    Max Height : 742.20 | Min Height : -4514.32 | Max Range : 2336.83
    Max Height : 742.20 | Min Height : -4514.32 | Max Range : 2336.83
    Max Height : 742.20 | Min Height : -4514.32 | Max Range : 2336.83
    Max Height : 742.20 | Min Height : -4514.32 | Max Range : 2336.83
    Max Height : 742.20 | Min Height : -4514.32 | Max Range : 2336.83
    Max Height : 742.20 | Min Height : -4514.32 | Max Range : 2336.83
    Max Height : 742.20 | Min Height : -4514.32 | Max Range : 2336.83
    Max Height : 742.20 | Min Height : -4514.32 | Max Range : 2336.83

    Max Height : 742.20 | Min Height : -4514.32 | Max Range : 2336.83
    Total coverable Y = 5256
    Total coverable X = 2336
*/

/*
TODO : 
    -> Simulate & estimate the max & min height and Max Range.
    -> allocate memory to LUT according to that & Store that imfo.
    -> fill up using that memory.
    -> decide whether to make a operator or fn to look up.
    -> test it.
*/

void TrajactoryLUT_t::Initialize(BaseEntity* pLocalPlayer, baseWeapon* pActiveWeapon)
{
    if (pActiveWeapon->GetWeaponDefinitionID() != m_iWpnDefID || m_pTrajactoryLUT == nullptr)
    {
        _AllocToLUT(pLocalPlayer, pActiveWeapon);
        
        // setting this to false forces refilling of LUT.
        // and since we reallocated memory and set it to 0. we want refil.
        m_bFilledTrajactoryLUT = false;
    }

    if (m_bFilledTrajactoryLUT == false)
    {
        _Fill(pLocalPlayer, pActiveWeapon);
    }

    m_iWpnDefID = pActiveWeapon->GetWeaponDefinitionID();
}


void TrajactoryLUT_t::_Fill(BaseEntity* pLocalPlayer, baseWeapon* pActiveWeapon)
{
    if (m_bFilledTrajactoryLUT == true)
        return;

    float flInitialSimAngle = m_flSimAngle;
    //for(float flSimAngle = -89.0f; m_flSimAngle < flInitialSimAngle + 10.0f; m_flSimAngle += 1.0f)
    for(float flSimAngle = -89.0f; flSimAngle <= 89.0f; flSimAngle += 1.0f)
    {
        F::projectileEngine.Initialize(pLocalPlayer, pActiveWeapon, qangle(flSimAngle, 0.0f, 0.0f));

        vec vProjOrigin = F::projectileEngine.m_projInfo.m_vStart;

        for (int iTick = 0; iTick < TIME_TO_TICK(3.0f); iTick++)
        {
            F::projectileEngine.RunTick(false, nullptr);

            // get pos relative to origin & store best time to reach.
            vec   vProjPos      = F::projectileEngine.GetPos();
            float x             = vProjPos.Dist2Dto(vProjOrigin);
            float y             = vProjPos.z - vProjOrigin.z;

            float flTimeToReach = TICK_TO_TIME(iTick);

            // Storing x, y & time to reach.
            _Set(x, y, flTimeToReach, flSimAngle);
        }
    }

    // if we reached from MinPitch to MaxPitch then we have filled up our Lookuptable.
    /*if (m_flSimAngle >= MAX_PITCH - 1.0f)
    {
        m_bFilledTrajactoryLUT = true;
    }*/

    m_bFilledTrajactoryLUT = true;
}


float TrajactoryLUT_t::_SafeGetter(uint32_t iRow, uint32_t iCol)
{
    // out of range check
    if (iRow * m_nLUTCols + iCol > m_nLUTRows * m_nLUTRows)
    {
        return -1.0f;
    }

    return m_pTrajactoryLUT[iRow * m_nLUTCols + iCol];
}


void TrajactoryLUT_t::_Set(float x, float y, float flTimeToReach, float flAngle = 0.0f)
{
    // clamping just in case. My code is very prone to bullshit cause me head crooked :)
    int32_t iRange = static_cast<int32_t>(std::clamp(x, 0.0f, m_flMaxRange));
    int32_t iHeight = static_cast<int32_t>(std::clamp(y, m_flMinHeight, m_flMaxHeight));

    int32_t iRow = static_cast<int32_t>(m_iLUTRowForZeroY) - (iHeight / static_cast<int32_t>(m_iStepSize)); // NOTE : here we are going up from y = 0 on the LUT, since the first row store data for the greatest Y.
    int32_t iCol = iRange / static_cast<int32_t>(m_iStepSize);

    if (_SafeGetter(iRow, iCol) < 0.0f)
    {
        FAIL_LOG("Trying to store at index [ %d ][ %d ], while the LUT is [ %d ][ %d ]", iRow, iCol, m_nLUTRows, m_nLUTCols);
        return;
    }

    float& flTargetSlot = m_pTrajactoryLUT[(iRow * m_nLUTCols) + iCol];
    
    // Only store this Time_To_Reach if this slot if uninitialized or this Time_To_Reach is faster.
    if (flTargetSlot == 0.0f || flTimeToReach < flTargetSlot)
    {
        flTargetSlot = flTimeToReach;
    }
}


float TrajactoryLUT_t::GetTravelTime(const float x, const float y, bool bInterpolation)
{
    PROFILE_FUNCTION("GetTravelTime");

    if(m_pTrajactoryLUT == nullptr)
    {
        return std::numeric_limits<float>::infinity();
    }

    //int32_t iRange = static_cast<int32_t>(std::clamp(x, 0.0f, m_flMaxRange));
    //int32_t iHeight = static_cast<int32_t>(std::clamp(y, m_flMinHeight, m_flMaxHeight));

    // make sure you cast the fucking unsigned ints to signed ints else bullshit will occur. Not like It happened to me.
    int32_t iRow = static_cast<int32_t>(m_iLUTRowForZeroY) - (y / static_cast<int32_t>(m_iStepSize)); // NOTE : here we are going up from y = 0 on the LUT, since the first row store data for the greatest Y.
    int32_t iCol = x / static_cast<int32_t>(m_iStepSize);

    float flBaseTimeToReach = _SafeGetter(iRow, iCol);

    // don't want interpolation or we can't reach that position ( < 0 )
    if (bInterpolation == false || flBaseTimeToReach <= 0.0f)
    {
        return flBaseTimeToReach;
    }

    // INTERPOLATING the value to get a more accurate travel time.
    {
        float flClosestX = static_cast<float>(iCol * m_iStepSize);
        float flClosestY = (static_cast<int32_t>(m_iLUTRowForZeroY) - static_cast<int32_t>(iRow)) * static_cast<int32_t>(m_iStepSize);

        float flDeltaX = x - flClosestX;
        float flDeltaY = y - flClosestY;
        float flDeltaMagnitude = sqrtf(flDeltaX * flDeltaX + flDeltaY * flDeltaY);

        // Checking if interpolation is possible or not. ( we need one heigher & one adjacent value to interpolate
        // Do we have the data to interpolate this projectile going up?
        if (flDeltaY > 0.0f)
        {
            if (m_nLUTRows < (iRow + 1) + 1 || _SafeGetter(iRow - 1, iCol) < 0.01f)
                return flBaseTimeToReach;
        }
        else // do we have data to interpolate this projectile going down?
        {
            if (m_nLUTRows < (iRow + 1) + 1 || _SafeGetter(iRow + 1, iCol) < 0.01f)
                return flBaseTimeToReach;
        }

        // Do we have data for 40 ( step size ) units further ?
        if (m_nLUTCols < (iCol + 1) + 1 || _SafeGetter(iRow, iCol + 1) < 0.01f)
        {
            return flBaseTimeToReach;
        }

        float flThetaInRad = atan2f(flDeltaY, flDeltaX);

        // if projectile going up ( delta > 0 ) get time to reach 40 units above else below.
        float flSuccessorTimeY  = flDeltaY > 0.0f ? _SafeGetter(iRow - 1, iCol) : _SafeGetter(iRow + 1, iCol);
        float flSuccessorTimeX  = _SafeGetter(iRow, iCol + 1);

        float flDeltaTimeX      = flSuccessorTimeX - flBaseTimeToReach;
        float flDeltaTimeY      = flSuccessorTimeY - flBaseTimeToReach;

        float flInterpOffset    = flDeltaTimeX * cosf(flThetaInRad) + flDeltaTimeY * sinf(flThetaInRad);

        // Delete this
        //printf("Base Time to reach was [ %.2f ] inter offset [ %.2f ] | FINAL TIME TO REACH : %.2f\n", flBaseTimeToReach, flInterpOffset, flBaseTimeToReach + flInterpOffset);

        return flBaseTimeToReach + flInterpOffset;
    }

}


void TrajactoryLUT_t::DrawClosestPointAvialable(const vec& vOrigin, const vec& vTarget)
{
    float x = vTarget.Dist2Dto(vOrigin);
    float y = vTarget.z - vOrigin.z;

    uint32_t iRow = static_cast<int32_t>(m_iLUTRowForZeroY) - (y / static_cast<int32_t>(m_iStepSize));
    uint32_t iCol = x / static_cast<int32_t>(m_iStepSize);

    float flClosestX = static_cast<float>(iCol * m_iStepSize);
    float flClosestY = (static_cast<int32_t>(m_iLUTRowForZeroY) - static_cast<int32_t>(iRow)) * static_cast<int32_t>(m_iStepSize);

    float flTimeToReach = _SafeGetter(iRow, iCol);
    if (flTimeToReach < 0.0f)
    {
        FAIL_LOG("Can't reach this place");
        return;
    }

    vec vAttackerToTarget = vTarget - vOrigin;
    vAttackerToTarget.NormalizeInPlace();

    float flMag       = sqrtf(flClosestX * flClosestX + flClosestY * flClosestY);
    vec   vClosestPos = vOrigin + (vAttackerToTarget * flMag);

    I::IDebugOverlay->AddBoxOverlay(vOrigin + (vAttackerToTarget * flMag), vec(-5.0f), vec(5.0f), qangle(0.0f), 0, 0, 255, 255, 1.0f);

    printf("Drew box @ [ %.2f ] away from target\n", vTarget.DistTo(vClosestPos));
}


void TrajactoryLUT_t::_GetMaxRange(BaseEntity* pLocalPlayer, baseWeapon* pActiveWeapon, float& flMaxHeightOut, float& flMinHeightOut, float& flMaxRangeOut)
{
    // Roughly calculating angle for max height ( -89 won't do due to the extra upward velocity thats been added to drag weapons )
    const auto& projInfo   = F::projectileEngine.SetupProjectile(pActiveWeapon, pLocalPlayer, qangle(0.0f));
    float flMaxHeightAngle = 89.0f - RAD2DEG(atan2f(projInfo.m_flUpwardVelOffset, projInfo.m_flSpeed));

    // MAX HEIGHT
    F::projectileEngine.Initialize(pLocalPlayer, pActiveWeapon, qangle(-flMaxHeightAngle, 0.0f, 0.0f));
    
    vec   vStart       = F::projectileEngine.m_projInfo.m_vStart.z;
    float flLastHeight = 0.0f;

    for (int iTick = 0; iTick < TIME_TO_TICK(3.0f); iTick++)
    {
        F::projectileEngine.RunTick(false, nullptr);

        // Height relative to the projectile origin.
        float flHeight      = F::projectileEngine.GetPos().z - F::projectileEngine.m_projInfo.m_vStart.z;
        float flDeltaHeight = flHeight - flLastHeight;
        
        // if delta is less than 0, the projectile is now going down. store & exit!
        if (flDeltaHeight < 0.0f)
        {
            flMaxHeightOut = flHeight;
            break;
        }

        flLastHeight = flHeight;
    }


    // MIN - HEIGHT
    F::projectileEngine.Initialize(pLocalPlayer, pActiveWeapon, qangle(89.0f, 0.0f, 0.0f));
    for (int iTick = 0; iTick < TIME_TO_TICK(3.0f); iTick++)
    {
        F::projectileEngine.RunTick(false, nullptr);
    }
    // Min Height will be negative. ( something like -4000 )
    flMinHeightOut = F::projectileEngine.GetPos().z - F::projectileEngine.m_projInfo.m_vStart.z; 
    
    
    // MAX - RANGE
    // max range is at launch angle of 45 but due to drag its should be a little lower than 45.
    // From my experiments, -1 degree ( or 1 degree in source engine terms ) will give us the maximum range.
    /*1851.17 @ 35  | 1973.99 @ 30 | 2078.95 @ 25 | 2336.26 @ -1*/
    F::projectileEngine.Initialize(pLocalPlayer, pActiveWeapon, qangle(1.0f, 0.0f, 0.0f)); 
    for (int iTick = 0; iTick < TIME_TO_TICK(3.0f); iTick++)
    {
        F::projectileEngine.RunTick(false, nullptr);
    }
    // Min Height will be in negative. ( something like -4000 )
    flMaxRangeOut = F::projectileEngine.GetPos().Dist2Dto(F::projectileEngine.m_projInfo.m_vStart);
}


void TrajactoryLUT_t::_AllocToLUT(BaseEntity* pLocalPlayer, baseWeapon* pActiveWeapon)
{
    _GetMaxRange(pLocalPlayer, pActiveWeapon, m_flMaxHeight, m_flMinHeight, m_flMaxRange);

    uint32_t nRow = static_cast<uint32_t>(fabs(m_flMaxHeight) + fabs(m_flMinHeight) + 1) / m_iStepSize; // NOTE : this +1 is for row representing y = 0. to make our calc easier
    uint32_t nCol = (static_cast<uint32_t>(fabs(m_flMaxRange)) / m_iStepSize) + 1; // NOTE : since x = 0 is at col 0, we gotta have to do +1 do add space for the extra 0 colum.

    uint32_t iMemRequired = nRow * nCol * sizeof(float);

    // Checking and clamping size required.
    if (iMemRequired > m_iMaxMemInBytes)
    {
        // Cutting down the range so it fits under 50 KiBs.
        float    iColPerByte = static_cast<float>(nCol) / static_cast<float>(iMemRequired); // ex : 0.1 coloum per byte
        
        // converting the extra memory into extra coloums & rounding to nearst bigger integer.
        uint32_t iExtraCols  = std::ceilf((iMemRequired - m_iMaxMemInBytes) * iColPerByte); 
        nCol -= iExtraCols;

        // so I can see if something fucks midway :)
        FAIL_LOG("Look up table is taking [ %u ] Bytes more than threshold of [ %u ] KiB. Cutting max range by [ %u ]. New col count [ %u ]",
            iMemRequired - m_iMaxMemInBytes, m_iMaxMemInBytes / 1024, iExtraCols * m_iStepSize, nCol);
    }

    // We don't want that shit no more
    if (m_pTrajactoryLUT != nullptr)
        free(m_pTrajactoryLUT);

    // NOTE : cols are trimmed & rows are not.
    m_nLUTRows = nRow;
    m_nLUTCols = nCol;
    m_iLUTRowForZeroY = (static_cast<uint32_t>(m_flMaxHeight) / m_iStepSize) /* + 1 */;

    m_pTrajactoryLUT = reinterpret_cast<float*>(malloc(nRow * nCol * sizeof(float)));

    if (m_pTrajactoryLUT == nullptr)
    {
        FAIL_LOG("Failed allocation :(");
        return;
    }

    memset(m_pTrajactoryLUT, 0, nRow * nCol * sizeof(float));
    WIN_LOG("Allocated mem to LUT successfully [ %d x %d ] %d KiB | [ %.2f <-> %.2f] [ %.2f ], Center @ %d:)", 
        nRow, nCol, iMemRequired / 1024, m_flMinHeight, m_flMaxHeight, m_flMaxRange, m_iLUTRowForZeroY);
}


void TrajactoryLUT_t::Delete()
{
    if (m_pTrajactoryLUT != nullptr)
    {
        free(m_pTrajactoryLUT);
        m_pTrajactoryLUT = nullptr;
    }
}
