//=========================================================================
//                      MENU UI
//=========================================================================
// by      : INSANE
// created : 01/10/2025
// 
// purpose : Renders UI and stores useful info. to keep UI consistent across all widgets.
//-------------------------------------------------------------------------
#pragma once
#include <chrono>

#include "../../../SDK/class/Basic Structures.h"
#include "../../FeatureHandler.h"
#include "../AnimationHandler.h"


class BoxFilled2D_t;
struct ImVec2;
struct ImFont;

constexpr float MENU_PADDING_IN_PXL       = 10.0f; // Padding between main menu & anything near it.

constexpr float WIDGET_ROUNDING           =  3.0f;
constexpr float WIDGET_BORDER_THICKNESS   =  1.5f;
constexpr float POPUP_ROUNDING            =  5.0f;
constexpr float POPUP_BORDER_THICKNESS    =  1.5f;
constexpr float TRACK_THICKNESS_PXL       =  4.0f;
constexpr float KNOB_SIZE_PXL             = 10.0f;

constexpr float SIDEMENU_SCALE            =  0.2f; // Percentage of main body allocated to side menu.
constexpr float FRAME_PADDING_PXL         = 15.0f; // Padding between Main body & its contents.
constexpr float SECTION_PADDING_PXL       = 10.0f; // Padding between Section walls & its contents.
constexpr float INTER_FEATURE_PADDING_PXL =  5.0f; // Padding between each feature.
constexpr float FEATURE_PADDING_PXL       =  5.0f; // Padding between feautres walls & its contents.
constexpr float FEATURE_HEIGHT            = 30.0f; // Height of each feature.
constexpr float SECTION_NAME_PADDING      = 10.0f; // Padding above and below section names in main body. 

constexpr float TAB_NAME_PADDING_IN_PXL   =  8.0f; // Padding above and below a tab's name in side menu.
constexpr float CTG_NAME_PADDING_IN_PXL   = 20.0f;


///////////////////////////////////////////////////////////////////////////
class MenuGUI_t
{
public:
    MenuGUI_t();

    void Draw();
    void SetVisible(bool bVisible);
    bool IsVisible() const;

    bool ShouldRecordKey() const;
    void ReturnRecordedKey(int64_t iKey);

    void GetPos(float& x, float& y) const;
    void GetSize(float& flWidth, float& flHeight) const;

    RGBA_t GetPrimaryClr()   const;
    RGBA_t GetSecondaryClr() const;
    RGBA_t GetThemeClr()     const;

    void CalcTextClrForBg(RGBA_t& vTextClrOut, const RGBA_t& vBgClr) const;
    void _FindElevatedClr(RGBA_t& vClrOut, const RGBA_t& vBGClr) const;

    // General purpose widget rendering functions ( takes care of all colors n stuff )
    bool DrawIntSlider(const char* szLabel, ImVec2 vMin, ImVec2 vMax, int* pData, const int iMin, const int iMax, RGBA_t clrBackground, const float* pTrackThickness = nullptr, const float* pKnowSize = nullptr, const float* pAnimationState = nullptr);
    void DrawIntInput (const char* szLabel, ImVec2 vMin, ImVec2 vMax, int* pData, const int* pMin = nullptr, const int* pMax = nullptr, RGBA_t* pFrameClr = nullptr, const float* pAnimation = nullptr);

    // This is just a wrapper for DrawIntSlider and DrawIntInput. 
    bool DrawIntInputWidget(
        const char* szText, const char* szLabel, 
        ImVec2 vMin, ImVec2 vMax, 
        int* pData, const int iMin, const int iMax, 
        RGBA_t clrBackground, 
        float flSliderPercentage, float flIntInputPercentage, 
        const float* pTrackThickness = nullptr, const float* pKnobSize = nullptr, const float* pAnimationState = nullptr
    );

    bool DrawFloatSlider(const char* szLabel, ImVec2 vMin, ImVec2 vMax, float* pData, const float flMin, const int flMax, RGBA_t clrBackground, const float* pTrackThickness = nullptr, const float* pKnowSize = nullptr, const float* pAnimationState = nullptr);
    void DrawFloatInput (const char* szLabel, ImVec2 vMin, ImVec2 vMax, float* pData, const float* pMin = nullptr, const float* pMax = nullptr, RGBA_t* pFrameClr = nullptr, const float* pAnimation = nullptr);

    // This is just a wrapper for DrawFloatSlider and DrawFloatInput. 
    bool DrawFloatInputWidget(
        const char* szText, const char* szLabel,                    // Labels.
        ImVec2 vMin,        ImVec2 vMax,                            // Dimensions
        float* pData,       const float flMin, const float flMax,   // Data for slider & inputs feild.
        RGBA_t clrBackground,                                       // Used to calculate text color.
        float flSliderPercentage, float flIntInputPercentage, 
        const float* pTrackThickness = nullptr, const float* pKnobSize = nullptr, const float* pAnimationState = nullptr
    );


