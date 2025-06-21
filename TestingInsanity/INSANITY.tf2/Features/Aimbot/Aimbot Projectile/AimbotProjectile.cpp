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
    // Fuck off
    if (Features::Aimbot::Aimbot_Projectile::ProjAimbot_Enable.IsActive() == false)
        return;

    _InitliazeCVars();

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
        _GetShootOffset(pLocalPlayer, pActiveWeapon);
        float flProjectileSpeed = _GetProjectileSpeed(pActiveWeapon);

        //vec vFutureAttackerToTarget = _GetBestTickToAim(pLocalPlayer, pActiveWeapon, m_pBestTarget, flProjectileSpeed);
        qangle qBestTargetAngles;
        Maths::VectorAnglesFromSDK(m_vBestTargetFuturePos - pLocalPlayer->GetEyePos(), qBestTargetAngles);

        // Compensating for pitch cause valve has hardcoded some initial velocity offset for projectiles
        if (pActiveWeapon->GetWeaponTypeID() == TF_WEAPON_PIPEBOMBLAUNCHER || pActiveWeapon->GetWeaponTypeID() == TF_WEAPON_GRENADELAUNCHER)
        {
            constexpr float flPipeVelOffsetUp = 200.0f;
            float flPitchOffset = RAD2DEG(atanf(flPipeVelOffsetUp / flProjectileSpeed));
            qBestTargetAngles.pitch = (m_flBestAimbotPitch - flPitchOffset) * -1.0f;

            printf("Pitch Offset : %.2f | New Pitch : %.2f | Projectile Speed : %.2f\n", flPitchOffset, qBestTargetAngles.pitch, flProjectileSpeed);
        }

        pCmd->viewangles = qBestTargetAngles;
        *pCreatemoveResult = false;

        Reset();
    }
}

void AimbotProjectile_t::_InitliazeCVars()
{
    if (m_bInitializedCVars == true)
        return;

    // Getting all necessary CVars
    m_bFlipViewModels = I::iCvar->FindVar("cl_flipviewmodels")->GetInt();

    WIN_LOG("Initializd CVars : flip status -> %d", m_bFlipViewModels);

    // Initialized CVars
    m_bInitializedCVars = true;
}

