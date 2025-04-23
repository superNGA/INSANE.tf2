#pragma once
#include "../features.h"

class CUserCmd;

class Movement_t
{
public:
    void Run(CUserCmd* pCmd, bool& result);

private:
    void _Bhop(CUserCmd* pCmd, bool& result);
    void _RocketJump(CUserCmd* pCmd, bool& result);
    void _ThirdPerson(CUserCmd* pCmd, bool& result);

};

ADD_FEATURE(movement, Movement_t)