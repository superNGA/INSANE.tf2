//=========================================================================
//                      PROJECTILE SIMULATOR
//=========================================================================
// by      : INSANE
// created : 22/06/2025
// 
// purpose : Simulate's projectile
//-------------------------------------------------------------------------
#include "ProjectileSimulation.h"

// SDK
#include "../../Extra/math.h"
#include "../../SDK/TF object manager/TFOjectManager.h"
#include "../../SDK/class/BaseEntity.h"
#include "../../SDK/class/IEngineTrace.h"
#include "../../SDK/class/IVDebugOverlay.h"

void ProjectileSimulator_t::Initialize(
    const vec&  vLaunchPos,            const qangle& qLaunchAngle, 
    const float flProjVelocity,        const float   flProjGravity, 
    const float flProjUpwardVelOffset, const vec&    vHullSize,
    BaseEntity* pOwner,                uint32_t      iTraceMask)
{
    // restoring before using
    Restore();

    vec vForward, vRight, vUp;
    Maths::AngleVectors(qLaunchAngle, &vForward, &vRight, &vUp);
    
    vForward.NormalizeInPlace();
    vUp.NormalizeInPlace();

    vec vVelocity = vForward * flProjVelocity;
    if (flProjUpwardVelOffset > 0.1f)
        vVelocity += vUp * flProjUpwardVelOffset;

    // Storing initial information
    m_projectileInfo.StoreInfo(
        vLaunchPos, vVelocity,
        flProjGravity, flProjVelocity,
        vHullSize,     pOwner, 
        flProjUpwardVelOffset > 0.1f,
        iTraceMask); // all Upward velocity offset having projectile ( pipes, jars & stuff ) use drag
}

void ProjectileSimulator_t::RunTick(bool bTrace, bool bDraw, float flDrawTime)
{
    vec vEndPos;

    // TODO : This is as bullshit of a way to compensate for drag as it can be. 
    //        Find something more absolute.
    if (m_projectileInfo.m_bUseDrag == true)
    {
        //m_projectileInfo.m_flXVel *= 0.982f;
        //m_projectileInfo.m_flYVel *= 0.973f;
    }

    vEndPos       = m_projectileInfo.m_vLauchPos + (m_projectileInfo.m_vVelocity * TICK_INTERVAL);
    vEndPos.z     = m_projectileInfo.m_vLauchPos.z + (m_projectileInfo.m_vVelocity.z * TICK_INTERVAL) + (0.5f * m_projectileInfo.m_flProjGravity * TICK_INTERVAL * TICK_INTERVAL);
    m_projectileInfo.m_vVelocity.z += m_projectileInfo.m_flProjGravity * TICK_INTERVAL;

    // Ray tracing and returning if we hit something.
    if (bTrace == true)
    {
        trace_t trace;
        i_trace_filter filter(m_projectileInfo.m_pOwner);
        m_projectileInfo.m_vHullSize.IsEmpty() == true ?
            I::EngineTrace->UTIL_TraceRay(m_projectileInfo.m_vLauchPos, vEndPos, m_projectileInfo.m_iTraceMask, &filter, &trace) :
            I::EngineTrace->UTIL_TraceHull(m_projectileInfo.m_vLauchPos, vEndPos, m_projectileInfo.m_vHullSize * -1.0f, m_projectileInfo.m_vHullSize, m_projectileInfo.m_iTraceMask, &filter, &trace);
    
        // Did we hit something
        if (trace.m_fraction < 1.0f)
        {
            m_projectileInfo.m_vEndPos  = trace.m_end;
            m_projectileInfo.m_pEntHit  = trace.m_entity;
            m_projectileInfo.m_iContent = trace.m_contents;
            m_projectileInfo.m_bDidHit  = true;

            memcpy(&m_projectileInfo.m_trace, &trace, sizeof(trace_t));
        }
    }

    // Drawing projectile path
    if(bDraw == true)
    {
        constexpr vec vProjSimBoxSize(0.5f, 0.5f, 0.5f);
        I::IDebugOverlay->AddBoxOverlay(vEndPos, vProjSimBoxSize * -1.0f * 0.5f, vProjSimBoxSize * 0.5f, qangle(0.0f, 0.0f, 0.0f), 145, 145, 145, 150, flDrawTime);
        I::IDebugOverlay->AddLineOverlay(m_projectileInfo.m_vLauchPos, vEndPos, 255, 255, 255, true, flDrawTime);
    }

    // Storing new info
    m_projectileInfo.StoreInfo(vEndPos);
}


void ProjectileInfo_t::StoreInfo(
    const vec& vStartPos,       const vec& vVelocity,
    const float flProjGravity,  const float flProjVelocity,
    const vec& vHullSize,       BaseEntity* pOwner,
    bool bUseDrag,              uint32_t    iTraceMask)
{
    m_vLauchPos      = vStartPos;
    m_vVelocity      = vVelocity;
    m_vHullSize      = vHullSize;
    m_flProjGravity  = flProjGravity;
    m_flProjVelocity = flProjVelocity;
    m_pOwner         = pOwner;
    m_bUseDrag       = bUseDrag;
    m_iTraceMask     = iTraceMask;
}

void ProjectileInfo_t::StoreInfo(const vec& vStartPos)
{
    m_vLauchPos = vStartPos;
}

void ProjectileInfo_t::Reset()
{
    m_vLauchPos.Init();
    m_vHullSize.Init();
    m_vEndPos.Init();
    m_vVelocity.Init();
    m_flProjGravity  = 0.0f;
    m_flProjVelocity = 0.0f;
    m_bDidHit        = false;
    m_pOwner         = nullptr;
    m_pEntHit        = nullptr;
    m_bUseDrag       = false;
    m_iTraceMask     = 0;
    m_iContent       = 0;

    memset(&m_trace, 0, sizeof(trace_t));
}