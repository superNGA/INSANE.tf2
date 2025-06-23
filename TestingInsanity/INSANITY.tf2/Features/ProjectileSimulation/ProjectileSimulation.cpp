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
    BaseEntity* pOwner)
{
    // restoring before using
    Restore();

    // Fixing angle by upward velocity offset
    float flAngleToCompensate = RAD2DEG(atanf(flProjUpwardVelOffset / flProjVelocity));
    qangle qFixedLauchAngles  = qLaunchAngle;
    float flLaunchPitch       = qFixedLauchAngles.pitch * -1.0f; // Inverting pitch back
    flLaunchPitch             += flAngleToCompensate; // Compensating pitch

    vec vLaunchAngles;
    Maths::AngleVectors(qFixedLauchAngles, &vLaunchAngles);
    vLaunchAngles.NormalizeInPlace();

    // Storing initial information
    m_projectileInfo.StoreInfo(
        vLaunchPos, vLaunchAngles, 
        flProjVelocity * cosf(DEG2RAD(flLaunchPitch)), 
        flProjVelocity * sinf(DEG2RAD(flLaunchPitch)),
        flProjGravity, flProjVelocity,
        flLaunchPitch, vHullSize,
        pOwner);
}

void ProjectileSimulator_t::RunTick(bool bDraw, float flDrawTime)
{
    vec vEndPos;

    // TODO : This is as bullshit of a way to compensate for drag as it can be. 
    //        Find something more absolute.
    if (m_projectileInfo.m_vHullSize.IsEmpty() == false)
    {
        m_projectileInfo.m_flXVel *= 0.982f;
        m_projectileInfo.m_flYVel *= 0.973f;
    }

    // Calculated new X & Y position.
    float flRangeCovered = TICK_INTERVAL * m_projectileInfo.m_flXVel;
    vEndPos              = m_projectileInfo.m_vLauchPos + (m_projectileInfo.m_vLaunchAngle * flRangeCovered);
    vEndPos.z            = 0.0f;

    // Calculating Z axis position.
    float flHeightCovered = (m_projectileInfo.m_flYVel * TICK_INTERVAL) + (0.5f * m_projectileInfo.m_flProjGravity * TICK_INTERVAL * TICK_INTERVAL);
    vEndPos.z             = m_projectileInfo.m_vLauchPos.z + flHeightCovered;

    // Ray trace here and decided whether to quit or not
    i_trace_filter filter(m_projectileInfo.m_pOwner);
    trace_t trace;
    m_projectileInfo.m_vHullSize.IsEmpty() == true ?
        I::EngineTrace->UTIL_TraceRay(m_projectileInfo.m_vLauchPos, vEndPos, MASK_SHOT, &filter, &trace) :
        I::EngineTrace->UTIL_TraceHull(m_projectileInfo.m_vLauchPos, vEndPos, m_projectileInfo.m_vHullSize * -1.0f, m_projectileInfo.m_vHullSize, MASK_SHOT, &filter, &trace);

    if (trace.m_fraction < 1.0f)
    {
        m_projectileInfo.m_vEndPos = trace.m_end;
        m_projectileInfo.m_bDidHit = true;
    }

    if(bDraw == true)
    {
        constexpr vec vProjSimBoxSize(0.5f, 0.5f, 0.5f);
        I::IDebugOverlay->AddBoxOverlay(vEndPos, vProjSimBoxSize * -1.0f * 0.5f, vProjSimBoxSize * 0.5f, qangle(0.0f, 0.0f, 0.0f), 145, 145, 145, 150, flDrawTime);
        I::IDebugOverlay->AddLineOverlay(m_projectileInfo.m_vLauchPos, vEndPos, 255, 255, 255, true, flDrawTime);
    }

    // Storing updated position and velocity
    float flNewYVel  = m_projectileInfo.m_flYVel + (m_projectileInfo.m_flProjGravity * TICK_INTERVAL);
    float flNewPitch = RAD2DEG(atanf(flNewYVel / m_projectileInfo.m_flXVel));

    m_projectileInfo.StoreInfo(
        vEndPos, m_projectileInfo.m_vLaunchAngle,
        m_projectileInfo.m_flXVel,
        m_projectileInfo.m_flYVel,
        m_projectileInfo.m_flProjGravity, m_projectileInfo.m_flProjVelocity,
        flNewPitch, m_projectileInfo.m_vHullSize, m_projectileInfo.m_pOwner);
}


void ProjectileInfo_t::StoreInfo(
    const vec& vStartPos, const vec& vLaunchAngle, 
    const float flXVel, const float flYVel, 
    const float flProjGravity, const float flProjVelocity,
    const float flLauchPitch, const vec& vHullSize,
    BaseEntity* pOwner)
{
    m_vLauchPos      = vStartPos;
    m_flXVel         = flXVel;
    m_flYVel         = flYVel;
    m_flLaunchPitch  = flLauchPitch;
    m_vLaunchAngle   = vLaunchAngle;
    m_vHullSize      = vHullSize;
    m_flProjGravity  = flProjGravity;
    m_flProjVelocity = flProjVelocity;
    m_pOwner         = pOwner;
}

void ProjectileInfo_t::Reset()
{
    m_vLauchPos.Init();
    m_vLaunchAngle.Init();
    m_vHullSize.Init();
    m_vEndPos.Init();
    m_flLaunchPitch = 0.0f;
    m_flXVel        = 0.0f;
    m_flYVel        = 0.0f;
    m_flProjGravity = 0.0f;
    m_flProjVelocity = 0.0f;
    m_bDidHit       = false;
    m_pOwner = nullptr;
}