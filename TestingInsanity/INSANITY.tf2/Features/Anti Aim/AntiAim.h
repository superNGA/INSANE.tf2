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
* -> Render the anti aim chams
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