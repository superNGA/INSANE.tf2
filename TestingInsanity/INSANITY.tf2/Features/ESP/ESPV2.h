#pragma once

#include <vector>
#include "../FeatureHandler.h"
#include "../../SDK/class/HitboxDefs.h"


class  IDrawObj_t;
class  BaseEntity;
struct BackTrackRecord_t;
struct mstudiohitboxset_t;


///////////////////////////////////////////////////////////////////////////
struct HitboxDrawObj_t
{
    HitboxDrawObj_t();
    void InitDrawObjs();

    IDrawObj_t* m_hitboxDrawObj[HitboxPlayer_Count];
    size_t      m_nHitbox = HitboxPlayer_Count;
};
///////////////////////////////////////////////////////////////////////////


///////////////////////////////////////////////////////////////////////////
struct SkeletonDrawObj_t
{
    SkeletonDrawObj_t();
    void InitDrawObj();

    IDrawObj_t* m_boneDrawObj[HitboxPlayer_Count - 1];
    size_t      m_nBones = HitboxPlayer_Count - 1;
};
///////////////////////////////////////////////////////////////////////////


///////////////////////////////////////////////////////////////////////////
class ESP_t
{
public:
    ESP_t();

    // NOTE : This esp feature, runs partially in createmove hook and partially in the endscene hook.
    //        In CreateMove we handle esp for buildings n mostly stationary shit like that.
    //        In EndScene hook we handle player esp, cause player esp needs to be smooth, and we can do that
    //        by using bones & not BaseEntity::GetAbsOrigin() every frame to acheieve a real smooth esp.
    //
    void RunCreateMove();
    void RunEndScene();


private:
    void _DrawEspList(std::vector<IDrawObj_t*>& pVecEsp, std::vector<BaseEntity*>* pVecEntities, float flRGBSpeed = -1.0f, float flWidthOffset = 0.0f, float flHeightOffset = 0.0f);
    void _DisableAllEsp(std::vector<IDrawObj_t*>& pVecEsp, size_t iStartIndex);

    
    // Player esp stuff...
    void _DrawPlayerEsp     (BaseEntity* pEnt, size_t iEspIndex, const vec& vViewAngleRight, const BackTrackRecord_t& record, mstudiohitboxset_t* pHitBoxSet);
    void _DrawPlayerHitbox  (BaseEntity* pEnt, size_t iEspIndex, const BackTrackRecord_t& record, mstudiohitboxset_t* pHitBoxSet);
    void _DrawPlayerSkeleton(BaseEntity* pEnt, size_t iEspIndex, const BackTrackRecord_t& record, mstudiohitboxset_t* pHitBoxSet);


    // ESP lists...
    std::vector<IDrawObj_t*> m_vecPlayerEsp     = {};
    std::vector<IDrawObj_t*> m_vecSentryEsp     = {};
    std::vector<IDrawObj_t*> m_vecTeleporterEsp = {};
    std::vector<IDrawObj_t*> m_vecDispenserEsp  = {};


    // Player hitbox draw objects list.
    std::vector<HitboxDrawObj_t> m_vecPlayerHitbox = {};


    // Player skeleton draw objects list.
    std::vector<SkeletonDrawObj_t> m_vecPlayerSkeletons = {};
};
///////////////////////////////////////////////////////////////////////////

