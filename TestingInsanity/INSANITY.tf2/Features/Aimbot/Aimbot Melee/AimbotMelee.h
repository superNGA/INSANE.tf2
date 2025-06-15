//=========================================================================
//                      MELEE AIMBOT
//=========================================================================
// by      : INSANE
// created : 14/06/2025
// 
// purpose : Hits enemy perfectly with melee weapons ( doesn't contain auto backstab )
//-------------------------------------------------------------------------
#pragma once
#include "../../FeatureHandler.h"

class CUserCmd;
class BaseEntity;
class baseWeapon;

// TODO : Swing range optimization ( We can use the corner of the 
//                                  melee swing hull for more range. )

class AimbotMelee_t
{
public:
    void RunV3(BaseEntity* pLocalPlayer, baseWeapon* pActiveWeapon, CUserCmd* pCmd, bool* pCreatemoveResult);
    void Reset();

private:
    BaseEntity* _ChooseTarget(BaseEntity* pAttacker, float flSmackDelay, float flSwingRange);
    
    bool _ShouldAim(BaseEntity* pAttacker, baseWeapon* pActiveWeapon, CUserCmd* pCmd);

    // Determines closest point on enemy collision hull from our Eye pos.
    const vec _GetClosestPointOnEntity(BaseEntity* pLocalPlayer, const BaseEntity* pEnt) const;
    const vec _GetClosestPointOnEntity(BaseEntity* pAttacker, const vec& vAttackerEyePos, const BaseEntity* pTarget, const vec& vTargetOrigin) const;

    void _DrawPredictionDebugInfo(BaseEntity* pAttacker, baseWeapon* pActiveWeapon, BaseEntity* pTarget);

    // Determines swing range for our current weapon.
    float _GetLooseSwingRange(BaseEntity* pLocalPlayer, baseWeapon* pActiveWeapon);
    float _GetSwingHullRange(BaseEntity* pLocalPlayer, baseWeapon* pActiveWeapon);
 
    bool _CanBackStab(BaseEntity* pAttacker, BaseEntity* pTarget);

    vec         m_vAttackerFutureEyePos;
    vec         m_vBestTargetFuturePos;
    vec         m_vBestTargetPosAtLock;
    BaseEntity* m_pBestTarget  = nullptr;
};
DECLARE_FEATURE_OBJECT(aimbotMelee, AimbotMelee_t)


DEFINE_SECTION(Melee_Aimbot, "Aimbot", 3)

DEFINE_FEATURE(
    MeleeAimbot, bool, Melee_Aimbot, Aimbot, 1, false,
    FeatureFlag_SupportKeyBind | FeatureFlag_DisableWhileMenuOpen,
    "Aims for you when smacking someone."
)

DEFINE_FEATURE(
    MeleeAimbot_AutoFire, bool, Melee_Aimbot, Aimbot, 2, false,
    FeatureFlag_SupportKeyBind,
    "Hits as soon as a target is found"
)

DEFINE_FEATURE(
    MeleeAimbot_OnlyDoBackStabs_Spy, bool, Melee_Aimbot, Aimbot, 3, false,
    FeatureFlag_SupportKeyBind,
    "Only Allow back stabs with spy"
)

DEFINE_FEATURE(
    MeleeAimbot_DebugPrediction, bool, Melee_Aimbot, Aimbot, 4, false,
    FeatureFlag_None, "Draws future position for locked targets"
)