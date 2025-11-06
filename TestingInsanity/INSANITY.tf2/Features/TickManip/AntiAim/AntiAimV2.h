//=========================================================================
//                      ANTI-AIM
//=========================================================================
// by      : INSANE
// created : 11/05/2025
// 
// purpose : Makes you harder to hit for enemies aimbot.
//-------------------------------------------------------------------------
#pragma once
#include "../../FeatureHandler.h"
#include "../../../SDK/class/Basic Structures.h"


class BaseEntity;
class baseWeapon;
class CUserCmd;



///////////////////////////////////////////////////////////////////////////
class AntiAimV2_t
{
public:
    void Run(BaseEntity* pLocalPlayer, baseWeapon* pActiveWeapon, CUserCmd* pCmd, bool* pSendPacket, bool* pCreateMoveResult);
    
    int    GetAntiAimTicks();
    
    qangle GetFakeAngles() const;
    qangle GetRealAngles() const;

private:
    void _FixMovement(CUserCmd* pCmd);

    qangle _GetFakeAngles(CUserCmd* pCmd) const;
    qangle _GetRealAngles(CUserCmd* pCmd) const;

    qangle m_qFakeAngles;
    qangle m_qRealAngles;

};
///////////////////////////////////////////////////////////////////////////

DECLARE_FEATURE_OBJECT(antiAimV2, AntiAimV2_t)

DEFINE_TAB(AntiAim, 5)
DEFINE_SECTION(AntiAim, "AntiAim", 1)

DEFINE_FEATURE(AntiAim_Enable, "AntiAim",          bool, AntiAim, AntiAim, 1, false, FeatureFlag_SupportKeyBind, "Run AntiAim, so enemy aim = anti?")
DEFINE_FEATURE(AntiAim_Draw,   "Draw Fake Angles", bool, AntiAim, AntiAim, 2, false, FeatureFlag_SupportKeyBind, "Draw model with fake angles.")


DEFINE_FEATURE(AntiAim_CustomFakeAngles, "Custom Fake Angles", bool,          AntiAim, AntiAim, 3, false,                              FeatureFlag_SupportKeyBind, "Draw model with fake angles.")
DEFINE_FEATURE(AntiAim_FakePitch,        "Fake Pitch",         FloatSlider_t, AntiAim, AntiAim, 4, FloatSlider_t(0.0f, -89.0f, 89.0f), FeatureFlag_SupportKeyBind, "Pitch the others see.")
DEFINE_FEATURE(AntiAim_FakeYawOffset,    "Fake Yaw Offest",    FloatSlider_t, AntiAim, AntiAim, 5, FloatSlider_t(0.0f, 0.0f, 360.0f),  FeatureFlag_SupportKeyBind, "how much angle between your view angles & fake yaw.")

DEFINE_FEATURE(AntiAim_CustomRealAngles, "Custom Real Angles", bool,          AntiAim, AntiAim, 6, false,                              FeatureFlag_SupportKeyBind, "Draw model with fake angles.")
DEFINE_FEATURE(AntiAim_RealPitch,        "Real Pitch",         FloatSlider_t, AntiAim, AntiAim, 7, FloatSlider_t(0.0f, -89.0f, 89.0f), FeatureFlag_SupportKeyBind, "Pitch the others see.")
DEFINE_FEATURE(AntiAim_RealYawOffset,    "Real Yaw Offest",    FloatSlider_t, AntiAim, AntiAim, 8, FloatSlider_t(0.0f, 0.0f, 360.0f),  FeatureFlag_SupportKeyBind, "how much angle between your view angles & real yaw.")