DECLARE_FEATURE_OBJECT(esp, ESP_t)
// Player esp settings.
DEFINE_FEATURE(ESPPlayer_Enable,                "ESP",                       bool,          Player, Materials, 3, false, FeatureFlag_None, "Enable esp boxes")
DEFINE_FEATURE(ESPPlayer_ESPColorTopLeft,       "Top-Left clr",              ColorData_t,   Player, Materials, 4, ColorData_t(1.0f, 1.0f, 1.0f, 1.0f))
DEFINE_FEATURE(ESPPlayer_ESPColorTopRight,      "Top-Right clr",             ColorData_t,   Player, Materials, 5, ColorData_t(1.0f, 1.0f, 1.0f, 1.0f))
DEFINE_FEATURE(ESPPlayer_ESPColorBottomLeft,    "Bottom-Left clr",           ColorData_t,   Player, Materials, 6, ColorData_t(1.0f, 1.0f, 1.0f, 1.0f))
DEFINE_FEATURE(ESPPlayer_ESPColorBottomRight,   "Bottom-Right clr",          ColorData_t,   Player, Materials, 7, ColorData_t(1.0f, 1.0f, 1.0f, 1.0f))
DEFINE_FEATURE(ESPPlayer_RGB,                   "RGB",                       FloatSlider_t, Player, Materials, 8, FloatSlider_t(-1.0f,   -1.0f,  10.0f), FeatureFlag_None, "RGB esp boxes")
DEFINE_FEATURE(ESPPlayer_HeightOffset,          "Height Offset",             FloatSlider_t, Player, Materials, 9, FloatSlider_t( 0.0f, -200.0f, 200.0f), FeatureFlag_None, "Don't like box's height? change it.")
DEFINE_FEATURE(ESPPlayer_WidthOffset,           "Width Offset",              FloatSlider_t, Player, Materials, 10, FloatSlider_t( 0.0f, -200.0f, 200.0f), FeatureFlag_None, "I'm pretty sure that this box is thicker than you, but anyways.")
DEFINE_FEATURE(ESPPlayer_DrawHitbox,            "Draw Hitboxes",             bool,          Player, Materials, 11, false, FeatureFlag_None, "Enable esp boxes")
DEFINE_FEATURE(ESPPlayer_DrawSkeleton,          "Draw Skeleton",             bool,          Player, Materials, 12, false, FeatureFlag_None, "Enable esp boxes")
DEFINE_FEATURE(ESPPlayer_SkeletonRGB,           "Skeleton RGB",              FloatSlider_t, Player, Materials, 13, FloatSlider_t(-1.0f,   -1.0f,  10.0f), FeatureFlag_None, "RGB Skeletons")
DEFINE_FEATURE(ESPPlayer_ExtraColorTopLeft,     "Skeleton Top-Left clr",     ColorData_t,   Player, Materials, 14, ColorData_t(1.0f, 1.0f, 1.0f, 1.0f))
DEFINE_FEATURE(ESPPlayer_ExtraColorTopRight,    "Skeleton Top-Right clr",    ColorData_t,   Player, Materials, 15, ColorData_t(1.0f, 1.0f, 1.0f, 1.0f))
DEFINE_FEATURE(ESPPlayer_ExtraColorBottomLeft,  "Skeleton Bottom-Left clr",  ColorData_t,   Player, Materials, 16, ColorData_t(1.0f, 1.0f, 1.0f, 1.0f))
DEFINE_FEATURE(ESPPlayer_ExtraColorBottomRight, "Skeleton Bottom-Right clr", ColorData_t,   Player, Materials, 17, ColorData_t(1.0f, 1.0f, 1.0f, 1.0f))

// Sentry esp settings.
DEFINE_FEATURE(ESPSentry_Enable,           "ESP",           bool,          Sentry,     Materials, 3, false,                                 FeatureFlag_None, "Enable esp boxes")
DEFINE_FEATURE(ESPSentry_RGB,              "RGB",           FloatSlider_t, Sentry,     Materials, 4, FloatSlider_t(-1.0f,   -1.0f,  10.0f), FeatureFlag_None, "RGB esp boxes")
DEFINE_FEATURE(ESPSentry_HeightOffset,     "Height Offset", FloatSlider_t, Sentry,     Materials, 5, FloatSlider_t( 0.0f, -200.0f, 200.0f), FeatureFlag_None, "Don't like box's height? change it.")
DEFINE_FEATURE(ESPSentry_WidthOffset,      "Width Offset",  FloatSlider_t, Sentry,     Materials, 6, FloatSlider_t( 0.0f, -200.0f, 200.0f), FeatureFlag_None, "I'm pretty sure that this box is thicker than you, but anyways.")

// Dispenser esp settings.
DEFINE_FEATURE(ESPDispenser_Enable,        "ESP",           bool,          Dispenser,  Materials, 3, false,                                 FeatureFlag_None, "Enable esp boxes")
DEFINE_FEATURE(ESPDispenser_RGB,           "RGB",           FloatSlider_t, Dispenser,  Materials, 4, FloatSlider_t(-1.0f,   -1.0f,  10.0f), FeatureFlag_None, "RGB esp boxes")
DEFINE_FEATURE(ESPDispenser_HeightOffset,  "Height Offset", FloatSlider_t, Dispenser,  Materials, 5, FloatSlider_t( 0.0f, -200.0f, 200.0f), FeatureFlag_None, "Don't like box's height? change it.")
DEFINE_FEATURE(ESPDispenser_WidthOffset,   "Width Offset",  FloatSlider_t, Dispenser,  Materials, 6, FloatSlider_t( 0.0f, -200.0f, 200.0f), FeatureFlag_None, "I'm pretty sure that this box is thicker than you, but anyways.")

// Teleporter esp settings.
DEFINE_FEATURE(ESPTeleporter_Enable,       "ESP",           bool,          Teleporter, Materials, 3, false,                                 FeatureFlag_None, "Enable esp boxes")
DEFINE_FEATURE(ESPTeleporter_RGB,          "RGB",           FloatSlider_t, Teleporter, Materials, 4, FloatSlider_t(-1.0f,   -1.0f,  10.0f), FeatureFlag_None, "RGB esp boxes")
DEFINE_FEATURE(ESPTeleporter_HeightOffset, "Height Offset", FloatSlider_t, Teleporter, Materials, 5, FloatSlider_t( 0.0f, -200.0f, 200.0f), FeatureFlag_None, "Don't like box's height? change it.")
DEFINE_FEATURE(ESPTeleporter_WidthOffset,  "Width Offset",  FloatSlider_t, Teleporter, Materials, 6, FloatSlider_t( 0.0f, -200.0f, 200.0f), FeatureFlag_None, "I'm pretty sure that this box is thicker than you, but anyways.")