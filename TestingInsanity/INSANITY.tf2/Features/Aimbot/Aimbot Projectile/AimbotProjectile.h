#pragma once

#include "../../FeatureHandler.h"
#include "../../../SDK/class/FileWeaponInfo.h"

class BaseEntity;
class baseWeapon;
class CUserCmd;

class ProjectileWeaponInfo_t
{
public:
    void  UpdateWpnInfo(baseWeapon* pActiveWeapon);

    vec   GetShootPosOffset(BaseEntity* pWeaponOwner, int bFlipedViewModels) const;
    float GetProjectileSpeed(BaseEntity* pWeaponOwner) const;
    float GetProjectileGravity(BaseEntity* pWeaponOwner) const;

    void  Reset();

    baseWeapon*      m_pWeapon;
    vec              m_vShootPosOffset;
    float            m_flProjectileBaseSpeed;
    float            m_flProjectileBaseGravityMult;
    float            m_flUpwardVelOffset;
    WeaponData_t*    m_pWeaponFileInfo;

private:
    const vec   _GetWpnBaseShootPosOffset(const baseWeapon * pWeapon, const ProjectileType_t iProjectileType) const;
    const float _GetBaseProjectileSpeed(const WeaponData_t * pWeaponFileInfo, const baseWeapon * pWeapon) const;
    const float _GetBaseProjectileGravityMult(const ProjectileType_t iProjectileType);
    const float _GetUpwardVelocityOffset(ProjectileType_t iProjectileType);
};

class AimbotProjectile_t
{
public:
    void Run(BaseEntity* pLocalPlayer, baseWeapon* pActiveWeapon, CUserCmd* pCmd, bool* pCreatemoveResult);
    void Reset();

private:
    BaseEntity* _ComputeBestTarget(BaseEntity* pLocalPLayer, baseWeapon* pActiveWeapon); 
    float       _GetAngleFromCrosshair(BaseEntity* pLocalPlayer, const vec& vTargetPos);

    bool        _FindBestVisibleHullPoint(BaseEntity* pLocalPlayer, BaseEntity* pTarget, uint32_t iFlags, const vec& vTargetPos, vec& vBestPointOut);
    bool        _SolveProjectileMotion(const vec& vLauchPos, const vec& vTargetPos, const float flProjVelocity, const float flGravity, float & flAngleOut, float & flTimeToReach);

    const vec   _GetShootOffset(BaseEntity* pLocalPlayer, baseWeapon* pActiveWeapon) const;
    float       _GetProjectileSpeed(baseWeapon* pWeapon);

    ProjectileWeaponInfo_t m_weapon;

    BaseEntity* m_pBestTarget = nullptr;
    vec m_vBestTargetFuturePos;
    float m_flBestAimbotPitch = 0.0f; // Angle to set for parabolic path projectiles.

    void _InitliazeCVars();
    bool m_bInitializedCVars = false;
    int  m_bFlipViewModels   = false;
};
DECLARE_FEATURE_OBJECT(aimbotProjectile, AimbotProjectile_t)

// Feature's UI
DEFINE_SECTION(Aimbot_Projectile, "Aimbot", 1);

DEFINE_FEATURE(
    ProjAimbot_Enable, bool, Aimbot_Projectile, Aimbot, 1, false,
    FeatureFlag_SupportKeyBind, "Enables Projectile Aimbot :)"
)

DEFINE_FEATURE(
    ProjAimbot_DebugPrediction, bool, Aimbot_Projectile, Aimbot, 2, false,
    FeatureFlag_None, "Shows predicted position"
)

DEFINE_FEATURE(
    ProjAimbot_HitClosestEnemy, bool, Aimbot_Projectile, Aimbot, 3, false,
    FeatureFlag_None,
    "Targets closest enemy to you, intsead of closest to your crosshair"
)

DEFINE_FEATURE(
    ProjAimbot_FOV, FloatSlider_t, Aimbot_Projectile, Aimbot, 4,
    FloatSlider_t(10.0f, 0.0f, 180.0f),
    FeatureFlag_SupportKeyBind,
    "FOV range for Projectile aimbot target"
)

DEFINE_FEATURE(
    ProjAimbot_DebugMultiPoint, bool, Aimbot_Projectile, Aimbot, 5, false,
    FeatureFlag_None, "Draws all hitpoints searched for"
)