void AimbotProjectile_t::Reset()
{
    m_bInitializedCVars = false;
    m_bFlipViewModels = 0;

    m_pBestTarget = nullptr;
    m_flBestAimbotPitch = 0.0f;
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

    bool bShouldDoPipePhysics = (pActiveWeapon->GetWeaponTypeID() == TF_WEAPON_GRENADELAUNCHER || pActiveWeapon->GetWeaponTypeID() == TF_WEAPON_PIPEBOMBLAUNCHER);

    for (BaseEntity* pTarget : vecEnemyPlayers)
    {   
        constexpr float flMaxTimeToSimulateInSec = 2.0f;
        uint32_t        nTicksToSimulate         = TIME_TO_TICK(flMaxTimeToSimulateInSec);
        uint32_t        iEntFlags                = 0;
        const vec&      vTargetInitialOrigin     = pTarget->GetAbsOrigin();
        vec             vTargetFutureOrigin;
        float           flBestAimbotPitch        = 0.0f;

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
            float flTimeToReachProjectile = 0.0f;

            if (bShouldDoPipePhysics == false)
            {
                flTimeToReachProjectile = vLocalPlayerEyePos.DistTo(vTargetOrigin) / flProjectileSpeed;
            }
            else
            {
                constexpr float flPipeSpeed = 1200.0f;
                constexpr float flGravity   = -800.0f;
                _SolveProjectileMotion(vLocalPlayerEyePos, vTargetOrigin, flPipeSpeed, flGravity, flBestAimbotPitch, flTimeToReachProjectile);
            }

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
            m_flBestAimbotPitch    = flBestAimbotPitch;
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


const vec AimbotProjectile_t::_GetShootOffset(BaseEntity* pLocalPlayer, baseWeapon* pActiveWeapon) const
{
    vec              vShootPosOffset;
    bool             bDucking        = (pLocalPlayer->m_fFlags() & IN_DUCK);
    ProjectileType_t iProjectileType = pActiveWeapon->GetTFWeaponInfo()->GetWeaponData(0)->m_iProjectile;
    switch (iProjectileType)
    {
    case TF_PROJECTILE_ROCKET:
    {
        vShootPosOffset = { 23.5f, 12.0f, -3.0f };
        
        if (bDucking == true)
            vShootPosOffset.z = 8.0f;

        break;
    }

    /*
    #define SYRINGE_GRAVITY		0.3f
    #define SYRINGE_VELOCITY	1000.0f
    */
    case TF_PROJECTILE_SYRINGE:
    {
        vShootPosOffset = { 16.0f, 6.0f, -8.0f};
        break;
    }

    /*
    * #define FLARE_GRAVITY				0.3f
    #define FLARE_SPEED					2000.0f

    call this float attribute "mult_projectile_speed"
    */
    case TF_PROJECTILE_FLARE:
    {
        vShootPosOffset = { 23.5f, 12.0f, -3.0f };

        if (bDucking == true)
            vShootPosOffset.z = 8.0f;

        break;
    }

    /*
    add 200.0f in UP vector of velocity. 
    call this float attribute "mult_projectile_range" on speed
    Get speed via TF weapon info or ? hardcode it maybe?

    NOTE : There a charing mechanism for stickies
    */
    case TF_PROJECTILE_PIPEBOMB:
    case TF_PROJECTILE_PIPEBOMB_REMOTE:
    case TF_PROJECTILE_PIPEBOMB_PRACTICE:
    case TF_PROJECTILE_CANNONBALL:
    {
        vShootPosOffset = { 16.0f, 8.0f, -6.0f};
        break;
    }

    // I think all these jars and shit use the same logic as Pipe bombs
    /*
    add 200.0f in UP vector of velocity.
    Get speed via TF weapon info or ? hardcode it maybe?
    no attribute to call here
    */
    case TF_PROJECTILE_JAR:
    case TF_PROJECTILE_JAR_MILK:
    case TF_PROJECTILE_CLEAVER:
    case TF_PROJECTILE_THROWABLE:
    case TF_PROJECTILE_FESTIVE_JAR:
    case TF_PROJECTILE_BREADMONSTER_JARATE:
    case TF_PROJECTILE_BREADMONSTER_MADMILK:
    {
        vShootPosOffset = { 16.0f, 8.0f, -6.0f};
        break;
    }

    /*
    Get velocity from charge, call the fn or some shit
    */
    case TF_PROJECTILE_ARROW:
    case TF_PROJECTILE_HEALING_BOLT:
    case TF_PROJECTILE_BUILDING_REPAIR_BOLT:
    case TF_PROJECTILE_FESTIVE_ARROW:
    case TF_PROJECTILE_FESTIVE_HEALING_BOLT:
    case TF_PROJECTILE_GRAPPLINGHOOK:
    {
        vShootPosOffset = { 23.5f, -8.0f, -3.0f };   
        break;
    }

    case TF_PROJECTILE_FLAME_ROCKET: // Drangon's furry uses this maybe
    {
        vShootPosOffset = { 23.5f, -8.0f, -3.0f };

        if (bDucking == true)
            vShootPosOffset.z = 8.0f;

        break;
    }

    case TF_PROJECTILE_ENERGY_BALL: // Cow-Mangler's projectile
    case TF_PROJECTILE_ENERGY_RING: // Rightous bison's projectile
    {
        vShootPosOffset = { 23.5f, -8.0f, -3.0f };

        if (bDucking == true)
            vShootPosOffset.z = 8.0f;
        break;
    }

    default:
        break;
    }

    // Adjusting for center fire weapons
    int iCenterFireProjectile = 0;
    pActiveWeapon->CALL_ATRIB_HOOK_INT(iCenterFireProjectile, "centerfire_projectile");
    if (iCenterFireProjectile == 1)
        vShootPosOffset.y = 0.0f;

    // Flipping Y offset if model fliped
    if (m_bFlipViewModels == 1)
        vShootPosOffset.y *= -1.0f;

    return vShootPosOffset;
}


float AimbotProjectile_t::_GetProjectileSpeed(baseWeapon* pWeapon)
{
    auto  iProjectileType   = pWeapon->GetTFWeaponInfo()->GetWeaponData()->m_iProjectile;
    float flProjectileSpeed = pWeapon->GetTFWeaponInfo()->GetWeaponData()->m_flProjectileSpeed;

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
        constexpr float SYRINGE_GRAVITY  = 0.3f;
        constexpr float SYRINGE_VELOCITY = 1000.0f;

        return SYRINGE_VELOCITY;
    }
    case TF_PROJECTILE_FLARE:
    {
        constexpr float FLARE_GRAVITY = 0.3f;
        constexpr float FLARE_SPEED   = 2000.0f;

        float flFlareSpeed = FLARE_SPEED;
        pWeapon->CALL_ATRIB_HOOK_FLOAT(flFlareSpeed, "mult_projectile_speed");
        return flFlareSpeed;
    }
    // 200.0f offset upward
    case TF_PROJECTILE_PIPEBOMB:
    case TF_PROJECTILE_PIPEBOMB_REMOTE:
    case TF_PROJECTILE_PIPEBOMB_PRACTICE:
    case TF_PROJECTILE_CANNONBALL:
    {
        pWeapon->CALL_ATRIB_HOOK_FLOAT(flProjectileSpeed, "mult_projectile_range");
        return flProjectileSpeed;
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
        return flProjectileSpeed;
    }
    case TF_PROJECTILE_ARROW:
    case TF_PROJECTILE_HEALING_BOLT:
    case TF_PROJECTILE_BUILDING_REPAIR_BOLT:
    case TF_PROJECTILE_FESTIVE_ARROW:
    case TF_PROJECTILE_FESTIVE_HEALING_BOLT:
    case TF_PROJECTILE_GRAPPLINGHOOK:
    {

    }

    default:
        break;
    }

    int iWeaponID = pWeapon->GetWeaponTypeID();
    
    if (iWeaponID == TF_WEAPON_ROCKETLAUNCHER || iWeaponID == TF_WEAPON_ROCKETLAUNCHER_DIRECTHIT)
    {
        float flRocketSpeedBase = 1100.0f;
        pWeapon->CALL_ATRIB_HOOK_FLOAT(flRocketSpeedBase, "mult_projectile_speed");
        return flRocketSpeedBase;
    }
    else if(iWeaponID == TF_WEAPON_GRENADELAUNCHER || iWeaponID == TF_WEAPON_PIPEBOMBLAUNCHER)
    {
        float flBasePipeSpped = 1200.0f;
        pWeapon->CALL_ATRIB_HOOK_FLOAT(flBasePipeSpped, "mult_projectile_range");
        return flBasePipeSpped;
    }

    return 0.0f;
}