    // Animation 
    AnimationHandler_t m_menuAnim;
    AnimationHandler_t m_popupAnim;
    AnimationHandler_t m_colorPickerAnim;
    AnimationHandler_t m_configButtonAnim;
    AnimationHandler_t m_configLoadAnim;
    AnimationHandler_t m_newFileAnim;

private:
    bool m_bVisible = true;

    ImVec2 _ClampWindowPos(const ImVec2 vWindowPos);

    void _StartRecordingKey();
    bool    m_bRecordingKey = false;
    int64_t m_iRecordedKey  = 0;

    bool   _Initialize();
    void   _InitFonts();

    ImVec2 _DrawMainBody(float flWidth, float flHeight);
    void   _DrawTabBar(float flWidth, float flHeight, float x, float y);
    void   _DrawSections(Tab_t* pTab, float flWidth, float flHeight, float x, float y, ImVec2 vWindowPos);
    void   _DrawConfigPanel(float x, float y, float flWidth, float flHeight);
    void   _DrawConfigInfo(ImVec2 vConfigInfoPos, ImVec2 vConfigInfoSize, const float flConfigInfoRounding);
    void   _DrawConfigList(ImVec2 vConfigListSize);
    void   _DrawConfigButtons(ImVec2 vConfigButtonPos, ImVec2 vConfigButtonSize);
    bool   m_bConfigPanelActive = false;
    ImVec2 m_vMenuPos;
    ImVec2 m_vMenuSize;
    // These are some important variables, they hold active config state n shit.
    int    m_iActiveConfigIndex = -1; 
    int    m_iLoadedConfigIndex = -1;
    int    m_iNewFileIndex      = -1;
    void _GetFileInfo(const std::string& szFilePath);
    std::string m_szLoadedConfigName           = "";
    std::string m_szLoadedConfigPath           = "";
    std::string m_szLoadedConfigLastModifyTime = "";
    size_t      m_iLoadedConfigSize            = 0llu;


    ImVec2 _CalculateSectionSize(int nFeatures, float flInterFeaturePadding, float flSectionPadding, float flFeatureWidth, float flFeatureHeight) const;

    // Feature specfic...
    void _DrawBoolean     (IFeature* pFeature, ImVec2 vMinWithPadding, ImVec2 vMaxWithPadding, ImVec2 vWindowPos);
    void _DrawIntSlider   (IFeature* pFeature, ImVec2 vMinWithPadding, ImVec2 vMaxWithPadding, ImVec2 vWindowPos);
    void _DrawFloatSlider (IFeature* pFeature, ImVec2 vMinWithPadding, ImVec2 vMaxWithPadding, ImVec2 vWindowPos);
    void _DrawDropDown    (IFeature* pFeature, ImVec2 vMinWithPadding, ImVec2 vMaxWithPadding, ImVec2 vWindowPos);
    void _DrawColor       (IFeature* pFeature, ImVec2 vMinWithPadding, ImVec2 vMaxWithPadding, ImVec2 vWindowPos);

    void _TriggerPopup        (IFeature* pFeature) const;

    // Popups :)..
    void _DrawColorPopup      (IFeature* pFeature, ImVec2 vWindowPos);
    void _DrawFloatSliderPopup(IFeature* pFeature, ImVec2 vWindowPos);
    void _DrawIntSliderPopup  (IFeature* pFeature, ImVec2 vWindowPos);
    void _DrawBooleanPopup    (IFeature* pFeature, ImVec2 vWindowPos);

    // Helper functions...
    void _CalculateColors();

