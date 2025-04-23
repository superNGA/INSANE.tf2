#pragma once
#include "../features.h"
#include "../../SDK/class/Basic Structures.h"

class CUserCmd;
class CMultiPlayerAnimState;

/* OBSERVATIONS :
* -> changing eye yaw, will make the character model look at desiered angle, and thsi will also
*       somewhat rotate the body, but we want a full'ly rotated body, not like this.
*/

/*
* TODO : 
* -> taunt yaw fix
* -> scoped in yaw ?
* -> make some macro that allow for easy features addition to config.
*/

class AntiAim_t
{
public:
    void Run(CUserCmd* cmd, bool& bResult);
    void StoreAABones();

    matrix3x4_t pBone[MAX_STUDIO_BONES] = {};

private:
    void _FixMovement(CUserCmd* pCmd);
    qangle m_qAAAngles;

};

ADD_FEATURE(antiAim, AntiAim_t);

MAKE_FEATURE_BOOL(AA_Switch,        "antiaim->global_antiaim",              1)
MAKE_FEATURE_BOOL(AA_chams,         "antiaim->global_antiaim->cham",        2)
MAKE_FEATURE_COLOR(AA_chams_color,  "antiaim->global_antiaim->cham->color", 2)
MAKE_FEATURE_FLOAT(AA_fakePitch,    "antiaim->global_antiaim->fake_pitch",  1, -89.0f, 89.0f)
MAKE_FEATURE_FLOAT(AA_fakeYaw,      "antiaim->global_antiaim->fake_yaw",    1, -180.0f, 180.0f)
MAKE_FEATURE_FLOAT(AA_realYaw,      "antiaim->global_antiaim->real_yaw",    1, -180.0f, 180.0f)
MAKE_FEATURE_FLOAT(AA_realPitch,    "antiaim->global_antiaim->real_pitch",  1, -89.0f, 89.0f)