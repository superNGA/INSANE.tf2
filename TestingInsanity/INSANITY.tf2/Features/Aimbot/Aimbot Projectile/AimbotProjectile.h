#pragma once

#include "../../FeatureHandler.h"
#include "../../../Extra/math.h"
#include "../../../SDK/class/FileWeaponInfo.h"

class BaseEntity;
class baseWeapon;
class CUserCmd;
class ProjectileInfo_t;
struct GraphicInfo_t;


class TrajactoryLUT_t
{
public:
    void Initialize(BaseEntity* pLocalPlayer, baseWeapon* pActiveWeapon);
    void Delete();

    // Make sure x & y are relative to projetile origin0;
    float GetTravelTime(const float x, const float y, bool bInterpolation = true);

    // Debugging fns
    void DrawClosestPointAvialable(const vec& vOrigin, const vec& vTarget);

private:
    // -1 means not in range.
    float _SafeGetter(uint32_t iRow, uint32_t iCol);

    void _Set(float x, float y, float flTimeToReach, float flAngle);
    void _Fill(BaseEntity* pLocalPlayer, baseWeapon* pActiveWeapon);
    void _GetMaxRange(BaseEntity* pLocalPlayer, baseWeapon* pActiveWeapon, float& flMaxHeightOut, float& flMinHeightOut, float& flMaxRangeOut);
    void _AllocToLUT(BaseEntity* pLocalPlayer, baseWeapon* pActiveWeapon);

    // Tracks LUT fill progress
    float*      m_pTrajactoryLUT       = nullptr;
    float       m_flSimAngle           = MIN_PITCH;
    bool        m_bFilledTrajactoryLUT = false;
    uint32_t    m_iStepSize            = /*40*/30;

    // Weapon
    baseWeapon* pWeapon     = nullptr;
    uint32_t    m_iWpnDefID = 0;

    // LUT size
    float    m_flMaxHeight = 0.0f;
    float    m_flMinHeight = 0.0f;
    float    m_flMaxRange  = 0.0f;

    uint32_t m_nLUTCols         = 0;
    uint32_t m_nLUTRows         = 0;
    uint32_t m_iLUTRowForZeroY  = 0; // this is the row index for Y = 0.

    const uint32_t m_iMaxMemInBytes = 60 * 1024; // 60 KiBs

    /*
    NOTE : This trajactory Look up table is meant to be a 2D array, with rows 
            representing height of the projectile relative of projectile origin 
            at a step size of 40 
            AND
            colums represnting 2D distance ( distance in the x,y plane ) of projectile 
            relative to the projectile origin.
    */
};


class AimbotProjectile_t
{
public:
    void Run(BaseEntity* pLocalPlayer, baseWeapon* pActiveWeapon, CUserCmd* pCmd, bool* pCreatemoveResult);
    void Reset();
    void DeleteProjLUT();

private:
    BaseEntity* _GetBestTarget(ProjectileInfo_t& projInfo, BaseEntity* pAttacker, baseWeapon* pWeapon, CUserCmd* pCmd);
    bool _GetBestHitPointOnTargetHull(BaseEntity* pTarget, const vec& vTargetOrigin, ProjectileInfo_t& projInfo, vec& vBestPointOut, const vec& vProjectileOrigin, const float flProjVelocity, const float flProjGravity, BaseEntity* pProjectileOwner, baseWeapon* pWeapon);

    float _GetAngleFromCrosshair(const vec& vTargetPos, const vec& vOrigin, const qangle& qViewAngles);
    bool _ShouldAim(BaseEntity* pLocalPlayer, baseWeapon* pActiveWeapon, CUserCmd* pCmd, ProjectileInfo_t& projInfo);
    bool m_bLastShouldAim = false;

    bool _SolveProjectileMotion(BaseEntity* pLocalPlayer, baseWeapon* pActiveWeapon, ProjectileInfo_t& projInfo, const vec& vTarget, float& flAngleOut, float& flTimeToReachOut);

    // This is trajactory's look up table, used to estimate the time to reach for drag VPhyics projectiles ( which are affected by drag ).
    TrajactoryLUT_t m_lutTrajactory;

    // Drawing funcitons ( RGB drawing!, cool init )
    void _DrawTargetFuturePos(GraphicInfo_t * pGraphicInfo, const qangle& qNormal);
    void _DrawTargetPath(const qangle& qNormal, GraphicInfo_t* pGraphicInfo);
    void _DrawProjectilePath(BaseEntity* pLocalPlayer, baseWeapon* pActiveWeapon, const qangle& qProjLauchAngles, GraphicInfo_t* pGraphicInfo) const;
    std::vector<vec> m_vecTargetPathRecord = {};


    // Target data...
    const qangle _GetTargetAngles(
        ProjectileInfo_t&   projInfo, 
        BaseEntity*         pAttacker, 
        baseWeapon*         pWeapon, 
        const qangle&       qViewAngles);
    inline void _ResetTargetData() { m_pBestTarget = nullptr; }
    BaseEntity* m_pBestTarget = nullptr;
    vec         m_vBestTargetFuturePos;
    vec         m_vFutureFootPos;


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


DEFINE_SECTION(Visuals, "Aimbot", 11);


DEFINE_FEATURE(Speed, FloatSlider_t, Visuals, Aimbot,
    1, FloatSlider_t(100.0f, 0.0f, 500.0f), FeatureFlag_None,
    "RGB speed")

DEFINE_FEATURE(Thickness, FloatSlider_t, Visuals, Aimbot,
    2, FloatSlider_t(5.0f, 1.0f, 100.0f), FeatureFlag_None,
    "ESP border thickness")

DEFINE_FEATURE(CLR1, ColorData_t, Visuals, Aimbot,
    3, ColorData_t(0.0f, 0.0f, 0.0f, 0.0f), FeatureFlag_None,
    "TOP_LEFT corner clr")
DEFINE_FEATURE(CLR2, ColorData_t, Visuals, Aimbot,
    4, ColorData_t(0.0f, 0.0f, 0.0f, 0.0f), FeatureFlag_None,
    "TOP_RIGHT corner clr")
DEFINE_FEATURE(CLR3, ColorData_t, Visuals, Aimbot,
    5, ColorData_t(0.0f, 0.0f, 0.0f, 0.0f), FeatureFlag_None,
    "BOTTON_LEFT corner clr")
DEFINE_FEATURE(CLR4, ColorData_t, Visuals, Aimbot,
    6, ColorData_t(0.0f, 0.0f, 0.0f, 0.0f), FeatureFlag_None,
    "BOTTOM_RIGHT corner clr")

DEFINE_FEATURE(GlowPower, FloatSlider_t, Visuals, Aimbot,
    7, FloatSlider_t(3.0f, 0.0f, 25.0f), FeatureFlag_None,
    "Glow power maybe, IDK")