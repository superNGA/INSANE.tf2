//=========================================================================
//                      ANTI-AIM
//=========================================================================
// by      : INSANE
// created : 11/05/2025
// 
// purpose : Makes you harder to hit for enemies aimbot.
//-------------------------------------------------------------------------
#include "AntiAimV2.h"

// UTILITY
#include "../../../SDK/class/Basic Structures.h"
#include "../../../Extra/math.h"
 
// SDK
#include "../../../SDK/class/BaseEntity.h"
#include "../../../SDK/class/BaseWeapon.h"
#include "../../../SDK/class/CUserCmd.h"
#include "../../../SDK/class/IVEngineClient.h"



///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
void AntiAimV2_t::Run(BaseEntity* pLocalPlayer, baseWeapon* pActiveWeapon, CUserCmd* pCmd, bool* pSendPacket, bool* pCreateMoveResult)
{
    if (Features::AntiAim::AntiAim::AntiAim_Enable.IsActive() == false)
        return;


    qangle* pQActiveAngles = nullptr;
    
    if (*pSendPacket == true)
    {
        m_qFakeAngles  = _GetFakeAngles(pCmd);
        pQActiveAngles = &m_qFakeAngles;
    }
    else
    {
        m_qRealAngles  = _GetRealAngles(pCmd);
        pQActiveAngles = &m_qRealAngles;
    }
    
    pCmd->viewangles = *pQActiveAngles;

    // Fix movement, so we can move properly with those fucked up Anti-Aim angles.
    _FixMovement(pCmd);

    // so view angles don't get set to engine angles.
    *pCreateMoveResult = false;
}


///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
int AntiAimV2_t::GetAntiAimTicks()
{
    return 2;
}


///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
qangle AntiAimV2_t::GetFakeAngles() const
{
    return m_qFakeAngles;
}


///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
qangle AntiAimV2_t::GetRealAngles() const
{
    return m_qRealAngles;
}


///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
static inline float normalizeAngle(float flAngle)
{
    if (flAngle < 0.0f)
        return flAngle + 360.0f;
    return flAngle;
}


///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
void AntiAimV2_t::_FixMovement(CUserCmd* pCmd)
{
    qangle qEngineAngles;
    I::iEngine->GetViewAngles(qEngineAngles);

    float fakeAnglesInDeg = 360.0f - normalizeAngle(pCmd->viewangles.yaw);
    float realAnglesInDeg = 360.0f - normalizeAngle(qEngineAngles.yaw);

    float deltaAngleInRad = DEG2RAD((realAnglesInDeg - fakeAnglesInDeg));

    float orignalForwardMove = pCmd->forwardmove;
    float orignalSideMove    = pCmd->sidemove;

    pCmd->forwardmove	= cos(deltaAngleInRad) * orignalForwardMove - sin(deltaAngleInRad) * orignalSideMove;
    pCmd->sidemove		= cos(deltaAngleInRad) * orignalSideMove + sin(deltaAngleInRad) * orignalForwardMove;
}


///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
qangle AntiAimV2_t::_GetFakeAngles(CUserCmd* pCmd) const
{
    qangle qAnglesOut(pCmd->viewangles);

    if (Features::AntiAim::AntiAim::AntiAim_CustomFakeAngles.IsActive() == false)
        return qAnglesOut;
    
    qAnglesOut.pitch = Features::AntiAim::AntiAim::AntiAim_FakePitch.GetData().m_flVal;
    qAnglesOut.yaw  += Features::AntiAim::AntiAim::AntiAim_FakeYawOffset.GetData().m_flVal;
    qAnglesOut.roll  = 0.0f;

    Maths::WrapYaw(qAnglesOut);
    return qAnglesOut;
}


///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
qangle AntiAimV2_t::_GetRealAngles(CUserCmd* pCmd) const
{
    qangle qAnglesOut(pCmd->viewangles);

    if (Features::AntiAim::AntiAim::AntiAim_CustomRealAngles.IsActive() == false)
        return qAnglesOut;

    qAnglesOut.pitch = Features::AntiAim::AntiAim::AntiAim_RealPitch.GetData().m_flVal;
    qAnglesOut.yaw += Features::AntiAim::AntiAim::AntiAim_RealYawOffset.GetData().m_flVal;
    qAnglesOut.roll = 0.0f;

    Maths::WrapYaw(qAnglesOut);
    return qAnglesOut;
}
