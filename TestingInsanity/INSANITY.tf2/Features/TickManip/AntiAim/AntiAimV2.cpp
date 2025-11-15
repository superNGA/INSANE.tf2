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
#include "../../../Utility/ConsoleLogging.h"
 
// SDK
#include "../../../SDK/class/BaseEntity.h"
#include "../../../SDK/class/BaseWeapon.h"
#include "../../../SDK/class/CUserCmd.h"
#include "../../../SDK/class/IVEngineClient.h"
#include "../../../SDK/class/IVDebugOverlay.h"
#include "../../../SDK/class/IEngineTrace.h"
#include "../../ImGui/InfoWindowV2/InfoWindow.h"



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
        m_qRealAngles = _GetRealAngles(pCmd);
        
        if (Features::AntiAim::AntiAim::AntiAim_EdgeDetection.IsActive() == true)
        {
            qangle qSafeAngles; _EdgeDetection(pLocalPlayer, pCmd, qSafeAngles, m_qRealAngles.pitch, m_bEdgeDetected);
            
            if(m_bEdgeDetected == true)
            {
                m_qRealAngles.yaw = qSafeAngles.yaw;
            }
        }
        else
        {
            m_bEdgeDetected = false;
        }


        pQActiveAngles = &m_qRealAngles;
    }

    pCmd->viewangles = *pQActiveAngles;

    // Fix movement, so we can move properly with those fucked up Anti-Aim angles.
    FixMovement(pCmd);

    // so view angles don't get set to engine angles.
    *pCreateMoveResult = false;


    _DrawWidget();
}


