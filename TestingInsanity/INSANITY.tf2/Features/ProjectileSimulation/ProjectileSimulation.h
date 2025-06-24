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
#include "../../SDK/class/IEngineTrace.h"

class BaseEntity;

struct ProjectileInfo_t
{
    ProjectileInfo_t() { Reset(); }

    void StoreInfo(
        const vec& vStartPos,       const vec& vVelocity,
        const float flProjGravity,  const float flProjVelocity,
        const vec& vHullSize,       BaseEntity* pOwner,         
        bool bUseDrag,              uint32_t iTraceMask);

    void StoreInfo(const vec& vStartPos);

    void Reset();

    float       m_flProjGravity;
    float       m_flProjVelocity;
    vec         m_vLauchPos;
    vec         m_vHullSize;

    vec         m_vVelocity;
    vec         m_vEndPos;

    BaseEntity* m_pOwner;
    bool        m_bUseDrag;
    bool        m_bDidHit;
    BaseEntity* m_pEntHit;
    uint32_t    m_iTraceMask;
    uint32_t    m_iContent;
    trace_t     m_trace;
};

class ProjectileSimulator_t
{
public:
    ProjectileSimulator_t() { m_projectileInfo.Reset(); }

    void Initialize(
        const vec&  vLaunchPos,            const qangle& qLaunchAngle, 
        const float flProjVelocity,        const float   flProjGravity, 
        const float flProjUpwardVelOffset, const vec&    vHullSize,
        BaseEntity* pOwner,                uint32_t iTraceMask = MASK_SHOT);
    void RunTick(bool bTrace = true, bool bDraw = false, float flDrawTime = 0.0f);
    inline void Restore() { m_projectileInfo.Reset(); }

    ProjectileInfo_t m_projectileInfo;
};
DECLARE_FEATURE_OBJECT(projectileSimulator, ProjectileSimulator_t)