#pragma once

#include "../../FeatureHandler.h"


// Forward Declerations...
class  CUserCmd;
class  BaseEntity;
class  baseWeapon;
struct mstudiobbox_t;


///////////////////////////////////////////////////////////////////////////
class AimbotHitscanV2_t
{
public:
    void Run(CUserCmd* pCmd, BaseEntity* pLocalPlayer, baseWeapon* pActiveWeapon, bool* pCreateMoveResult);

private:
    void        _DoAutoScope(BaseEntity* pLocalPlayer, baseWeapon* pActiveWeapon, CUserCmd* pCmd);
    bool        _ShouldAutoFire(BaseEntity* pLocalPlayer, baseWeapon* pActiveWeapon);
    float       _EstimateSniperDamage(BaseEntity* pLocalPlayer, baseWeapon* pActiveWeapon);

    BaseEntity* _ChooseTarget(BaseEntity* pLocalPlayer, baseWeapon* pActiveWeapon, CUserCmd* pCmd);
    BaseEntity* _ChoosePlayerTarget(BaseEntity* pLocalPlayer, baseWeapon* pActiveWeapon, CUserCmd* pCmd);
    BaseEntity* _ChooseBuildingTarget(const std::vector<BaseEntity*>* vecTargets, BaseEntity* pLocalPlayer, baseWeapon* pActiveWeapon, CUserCmd* pCmd);

    void        _ShootAtTarget(BaseEntity* pLocalPlayer, CUserCmd* pCmd, bool* pCreateMoveResult);

    void        _SortTargetList(const std::vector<BaseEntity*>* vecSource, std::vector<BaseEntity*>& vecDestination, BaseEntity* pLocalPlayer, const qangle& qViewAngles);
    void        _ConstructHitboxPriorityList(std::vector<int>* vecHitboxes);

    bool        _IsInFOV(const matrix3x4_t* bone, mstudiobbox_t* pHitbox, const vec& vAttackerEyePos, const qangle& qAttackerViewAngles) const;
    float       _GetAngleFromCrosshair(const vec& vTargetPos, const vec& vAttackerEyePos, const qangle& qViewAngles) const;

    bool        _IsVisible(const matrix3x4_t* bone, mstudiobbox_t* pHitbox, const vec& vAttackerEyePos, vec& vBestTargetPosOut, BaseEntity* pLocalPlayer, BaseEntity* pTarget);
    
    
    BaseEntity* m_pBestTarget = nullptr;
    vec         m_vBestTargetPos;
    int         m_iTargetHitbox = -1;

};
///////////////////////////////////////////////////////////////////////////

DECLARE_FEATURE_OBJECT(aimbotHitscanV2, AimbotHitscanV2_t)

DEFINE_SECTION(AimbotHitscanV2, "Aimbot", 6)
DEFINE_FEATURE(AimbotHitscan_Enable,    "Aimbot",     bool, AimbotHitscanV2, Aimbot, 1, false, FeatureFlag_SupportKeyBind, "Enables Aimbot")
DEFINE_FEATURE(AimbotHitscan_AutoFire,  "Auto-Fire",  bool, AimbotHitscanV2, Aimbot, 2, false, FeatureFlag_SupportKeyBind, "Automatically fire when target found.")
DEFINE_FEATURE(AimbotHitscan_FOV,       "FOV",        FloatSlider_t, AimbotHitscanV2, Aimbot, 3, FloatSlider_t(10.0f, 0.0f, 180.0f), FeatureFlag_SupportKeyBind, "FOV for aimbot.")
DEFINE_FEATURE(AimbotHitscan_SilentAim, "Silent Aim", bool, AimbotHitscanV2, Aimbot, 4, false, FeatureFlag_None, "Supress view angle changes on client.")

static const char* g_szTargetPriority[] = { "Closet to you" , "Closest to crosshair" };
DEFINE_FEATURE(AimbotHitscan_TargetPriority, "Target Priority", DropDown_t, AimbotHitscanV2, Aimbot, 5, DropDown_t(g_szTargetPriority, 2), FeatureFlag_None, "Which target gets checked first.")

// A negative max distance means no max distance check...
DEFINE_FEATURE(AimbotHitscan_MaxDistance, "Max Distance", FloatSlider_t, AimbotHitscanV2, Aimbot, 6, FloatSlider_t(-1.0f, -1.0f, 3000.0f), FeatureFlag_SupportKeyBind, "FOV for aimbot.")
DEFINE_FEATURE(AimbotHitscan_DontShootUnscoped,  "Don't shoot unscoped", bool, AimbotHitscanV2, Aimbot, 7, false, FeatureFlag_None, "Won't auto fire while not scoped")
DEFINE_FEATURE(AimbotHitscan_AutoScope,          "Auto-Scope", bool, AimbotHitscanV2, Aimbot, 8, false, FeatureFlag_None, "Automatically Zoom in if target found.")
DEFINE_FEATURE(AimbotHitscan_AutoFireWhenLethal, "Auto-Fire if Lethal", bool, AimbotHitscanV2, Aimbot, 9, false, FeatureFlag_None, "Only fire if shot if lethal or fully charged. ( only for sniper )")



// Hit-Box seletion for user.
DEFINE_SECTION(AimbotHitscan_Hitbox, "Aimbot", 7)
DEFINE_FEATURE(Hitbox_Head,       "Head",       bool, AimbotHitscan_Hitbox, Aimbot, 1, false)
DEFINE_FEATURE(Hitbox_Torso,      "Torso",      bool, AimbotHitscan_Hitbox, Aimbot, 2, false)
DEFINE_FEATURE(Hitbox_Arms,       "Arms",       bool, AimbotHitscan_Hitbox, Aimbot, 3, false)
DEFINE_FEATURE(Hitbox_Legs,       "Legs",       bool, AimbotHitscan_Hitbox, Aimbot, 4, false)
DEFINE_FEATURE(Hitbox_Sentry,     "Sentry",     bool, AimbotHitscan_Hitbox, Aimbot, 5, false)
DEFINE_FEATURE(Hitbox_Dispenser,  "Dispenser",  bool, AimbotHitscan_Hitbox, Aimbot, 6, false)
DEFINE_FEATURE(Hitbox_Teleporter, "Teleporter", bool, AimbotHitscan_Hitbox, Aimbot, 7, false)