//=========================================================================
//                     ProjectileWeaponInfo_t METHODS
//=========================================================================
void ProjectileWeaponInfo_t::Reset()
{
    m_vShootPosOffset.Init();
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
    m_flProjectileBaseSpeed       = _GetBaseProjectileSpeed(m_pWeaponFileInfo, m_pWeapon);
    m_flProjectileBaseGravityMult = _GetBaseProjectileGravityMult(m_pWeaponFileInfo->m_iProjectile);
    m_flUpwardVelOffset           = _GetUpwardVelocityOffset(m_pWeaponFileInfo->m_iProjectile);
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
        vShootPosOffset = { 23.5f, -8.0f, -3.0f };
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
    float flProjectileSpeed = pWeaponFileInfo->m_flProjectileSpeed;

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
        pWeapon->CALL_ATRIB_HOOK_FLOAT(flProjectileSpeed, "mult_projectile_range");
        return flProjectileSpeed;
        break;
    }
    // these are chargable, so we can't store these ahead of time
    //case TF_PROJECTILE_PIPEBOMB_REMOTE:
    //case TF_PROJECTILE_PIPEBOMB_PRACTICE:
    
    case TF_PROJECTILE_JAR:
    case TF_PROJECTILE_JAR_MILK:
    case TF_PROJECTILE_CLEAVER:
    case TF_PROJECTILE_THROWABLE:
    case TF_PROJECTILE_FESTIVE_JAR:
    case TF_PROJECTILE_BREADMONSTER_JARATE:
    case TF_PROJECTILE_BREADMONSTER_MADMILK:
    {
        return flProjectileSpeed;
    }
    //case TF_PROJECTILE_ARROW: this has charging logic, so not saving upfront for this
    //case TF_PROJECTILE_FESTIVE_ARROW: this has charging logic, so not saving upfront for this
    case TF_PROJECTILE_BUILDING_REPAIR_BOLT:
    case TF_PROJECTILE_FESTIVE_HEALING_BOLT:
    case TF_PROJECTILE_HEALING_BOLT: // According to TF official WIKI, this shit also has velocity of 2400.0f 
    //case TF_PROJECTILE_GRAPPLINGHOOK: I don't think this will benifit form being Projectile Aimbot-ed
    {
        constexpr float flBaseArrowSpeed = 2400.0f;
        return flBaseArrowSpeed;
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
    case TF_PROJECTILE_JAR:
    case TF_PROJECTILE_JAR_MILK:
    case TF_PROJECTILE_CLEAVER:
    case TF_PROJECTILE_THROWABLE:
    case TF_PROJECTILE_FESTIVE_JAR:
    case TF_PROJECTILE_BREADMONSTER_JARATE:
    case TF_PROJECTILE_BREADMONSTER_MADMILK:
        return TF_DEFAULT_NADE_GRAVITY;

    case TF_PROJECTILE_SYRINGE:
        constexpr float SYRINGE_GRAVITY_MULTIPLIER = 0.3f;
        return SYRINGE_GRAVITY_MULTIPLIER;
    case TF_PROJECTILE_FLARE:
        constexpr float FLARE_GRAVITY_MULTIPLIER = 0.3f;
        return FLARE_GRAVITY_MULTIPLIER;
    case TF_PROJECTILE_PIPEBOMB:
    case TF_PROJECTILE_CANNONBALL:
    case TF_PROJECTILE_PIPEBOMB_REMOTE:
    case TF_PROJECTILE_PIPEBOMB_PRACTICE:
        constexpr float TF_WEAPON_PIPEBOMB_GRAVITY = 0.5f;
        return TF_WEAPON_PIPEBOMB_GRAVITY;

    case TF_PROJECTILE_BUILDING_REPAIR_BOLT:
    case TF_PROJECTILE_FESTIVE_HEALING_BOLT:
    case TF_PROJECTILE_HEALING_BOLT:
        constexpr float TF_HEALING_BOLT_GRAVITY_MULTIPLIER = 0.2f;
        return TF_HEALING_BOLT_GRAVITY_MULTIPLIER;

    // Although the arrow gravity is depended on the charge level, 
    // I'm storing it as default here, cause if we can hit someone with
    // defult gravity, we shoudl be able to hit them with lowered gravity. :)
    case TF_PROJECTILE_ARROW:
    case TF_PROJECTILE_FESTIVE_ARROW:
        constexpr float TF_DEFAULT_ARROW_GRAVITY = 0.5f;
        return TF_DEFAULT_ARROW_GRAVITY;

    }
}

const float ProjectileWeaponInfo_t::_GetUpwardVelocityOffset(ProjectileType_t iProjectileType)
{
    constexpr float UPWARD_VELOCITY_OFFSET = 200.0f;

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
        return UPWARD_VELOCITY_OFFSET;
    default:
        return 0.0f;
    }

    return 0.0f;
}