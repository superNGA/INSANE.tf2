#pragma once
#include <chrono>

#include "../../../SDK/class/Basic Structures.h"
#include "../../FeatureHandler.h"


class BoxFilled2D_t;
struct ImVec2;
struct ImFont;


///////////////////////////////////////////////////////////////////////////
class MenuGUI_t
{
public:
    MenuGUI_t();

    void Draw();
    void SetVisible(bool bVisible);

    bool ShouldRecordKey() const;
    void ReturnRecordedKey(int64_t iKey);

private:
    bool m_bVisible = true;

    void _StartRecordingKey();
    bool    m_bRecordingKey = false;
    int64_t m_iRecordedKey  = 0;

    bool   _Initialize();
    void   _InitFonts();

    ImVec2 _DrawMainBody(float flWidth, float flHeight);
    void   _DrawTabBar(float flWidth, float flHeight, float x, float y);
    void   _DrawSections(Tab_t* pTab, float flWidth, float flHeight, float x, float y);

    ImVec2 _CalculateSectionSize(int nFeatures, float flInterFeaturePadding, float flSectionPadding, float flFeatureWidth, float flFeatureHeight) const;

    // Feature specfic...
    void _DrawBoolean     (IFeature* pFeature, ImVec2 vMinWithPadding, ImVec2 vMaxWithPadding);
    void _DrawIntSlider   (IFeature* pFeature, ImVec2 vMinWithPadding, ImVec2 vMaxWithPadding);
    void _DrawFloatSlider (IFeature* pFeature, ImVec2 vMinWithPadding, ImVec2 vMaxWithPadding);
    void _DrawDropDown    (IFeature* pFeature, ImVec2 vMinWithPadding, ImVec2 vMaxWithPadding);
    void _DrawColor       (IFeature* pFeature, ImVec2 vMinWithPadding, ImVec2 vMaxWithPadding);

    void _TriggerPopup        (IFeature* pFeature) const;

    // Popups :)..
    void _DrawColorPopup      (IFeature* pFeature);
    void _DrawFloatSliderPopup(IFeature* pFeature);
    void _DrawIntSliderPopup  (IFeature* pFeature);
    void _DrawBooleanPopup    (IFeature* pFeature);

    // Helper functions...
    void _CalculateColors();
    void _CalcTextClrForBg(RGBA_t& vTextClrOut, const RGBA_t& vBgClr) const;
    void _FindElevatedClr(RGBA_t& vClrOut, const RGBA_t& vBGClr) const;

    // Animation 
    void _CalculateAnim(const std::chrono::high_resolution_clock::time_point& animStartTime, float& flAnimationOut, const float flAnimCompleteTime);
    void _ResetAnimation(std::chrono::high_resolution_clock::time_point& animStartTime, float& flAnimationOut);
    std::chrono::high_resolution_clock::time_point m_lastResetTime;
    float m_flAnimation = 0.0f;

    std::chrono::high_resolution_clock::time_point m_popupOpenTime;
    float m_flPopupAnimation = 0.0f;

    Tab_t* m_pActiveTab = nullptr;
    std::vector<BoxFilled2D_t*> m_vecSectionBoxes = {}; // Holds draw objects for secton UI boxes.

    RGBA_t m_clrPrimary;
    RGBA_t m_clrSecondary;
    RGBA_t m_clrTheme;
    RGBA_t m_clrSideMenuButtons;
    RGBA_t m_clrSectionBox;
    RGBA_t m_clrFeatureText;

    int            m_nPushedStyleColors = 0;
    int            m_nPushedStyleVars   = 0;

    BoxFilled2D_t* m_pMainMenu          = nullptr;
    BoxFilled2D_t* m_pSideMenu          = nullptr;

    // Fonts.
    ImFont*        m_pFontFeatures      = nullptr;
    ImFont*        m_pFontSectionName   = nullptr;
    ImFont*        m_pFontSideMenu      = nullptr;
    ImFont*        m_pFontCatagoryName  = nullptr;
    ImFont*        m_pPopupFont         = nullptr;
    ImFont*        m_pTitleFont         = nullptr;
};
///////////////////////////////////////////////////////////////////////////

DECLARE_CUSTOM_OBJECT(menuGUI, MenuGUI_t, Render)

DEFINE_TAB(Menu, 9)


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
DEFINE_FEATURE(Draw_Guides,      bool,          Menu, Menu, 10, true)


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
DEFINE_FEATURE(AnimAccentSize,   FloatSlider_t, SideMenu, Menu, 10, FloatSlider_t(30.0f, 0.0f, 200.0f))


////////////////////////////// THEME ////////////////////////////////////
DEFINE_SECTION(Theme, "Menu", 3)
DEFINE_FEATURE(Theme, ColorData_t, Theme, Menu, 1, ColorData_t(RGBA_t((unsigned char)255, 0, 0, 255)))


////////////////////////////// SECTION BOXES ////////////////////////////////////
DEFINE_SECTION(SectionBoxes, "Menu", 4)
DEFINE_FEATURE(Blur,             FloatSlider_t, SectionBoxes, Menu, 1, FloatSlider_t(0.0f, 0.0f, 8.0f))

DEFINE_FEATURE(ColorTopRight,    ColorData_t,   SectionBoxes, Menu, 2, ColorData_t(RGBA_t((unsigned char)255, 255, 255, 255)))
DEFINE_FEATURE(ColorTopLeft,     ColorData_t,   SectionBoxes, Menu, 3, ColorData_t(RGBA_t((unsigned char)255, 255, 255, 255)))
DEFINE_FEATURE(ColorBottomRight, ColorData_t,   SectionBoxes, Menu, 4, ColorData_t(RGBA_t((unsigned char)255, 255, 255, 255)))
DEFINE_FEATURE(ColorBottomLeft,  ColorData_t,   SectionBoxes, Menu, 5, ColorData_t(RGBA_t((unsigned char)255, 255, 255, 255)))

DEFINE_FEATURE(rgb,              bool,          SectionBoxes, Menu, 6, false)
DEFINE_FEATURE(RGBSpeed,         FloatSlider_t, SectionBoxes, Menu, 7, FloatSlider_t(0.0f, 0.0f, 10.0f))

DEFINE_FEATURE(Rounding,         FloatSlider_t, SectionBoxes, Menu, 8, FloatSlider_t(15.0f, 0.0f, 100.0f), FeatureFlag_SupportKeyBind)
DEFINE_FEATURE(ThemeBorder,      bool,          SectionBoxes, Menu, 9, false)
