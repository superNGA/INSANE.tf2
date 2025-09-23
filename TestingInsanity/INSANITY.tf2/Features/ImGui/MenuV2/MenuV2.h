#pragma once

#include "../../../SDK/class/Basic Structures.h"
#include "../../FeatureHandler.h"

class BoxFilled2D_t;
struct ImVec2;

///////////////////////////////////////////////////////////////////////////
class MenuGUI_t
{
public:
    MenuGUI_t();

    void Draw();
    void SetVisible(bool bVisible);

private:
    bool m_bVisible = true;

    bool   _Initialize();

    ImVec2 _DrawMainBody(float flWidth, float flHeight);
    void   _DrawTabBar(float flWidth, float flHeight, float x, float y);
    void   _DrawSections(Tab_t* pTab, float flWidth, float flHeight, float x, float y);

    ImVec2 _CalculateSectionSize(int nFeatures, float flInterFeaturePadding, float flSectionPadding, float flFeatureWidth, float flFeatureHeight) const;

    // Feature specfic...
    void _DrawBoolean    (IFeature* pFeature) const;
    void _DrawIntSlider  (IFeature* pFeature) const;
    void _DrawFloatSlider(IFeature* pFeature) const;
    void _DrawDropDown   (IFeature* pFeature) const;
    void _DrawColor      (IFeature* pFeature) const;

    void   _CalculateColors();
    void   _StyleSideMenuBottons();
    void   _PopAllStyles();

    Tab_t* m_pActiveTab = nullptr;

    RGBA_t m_clrPrimary;
    RGBA_t m_clrSecondary;
    RGBA_t m_clrTheme;
    RGBA_t m_clrSideMenuButtons;

    int            m_nPushedStyleColors = 0;
    int            m_nPushedStyleVars   = 0;

    BoxFilled2D_t* m_pMainMenu = nullptr;
    BoxFilled2D_t* m_pSideMenu = nullptr;
};
///////////////////////////////////////////////////////////////////////////

DECLARE_CUSTOM_OBJECT(menuGUI, MenuGUI_t, Render)

DEFINE_TAB(Menu, 7)

////////////////////////////// MAIN BODY ////////////////////////////////////
DEFINE_SECTION(Menu, "Menu", 1)
DEFINE_FEATURE(Scale,            FloatSlider_t, Menu, Menu, 1, FloatSlider_t(1.0f, 0.25f, 2.0f))
DEFINE_FEATURE(Blur,             FloatSlider_t, Menu, Menu, 2, FloatSlider_t(0.0f, 0.0f, 8.0f))

DEFINE_FEATURE(ColorTopRight,    ColorData_t,   Menu, Menu, 3, ColorData_t(RGBA_t((unsigned char)255, 255, 255, 255)))
DEFINE_FEATURE(ColorTopLeft,     ColorData_t,   Menu, Menu, 4, ColorData_t(RGBA_t((unsigned char)255, 255, 255, 255)))
DEFINE_FEATURE(ColorBottomRight, ColorData_t,   Menu, Menu, 5, ColorData_t(RGBA_t((unsigned char)255, 255, 255, 255)))
DEFINE_FEATURE(ColorBottomLeft,  ColorData_t,   Menu, Menu, 6, ColorData_t(RGBA_t((unsigned char)255, 255, 255, 255)))

DEFINE_FEATURE(rgb,              bool,          Menu, Menu, 7, false)
DEFINE_FEATURE(RGBSpeed,         FloatSlider_t, Menu, Menu, 8, FloatSlider_t(0.0f, 0.0f, 10.0f))

DEFINE_FEATURE(Rounding,         FloatSlider_t, Menu, Menu, 9, FloatSlider_t(15.0f, 0.0f, 100.0f)) 

////////////////////////////// SIDE MENU ////////////////////////////////////
DEFINE_SECTION(SideMenu, "Menu", 2)
DEFINE_FEATURE(Scale,            FloatSlider_t, SideMenu, Menu, 1, FloatSlider_t(1.0f, 0.25f, 2.0f))
DEFINE_FEATURE(Blur,             FloatSlider_t, SideMenu, Menu, 2, FloatSlider_t(0.0f, 0.0f, 8.0f))

DEFINE_FEATURE(ColorTopRight,    ColorData_t,   SideMenu, Menu, 3, ColorData_t(RGBA_t((unsigned char)255, 255, 255, 255)))
DEFINE_FEATURE(ColorTopLeft,     ColorData_t,   SideMenu, Menu, 4, ColorData_t(RGBA_t((unsigned char)255, 255, 255, 255)))
DEFINE_FEATURE(ColorBottomRight, ColorData_t,   SideMenu, Menu, 5, ColorData_t(RGBA_t((unsigned char)255, 255, 255, 255)))
DEFINE_FEATURE(ColorBottomLeft,  ColorData_t,   SideMenu, Menu, 6, ColorData_t(RGBA_t((unsigned char)255, 255, 255, 255)))

DEFINE_FEATURE(rgb,              bool,          SideMenu, Menu, 7, false)
DEFINE_FEATURE(RGBSpeed,         FloatSlider_t, SideMenu, Menu, 8, FloatSlider_t(0.0f, 0.0f, 10.0f))

DEFINE_FEATURE(Rounding,         FloatSlider_t, SideMenu, Menu, 9, FloatSlider_t(15.0f, 0.0f, 100.0f))

////////////////////////////// THEME ////////////////////////////////////
DEFINE_SECTION(Theme, "Menu", 3)
DEFINE_FEATURE(Theme, ColorData_t, Theme, Menu, 1, ColorData_t(RGBA_t((unsigned char)255, 0, 0, 255)))