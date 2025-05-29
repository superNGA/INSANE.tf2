#pragma once

#include "../../FeatureHandler.h"

class CUserCmd;
class BaseEntity;
class baseWeapon;

/*
we will assume that this is the only aimbot we gonna make, 
this will result in bad & not very scalable implementations, 
but this will greatly increase the speed of development.
*/

/*

*/

class AimbotMelee_t
{
public:
    void Run(BaseEntity* pLocalPlayer, baseWeapon* pActiveWeapon, CUserCmd* pCmd, bool* pSendPacket);

    void Reset();

private:
    const BaseEntity* _ChooseTarget(BaseEntity* pLocalPlayer) const;
    const vec _GetClosestPointOnEntity(BaseEntity* pLocalPlayer, const BaseEntity* pEnt) const;

    bool _ShouldSwing(BaseEntity* pLocalPlayer, baseWeapon* pActiveWeapon);
    bool _ShouldSetAngle(BaseEntity* pLocalPlayer, baseWeapon* pActiveWeapon);

    float _GetSwingRange(BaseEntity* pLocalPlayer, baseWeapon* pActiveWeapon);
    float _GetSwingHullRange(BaseEntity* pLocalPlayer, baseWeapon* pActiveWeapon);

    bool _IsPathObstructed(const vec& vStart, const vec& vEnd, BaseEntity* pLocalPlayer);

    void _DrawMeleeHull(BaseEntity* pLocalPlayer, baseWeapon* pActiveWeapon, CUserCmd* pCmd);
    void _DrawMeleeSwingRadius(BaseEntity* pLocalPlayer, baseWeapon* pActiveWeapon);
    void _DrawEyePos(BaseEntity* pLocalPlayer, baseWeapon* pActiveWeapon);
    void _DrawEntityCollisionHull(const BaseEntity* pEnt) const;

    float m_flLastAttackTime = 0.0f;
    bool m_bSwingActive = false;
};
DECLARE_FEATURE_OBJECT(aimbotMelee, AimbotMelee_t)


DEFINE_SECTION(Melee_Aimbot, "Aimbot", 3)

// TODO : make sure this doesn't run when menu is open.
// Only run it when autoFire is enabled.
DEFINE_FEATURE(
    MeleeAimbot, bool, "Melee_Aimbot", "Aimbot", 1, false,
    FeatureFlag_SupportKeyBind | FeatureFlag_DisableWhileMenuOpen,
    "Aims for you when smacking someone.");

DEFINE_FEATURE(
    MeleeAimbot_AutoFire, bool, "Melee_Aimbot", "Aimbot", 2, false,
    FeatureFlag_SupportKeyBind,
    "Hits as soon as a target is found");

DEFINE_FEATURE(
    MeleeAimbot_FOV, FloatSlider_t, "Melee_Aimbot", "Aimbot", 3,
    FloatSlider_t(0.0f, 0.0f, 180.0f), FeatureFlag_SupportKeyBind);

DEFINE_FEATURE(
    MeleeRange_Circle, bool, "Melee_Aimbot", "Aimbot", 4, false,
    FeatureFlag_None, "Draws the melee range for you current weapon");

DEFINE_FEATURE(
    MeleeEyePos, bool, "Melee_Aimbot", "Aimbot", 5, false,
    FeatureFlag_None, "Draws eye pos (Thats where the bullets are fired from)");

DEFINE_FEATURE(
    MeleeRange_HULL, bool, "Melee_Aimbot", "Aimbot", 6, false,
    FeatureFlag_None, "Draw the effective range of you Melee");

DEFINE_FEATURE(
    MeleeDrawCollisionHull, bool, "Melee_Aimbot", "Aimbot", 7, false,
    FeatureFlag_None, "The all-mightly collision box");