//=========================================================================
//                      PROJECTILE ENGINE
//=========================================================================
// by      : INSANE
// created : 28/06/2025
// 
// purpose : Simulates projectiles using high school physics & Source engine's physics objects
//-------------------------------------------------------------------------
#pragma once

#include "../../SDK/class/Basic Structures.h"
#include "../FeatureHandler.h"

// Forward declares
class IPhysicsEnvironment;
class IPhysicsObject;

class baseWeapon;
class BaseEntity;
struct WeaponData_t;

struct ProjectileInfo_tV2
{
    ProjectileInfo_tV2() { Reset(); }
    void Initialize(baseWeapon* pWeapon, const bool bDucking, const bool bViewModelFlipped);
    void SetProjectileAngle(const vec& vOwnerEyePos, const qangle & qOwnerAngles);

    void Reset();
    void ClearPosData();

    // This data is doesn't get updated until weapon is changed
    float    m_flSpeed;
    float    m_flUpwardVelOffset;
    float    m_flGravityMult;
    vec      m_vAngImpulse;
    
    vec      m_vInertia;
    vec      m_vDragBasis;
    vec      m_vAngDragBases;
    
    vec      m_vShootPosOffset;
    uint32_t m_iWeaponDefID;
    WeaponData_t* m_pTFWpnFileInfo;

    bool     m_bUsesDrag;

    // This data gets cleared every call
    vec      m_vStart;
    vec      m_vEnd;
    vec      m_vOrigin; // current origin
    qangle   m_qAngles;

};

class ProjectileEngine_t
{
public:
    ProjectileEngine_t();

    bool Initialize(BaseEntity* pWeaponOwner, baseWeapon* pWeapon, const qangle& qOwnerAngles);
    void RunTick();
    void Reset();

    ProjectileInfo_tV2& SetupProjectile(baseWeapon* pWeapon, BaseEntity* pWeaponOwner, const qangle& qOwnerAngles);
    ProjectileInfo_tV2 m_projInfo;
    inline vec GetPos() const { return m_projInfo.m_vOrigin; } 

private:
    void _AccountForCharge(baseWeapon* pWeapon, BaseEntity* pWeaponOwner);

    bool _SetupPhysicsObject();
    void _DeletePhysicsObjects();

    IPhysicsEnvironment* m_pEnv;
    IPhysicsObject*      m_pObj;

    // CVars...
    void _InitializeCVars();
    bool  m_bCVarsInitialize;
    bool  m_bFlipViewModels;
    float m_flGravity;
};
DECLARE_FEATURE_OBJECT(projectileEngine, ProjectileEngine_t)