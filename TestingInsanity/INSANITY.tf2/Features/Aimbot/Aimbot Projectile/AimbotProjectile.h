#pragma once

#include "../../FeatureHandler.h"
#include "../../../SDK/class/FileWeaponInfo.h"

class BaseEntity;
class baseWeapon;
class CUserCmd;

/*
* TODO : 
-> The Quadractic solver isn't compensating for drag n shit, causing trouble with pipes
-> The set aimbot angle are bad, always going below the desired destination.

-> We are tracing to the perfect position but we can't fire at that IDK why
*/

class ProjectileWeaponInfo_t
{
public:
    ProjectileWeaponInfo_t() { Reset(); }
    void  UpdateWpnInfo(baseWeapon* pActiveWeapon);

    vec   GetProjectileOrigin(BaseEntity* pWeaponOwner, const vec& vShootPosOffset, const qangle& qViewAngles) const;
    vec   GetShootPosOffset(BaseEntity* pWeaponOwner, int bFlipedViewModels) const;
    float GetProjectileSpeed(BaseEntity* pWeaponOwner) const;
    float GetProjectileGravity(BaseEntity* pWeaponOwner, const float flBaseGravity) const;

    bool  IsUpdated(baseWeapon* pWeapon);

    void  Reset();

    baseWeapon*      m_pWeapon;
    vec              m_vShootPosOffset;
    vec              m_vHullSize;
    float            m_flProjectileBaseSpeed;
    float            m_flProjectileBaseGravityMult;
    float            m_flUpwardVelOffset;
    WeaponData_t*    m_pWeaponFileInfo;

private:
    const vec   _GetWpnBaseShootPosOffset(const baseWeapon* pWeapon, const ProjectileType_t iProjectileType) const;
    const float _GetBaseProjectileSpeed(const WeaponData_t* pWeaponFileInfo, const baseWeapon * pWeapon) const;
    const float _GetBaseProjectileGravityMult(const ProjectileType_t iProjectileType);
    const float _GetUpwardVelocityOffset(ProjectileType_t iProjectileType);
    const vec   _GetHullSize(ProjectileType_t iProjectileType);
};

class AimbotProjectile_t
{
public:
    void Run(BaseEntity* pLocalPlayer, baseWeapon* pActiveWeapon, CUserCmd* pCmd, bool* pCreatemoveResult);
    void Reset();

private:
    BaseEntity* _GetBestTarget(const ProjectileWeaponInfo_t& weaponInfo, BaseEntity* pAttacker, CUserCmd* pCmd);
   
    bool _SolveProjectileMotion(
        const vec& vLauchPos,       const vec& vTargetPos, 
        const float flProjVelocity, const float flGravity, 
        float& flAngleOut,          float& flTimeToReach);

    bool _GetBestHitPointOnTargetHull(
        BaseEntity* pTarget,                      const vec& vTargetOrigin,
        const ProjectileWeaponInfo_t& weaponInfo, vec& vBestPointOut,
        const vec& vProjectileOrigin,             const float flProjVelocity,
        const float flProjGravity,                BaseEntity* pProjectileOwner);

    float _GetAngleFromCrosshair(const vec& vTargetPos, const vec& vOrigin, const qangle& qViewAngles);
    
    bool _ShouldAim(BaseEntity* pLocalPlayer, baseWeapon* pActiveWeapon, CUserCmd* pCmd);
    bool m_bLastShouldAim = false;

    ProjectileWeaponInfo_t m_weaponInfo;

    // Target's Data
    void _DrawPredictionHistory();
    void _DrawProjectilePathPred(BaseEntity* pLocalPlayer, baseWeapon* pActiveWeapon);
    void _ClearPredictionHistory() { m_nValidPredictionRecord = 0; }
    std::vector<vec>       m_vecTargetPredictionHistory = {};
    std::vector<vec>       m_vecProjectilePathPred = {};
    bool                   m_bProjectilePathPredicted = false;
    qangle                 m_qLastAimbotAngles;
    uint32_t               m_nValidPredictionRecord = 0;
    uint32_t               m_nValidProjectilePathRecord = 0;
    float                  m_flDrawStartTime = 0.0f;
    static constexpr float flPredictionHistoryDrawingLife = 5.0f;

    const qangle _GetTargetAngles(BaseEntity* pAttacker, const qangle& qViewAngles);
    inline void _ResetTargetData() 
    { 
        m_pBestTarget = nullptr; 
        m_vBestTargetFuturePos.Init(); 
    }
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

DEFINE_FEATURE(
    ProjAimbot_MaxSimulationTime, FloatSlider_t, Aimbot_Projectile, Aimbot, 6,
    FloatSlider_t(2.0f, 0.5f, 5.0f), FeatureFlag_None, 
    "Maximum future position to check for hitability. ( Affects Performance!! )"
)