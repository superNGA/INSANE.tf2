#pragma once

#include "../../FeatureHandler.h"
#include "../../../SDK/class/FileWeaponInfo.h"

class BaseEntity;
class baseWeapon;
class CUserCmd;
class ProjectileInfo_tV2;

class AimbotProjectile_t
{
public:
    void Run(BaseEntity* pLocalPlayer, baseWeapon* pActiveWeapon, CUserCmd* pCmd, bool* pCreatemoveResult);
    void Reset();

private:
    BaseEntity* _GetBestTarget(ProjectileInfo_tV2& projInfo, BaseEntity* pAttacker, baseWeapon* pWeapon, CUserCmd* pCmd);
    bool _GetBestHitPointOnTargetHull(BaseEntity* pTarget, const vec& vTargetOrigin, ProjectileInfo_tV2& projInfo, vec& vBestPointOut, const vec& vProjectileOrigin, const float flProjVelocity, const float flProjGravity, BaseEntity* pProjectileOwner, baseWeapon* pWeapon);

    float _GetAngleFromCrosshair(const vec& vTargetPos, const vec& vOrigin, const qangle& qViewAngles);
    bool _ShouldAim(BaseEntity* pLocalPlayer, baseWeapon* pActiveWeapon, CUserCmd* pCmd, ProjectileInfo_tV2& projInfo);
    bool m_bLastShouldAim = false;

    bool _SolveProjectileMotion(BaseEntity* pLocalPlayer, baseWeapon* pActiveWeapon, ProjectileInfo_tV2& projInfo, const vec& vTarget, float& flAngleOut, float& flTimeToReachOut);

    const qangle _GetTargetAngles(
        ProjectileInfo_tV2& projInfo, 
        BaseEntity*         pAttacker, 
        baseWeapon*         pWeapon, 
        const qangle&       qViewAngles);
    inline void _ResetTargetData() { m_pBestTarget = nullptr; }
    BaseEntity* m_pBestTarget = nullptr;
    vec         m_vBestTargetFuturePos;

    // CVars
    void  _InitliazeCVars();
    bool  m_bInitializedCVars = false;
    int   m_bFlipViewModels   = false;
    float m_flGravity         = 0.0f;
};
DECLARE_FEATURE_OBJECT(aimbotProjectile, AimbotProjectile_t)



//======================= FEATURE'S UI =======================
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
    ProjAimbot_MaxDistance, FloatSlider_t, Aimbot_Projectile, Aimbot, 3, FloatSlider_t(200.0f, 0.0f, 1000.0f),
    FeatureFlag_None,
    "Max distance to target"
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

DEFINE_FEATURE(
    ProjAimbot_MaxSimulationTime, FloatSlider_t, Aimbot_Projectile, Aimbot, 6,
    FloatSlider_t(2.0f, 0.5f, 5.0f), FeatureFlag_None, 
    "Maximum future position to check for hitability. ( Affects Performance!! )"
)

DEFINE_FEATURE(
    ProjAimbot_SimulateProjectile, bool, Aimbot_Projectile, Aimbot, 7, 
    false, FeatureFlag_SupportKeyBind,
    "Shows the predicted path of projectiles"
)

/*
When enemy at x,y,z

shooting will almsot alwayas cause wrong angles.
*/