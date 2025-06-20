#pragma once

#include "../../FeatureHandler.h"

class BaseEntity;
class baseWeapon;
class CUserCmd;

class AimbotProjectile_t
{
public:
    void Run(BaseEntity* pLocalPlayer, baseWeapon* pActiveWeapon, CUserCmd* pCmd, bool* pCreatemoveResult);
    void Reset();

private:
    BaseEntity* _ComputeBestTarget(BaseEntity* pLocalPLayer, baseWeapon* pActiveWeapon); 
    float       _GetAngleFromCrosshair(BaseEntity* pLocalPlayer, const vec& vTargetPos);

    bool _FindBestVisibleHullPoint(BaseEntity* pLocalPlayer, BaseEntity* pTarget, uint32_t iFlags, const vec& vTargetPos, vec& vBestPointOut);

    BaseEntity* _ChooseTarget(BaseEntity* pLocalPlayer);
    const vec   _GetBestTickToAim(BaseEntity* pAttacker, baseWeapon* pActiveWeapon, BaseEntity* pBestTaret, float flProjectileSpeed);

    float _GetProjectileSpeed(baseWeapon* pWeapon);

    void _DrawEntityCollisionHull(BaseEntity* pEnt, int r, int g, int b, int a, float flDuration);
    void _DrawEntityCollisionHull(BaseEntity* pEnt, const vec& vPosition, int r, int g, int b, int a, float flDuration);

    BaseEntity* m_pBestTarget = nullptr;
    vec m_vBestTargetFuturePos;
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