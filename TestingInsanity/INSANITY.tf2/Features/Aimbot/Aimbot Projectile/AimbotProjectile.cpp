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

constexpr vec vRocketHullMin(-1.0f, -1.0f, -1.0f);
constexpr vec vRocketHullMax( 1.0f,  1.0f,  1.0f);

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

    LOG("Found target");

    // if in attack, hit that nigga here.
    float flCurTime  = TICK_TO_TIME(pLocalPlayer->m_nTickBase());
    bool  bShouldAim = flCurTime >= pActiveWeapon->m_flNextPrimaryAttack() && (pCmd->buttons & IN_ATTACK);
    if (bShouldAim == true)
    {
        _DrawPredictionHistory();
        _ClearPredictoinHistory();

        // visualize tragectory

        pCmd->viewangles = _GetTargetAngles(pLocalPlayer, pCmd->viewangles);
        *pCreatemoveResult = false;
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
}

//=========================================================================
//                     PRIVATE METHODS
//=========================================================================
BaseEntity* AimbotProjectile_t::_GetBestTarget(const ProjectileWeaponInfo_t& weaponInfo, BaseEntity* pAttacker, CUserCmd* pCmd)
{
    std::vector<BaseEntity*> vecTargets = FeatureObj::aimbotHelper.GetAimbotTargetData().m_vecEnemyPlayers;

    float flAngBestDistance = std::numeric_limits<float>::infinity();
    BaseEntity* pBestTarget = nullptr;

    // Projectile's Gravity, Speed & origin
    float flProjGravity     = weaponInfo.GetProjectileGravity(pAttacker);
    float flProjVelocity    = weaponInfo.GetProjectileSpeed(pAttacker);
    vec   vShootPosOffset   = weaponInfo.GetShootPosOffset(pAttacker, m_bFlipViewModels);
    vec   vProjectileOrigin = weaponInfo.GetProjectileOrigin(pAttacker, vShootPosOffset, pCmd->viewangles);

    uint32_t nTickToSimulate = TIME_TO_TICK(Features::Aimbot::Aimbot_Projectile::ProjAimbot_MaxSimulationTime.GetData().m_flVal);
    m_vecTargetPredictionHistory.resize(nTickToSimulate + 1U);

    // Looping though all targets and finding the most "killable"
    for (BaseEntity* pTarget : vecTargets)
    {
        // Checking if in FOV or not
        vec vTargetsInitialPos = pTarget->GetAbsOrigin();
        vec vTargetFuturePos;
        float flAngDistance = _GetAngleFromCrosshair(pAttacker, vTargetsInitialPos);
        if (flAngDistance > Features::Aimbot::Aimbot_Projectile::ProjAimbot_FOV.GetData().m_flVal)
            continue;

        // Simulating Target
        FeatureObj::movementSimulation.Initialize(pTarget);
        for (int iTick = 0; iTick < nTickToSimulate; iTick++)
        {
            // I absolutely didn't forgot this
            FeatureObj::movementSimulation.RunTick();

            float flTargetsTimeToReach          = TICK_TO_TIME(iTick);
            float flProjectilesTimeToReach      = 0.0f;
            float flProjLaunchAngle             = 0.0f;
            m_vecTargetPredictionHistory[iTick] = FeatureObj::movementSimulation.GetSimulationPos();

            // Calculaing our projectile's time to reach to the center of the target
            _SolveProjectileMotion(
                vProjectileOrigin,
                m_vecTargetPredictionHistory[iTick],
                flProjVelocity,
                flProjGravity,
                flProjLaunchAngle,
                flProjectilesTimeToReach);

            // Exit on the first tick when projectile can reach target faster
            if (flProjectilesTimeToReach < flTargetsTimeToReach)
            {
                vTargetFuturePos         = m_vecTargetPredictionHistory[iTick];
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
            pTarget, vTargetFuturePos,
            weaponInfo, vBestHitPoint,
            vProjectileOrigin, flProjVelocity,
            flProjGravity, pAttacker
        );
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

void AimbotProjectile_t::_DrawPredictionHistory()
{
    for (int i = 1; i <= m_nValidPredictionRecord; i++)
    {
        I::IDebugOverlay->AddLineOverlay(m_vecTargetPredictionHistory[i - 1], m_vecTargetPredictionHistory[i], 255, 255, 255, true, 5.0f);
    }
}

bool AimbotProjectile_t::_SolveProjectileMotion(const vec& vLauchPos, const vec& vTargetPos, const float flProjVelocity, const float flGravity, float& flAngleOut, float& flTimeToReach)
{
    float x = vLauchPos.DistTo(vTargetPos);
    float y = vTargetPos.z - vLauchPos.z;

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

    // Choose HitPoint priority list & loop through it simulating each shit
    vec vBestHitpoint;
    for (const vec* vHitPointOffset : *vecHitPointPriorityList)
    {
        vec vTarget = vTargetOrigin + *vHitPointOffset;

        float flProjLaunchAngle = 0.0f;
        float flTimeToReach = 0.0f;
        _SolveProjectileMotion(
            vProjectileOrigin, 
            vTarget,
            flProjVelocity, flProjGravity, 
            flProjLaunchAngle, flTimeToReach
        );

        // Calculaing aimbot view angles
        qangle qProjLaunchAngles;
        Maths::VectorAnglesFromSDK(vTarget - vProjectileOrigin, qProjLaunchAngles);
        qProjLaunchAngles.pitch = flProjLaunchAngle;

        uint32_t nTicksToSimulate = TIME_TO_TICK(flTimeToReach);
        FeatureObj::projectileSimulator.Initialize(
            vProjectileOrigin, qProjLaunchAngles, flProjVelocity,
            flProjGravity, weaponInfo.m_flUpwardVelOffset,
            weaponInfo.m_vHullSize, pProjectileOwner
        );

        for (int iTick = 0; iTick < nTicksToSimulate; iTick++)
            FeatureObj::projectileSimulator.RunTick();

        constexpr vec vMultiPointBoxMins(3.0f, 3.0f, 3.0f);

        // we can hit this hitpoint.
        constexpr float flProjectileHitTolerance = 3.0f;
        if (FeatureObj::projectileSimulator.m_projectileInfo.m_bDidHit == false ||
            FeatureObj::projectileSimulator.m_projectileInfo.m_vEndPos.DistTo(vTarget) < flProjectileHitTolerance)
        {
            vBestHitpoint = vTarget;

            if (Features::Aimbot::Aimbot_Projectile::ProjAimbot_DebugMultiPoint.IsActive() == true)
                I::IDebugOverlay->AddBoxOverlay(vTarget, vMultiPointBoxMins * -1.0f, vMultiPointBoxMins, qangle(0.0f, 0.0f, 0.0f), 0, 255, 0, 40, 1.0f);

            break;
        }

        if (Features::Aimbot::Aimbot_Projectile::ProjAimbot_DebugMultiPoint.IsActive() == true)
            I::IDebugOverlay->AddBoxOverlay(vTarget, vMultiPointBoxMins * -1.0f, vMultiPointBoxMins, qangle(0.0f, 0.0f, 0.0f), 255, 255, 255, 40, 1.0f);
    }

    // if not empty then return true
    vBestPointOut = vBestHitpoint;
    LOG_VEC3(vBestPointOut);
    return (vBestPointOut.IsEmpty() == true ? false : true);
}


const qangle AimbotProjectile_t::_GetTargetAngles(BaseEntity* pAttacker, const qangle& qViewAngles)
{
    vec vShootOffset = m_weaponInfo.GetShootPosOffset(pAttacker, m_bFlipViewModels);
    vec vProjectileOrigin = m_weaponInfo.GetProjectileOrigin(pAttacker, vShootOffset, qViewAngles);
    float flProjVel = m_weaponInfo.GetProjectileSpeed(pAttacker);
    float flProjGravity = m_weaponInfo.GetProjectileGravity(pAttacker);

    qangle qBestAngles;
    Maths::VectorAnglesFromSDK(m_vBestTargetFuturePos - vProjectileOrigin, qBestAngles);
    
    float flProjLaunchAngle = 0.0f;
    float flTimeToReach = 0.0f;
    _SolveProjectileMotion(
        vProjectileOrigin, m_vBestTargetFuturePos,
        flProjVel, flProjGravity,
        flProjLaunchAngle,
        flTimeToReach
    );
    qBestAngles.pitch = flProjLaunchAngle * -1.0f; // Gotta invert the pitch, cause valve is a nigger

    FeatureObj::projectileSimulator.Initialize(
        vProjectileOrigin, qBestAngles,
        flProjVel, flProjGravity, m_weaponInfo.m_flUpwardVelOffset,
        m_weaponInfo.m_vHullSize, pAttacker
    );
    for (int i = 0; i < TIME_TO_TICK(flTimeToReach); i++)
        FeatureObj::projectileSimulator.RunTick(true, 5.0f);

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
    FAIL_LOG("Weapon change detected, Refreshing weapon's stats");
    LOG("WPN info updated to the following : ");
    printf("Base Projectile Speed : %.2f\n", m_flProjectileBaseSpeed);
    printf("Base Gravity mult     : %.2f\n", m_flProjectileBaseGravityMult);
    printf("Upward velocty offset : %.2f\n", m_flUpwardVelOffset);
    LOG_VEC3(m_vShootPosOffset);
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
        float flCharge = Maths::MIN<float>(flCurTime - m_pWeapon->m_flChargeBeginTime(), 1.0f);
        return Maths::RemapValClamped(flCharge, 0.0f, 1.f, 1800, 2600);
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


float ProjectileWeaponInfo_t::GetProjectileGravity(BaseEntity* pWeaponOwner) const
{
    // Only arrow's gravity is influenced by charging.
    switch (m_pWeaponFileInfo->m_iProjectile)
    {
    case TF_PROJECTILE_ARROW:
    case TF_PROJECTILE_FESTIVE_ARROW:
    {
        float flCurTime = TICK_TO_TIME(pWeaponOwner->m_nTickBase());
        float flCharge  = Maths::MIN<float>(flCurTime - m_pWeapon->m_flChargeBeginTime(), 1.0f);
        return Maths::RemapValClamped(flCharge, 0.0f, 1.f, 0.5, 0.1);
    }
    default:
        return m_flProjectileBaseGravityMult;
    }

    return m_flProjectileBaseGravityMult;
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