    void _AnimateModel();
    AnimationHandler_t m_modelAnim;

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
DEFINE_FEATURE(Scale,            "Scale", FloatSlider_t, Menu, Menu, 1, FloatSlider_t(1.0f, 0.25f, 2.0f))
DEFINE_FEATURE(Blur,             "Blur", FloatSlider_t, Menu, Menu, 2, FloatSlider_t(0.0f, 0.0f, 8.0f))

DEFINE_FEATURE(ColorTopRight,    "Top-Right Color", ColorData_t,   Menu, Menu, 3, ColorData_t(RGBA_t((unsigned char)255, 255, 255, 255)))
DEFINE_FEATURE(ColorTopLeft,     "Top-Left Color", ColorData_t,   Menu, Menu, 4, ColorData_t(RGBA_t((unsigned char)255, 255, 255, 255)))
DEFINE_FEATURE(ColorBottomRight, "Bottom-Right Color", ColorData_t,   Menu, Menu, 5, ColorData_t(RGBA_t((unsigned char)255, 255, 255, 255)))
DEFINE_FEATURE(ColorBottomLeft,  "Bottom-Left Color", ColorData_t,   Menu, Menu, 6, ColorData_t(RGBA_t((unsigned char)255, 255, 255, 255)))

DEFINE_FEATURE(rgb,              "RGB", bool,          Menu, Menu, 7, false)
DEFINE_FEATURE(RGBSpeed,         "RGB Speed", FloatSlider_t, Menu, Menu, 8, FloatSlider_t(0.0f, 0.0f, 10.0f))

DEFINE_FEATURE(Rounding,         "Rounding", FloatSlider_t, Menu, Menu, 9, FloatSlider_t(15.0f, 0.0f, 100.0f)) 
DEFINE_FEATURE(Draw_Guides,      "Draw Guides", bool,          Menu, Menu, 10, true)


////////////////////////////// SIDE MENU ////////////////////////////////////
DEFINE_SECTION(SideMenu, "Menu", 2)
DEFINE_FEATURE(Scale,            "Scale", FloatSlider_t, SideMenu, Menu, 1, FloatSlider_t(1.0f, 0.25f, 2.0f))
DEFINE_FEATURE(Blur,             "Blur", FloatSlider_t, SideMenu, Menu, 2, FloatSlider_t(0.0f, 0.0f, 8.0f))

DEFINE_FEATURE(ColorTopRight,    "Top-Right Color", ColorData_t,   SideMenu, Menu, 3, ColorData_t(RGBA_t((unsigned char)255, 255, 255, 255)))
DEFINE_FEATURE(ColorTopLeft,     "Top-Left Color", ColorData_t,   SideMenu, Menu, 4, ColorData_t(RGBA_t((unsigned char)255, 255, 255, 255)))
DEFINE_FEATURE(ColorBottomRight, "Bottom-Right Color", ColorData_t,   SideMenu, Menu, 5, ColorData_t(RGBA_t((unsigned char)255, 255, 255, 255)))
DEFINE_FEATURE(ColorBottomLeft,  "Bottom-Left Color", ColorData_t,   SideMenu, Menu, 6, ColorData_t(RGBA_t((unsigned char)255, 255, 255, 255)))

DEFINE_FEATURE(rgb,              "RGB", bool,          SideMenu, Menu, 7, false)
DEFINE_FEATURE(RGBSpeed,         "RGB Speed", FloatSlider_t, SideMenu, Menu, 8, FloatSlider_t(0.0f, 0.0f, 10.0f))

DEFINE_FEATURE(Rounding,         "Rounding", FloatSlider_t, SideMenu, Menu, 9, FloatSlider_t(15.0f, 0.0f, 100.0f))
DEFINE_FEATURE(AnimAccentSize,   "Animation Accent Size", FloatSlider_t, SideMenu, Menu, 10, FloatSlider_t(30.0f, 0.0f, 200.0f))


////////////////////////////// THEME ////////////////////////////////////
DEFINE_SECTION(Theme, "Menu", 3)
DEFINE_FEATURE(Theme, "Theme Color", ColorData_t, Theme, Menu, 1, ColorData_t(RGBA_t((unsigned char)255, 0, 0, 255)))


////////////////////////////// SECTION BOXES ////////////////////////////////////
DEFINE_SECTION(SectionBoxes, "Menu", 4)
DEFINE_FEATURE(Blur,             "Blur", FloatSlider_t, SectionBoxes, Menu, 1, FloatSlider_t(0.0f, 0.0f, 8.0f))

DEFINE_FEATURE(ColorTopRight,    "Top-Right Color", ColorData_t,   SectionBoxes, Menu, 2, ColorData_t(RGBA_t((unsigned char)255, 255, 255, 255)))
DEFINE_FEATURE(ColorTopLeft,     "Top-Left Color", ColorData_t,   SectionBoxes, Menu, 3, ColorData_t(RGBA_t((unsigned char)255, 255, 255, 255)))
DEFINE_FEATURE(ColorBottomRight, "Bottom-Right Color", ColorData_t,   SectionBoxes, Menu, 4, ColorData_t(RGBA_t((unsigned char)255, 255, 255, 255)))
DEFINE_FEATURE(ColorBottomLeft,  "Bottom-Left Color", ColorData_t,   SectionBoxes, Menu, 5, ColorData_t(RGBA_t((unsigned char)255, 255, 255, 255)))

DEFINE_FEATURE(rgb,              "RGB", bool,          SectionBoxes, Menu, 6, false)
DEFINE_FEATURE(RGBSpeed,         "RGB Speed", FloatSlider_t, SectionBoxes, Menu, 7, FloatSlider_t(0.0f, 0.0f, 10.0f))

DEFINE_FEATURE(Rounding,         "Rounding", FloatSlider_t, SectionBoxes, Menu, 8, FloatSlider_t(15.0f, 0.0f, 100.0f), FeatureFlag_SupportKeyBind)
DEFINE_FEATURE(ThemeBorder,      "Theme Borders", bool,          SectionBoxes, Menu, 9, false)
