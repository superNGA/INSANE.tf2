//=========================================================================
//                      PROJECTILE SIMULATOR
//=========================================================================
// by      : INSANE
// created : 22/06/2025
// 
// purpose : Simulate's projectile
//-------------------------------------------------------------------------
#pragma once

#include "../FeatureHandler.h"

class BaseEntity;

struct ProjectileInfo_t
{
    ProjectileInfo_t() { Reset(); }

    void StoreInfo(
        const vec& vStartPos,       const vec& vLaunchAngle, 
        const float flXVel,         const float flYVel,
        const float flProjGravity,  const float flProjVelocity,
        const float flLauchPitch,   const vec& vHullSize,
        BaseEntity* pOwner);
    void Reset();

    float       m_flLaunchPitch;
    float       m_flXVel;
    float       m_flYVel;
    float       m_flProjGravity;
    float       m_flProjVelocity;
    vec         m_vLauchPos;
    vec         m_vLaunchAngle;
    vec         m_vHullSize;
    vec         m_vEndPos;
    BaseEntity* m_pOwner;
    bool        m_bDidHit;
};

class ProjectileSimulator_t
{
public:
    ProjectileSimulator_t() { m_projectileInfo.Reset(); }

    void Simulate(const vec& vStartPos, const qangle& qLauchAngles, BaseEntity* pProjectileOwner, const float flTimeToSimulateInSec, const vec& vHullSize, const float flGravity, const float flVelocity, const float flUpwardVelcityOffset);
    
    void Initialize(
        const vec&  vLaunchPos,            const qangle& qLaunchAngle, 
        const float flProjVelocity,        const float   flProjGravity, 
        const float flProjUpwardVelOffset, const vec&    vHullSize,
        BaseEntity* pOwner);
    void RunTick(bool bDraw = false, float flDrawTime = 0.0f);
    inline void Restore() { m_projectileInfo.Reset(); }

    ProjectileInfo_t m_projectileInfo;
};
DECLARE_FEATURE_OBJECT(projectileSimulator, ProjectileSimulator_t)