#pragma once
#include "../FeatureHandler.h"
//#include "../../SDK/class/Source Entity.h"

class CUserCmd;
class baseWeapon;
class BaseEntity;


///////////////////////////////////////////////////////////////////////////
class Movement_t
{
public:
    Movement_t();
    void Reset();

    void Run(BaseEntity* pLocalPlayer, baseWeapon* pActiveWeapon, CUserCmd* pCmd, bool& result);

private:
    void _Bhop       (CUserCmd* pCmd, bool& result, BaseEntity* pLocalPlayer);
    void _RocketJump (CUserCmd* pCmd, bool& result, baseWeapon* pActiveWeapon, BaseEntity* pLocalPlayer);
    void _ThirdPerson(CUserCmd* pCmd, bool& result, BaseEntity* pLocalPlayer);
    void _AutoStrafer(BaseEntity* pLocalPlayer, CUserCmd* pCmd);
    void _AutoStop   (BaseEntity* pLocalPlayer, CUserCmd* pCmd, baseWeapon* pActiveWeapon);
    void _PeekAssist (BaseEntity* pLocalPlayer, CUserCmd* pCmd, baseWeapon* pActiveWeapon);

    void _InitializeKeyCodes();

    int32_t m_iJumpKeyCode          = 0;
    bool    m_bInitializedKeyCodes  = false;

    bool    m_bLastBhopState        = false;
    
};
///////////////////////////////////////////////////////////////////////////


DECLARE_FEATURE_OBJECT(movement, Movement_t)

DEFINE_TAB(Movement, 10);
DEFINE_SECTION(Movement, "Movement", 1)

DEFINE_FEATURE(Bhop,                 "Bhop",                       bool,          Movement, Movement, 1, false, FeatureFlag_SupportKeyBind | FeatureFlag_HoldOnlyKeyBind)
DEFINE_FEATURE(AutoRocketJump,       "Auto Rocket Jump",           bool,          Movement, Movement, 2, false, FeatureFlag_SupportKeyBind | FeatureFlag_HoldOnlyKeyBind | FeatureFlag_DisableWhileMenuOpen)
DEFINE_FEATURE(ThirdPerson,          "Third Person",               bool,          Movement, Movement, 3, false, FeatureFlag_SupportKeyBind | FeatureFlag_ToggleOnlyKeyBind)
DEFINE_FEATURE(DirectionStrafe,      "Direction Strafe",           bool,          Movement, Movement, 4, false, FeatureFlag_SupportKeyBind | FeatureFlag_ToggleOnlyKeyBind)
DEFINE_FEATURE(AutoStrafe_Agression, "Direction Strafe Aggresion", FloatSlider_t, Movement, Movement, 5, FloatSlider_t(0.05f, 0.0f, 0.5f), FeatureFlag_None, "Smaller agression is recommended ( Default is pretty good )")
DEFINE_FEATURE(PeekAssist,           "Peek Assist",                bool,          Movement, Movement, 6, false, FeatureFlag_SupportKeyBind | FeatureFlag_HoldOnlyKeyBind, "Automatically return to safe spot after taking a shot.")
DEFINE_FEATURE(PeekAssist_Clr,       "PeekAssist Marker clr",      ColorData_t,   Movement, Movement, 7, ColorData_t(1.0f, 1.0f, 1.0f, 1.0f))
DEFINE_FEATURE(PeekAssist_RGB,       "PeekAssist RGB",             FloatSlider_t, Movement, Movement, 8, FloatSlider_t(-1.0f, -1.0f, 10.0f))
DEFINE_FEATURE(PeekAssist_Size,      "PeekAssist Size",            FloatSlider_t, Movement, Movement, 8, FloatSlider_t(30.0f, 10.0f, 100.0f))