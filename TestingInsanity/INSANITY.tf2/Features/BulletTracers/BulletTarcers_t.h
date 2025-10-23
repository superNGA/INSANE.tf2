#pragma once
#include "../FeatureHandler.h"

#include <vector>

class BaseEntity;
class baseWeapon;


///////////////////////////////////////////////////////////////////////////
class TracerHandler_t
{
public:
    void BeginTracer(int iPlayerIndex, baseWeapon* pWeapon);
    void EndTracer();

    const char* m_vecFancyBulletTracers[13] =
    {
        "bullet_bignasty_tracer01_blue", 
        "bullet_bignasty_tracer01_red",
        "tfc_sniper_distortion_trail", 
        "dxhr_sniper_rail",
        "dxhr_sniper_rail_red", 
        "dxhr_sniper_rail_blue",
        "dxhr_lightningball_hit_zap_red", 
        "dxhr_lightningball_hit_zap_blue",
        "bullet_tracer_raygun_red", 
        "bullet_tracer_raygun_blue",
        "merasmus_zap", 
        "merasmus_zap_beam01", 
        "merasmus_zap_beam02"
    };

    const char* GetActiveTracer()  const;
    int         GetAttackerIndex() const;
    bool        IsShotActive()     const;  

    // This function is called in level_init. This causes the _Initialize() function
    // to run once again, and forces the slider widget's max values to be clamped again.
    // This is done because we are not sure if all maps load the same tracers or not?
    void        InvalidateTracerCount();

private:
    int m_iLastLocalPlayerTracer = -1;
    int m_iLastTeammateTracer    = -1;
    int m_iLastEnemyTracer       = -1;
    
    const char* m_szActiveTracerIndex = nullptr;
    int         m_iAttackerEntIndex   = 0;

    bool _Initialize();
    bool m_bInit       = false;
    bool m_bShotActive = false;

};
///////////////////////////////////////////////////////////////////////////

DECLARE_FEATURE_OBJECT(tracerHandler, TracerHandler_t)

DEFINE_SECTION(BulletTracers, "Misc", 4)

// -1 means default bullet tracer.
DEFINE_FEATURE(View_YourFancyTracers,     "Your Tracers",         DropDown_t,  BulletTracers, Misc, 1, DropDown_t(F::tracerHandler.m_vecFancyBulletTracers, 13));
DEFINE_FEATURE(View_YourTracers,          "Your all Tracers",     IntSlider_t, BulletTracers, Misc, 2, IntSlider_t(-1, 0, 0));
DEFINE_FEATURE(View_TeamatesFancyTracers, "Teammate Tracers",     DropDown_t,  BulletTracers, Misc, 3, DropDown_t(F::tracerHandler.m_vecFancyBulletTracers, 13));
DEFINE_FEATURE(View_TeammatesTracers,     "Teammate all Tracers", IntSlider_t, BulletTracers, Misc, 4, IntSlider_t(-1, 0, 0));
DEFINE_FEATURE(View_EnemyFancyTracers,    "Enemy Tracers",        DropDown_t,  BulletTracers, Misc, 5, DropDown_t(F::tracerHandler.m_vecFancyBulletTracers, 13));
DEFINE_FEATURE(View_EnemyTracers,         "Enemy all Tracers",    IntSlider_t, BulletTracers, Misc, 6, IntSlider_t(-1, 0, 0));

DEFINE_FEATURE(View_Bullshitin,           "Bullshitin",           FloatSlider_t, BulletTracers, Misc, 7, FloatSlider_t(0.0f, 0.0f, 1000.0f));