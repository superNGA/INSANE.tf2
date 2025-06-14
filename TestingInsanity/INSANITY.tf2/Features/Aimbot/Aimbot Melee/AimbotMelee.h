#pragma once

#include "../../FeatureHandler.h"

class CUserCmd;
class BaseEntity;
class baseWeapon;

/*
TODO : 
    -> Swing Range optimizations
    -> Fail Safe swing ?


    -> Auto BackStab is just some simple vector maths with 
    comparison to hardcoded numbers.
    -> Just make sure the range is calculated properly.

    -> Also determine under which feature to place it.
*/

class AimbotMelee_t
{
public:
    void Run(BaseEntity* pLocalPlayer, baseWeapon* pActiveWeapon, CUserCmd* pCmd, bool* pSendPacket);
    void RunV2(BaseEntity* pLocalPlayer, baseWeapon* pActiveWeapon, CUserCmd* pCmd, bool* pSendPacket);

    void Reset();

private:
    const BaseEntity* _ChooseTarget(BaseEntity* pLocalPlayer, float flSwingRange) const;
    BaseEntity* _ChooseTarget(BaseEntity* pAttacker, float flSmackDelay, float flSwingRange);

    // Gets Closest point accoring to current position
    const vec _GetClosestPointOnEntity(BaseEntity* pLocalPlayer, const BaseEntity* pEnt) const;

    // Works with future position.
    const vec _GetClosestPointOnEntity(BaseEntity* pAttacker, const vec& vAttackerOrigin, const BaseEntity* pTarget, const vec& vTargetOrigin) const;

    void _DrawPredictionDebugInfo(BaseEntity* pAttacker, baseWeapon* pActiveWeapon, BaseEntity* pTarget);

    float _GetLooseSwingRange(BaseEntity* pLocalPlayer, baseWeapon* pActiveWeapon);
    float _GetSwingHullRange(BaseEntity* pLocalPlayer, baseWeapon* pActiveWeapon);

    bool _IsPathObstructed(const vec& vStart, const vec& vEnd, BaseEntity* pLocalPlayer);

    void _DrawSwingRangeRay(BaseEntity * pLocalPlayer, baseWeapon * pActiveWeapon);
    void _DrawMeleeHull(BaseEntity* pLocalPlayer, baseWeapon* pActiveWeapon, CUserCmd* pCmd);
    void _DrawMeleeSwingRadius(BaseEntity* pLocalPlayer, baseWeapon* pActiveWeapon);
    void _DrawEyePos(BaseEntity* pLocalPlayer, baseWeapon* pActiveWeapon);
    void _DrawEntityCollisionHull(const BaseEntity* pEnt, const vec& vOrigin) const;

    vec m_vBestTargetFuturePos;
    vec m_vAttackerFuturePos;

    BaseEntity* m_pBestTarget  = nullptr;
};
DECLARE_FEATURE_OBJECT(aimbotMelee, AimbotMelee_t)


DEFINE_SECTION(Melee_Aimbot, "Aimbot", 3)

// TODO : make sure this doesn't run when menu is open.
// Only run it ( While menu is enabled ) when autoFire is enabled.
DEFINE_FEATURE(
    MeleeAimbot, bool, Melee_Aimbot, Aimbot, 1, false,
    FeatureFlag_SupportKeyBind | FeatureFlag_DisableWhileMenuOpen,
    "Aims for you when smacking someone.");

DEFINE_FEATURE(
    MeleeAimbot_AutoFire, bool, Melee_Aimbot, Aimbot, 2, false,
    FeatureFlag_SupportKeyBind,
    "Hits as soon as a target is found");

DEFINE_FEATURE(
    MeleeAimbot_DebugPrediction, bool, Melee_Aimbot, Aimbot, 3, false,
    FeatureFlag_None, "Draws future position for locked targets"
)

//DEFINE_FEATURE(
//    MeleeAimbot_FOV, FloatSlider_t, Melee_Aimbot, Aimbot, 3,
//    FloatSlider_t(0.0f, 0.0f, 180.0f), FeatureFlag_SupportKeyBind);
//
//DEFINE_FEATURE(
//    MeleeRange_Circle, bool, Melee_Aimbot, Aimbot, 4, false,
//    FeatureFlag_None, "Draws the melee range for you current weapon");
//
//DEFINE_FEATURE(
//    MeleeEyePos, bool, Melee_Aimbot, Aimbot, 5, false,
//    FeatureFlag_None, "Draws eye pos (Thats where the bullets are fired from)");
//
//DEFINE_FEATURE(
//    MeleeRange_HULL, bool, Melee_Aimbot, Aimbot, 6, false,
//    FeatureFlag_None, "Draw the effective range of you Melee");
//
//DEFINE_FEATURE(
//    MeleeDrawCollisionHull, bool, Melee_Aimbot, Aimbot, 7, false,
//    FeatureFlag_None, "The all-mightly collision box");
//
//DEFINE_FEATURE(
//    Melee_Swing_Range_Ray, bool, Melee_Aimbot, Aimbot, 8, false,
//    FeatureFlag_None, "Draws a line showing your melee range");