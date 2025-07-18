//=========================================================================
//                      PROJECTILE ENGINE
//=========================================================================
// by      : INSANE
// created : 28/06/2025
// 
// purpose : Simulates projectiles using high school physics & Source engine's physics objects
//-------------------------------------------------------------------------
#include "ProjectileEngine.h"

// SDK
#include "../../SDK/class/IPhysics.h"
#include "../../SDK/class/IPhysicsEnviorment.h"
#include "../../SDK/class/IPhysicsObject.h"
#include "../../SDK/class/IPhysicsCollide.h"

#include "../../SDK/class/BaseEntity.h"
#include "../../SDK/class/BaseWeapon.h"
#include "../../SDK/class/CVar.h"
#include "../../SDK/class/FileWeaponInfo.h"
#include "../../SDK/class/ETFWeaponType.h"
#include "../../SDK/TF object manager/TFOjectManager.h"
#include "../../SDK/class/IEngineTrace.h"

// UTILITY
#include "../../Utility/ConsoleLogging.h"
#include "../../Utility/CVar Handler/CVarHandler.h"
#include "../../Extra/math.h"

ProjectileEngine_t::ProjectileEngine_t()
{
    m_pEnv = nullptr;
    m_pObj = nullptr;

    m_projInfo.Reset();
}

//=========================================================================
//                     PUBLIC METHODS
//=========================================================================
bool ProjectileEngine_t::Initialize(BaseEntity* pWeaponOwner, baseWeapon* pWeapon, const qangle& qOwnerAngles)
{
    // Getting info about this projectile & setup angles & origin
    SetupProjectile(pWeapon, pWeaponOwner, qOwnerAngles);

    // Setting up physics objects
    if (_SetupPhysicsObject() == false)
    {
        FAIL_LOG("Failed to initialize physics objects");
        return false;
    }

    return true;
}


void ProjectileEngine_t::Reset()
{
    _DeletePhysicsObjects();
    m_projInfo.Reset();
}


void ProjectileEngine_t::RunTick(bool bTrace = true, BaseEntity* pIgnoreEnt = nullptr)
{
    if (m_pObj == nullptr)
        return;

    m_pEnv->Simulate(TICK_INTERVAL);
    m_pObj->GetPosition(&m_projInfo.m_vOrigin, nullptr);

    
    if (bTrace == false)
        return;

    // Ray tracing from projectile origin to detect collision
    vec vVelocity;
    m_pObj->GetVelocity(&vVelocity, nullptr);
    
    trace_t trace;
    ITraceFilter_IgnoreSpawnVisualizer filter(pIgnoreEnt);
    I::EngineTrace->UTIL_TraceRay(
        m_projInfo.m_vOrigin, m_projInfo.m_vOrigin + (vVelocity * TICK_INTERVAL), MASK_SHOT,
        &filter, &trace);

    if (trace.m_fraction < 1.0f || trace.m_start_solid == true)
    {
        m_projInfo.m_vEnd = trace.m_end;
    }
}


ProjectileInfo_t& ProjectileEngine_t::SetupProjectile(baseWeapon* pWeapon, BaseEntity* pWeaponOwner, const qangle& qOwnerAngles)
{
    m_projInfo.Initialize(pWeapon, pWeaponOwner->m_fFlags() & FL_DUCKING, CVars::cl_flipviewmodels != 0);

    // Adjusting velocity & gravity for charging weapons
    _AccountForCharge(pWeapon, pWeaponOwner);

    // Setting up Projectile angle & origin
    m_projInfo.SetProjectileAngle(pWeaponOwner->GetEyePos(), qOwnerAngles);

    m_projInfo.m_vStart = m_projInfo.m_vOrigin;

    return m_projInfo;
}


