//=========================================================================
//                      AIMBOT HELPER
//=========================================================================
// by      : INSANE
// created : 25/05/2025
// 
// purpose : Handles some common stuff for all different aimbots
//-------------------------------------------------------------------------
#pragma once

// STD Libs
#include <vector>
#include <unordered_map>

// SDK
#include "../FeatureHandler.h"


class CUserCmd;
class baseWeapon;
class BaseEntity;


///////////////////////////////////////////////////////////////////////////
class AimbotHelper_t
{
public:
    void Run(BaseEntity* pLocalPlayer, baseWeapon* pActiveWeapon, CUserCmd* pCmd, bool* pCreateMoveResult);

    struct AimbotTargetData_t
    {
        std::vector<BaseEntity*> m_vecEnemyPlayers;
        std::vector<BaseEntity*> m_vecFriendlyPlayers;

        std::vector<BaseEntity*> m_vecEnemySentry;
        std::vector<BaseEntity*> m_vecFriendlySentry;

        std::vector<BaseEntity*> m_vecEnemyDispensers;
        std::vector<BaseEntity*> m_vecFriendlyDispensers;

        std::vector<BaseEntity*> m_vecEnemyTeleporters;
        std::vector<BaseEntity*> m_vecFriendlyTeleporters;

        std::vector<BaseEntity*> m_vecEnemyRockets;
        std::vector<BaseEntity*> m_vecFriendlyRockets;

        std::vector<BaseEntity*> m_vecEnemyPipeBombs;
        std::vector<BaseEntity*> m_vecFriendlyPipeBombs;
    };

    inline const AimbotTargetData_t& GetAimbotTargetData()
    {
        _ClearAimbotData();
        _ConstructAimbotTargetData();
        return m_aimbotTargetData;
    }

    // Store's the passed in FOV, to be used in FOV-circle's radius calculation later.
    void NotifyGameFOV(const float flFOV);

private:
    AimbotTargetData_t m_aimbotTargetData;

    void _ConstructAimbotTargetData();
    void _ClearAimbotData();

    void _DrawFOVCircle(const float FOV, bool bTargetFound);
    float m_flGameFOV = -1.0f;
};
///////////////////////////////////////////////////////////////////////////


DECLARE_FEATURE_OBJECT(aimbotHelper, AimbotHelper_t)

DEFINE_TAB(Aimbot, 1)

DEFINE_SECTION(View, "Misc", 2)
DEFINE_FEATURE(View_FOV,          "FOV",       FloatSlider_t, View, Misc, 1, FloatSlider_t(90.0f, 0.0f, 180.0f),  FeatureFlag_None);
DEFINE_FEATURE(View_ScopedInFOV,  "Scope FOV", FloatSlider_t, View, Misc, 2, FloatSlider_t(30.0f, 0.0f, 180.0f),  FeatureFlag_None);
DEFINE_FEATURE(View_zNear,        "Z Near",    FloatSlider_t, View, Misc, 3, FloatSlider_t(-1.0f, -1.0f, 1000.0f), FeatureFlag_None);
DEFINE_FEATURE(View_zFar,         "Z Far",     FloatSlider_t, View, Misc, 4, FloatSlider_t(-1.0f, -1.0f, 1000.0f), FeatureFlag_None);

// View model settings...
DEFINE_FEATURE(View_ViewModelFOV,   "View Model FOV",   FloatSlider_t, View, Misc, 5, FloatSlider_t(30.0f, 0.0f, 180.0f),  FeatureFlag_None);
DEFINE_FEATURE(View_ViewModelZNear, "ViewModel Z Near", FloatSlider_t, View, Misc, 6, FloatSlider_t(-1.0f, -1.0f, 1000.0f), FeatureFlag_None);
DEFINE_FEATURE(View_ViewModelZFar,  "ViewModel Z Far",  FloatSlider_t, View, Misc, 7, FloatSlider_t(-1.0f, -1.0f, 1000.0f), FeatureFlag_None);

DEFINE_FEATURE(View_AlwaysDrawViewModel,  "Always Draw View Model",  bool, View, Misc, 8, false, FeatureFlag_None)
DEFINE_FEATURE(View_NoHud,                "No Hud",                  bool, View, Misc, 9, false, FeatureFlag_None, "Might cause some color issues, IDK WHY")

DEFINE_FEATURE(View_DrawModelWhileScoped, "Draw Model when scoped",  bool, View, Misc, 10, false, FeatureFlag_None, "Draws local player's model if thirdperson and zoomed-in")
