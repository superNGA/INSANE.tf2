#pragma once

#include "../FeatureHandler.h"
#include "../../SDK/class/Basic Structures.h"


///////////////////////////////////////////////////////////////////////////
class WorldColorModulation_t
{
public:
    WorldColorModulation_t();

    void Run();
    void GetWorldColorModulation(Vec4* pClrOut) const;
    vec* GetAmbientLight();

private:
    RGBA_t m_lastClr;
    vec    m_vAmbientLights[6];

};
///////////////////////////////////////////////////////////////////////////

DECLARE_FEATURE_OBJECT(worldColorMod, WorldColorModulation_t)

DEFINE_SECTION(World, "Materials", 5)
DEFINE_FEATURE(World_ColorMod,              "World Color",       ColorData_t, World, Materials, 1, ColorData_t((unsigned char)255, 255, 255, 255), FeatureFlag_None, "World material color override.")
DEFINE_FEATURE(World_StaticPropLightBack,   "Prop Light Back",   ColorData_t, World, Materials, 2, ColorData_t((unsigned char)255, 255, 255, 255), FeatureFlag_None, "Static-Prop ambient lighting ( Back ).")
DEFINE_FEATURE(World_StaticPropLightFront,  "Prop Light Front",  ColorData_t, World, Materials, 3, ColorData_t((unsigned char)255, 255, 255, 255), FeatureFlag_None, "Static-Prop ambient lighting ( Front ).")
DEFINE_FEATURE(World_StaticPropLightLeft,   "Prop Light Left",   ColorData_t, World, Materials, 4, ColorData_t((unsigned char)255, 255, 255, 255), FeatureFlag_None, "Static-Prop ambient lighting ( Left ).")
DEFINE_FEATURE(World_StaticPropLightRight,  "Prop Light Right",  ColorData_t, World, Materials, 5, ColorData_t((unsigned char)255, 255, 255, 255), FeatureFlag_None, "Static-Prop ambient lighting ( Right ).")
DEFINE_FEATURE(World_StaticPropLightTop,    "Prop Light Top",    ColorData_t, World, Materials, 6, ColorData_t((unsigned char)255, 255, 255, 255), FeatureFlag_None, "Static-Prop ambient lighting ( Top ).")
DEFINE_FEATURE(World_StaticPropLightBottom, "Prop Light Bottom", ColorData_t, World, Materials, 7, ColorData_t((unsigned char)255, 255, 255, 255), FeatureFlag_None, "Static-Prop ambient lighting ( Bottom ).")