#pragma once
#include "../features.h"
//#include "../../SDK/class/Source Entity.h"

class CUserCmd;
class baseWeapon;
class BaseEntity;

class Movement_t
{
public:
    void Run(CUserCmd* pCmd, bool& result, BaseEntity* pLocalPlayer, baseWeapon* pActiveWeapon);

private:
    void _Bhop(CUserCmd* pCmd, bool& result, BaseEntity* pLocalPlayer);
    void _RocketJump(CUserCmd* pCmd, bool& result, baseWeapon* pActiveWeapon);
    void _ThirdPerson(CUserCmd* pCmd, bool& result, BaseEntity* pLocalPlayer);

};

ADD_FEATURE(movement, Movement_t)