///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
int AntiAimV2_t::GetAntiAimTicks()
{
    // 2, so that it don't interfere with double tapping.
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
qangle* AntiAimV2_t::GetFakeAnglesP()
{
    return &m_qFakeAngles;
}


///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
qangle AntiAimV2_t::GetRealAngles() const
{
    return m_qRealAngles;
}


///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
qangle* AntiAimV2_t::GetRealAnglesP()
{
    return &m_qRealAngles;
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
void AntiAimV2_t::FixMovement(CUserCmd* pCmd)
{
    qangle qEngineAngles;
    I::iEngine->GetViewAngles(qEngineAngles);

    float fakeAnglesInDeg    = 360.0f - normalizeAngle(pCmd->viewangles.yaw);
    float realAnglesInDeg    = 360.0f - normalizeAngle(qEngineAngles.yaw);
                             
    float deltaAngleInRad    = DEG2RAD((realAnglesInDeg - fakeAnglesInDeg));

    float orignalForwardMove = pCmd->forwardmove;
    float orignalSideMove    = pCmd->sidemove;

    pCmd->forwardmove        = cos(deltaAngleInRad) * orignalForwardMove - sin(deltaAngleInRad) * orignalSideMove;
    pCmd->sidemove           = cos(deltaAngleInRad) * orignalSideMove    + sin(deltaAngleInRad) * orignalForwardMove;
}


///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
void AntiAimV2_t::_EdgeDetection(BaseEntity* pLocalPlayer, CUserCmd* pCmd, qangle& qSafeAngleOut, float flPitchIn, bool& bEdgeFound)
{
    vec vEyePos = pLocalPlayer->GetEyePos();

    // Left & right from head.
    qangle qEngineAngles; I::iEngine->GetViewAngles(qEngineAngles);
    vec vForward, vRight, vUp; Maths::AngleVectors(qEngineAngles, &vForward, &vRight, &vUp);
    constexpr float BASE_WIDTH_HALF = 15.0f;
    vec vRightMost = vEyePos + (vRight *  BASE_WIDTH_HALF);
    vec vLeftMost  = vEyePos + (vRight * -BASE_WIDTH_HALF);


    // Tracing from both sides, to find which is closer to wall.
    ITraceFilter_IgnoreSpawnVisualizer filter(pLocalPlayer);
    trace_t trace;

    // Right...
    I::EngineTrace->UTIL_TraceRay(vRightMost, vRightMost + (vForward * 200.0f), MASK_SHOT, &filter, &trace);
    vec vRightEnd = trace.m_end;

    // Left...
    I::EngineTrace->UTIL_TraceRay(vLeftMost, vLeftMost + (vForward * 200.0f), MASK_SHOT, &filter, &trace);
    vec vLeftEnd  = trace.m_end;


    // Calculating angle between "the line joining the left end to right end" & "line parallel to right vector".
    float flDist2D = fabsf(vLeftEnd.Dist2Dto(vRightEnd));
    float flBase   = fabsf(vLeftMost.Dist2Dto(vRightMost));
    float flAngleInDeg = RAD2DEG(acosf(flBase / flDist2D));

    float flDistLeft  = vLeftMost.Dist2Dto(vLeftEnd);
    float flDistRight = vRightMost.Dist2Dto(vRightEnd);
    
    constexpr float flEdgeTolerance = 10.0f;
    if (flAngleInDeg <= flEdgeTolerance)
    {
        m_bEdgeDetected = false;
        return;
    }
    m_bEdgeDetected = true;


    // Which point ( left or right ) is behind wall.
    vec vHiddenPoint  = fabsf(flDistLeft) < fabsf(flDistRight) ? vLeftMost  : vRightMost;
    vec vExposedPoint = fabsf(flDistLeft) < fabsf(flDistRight) ? vRightMost : vLeftMost;

    // Which point should we aim @ to hide our head the best. ( depends on pitch ).
    bool bLookingUp = flPitchIn < 0.0f;
    // if we are looking up, we need to aim at the exposed / unsafe point so our head moves to safe point.
    // else vise-versa.
    vec& vSafePoint = bLookingUp == true ? vExposedPoint : vHiddenPoint;

    // Get the angle we need to look at safe point.
    Maths::VectorAngles(vSafePoint - vEyePos, qSafeAngleOut);


    if (false)
    {
        I::IDebugOverlay->ClearAllOverlays();
        I::IDebugOverlay->AddTextOverlay(vLeftEnd, 1.0f, "%.2f", flDistLeft);
        I::IDebugOverlay->AddTextOverlay(vRightEnd, 1.0f, "%.2f", flDistRight);
        I::IDebugOverlay->AddBoxOverlay(vLeftMost,  vec(-2.0f), vec(2.0f), qangle(0.0f), 255, 0, 0, 255, 1.0f);
        I::IDebugOverlay->AddBoxOverlay(vLeftEnd,   vec(-2.0f), vec(2.0f), qangle(0.0f), 255, 0, 0, 255, 1.0f);
        I::IDebugOverlay->AddBoxOverlay(vRightMost, vec(-2.0f), vec(2.0f), qangle(0.0f), 255, 0, 0, 255, 1.0f);
        I::IDebugOverlay->AddBoxOverlay(vRightEnd,  vec(-2.0f), vec(2.0f), qangle(0.0f), 255, 0, 0, 255, 1.0f);
        I::IDebugOverlay->AddAngleOverlay(
            qSafeAngleOut,
            pLocalPlayer->GetAbsOrigin() + (0.0f, 0.0f, 20.0f),
            200.0f, 255, 255, 255, 255, 1.0f);
    }
}


///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
void AntiAimV2_t::_DrawWidget()
{
    // Hide widget if not drawing.
    if (Features::AntiAim::AntiAim::AntiAim_DrawWidget.IsActive() == false)
    {
        Render::infoWindowV2.Hide("AntiAim");
        return;
    }


    Render::infoWindowV2.AddOrUpdate("AntiAim", "Edge Detection", 0, InfoWindowWidget_t::Alignment_Left);
    if (Features::AntiAim::AntiAim::AntiAim_EdgeDetection.IsActive() == false)
    {
        Render::infoWindowV2.AddOrUpdate("AntiAim", "Disabled", 0, InfoWindowWidget_t::Alignment_Right, RGBA_t(1.0f, 0.0f, 0.0f, 1.0f));
    }
    else if (m_bEdgeDetected == false)
    {
        Render::infoWindowV2.AddOrUpdate("AntiAim", "Not-Detected", 0, InfoWindowWidget_t::Alignment_Right, RGBA_t(1.0f, 1.0f, 0.0f, 1.0f));
    }
    else
    {
        Render::infoWindowV2.AddOrUpdate("AntiAim", "Detected", 0, InfoWindowWidget_t::Alignment_Right, RGBA_t(0.0f, 1.0f, 0.0f, 1.0f));
    }
}


///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
qangle AntiAimV2_t::_GetFakeAngles(CUserCmd* pCmd) const
{
    qangle qAnglesOut; I::iEngine->GetViewAngles(qAnglesOut); //(pCmd->viewangles*);

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
    qangle qAnglesOut; I::iEngine->GetViewAngles(qAnglesOut); //(pCmd->viewangles);

    if (Features::AntiAim::AntiAim::AntiAim_CustomRealAngles.IsActive() == false)
        return qAnglesOut;

    qAnglesOut.pitch = Features::AntiAim::AntiAim::AntiAim_RealPitch.GetData().m_flVal;
    qAnglesOut.yaw += Features::AntiAim::AntiAim::AntiAim_RealYawOffset.GetData().m_flVal;
    qAnglesOut.roll = 0.0f;

    Maths::WrapYaw(qAnglesOut);
    return qAnglesOut;
}
