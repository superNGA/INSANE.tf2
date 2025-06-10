#pragma once
#include "../FeatureHandler.h"
//#include "../../SDK/class/Source Entity.h"

class CUserCmd;
class baseWeapon;
class BaseEntity;

class Movement_t
{
public:
    Movement_t();
    void Reset();

    void Run(CUserCmd* pCmd, bool& result, BaseEntity* pLocalPlayer, baseWeapon* pActiveWeapon);

private:
    void _Bhop(CUserCmd* pCmd, bool& result, BaseEntity* pLocalPlayer);
    void _RocketJump(CUserCmd* pCmd, bool& result, baseWeapon* pActiveWeapon, BaseEntity* pLocalPlayer);
    void _ThirdPerson(CUserCmd* pCmd, bool& result, BaseEntity* pLocalPlayer);

    void _InitializeKeyCodes();

    int32_t m_iJumpKeyCode          = 0;
    bool    m_bInitializedKeyCodes  = false;

    bool    m_bLastBhopState        = false;
    
};

DECLARE_FEATURE_OBJECT(movement, Movement_t)

DEFINE_TAB(Movement, 4);
DEFINE_SECTION(Movement, "Movement", 1)

DEFINE_FEATURE(Bhop,           bool, Movement, Movement, 1, false, FeatureFlag_SupportKeyBind | FeatureFlag_HoldOnlyKeyBind)
DEFINE_FEATURE(AutoRocketJump, bool, Movement, Movement, 2, false, FeatureFlag_SupportKeyBind | FeatureFlag_HoldOnlyKeyBind | FeatureFlag_DisableWhileMenuOpen)
DEFINE_FEATURE(ThirdPerson,    bool, Movement, Movement, 3, false, FeatureFlag_SupportKeyBind | FeatureFlag_ToggleOnlyKeyBind)