//=========================================================================
//                     PRIVATE METHODS
//=========================================================================
void ProjectileEngine_t::_AccountForCharge(baseWeapon* pWeapon, BaseEntity* pWeaponOwner)
{
    float flCurTime = TICK_TO_TIME(pWeaponOwner->m_nTickBase());

    switch (m_projInfo.m_pTFWpnFileInfo->m_iProjectile)
    {
    case TF_PROJECTILE_ARROW:
    case TF_PROJECTILE_FESTIVE_ARROW:
    {
        float flCharge = 0.0f;
        if (pWeapon->m_flChargeBeginTime() >= 0.01f)
        {
            flCharge = Maths::MIN<float>(flCurTime - pWeapon->m_flChargeBeginTime(), 1.0f);
        }

        m_projInfo.m_flSpeed       = Maths::RemapValClamped(flCharge, 0.0f, 1.f, 1800.0f, 2600.0f);
        m_projInfo.m_flGravityMult = Maths::RemapValClamped(flCharge, 0.0f, 1.f, 0.5, 0.1);

        break;
    }

    case TF_PROJECTILE_PIPEBOMB_REMOTE:
    case TF_PROJECTILE_PIPEBOMB_PRACTICE:
    {
        float flCharge = 0.0f;
        if (pWeapon->m_flChargeBeginTime() >= 0.01f)
        {
            flCharge = Maths::MIN<float>(flCurTime - pWeapon->m_flChargeBeginTime(), 1.0f);

        }

        float flMaxChargeTime = 4.0f;
        pWeapon->CALL_ATRIB_HOOK_FLOAT(flMaxChargeTime, "stickybomb_charge_rate");
        m_projInfo.m_flSpeed = Maths::RemapValClamped(flCharge, 0.0f, flMaxChargeTime, 900.0f, 2400.0f);
        break;
    }

    default:
        break;
    }
}



bool ProjectileEngine_t::_SetupPhysicsObject()
{
    // Phy Enviornment
    if (m_pEnv == nullptr)
    {
        m_pEnv = I::iPhysics->CreateEnvironment();
        if (m_pEnv == nullptr)
        {
            FAIL_LOG("Failed to create physics enviornment");
            return false;
        }
    }


    // Phy Object
    if (m_pObj == nullptr && m_pEnv != nullptr)
    {
        constexpr vec vProjCollide(8.0f);
        void* pProjCollide = I::iPhysicsCollision->BBoxToCollide(vProjCollide * -1.0f, vProjCollide);
        
        // Setting up object parameters
        objectparams_t objectParams{ g_PhysDefaultObjectParams };
        objectParams.damping          = 0.0f;
        objectParams.enableCollisions = false;
        objectParams.volume           = 100.0f;
        objectParams.inertia          = 0.0f;
        objectParams.rotdamping       = 0.0f;
        objectParams.rotInertiaLimit  = 0.0;

        m_pObj = m_pEnv->CreatePolyObject(pProjCollide, 0, vec(0.0f), qangle(0.0f), &objectParams);

        if (m_pObj == nullptr)
        {
            FAIL_LOG("Failed to create physics object");
            return false;
        }

        m_pObj->Wake();
    }

    
    // Setting up physics enviornment paramters
    physics_performanceparams_t envParams;
    envParams.Defaults();

    float m_flMaxVel    = 1000000.0f;
    float m_flMaxAngVel = 1000000.0f;
    switch (m_projInfo.m_pTFWpnFileInfo->m_iProjectile)
    {
    case TF_PROJECTILE_PIPEBOMB:
    //case TF_PROJECTILE_PIPEBOMB_REMOTE:
    //case TF_PROJECTILE_PIPEBOMB_PRACTICE:
    case TF_PROJECTILE_CANNONBALL:
    {
        m_flMaxAngVel = PHYENV_MAX_VELOCITY;
        m_flMaxAngVel = PHYENV_MAX_ANGULAR_VELOCITY;
        break;
    }
    default:
        break;
    }
    envParams.maxVelocity        = m_flMaxVel;
    envParams.maxAngularVelocity = m_flMaxAngVel;
    

    // Env settings
    m_pEnv->SetPerformanceSettings(&envParams);
    m_pEnv->SetGravity(vec(0.0f, 0.0f, -1.0f * CVars::sv_gravity * m_projInfo.m_flGravityMult));
    m_pEnv->SetAirDensity(2.0f); // this is default
    m_pEnv->ResetSimulationClock(); // I think its good to have, spook


    // Obj settings
    float flDrag{ 0.0f };
    if(m_projInfo.m_bUsesDrag == true)
    {
        flDrag = 1.0f;
        m_pObj->SetInertia(m_projInfo.m_vInertia);
        m_pObj->m_vDragBases    = m_projInfo.m_vDragBasis;
        m_pObj->m_vAngDragBases = m_projInfo.m_vAngDragBases;
    }
    m_pObj->SetDragCoefficient(&flDrag, &flDrag);
    
    vec vVelocity(0.0f);
    {
        vec vForward, vRight, vUp;
        Maths::AngleVectors(m_projInfo.m_qAngles, &vForward, &vRight, &vUp);
        vVelocity += (vForward * m_projInfo.m_flSpeed) + (vUp * m_projInfo.m_flUpwardVelOffset);
    }
    m_pObj->SetVelocity(&vVelocity, &m_projInfo.m_vAngImpulse);
    m_pObj->SetPosition(m_projInfo.m_vOrigin, m_projInfo.m_qAngles, true);

    return true;
}



