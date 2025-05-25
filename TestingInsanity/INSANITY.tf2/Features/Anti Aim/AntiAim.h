#pragma once
#include "../FeatureHandler.h"
#include "../../SDK/class/Basic Structures.h"

class CUserCmd;
class CMultiPlayerAnimState;
class BaseEntity;

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
    void Run(CUserCmd* cmd, bool& bResult, bool* bSendPacket, BaseEntity* pLocalPlayer);
    void StoreAABones(BaseEntity* pLocalPlayer);

    matrix3x4_t pBone[MAX_STUDIO_BONES] = {};
    qangle m_qAAAngles;

private:
    void _FixMovement(CUserCmd* pCmd);

};

DECLARE_FEATURE_OBJECT(antiAim, AntiAim_t);

DEFINE_TAB(AntiAim, 1)
DEFINE_SECTION(AntiAim, "AntiAim", 1)

DEFINE_FEATURE(AntiAim, bool, "AntiAim", "AntiAim", 1, false, FeatureFlag_None, "Makes you harder to hit :)")
DEFINE_FEATURE(Cham,    bool, "AntiAim", "AntiAim", 2, false, FeatureFlag_None, "Draw what enemies see")