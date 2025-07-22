#pragma once

#include "../../FeatureHandler.h"

class BaseEntity;
class baseWeapon;
class CUserCmd;

class AimbotHitscan_t
{
public:
    void Run(BaseEntity* pLocalPlayer, baseWeapon* pActiveWeapon, CUserCmd* pCmd, bool* bCreateMoveResult);
    void Reset();

private:
    BaseEntity* _GetBestTarget(BaseEntity* pLocalPlayer, CUserCmd* pCmd);
    const float _GetAngleFromCrosshair(const vec& vTargetPos, const vec & vAttackerEyePos, const qangle& qViewAngles) const;

    BaseEntity* m_pBestTarget = nullptr;
    vec         m_vBestPos    = { 0.0f };
};
DECLARE_FEATURE_OBJECT(aimbotHitscan, AimbotHitscan_t)

DEFINE_SECTION(HitscanAimbot, "Aimbot", 5)

DEFINE_FEATURE(Enable,   bool, HitscanAimbot, Aimbot, 1, false, 
    FeatureFlag_SupportKeyBind)

DEFINE_FEATURE(AutoFire, bool, HitscanAimbot, Aimbot, 2, false, 
    FeatureFlag_SupportKeyBind)

DEFINE_FEATURE(FOV, FloatSlider_t, HitscanAimbot, Aimbot, 3, FloatSlider_t(10.0f, 0.0f, 180.0f),
    FeatureFlag_SupportKeyBind, "FOV Range for aimbot to search")

DEFINE_FEATURE(ForceFail_BackTrack, bool, HitscanAimbot, Aimbot, 4, false)