void ProjectileEngine_t::_DeletePhysicsObjects()
{
    if (m_pEnv == nullptr)
        return;

    I::iPhysics->DestroyEnvironment(m_pEnv);
    m_pEnv = nullptr;
    m_pObj = nullptr;
}


//=========================================================================
//                     PROJECTILE INFO METHODS
//=========================================================================
void ProjectileInfo_t::Reset()
{
    memset(this, 0, sizeof(ProjectileInfo_t));
}

void ProjectileInfo_t::ClearPosData()
{
    m_vStart.Init();
    m_vEnd.Init();
    m_vOrigin.Init();
    m_qAngles.Init();
}



void ProjectileInfo_t::SetProjectileAngle(const vec& vOwnerEyePos, const qangle& qOwnerAngles)
{
    // Angles
    vec vOwnerAngles;
    Maths::AngleVectors(qOwnerAngles, &vOwnerAngles);
    Maths::VectorAnglesFromSDK(vOwnerAngles * 2000.0f, m_qAngles);

    // Origin
    vec vForward, vRight, vUp;
    Maths::AngleVectors(qOwnerAngles, &vForward, &vRight, &vUp);
    m_vOrigin = vOwnerEyePos +
        (vForward * m_vShootPosOffset.x) +
        (vRight   * m_vShootPosOffset.y) +
        (vUp      * m_vShootPosOffset.z);
}


void ProjectileInfo_t::Initialize(baseWeapon* pWeapon, const bool bDucking, const bool bViewModelFlipped)
{
    ClearPosData();

    // Don't fillup if same weapon
    if (pWeapon->GetWeaponDefinitionID() == m_iWeaponDefID)
        return;

    Reset();

    m_iWeaponDefID   = pWeapon->GetWeaponDefinitionID();
    m_pTFWpnFileInfo = pWeapon->GetTFWeaponInfo()->GetWeaponData(0);
    
    switch (m_pTFWpnFileInfo->m_iProjectile)
    {
    case TF_PROJECTILE_ROCKET:
    {
        // speed
        float flRocketVelocity = 1100.0f;
        pWeapon->CALL_ATRIB_HOOK_FLOAT(flRocketVelocity, "mult_projectile_speed");
        m_flSpeed = flRocketVelocity;

        // shoot offset
        m_vShootPosOffset   = { 23.5f, 12.0f, -3.0f };
        m_flUpwardVelOffset = 0.0f;
        m_flGravityMult     = 0.0f;

        // yea, cleavers projectile type from file info is same as rockets.
        if (pWeapon->GetWeaponTypeID() == TF_WEAPON_CLEAVER)
        {
            m_flUpwardVelOffset = 200.0f;
            m_flSpeed           = 3000.0f;
            m_vShootPosOffset   = { 16.0f, 8.0f, -6.0f };
            m_flGravityMult     = 1.0f;
        }

        break;
    }
    case TF_PROJECTILE_SYRINGE:
    {
        m_flGravityMult     = 0.3f;
        m_flSpeed           = 1000.0f;
        m_vShootPosOffset   = { 16.0f, 6.0f, -8.0f };
        m_flUpwardVelOffset = 0.0f;
        break;
    }
    case TF_PROJECTILE_FLARE:
    {
        m_flGravityMult     = 0.3f;
        float flFlareSpeed  = 2000.0f;
        pWeapon->CALL_ATRIB_HOOK_FLOAT(flFlareSpeed, "mult_projectile_speed");
        m_flSpeed = flFlareSpeed;

        m_vShootPosOffset = { 23.5f, 12.0f, -3.0f };
        m_flUpwardVelOffset = 0.0f;
        break;
    }
    case TF_PROJECTILE_PIPEBOMB:
    {
        // Angular vel
        m_vAngImpulse       = { 600.0f, -1200.0f, 0.0f };
        
        // Lock-n-load has no angular veloctiy
        int iGrenadeNoSpin = 0;
        pWeapon->CALL_ATRIB_HOOK_INT(iGrenadeNoSpin, "grenade_no_spin");
        if (iGrenadeNoSpin == 1)
            m_vAngImpulse.Init();

        m_flGravityMult     = 1.0f;
        m_flUpwardVelOffset = 200.0f;

        // speed
        float flPipeSpeed = 1200.0f;
        pWeapon->CALL_ATRIB_HOOK_FLOAT(flPipeSpeed, "mult_projectile_range");
        m_flSpeed = flPipeSpeed;

        m_vShootPosOffset = { 16.0f, 8.0f, -6.0f };

        // Drag n Interita
        m_vInertia      = { 0.010637f, 0.036975f, 0.036975f };
        m_vDragBasis    = { 0.003902f, 0.009962f, 0.009962f };
        m_vAngDragBases = { 0.003618f, 0.001514f, 0.001514f };
        m_bUsesDrag     = true;

        break;
    }
    case TF_PROJECTILE_CANNONBALL:
    {
        m_vAngImpulse       = { 600.0f, -1200.0f, 0.0f };
        m_flGravityMult     = 1.0f;
        m_flUpwardVelOffset = 200.0f;

        // speed
        float flPipeSpeed = 1200.0f;
        pWeapon->CALL_ATRIB_HOOK_FLOAT(flPipeSpeed, "mult_projectile_range");
        m_flSpeed = flPipeSpeed;

        m_vShootPosOffset = { 16.0f, 8.0f, -6.0f };

        // Drag n Interita
        m_vInertia      = { 0.044588f, 0.042257f, 0.044588f };
        m_vDragBasis    = { 0.020971f, 0.019420f, 0.020971f };
        m_vAngDragBases = { 0.012997f, 0.013496f, 0.013714f };
        m_bUsesDrag     = true;

        break;
    }
    case TF_PROJECTILE_PIPEBOMB_REMOTE:
    case TF_PROJECTILE_PIPEBOMB_PRACTICE:
    {
        m_vAngImpulse       = { 600.0f, -1200.0f, 0.0f };
        m_flGravityMult     = 1.0f;
        m_flUpwardVelOffset = 200.0f;
        m_flSpeed           = 900.0f;
        m_vShootPosOffset   = { 16.0f, 8.0f, -6.0f };

        // Drag n Interita
        m_vInertia      = { 0.017142f, 0.017142f, 0.016747f };
        m_vDragBasis    = { 0.007491f, 0.007491f, 0.007306f };
        m_vAngDragBases = { 0.002777f, 0.002842f, 0.002812f };
        m_bUsesDrag     = true;

        break;
    }
    case TF_PROJECTILE_CLEAVER:
    {
        m_flUpwardVelOffset = 200.0f;
        m_flSpeed           = 3000.0f;
        m_vShootPosOffset   = { 16.0f, 8.0f, -6.0f };
        m_flGravityMult     = 1.0f;
        break;
    }
    case TF_PROJECTILE_JAR:
    case TF_PROJECTILE_JAR_MILK:
    case TF_PROJECTILE_THROWABLE:
    case TF_PROJECTILE_FESTIVE_JAR:
    case TF_PROJECTILE_BREADMONSTER_JARATE:
    case TF_PROJECTILE_BREADMONSTER_MADMILK:
    {
        m_flGravityMult     = 1.0f;
        m_flUpwardVelOffset = 200.0f;
        m_flSpeed           = 1000.0f;
        m_vShootPosOffset   = { 16.0f, 8.0f, -6.0f };
        break;
    }
    case TF_PROJECTILE_ARROW:
    case TF_PROJECTILE_FESTIVE_ARROW:
    {
        m_flGravityMult     = 0.5f;
        m_flSpeed           = 1800.0f;
        m_vShootPosOffset   = { 23.5f, 8.0f, -3.0f };
        m_flUpwardVelOffset = 0.0f;

        // For some bizzare reason, the rescue ranger is apperaring as arrow, 
        // so we have to deal with it seperately.
        if (m_iWeaponDefID == 997) // 997 is for rescue ranger
        {
            m_flGravityMult = 0.2f;
            m_flSpeed       = 2400.0f;
        }
        break;
    }
    case TF_PROJECTILE_HEALING_BOLT:
    case TF_PROJECTILE_BUILDING_REPAIR_BOLT:
    case TF_PROJECTILE_FESTIVE_HEALING_BOLT:
    {
        m_flGravityMult     = 0.2f;
        m_flSpeed           = 2400.0f;
        m_vShootPosOffset   = { 23.5f, 8.0f, -3.0f };
        m_flUpwardVelOffset = 0.0f;
        break;
    }

    case TF_PROJECTILE_FLAME_ROCKET: // Drangon's furry uses this maybe
    {
        m_flSpeed           = 3000.0f;
        m_vShootPosOffset   = { 23.5f, -8.0f, -3.0f };
        m_flUpwardVelOffset = 0.0f;
        break;
    }

    case TF_PROJECTILE_ENERGY_BALL: // Cow-Mangler's projectile
    {
        m_flGravityMult     = 0.f;
        m_flSpeed           = 1100.0f;
        m_vShootPosOffset   = { 23.5f, 8.0f, -3.0f };
        m_flUpwardVelOffset = 0.0f;
        break;
    }
    case TF_PROJECTILE_ENERGY_RING: // Rightous bison's projectile
    {
        m_flGravityMult     = 0.f;
        m_flSpeed           = 1200.0f;
        m_vShootPosOffset   = { 23.5f, 8.0f, -3.0f };
        m_flUpwardVelOffset = 0.0f;
        break;
    }

    default:
        break;
    }

    LOG("speed : %.2f, gravity mult : %.2f\n", m_flSpeed, m_flGravityMult);
    LOG("Projectile Type : %d | WPN ID : %d", m_pTFWpnFileInfo->m_iProjectile, m_iWeaponDefID);

    // Centre fire wpns
    int iCenterFireProjectile = 0;
    pWeapon->CALL_ATRIB_HOOK_INT(iCenterFireProjectile, "centerfire_projectile");
    if (iCenterFireProjectile == 1)
        m_vShootPosOffset.y = 0.0f;

    // Fliping view model
    if (bViewModelFlipped == true)
        m_vShootPosOffset.y *= -1.0f;

    // Handling ducking
    if(bDucking == true)
    {
        switch (m_pTFWpnFileInfo->m_iProjectile)
        {
        case TF_PROJECTILE_ROCKET:
        case TF_PROJECTILE_FLARE:
        case TF_PROJECTILE_FLAME_ROCKET:
        case TF_PROJECTILE_ENERGY_BALL:
        case TF_PROJECTILE_ENERGY_RING:
            m_vShootPosOffset.z = 8.0f;
            break;
        default:
            break;
        }
    }


}
