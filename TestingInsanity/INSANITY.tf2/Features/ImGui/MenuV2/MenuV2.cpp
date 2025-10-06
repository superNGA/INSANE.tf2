#include "MenuV2.h"

#include <algorithm>
#include <chrono>
#include <cmath>
#include <cstdint>


#include "../../../Extra/math.h"
#include "../../../External Libraries/ImGui/imgui.h"
#include "../../../Hooks/DirectX Hook/DirectX_hook.h"
#include "../../../Resources/Fonts/FontManager.h"
#include "../../../SDK/class/IPanel.h"
#include "../../../SDK/class/IVEngineClient.h"
#include "../../FeatureHandler.h"
#include "../../ModelPreview/ModelPreview.h"

// Renderer
#include "../../Graphics Engine V2/Graphics.h"
#include "../../Graphics Engine V2/Draw Objects/Box/Box.h"
#include "../../Graphics Engine V2/Draw Objects/Line/Line.h"
#include "../../Graphics Engine V2/Draw Objects/Circle/Circle.h"


constexpr float SIDEMENU_SCALE            =  0.2f; // Percentage of main body allocated to side menu.
constexpr float FRAME_PADDING_PXL         = 15.0f; // Padding between Main body & its contents.
constexpr float SECTION_PADDING_PXL       = 10.0f; // Padding between Section walls & its contents.
constexpr float INTER_FEATURE_PADDING_PXL =  5.0f; // Padding between each feature.
constexpr float FEATURE_PADDING_PXL       =  5.0f; // Padding between feautres walls & its contents.
constexpr float FEATURE_HEIGHT            = 30.0f; // Height of each feature.
constexpr float SECTION_NAME_PADDING      = 10.0f; // Padding above and below section names in main body. 

constexpr float TAB_NAME_PADDING_IN_PXL   = 8.0f; // Padding above and below a tab's name in side menu.
constexpr float CTG_NAME_PADDING_IN_PXL   = 20.0f;

constexpr float ANIM_COMPLETION_TOLERANCE = 0.005f;
constexpr float ANIM_COMPLETE             = 1.0f;
constexpr float ANIM_ZERO                 = 0.0f;
constexpr float ANIM_DURATION_IN_SEC      = 0.3f;


///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
MenuGUI_t::MenuGUI_t()
{
    m_clrPrimary.Init(); m_clrSecondary.Init(); m_clrTheme.Init();

    m_lastResetTime       = std::chrono::high_resolution_clock::now();
    m_popupOpenTime       = std::chrono::high_resolution_clock::now();
    m_colorPickerOpenTime = std::chrono::high_resolution_clock::now();

    m_vMenuPos.x  = 0.0f; m_vMenuPos.y  = 0.0f;
    m_vMenuSize.x = 0.0f; m_vMenuSize.y = 0.0f;
}


///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
void MenuGUI_t::_InitFonts()
{
    // Initializing fonts.
    m_pFontFeatures     = Resources::Fonts::JetBrainsMonoNerd_Small;
    m_pFontSectionName  = Resources::Fonts::JetBrainsMonoNerd_Small; 
    m_pFontSideMenu     = Resources::Fonts::JetBrainsMonoNerd_Small;
    m_pFontCatagoryName = Resources::Fonts::JetBrainsMonoNerd_Small;
    m_pPopupFont        = Resources::Fonts::JetBrainsMonoNerd_Small;

    m_pTitleFont        = Resources::Fonts::JetBrains_SemiBold_NL_Mid;
}


///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
void MenuGUI_t::Draw()
{
    if (m_bVisible == false)
        return;

    if (_Initialize() == false)
        return;
    {
        ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0.0f, 0.0f, 0.0f, 0.0f));
    }
    
    _CalculateColors();
    
    // Animation calculations.
    CalculateAnim(m_lastResetTime,       m_flAnimation,       ANIM_DURATION_IN_SEC);
    CalculateAnim(m_popupOpenTime,       m_flPopupAnimation,  ANIM_DURATION_IN_SEC);
    CalculateAnim(m_colorPickerOpenTime, m_flColorPickerAnim, ANIM_DURATION_IN_SEC);

    // Drawing main body.
    float  flScaleMult = Features::Menu::Menu::Scale.GetData().m_flVal;
    m_vMenuSize = ImVec2(900.0f * flScaleMult, 650.0f * flScaleMult);
    m_vMenuPos  = _DrawMainBody(m_vMenuSize.x, m_vMenuSize.y);

    // Drawing side menu.
    _DrawTabBar(m_vMenuSize.x * SIDEMENU_SCALE, m_vMenuSize.y, m_vMenuPos.x, m_vMenuPos.y);


    {
        ImGui::PopStyleColor();
    }
}


///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
void MenuGUI_t::SetVisible(bool bVisible)
{
    m_bVisible = bVisible;

    if(m_pMainMenu != nullptr)
        m_pMainMenu->SetVisible(m_bVisible);

    if (m_pSideMenu != nullptr)
        m_pSideMenu->SetVisible(m_bVisible);

    for(IDrawObj_t* pDrawObjs : m_vecSectionBoxes)
        pDrawObjs->SetVisible(bVisible);

    if(bVisible == false)
    {
        ResetAnimation(m_lastResetTime, m_flAnimation);
        ResetAnimation(m_popupOpenTime, m_flPopupAnimation);
    }
}


///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
bool MenuGUI_t::IsVisible() const
{
    return m_bVisible;
}


///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
bool MenuGUI_t::_Initialize()
{
    if (m_pMainMenu == nullptr)
    {
        m_pMainMenu = new BoxFilled2D_t();
        
        if (m_pMainMenu == nullptr)
            return false;

        WIN_LOG("Main body init done successfully");
    }

    if (m_pSideMenu == nullptr)
    {
        m_pSideMenu = new BoxFilled2D_t();
        
        if (m_pSideMenu == nullptr)
            return false;

        WIN_LOG("Side menu init done successfully");
    }

    _InitFonts();

    return true;
}


///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
ImVec2 MenuGUI_t::_DrawMainBody(float flWidth, float flHeight)
{
    ImVec2           vWindowPos(0.0f, 0.0f);
    ImGuiWindowFlags iWindowFlags = 
        ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoTitleBar | 
        ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoBringToFrontOnFocus;

    ImGui::SetNextWindowBgAlpha(0.0f);
    ImGui::SetNextWindowSize(ImVec2(flWidth, flHeight));
    std::string szMainWindowName("##MainMenuGUI");
    if (ImGui::Begin(szMainWindowName.c_str(), nullptr, iWindowFlags) == true)
    {
        vWindowPos = ImGui::GetWindowPos();    

        // Clamp window pos
        vWindowPos = _ClampWindowPos(vWindowPos); 
        ImGui::SetWindowPos(vWindowPos);

        // Pos & Size
        m_pMainMenu->SetVertex(vec(vWindowPos.x, vWindowPos.y, 0.0f), vec(vWindowPos.x + flWidth, vWindowPos.y + flHeight, 0.0f));

        // Blur
        float flBlurAmmount = Features::Menu::Menu::Blur.GetData().m_flVal;
        m_pMainMenu->SetBlur(flBlurAmmount <= 0.0f ? -1 : static_cast<int>(flBlurAmmount)); // -1 blur is no blur.

        // Colors ( independent for each vertex )
        m_pMainMenu->SetColor(Features::Menu::Menu::ColorBottomLeft.GetData().GetAsBytes(),  IBoxFilled_t::VertexType_BottomLeft);
        m_pMainMenu->SetColor(Features::Menu::Menu::ColorBottomRight.GetData().GetAsBytes(), IBoxFilled_t::VertexType_BottomRight);
        m_pMainMenu->SetColor(Features::Menu::Menu::ColorTopLeft.GetData().GetAsBytes(),     IBoxFilled_t::VertexType_TopLeft);
        m_pMainMenu->SetColor(Features::Menu::Menu::ColorTopRight.GetData().GetAsBytes(),    IBoxFilled_t::VertexType_TopRight);

        // RGB animation
        float flRGBSpeed = Features::Menu::Menu::RGBSpeed.GetData().m_flVal;
        bool  bRGBActive = Features::Menu::Menu::rgb.IsActive();
        m_pMainMenu->SetRGBAnimSpeed(bRGBActive == true ? flRGBSpeed : -1.0f);

        // Rounding
        m_pMainMenu->SetRounding(Features::Menu::Menu::Rounding.GetData().m_flVal);

        float flAnimatedScroll = (ANIM_COMPLETE - m_flAnimation) * 50.0f;
        _DrawSections(m_pActiveTab, flWidth * (1.0f - SIDEMENU_SCALE), flHeight, vWindowPos.x + (flWidth * SIDEMENU_SCALE), vWindowPos.y - ImGui::GetScrollY() + flAnimatedScroll, vWindowPos);

        ImGui::End();
    }

    return vWindowPos;
}


///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
void MenuGUI_t::_DrawTabBar(float flWidth, float flHeight, float x, float y)
{
    ImGuiWindowFlags iWindowFlags = ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse;

    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing,  ImVec2(0.0f, 0.0f));
    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0.0f, 0.0f));

    ImGui::SetNextWindowBgAlpha(0.0f);
    ImGui::SetNextWindowSize(ImVec2(flWidth, flHeight));
    ImGui::SetNextWindowPos(ImVec2(x, y));
    if (ImGui::Begin("##SideMenuGUI", nullptr, iWindowFlags) == true)
    {
        // Pos & Size
        m_pSideMenu->SetVertex(vec(x, y, 0.0f), vec(x + flWidth, y + flHeight, 0.0f));

        // Blur
        float flBlurAmmount = Features::Menu::SideMenu::Blur.GetData().m_flVal;
        m_pSideMenu->SetBlur(flBlurAmmount <= 0.0f ? -1 : static_cast<int>(flBlurAmmount)); // -1 blur is no blur.

        // Colors ( independent for each vertex )
        m_pSideMenu->SetColor(Features::Menu::SideMenu::ColorBottomLeft.GetData().GetAsBytes(),  IBoxFilled_t::VertexType_BottomLeft);
        m_pSideMenu->SetColor(Features::Menu::SideMenu::ColorBottomRight.GetData().GetAsBytes(), IBoxFilled_t::VertexType_BottomRight);
        m_pSideMenu->SetColor(Features::Menu::SideMenu::ColorTopLeft.GetData().GetAsBytes(),     IBoxFilled_t::VertexType_TopLeft);
        m_pSideMenu->SetColor(Features::Menu::SideMenu::ColorTopRight.GetData().GetAsBytes(),    IBoxFilled_t::VertexType_TopRight);

        // RGB animation
        float flRGBSpeed = Features::Menu::SideMenu::RGBSpeed.GetData().m_flVal;
        bool  bRGBActive = Features::Menu::SideMenu::rgb.IsActive();
        m_pSideMenu->SetRGBAnimSpeed(bRGBActive == true ? flRGBSpeed : -1.0f);

        // Rounding
        m_pSideMenu->SetRounding(Features::Menu::SideMenu::Rounding.GetData().m_flVal);

        ImGui::PushFont(m_pFontSideMenu);

        // Drawing MY NAME :)
        ImDrawList* pDrawList = ImGui::GetWindowDrawList();
        ImVec2 vIntroSize;
        {
            ImGui::PushFont(m_pTitleFont);
            
            std::string szMyName("INSANE");
            ImGui::PushFont(Resources::Fonts::MontserratBlack);
            ImVec2 vMyNameSize(ImGui::CalcTextSize(szMyName.c_str()));
            ImGui::PopFont();

            std::string szTarget(".tf2");
            ImGui::PushFont(Resources::Fonts::ShadowIntoLight);
            ImVec2 vTargetTextSize(ImGui::CalcTextSize(szTarget.c_str()));
            ImGui::PopFont();

            vIntroSize = ImVec2((vMyNameSize.x + vTargetTextSize.x), vMyNameSize.y);

            ImGui::PushFont(Resources::Fonts::MontserratBlack);
            pDrawList->AddText(ImVec2(x + (flWidth - vIntroSize.x) / 2.0f, y + SECTION_PADDING_PXL), ImColor(m_clrTheme.GetAsImVec4()), szMyName.c_str());
            ImGui::PopFont();

            RGBA_t clrTargetName; CalcTextClrForBg(clrTargetName, m_clrSecondary);
            ImGui::PushFont(Resources::Fonts::ShadowIntoLight);
            pDrawList->AddText(ImVec2(x + ((flWidth - vIntroSize.x) / 2.0f) + vMyNameSize.x, y + SECTION_PADDING_PXL + (vMyNameSize.y - vTargetTextSize.y)), ImColor(clrTargetName.GetAsImVec4()), szTarget.c_str());
            ImGui::PopFont();

            ImGui::PopFont();
        }

        // Drawing tabs.
        ImVec2 vButtonSize(flWidth, ImGui::GetTextLineHeight() + (2.0f * TAB_NAME_PADDING_IN_PXL));
        ImVec2 vCursorScreenPos(x, y + (2.0f * SECTION_PADDING_PXL) + vIntroSize.y);

        // This will help us bundle tab buttons which are of same catagory.
        // Whenever the string changes, we leave a bigger gap & write the catagory name on top.
        std::vector<std::string> tabBundlingHelper = {
            "Combat", "Combat", "Combat", "Combat", "Combat", "Combat", 
            "Visual", "Visual", "Visual",
            "Misc", "Misc", "Misc" 
        };
        std::vector<const char8_t*> vecIconList = {
            u8"\uee15", // icon 1
            u8"\uf1e2", // icon 2
            u8"\uf050", // icon 3
            u8"\uf049", // icon 4
            u8"\uf2f1", // icon 5
            u8"\uf21b", // icon 6 
            u8"\uf1fc", // icon 7   
            u8"\uf121", // icon 8
            u8"\uf085", // icon 9
            u8"\uef0c", // icon 10
            u8"\uf1b2", // icon 11
            u8"\uf520"  // icon 12
        };
        constexpr float DISTINCT_CATAGORIES = 3.0f;
        std::string szCurrCatagory = "NULL";
        {
            RGBA_t clrHighLightClr; _FindElevatedClr(clrHighLightClr, m_clrSecondary);
            RGBA_t clrCatagoryText; CalcTextClrForBg(clrCatagoryText, m_clrSecondary);
            {
                ImGui::PushStyleColor(ImGuiCol_Button,             ImVec4(0.0f, 0.0f, 0.0f, 0.0f));
                ImGui::PushStyleColor(ImGuiCol_ButtonHovered,      clrHighLightClr.GetAsImVec4());
                ImGui::PushStyleColor(ImGuiCol_ButtonActive,       clrHighLightClr.GetAsImVec4());
                ImGui::PushStyleColor(ImGuiCol_Text,               clrCatagoryText.GetAsImVec4());

                ImGui::PushStyleVar(ImGuiStyleVar_FramePadding,    ImVec2(0.0f, 0.0f));
                ImGui::PushStyleVar(ImGuiStyleVar_ButtonTextAlign, ImVec2(0.0f, 0.5f));
            }

            const std::vector<Tab_t*> vecTabs = Config::featureHandler.GetFeatureMap();
            float flTabCount                  = static_cast<float>(vecTabs.size());
            float flSideMenuEffectiveHeight   = ((DISTINCT_CATAGORIES - 1.0f) * CTG_NAME_PADDING_IN_PXL) + (flTabCount * vButtonSize.y) + (ImGui::GetTextLineHeight() * DISTINCT_CATAGORIES);

            if(flSideMenuEffectiveHeight < flHeight)
                vCursorScreenPos = ImVec2(x, y + (flHeight - flSideMenuEffectiveHeight) / 3.0f);

            for (int iTabIndex = 0; iTabIndex < vecTabs.size(); iTabIndex++)
            {
                Tab_t* pTab = vecTabs[iTabIndex];

                // Drawing catagory name.
                if(iTabIndex < tabBundlingHelper.size())
                {
                    if(szCurrCatagory != tabBundlingHelper[iTabIndex])
                    {
                        szCurrCatagory = tabBundlingHelper[iTabIndex];

                        // Don't increment for the first tab, causing alignment issues.
                        if(iTabIndex > 0)
                            vCursorScreenPos.y += CTG_NAME_PADDING_IN_PXL;

                        // finally, writting catagory text.
                        clrCatagoryText.a = 150;
                        pDrawList->AddText(ImVec2(vCursorScreenPos.x + SECTION_PADDING_PXL, vCursorScreenPos.y), ImColor(clrCatagoryText.GetAsImVec4()), szCurrCatagory.c_str());
                        vCursorScreenPos.y += ImGui::GetTextLineHeight();
                        clrCatagoryText.a = 255;
                    }
                }

                // Maintain the cursor position strictly so padding doesn't fuck with our stuff.
                ImGui::SetCursorScreenPos(vCursorScreenPos);

                static bool bNoHighlighting = false;
                bNoHighlighting = (m_pActiveTab == pTab);

                if(m_pActiveTab != nullptr)
                {
                    if(m_pActiveTab == pTab)
                    {
                        ImVec2 vButtonAnimationMin((vCursorScreenPos.x + flWidth - (flWidth * m_flAnimation)), vCursorScreenPos.y);
                        ImVec2 vButtonAnimationMax(vCursorScreenPos.x + flWidth + 2.0f, vCursorScreenPos.y + vButtonSize.y); 
                        
                        pDrawList->AddRectFilled(
                            ImVec2(vButtonAnimationMin.x - Features::Menu::SideMenu::AnimAccentSize.GetData().m_flVal, vButtonAnimationMin.y),
                            ImVec2(vButtonAnimationMin.x, vButtonAnimationMax.y),
                            ImColor(m_clrTheme.GetAsImVec4()));

                        pDrawList->AddRectFilled(
                            vButtonAnimationMin,
                            vButtonAnimationMax,
                            ImColor(m_clrPrimary.GetAsImVec4()));

                        // If we are drawing highlighting for this button ourselves, then tell ImGui to fuck off. ( no hate )
                        bNoHighlighting = true;
                        ImGui::PushStyleColor(ImGuiCol_Button,        ImVec4(0.0f, 0.0f, 0.0f, 0.0f));
                        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.0f, 0.0f, 0.0f, 0.0f));
                        ImGui::PushStyleColor(ImGuiCol_ButtonActive,  ImVec4(0.0f, 0.0f, 0.0f, 0.0f));
                    }
                }

                if (ImGui::Button(("     " + pTab->m_szTabDisplayName).c_str(), vButtonSize) == true)
                {
                    m_pActiveTab = pTab;
                    ResetAnimation(m_lastResetTime, m_flAnimation);
                }
     
                ImGui::PushFont(Resources::Fonts::JetBrainsMonoNerd_Mid);
                float flIconWidth = ImGui::GetFont()->GetCharAdvance(' ');
                ImGui::GetWindowDrawList()->AddText(
                    ImVec2(vCursorScreenPos.x + flIconWidth, vCursorScreenPos.y + (vButtonSize.y - ImGui::GetTextLineHeight()) / 2.0f),
                    ImColor(m_pActiveTab == pTab ? m_clrTheme.GetAsImVec4() : clrCatagoryText.GetAsImVec4()),
                    reinterpret_cast<const char*>(vecIconList[iTabIndex])
                );
                ImGui::PopFont();

                if(bNoHighlighting == true)
                    ImGui::PopStyleColor(3);

                vCursorScreenPos.y += vButtonSize.y;
            }

            {
                ImGui::PopStyleColor(4); ImGui::PopStyleVar(2);
            }
        }

        ImGui::PopFont();
        ImGui::End();
    }

    ImGui::PopStyleVar(2);
}


///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
void MenuGUI_t::_DrawSections(Tab_t* pTab, float flWidth, float flHeight, float x, float y, ImVec2 vWindowPos)
{
    if (pTab == nullptr)
        return;

    ImGui::SetCursorScreenPos(ImVec2(x, y));

    {
        ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0.0f, 0.0f));
    }


    ImVec2 vLeftSectionCursor (x + FRAME_PADDING_PXL,                             y + FRAME_PADDING_PXL);
    ImVec2 vRightSectionCursor(x + (flWidth / 2.0f) + (FRAME_PADDING_PXL / 2.0f), y + FRAME_PADDING_PXL);
    bool bDrawingOnLeft = true;


    int iSectionIndex = 0; int nSections = pTab->m_vecSections.size();
    for (iSectionIndex = 0; iSectionIndex < nSections; iSectionIndex++)
    {
        Section_t* pSection = pTab->m_vecSections[iSectionIndex];

        // This is the screen pos for the cursor.
        ImVec2* pSectionScreenPos = bDrawingOnLeft == true ? &vLeftSectionCursor : &vRightSectionCursor;
        ImGui::SetCursorScreenPos(*pSectionScreenPos);
        ImVec2 vSectionLocalPos(pSectionScreenPos->x - vWindowPos.x, pSectionScreenPos->y - vWindowPos.y);
        

        // Drawing Section name.
        ImGui::PushFont(m_pFontSectionName);
        ImVec2 vSectionNamePos(pSectionScreenPos->x + FEATURE_PADDING_PXL, pSectionScreenPos->y + SECTION_NAME_PADDING);
        RGBA_t clrText; _FindElevatedClr(clrText, m_clrPrimary); clrText.a = 0xFF;
        ImGui::GetWindowDrawList()->AddText(vSectionNamePos, ImColor(clrText.GetAsImVec4()), pSection->m_szSectionDisplayName.c_str());
        pSectionScreenPos->y += ImGui::GetTextLineHeight() + (SECTION_NAME_PADDING * 2.0f); // Compensate for the section name, so section box doesn't draw over the name.
        ImGui::PopFont();


        // Adjusting cursor pos before drawing feautres.
        float flFeatureWidth = (flWidth / 2.0f) - (1.5f * FRAME_PADDING_PXL) - (2 * SECTION_PADDING_PXL);
        ImGui::SetCursorPos(ImVec2(vSectionLocalPos.x + SECTION_PADDING_PXL + FEATURE_PADDING_PXL, pSectionScreenPos->y - vWindowPos.y + SECTION_PADDING_PXL)); 

        for (IFeature* pFeature : pSection->m_vecFeatures)
        {
            ImVec2 vCursorPos        = ImGui::GetCursorPos();
            // These are relative to window.
            ImVec2 vFeatureMinPadded = vCursorPos;
            ImVec2 vFeatureMaxPadded(vCursorPos.x + flFeatureWidth - (2.0f * FEATURE_PADDING_PXL), vCursorPos.y + FEATURE_HEIGHT);

            if(Features::Menu::Menu::Draw_Guides.IsActive() == true)
            {
                ImVec2 vCursorScreenPos = ImGui::GetCursorScreenPos(); 
                vCursorScreenPos.x -= FEATURE_PADDING_PXL; // we added the feature padding before this & we don't that here. ( so we remove it )
                ImGui::GetWindowDrawList()->AddRect(vCursorScreenPos, ImVec2(vCursorScreenPos.x + flFeatureWidth, vCursorScreenPos.y + FEATURE_HEIGHT), ImColor(1.0f, 0.0f, 0.0f, 1.0f));
            }


            // Drawing the got dayem features.
            RGBA_t clrFetureText; CalcTextClrForBg(clrFetureText, m_clrSectionBox);
            ImGui::PushStyleColor(ImGuiCol_Text, clrFetureText.GetAsImVec4());
            ImGui::PushFont(m_pFontFeatures);
            switch (pFeature->m_iDataType)
            {
            case IFeature::DataType::DT_BOOLEAN:     _DrawBoolean    (pFeature, vFeatureMinPadded, vFeatureMaxPadded, vWindowPos); break;
            case IFeature::DataType::DT_COLORDATA:   _DrawColor      (pFeature, vFeatureMinPadded, vFeatureMaxPadded, vWindowPos); break;
            case IFeature::DataType::DT_INTSLIDER:   _DrawIntSlider  (pFeature, vFeatureMinPadded, vFeatureMaxPadded, vWindowPos); break;
            case IFeature::DataType::DT_FLOATSLIDER: _DrawFloatSlider(pFeature, vFeatureMinPadded, vFeatureMaxPadded, vWindowPos); break;
            case IFeature::DataType::DT_DROPDOWN:    _DrawDropDown   (pFeature, vFeatureMinPadded, vFeatureMaxPadded, vWindowPos); break;
            default: break;
            }
            ImGui::PopFont();
            ImGui::PopStyleColor();


            // Adjust cursor for the next feature.
            ImGui::SetCursorPosX(vSectionLocalPos.x + SECTION_PADDING_PXL + FEATURE_PADDING_PXL      );
            ImGui::SetCursorPosY(vCursorPos.y       + FEATURE_HEIGHT      + INTER_FEATURE_PADDING_PXL);
        }


        ImVec2 vSectionSize = _CalculateSectionSize(
            pSection->m_vecFeatures.size(),                 // No. of features.
            INTER_FEATURE_PADDING_PXL, SECTION_PADDING_PXL, // Padding
            flFeatureWidth, FEATURE_HEIGHT                  // Width & Height for each feature.
        );

        // Drawing Theme ImGuiCol_Border
        if(Features::Menu::SectionBoxes::ThemeBorder.IsActive() == true)
        {
            ImVec2 vThemeBorderMin = *pSectionScreenPos;
            vThemeBorderMin.x = std::clamp<float>(vThemeBorderMin.x, x,            x + flWidth);
            vThemeBorderMin.y = std::clamp<float>(vThemeBorderMin.y, vWindowPos.y, vWindowPos.y + flHeight);

            ImVec2 vThemeBorderMax(pSectionScreenPos->x + vSectionSize.x, pSectionScreenPos->y + vSectionSize.y);
            vThemeBorderMax.x = std::clamp<float>(vThemeBorderMax.x, x,            x + flWidth);
            vThemeBorderMax.y = std::clamp<float>(vThemeBorderMax.y, vWindowPos.y, vWindowPos.y + flHeight);

            ImGui::GetWindowDrawList()->AddRect(
                vThemeBorderMin, vThemeBorderMax,
                ImColor(m_clrTheme.GetAsImVec4()),
                Features::Menu::SectionBoxes::Rounding.GetData().m_flVal);
        }
        
        if (Features::Menu::Menu::Draw_Guides.IsActive() == true)
        {
            ImGui::GetWindowDrawList()->AddRect(
                *pSectionScreenPos,
                ImVec2(pSectionScreenPos->x + vSectionSize.x, pSectionScreenPos->y + vSectionSize.y),
                ImColor(1.0f, 1.0f, 1.0f, 1.0f)
            );
        }

        // Creating section's UI boxes.
        if (iSectionIndex >= m_vecSectionBoxes.size())
        {
            m_vecSectionBoxes.push_back(new BoxFilled2D_t());
        }

        // Setting Section's UI boxes size & pos
        BoxFilled2D_t* pBox = m_vecSectionBoxes[iSectionIndex];
        // Clamping vertex for section's UI boxes.
        vec vSectionBoxMin(pSectionScreenPos->x,                  pSectionScreenPos->y, 0.0f);
        vec vSectionBoxMax(pSectionScreenPos->x + vSectionSize.x, pSectionScreenPos->y + vSectionSize.y, 0.0f);
        {
            vSectionBoxMin.x = std::clamp<float>(vSectionBoxMin.x, x,            x + flWidth);
            vSectionBoxMin.y = std::clamp<float>(vSectionBoxMin.y, vWindowPos.y, vWindowPos.y + flHeight);

            vSectionBoxMax.x = std::clamp<float>(vSectionBoxMax.x, x,            x + flWidth);
            vSectionBoxMax.y = std::clamp<float>(vSectionBoxMax.y, vWindowPos.y, vWindowPos.y + flHeight);
        }

        // Setting Visual settings for section boxes.
        pBox->SetVertex(vSectionBoxMin, vSectionBoxMax);
        pBox->SetColor(Features::Menu::SectionBoxes::ColorTopLeft.GetData().GetAsBytes(),     IBoxFilled_t::VertexType_t::VertexType_TopLeft);
        pBox->SetColor(Features::Menu::SectionBoxes::ColorTopRight.GetData().GetAsBytes(),    IBoxFilled_t::VertexType_t::VertexType_TopRight);
        pBox->SetColor(Features::Menu::SectionBoxes::ColorBottomLeft.GetData().GetAsBytes(),  IBoxFilled_t::VertexType_t::VertexType_BottomLeft);
        pBox->SetColor(Features::Menu::SectionBoxes::ColorBottomRight.GetData().GetAsBytes(), IBoxFilled_t::VertexType_t::VertexType_BottomRight);
        pBox->SetRGBAnimSpeed(Features::Menu::SectionBoxes::rgb.IsActive() == false ? -1.0 : Features::Menu::SectionBoxes::RGBSpeed.GetData().m_flVal);    
        pBox->SetBlur(Features::Menu::SectionBoxes::Blur.GetData().m_flVal);
        pBox->SetRounding(Features::Menu::SectionBoxes::Rounding.GetData().m_flVal);
        pBox->SetVisible(true);

        // Adding section's size fo next section draws accordingly.
        pSectionScreenPos->y += vSectionSize.y + FRAME_PADDING_PXL;
        
        // Choosing the side with the most space.
        bDrawingOnLeft = (vLeftSectionCursor.y > vRightSectionCursor.y ? false : true);
    }

    // Disabling unused section UI boxes.
    size_t nBoxes = m_vecSectionBoxes.size();
    for (int iBoxIndex = iSectionIndex; iBoxIndex < nBoxes; iBoxIndex++)
    {
        m_vecSectionBoxes[iBoxIndex]->SetVisible(false);
    }


    {
        ImGui::PopStyleVar();
    }
}


///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
ImVec2 MenuGUI_t::_CalculateSectionSize(int nFeatures, float flInterFeaturePadding, float flSectionPadding, float flFeatureWidth, float flFeatureHeight) const
{
    return ImVec2(
        (flSectionPadding * 2.0f) + flFeatureWidth,
        (static_cast<float>(nFeatures) * flFeatureHeight) + (static_cast<float>(nFeatures - 1) * flInterFeaturePadding) + (2.0f * flSectionPadding)
    );
}


///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
void MenuGUI_t::_DrawBoolean(IFeature* pFeature, ImVec2 vMinWithPadding, ImVec2 vMaxWithPadding, ImVec2 vWindowPos)
{
    Feature<bool>* pBoolFeature = reinterpret_cast<Feature<bool>*>(pFeature);
    
    // Styling
    ImGui::PushStyleColor(ImGuiCol_FrameBg,        pBoolFeature->m_Data == true ? m_clrTheme.GetAsImVec4() : m_clrSecondary.GetAsImVec4());
    ImGui::PushStyleColor(ImGuiCol_FrameBgHovered, m_clrTheme.GetAsImVec4());
    ImGui::PushStyleColor(ImGuiCol_FrameBgActive,  m_clrTheme.GetAsImVec4());
    ImGui::PushStyleColor(ImGuiCol_CheckMark,      ImVec4(1.0f, 1.0f, 1.0f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_Border,         m_clrTheme.GetAsImVec4());
    ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, WIDGET_BORDER_THICKNESS);
    ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 3.0f);
    
    float  flCheckBoxSide  = ImGui::GetFrameHeight();
    float  flFeatureHeight = fabsf(vMinWithPadding.y - vMaxWithPadding.y);
    float  flFeatureWidth  = fabsf(vMaxWithPadding.x - vMinWithPadding.x);

    // Feature's name
    ImGui::SetCursorScreenPos(ImVec2(vWindowPos.x + vMinWithPadding.x, vWindowPos.y + vMinWithPadding.y + (flFeatureHeight - ImGui::GetTextLineHeight()) / 2.0f));
    ImGui::Text(pBoolFeature->m_szFeatureDisplayName.c_str()); ImGui::SameLine();

    // Checkbox
    ImVec2 vCheckBoxOrigin(vWindowPos.x + vMaxWithPadding.x - flCheckBoxSide, vWindowPos.y + vMinWithPadding.y + (flFeatureHeight - flCheckBoxSide) / 2.0f);
    ImGui::SetCursorScreenPos(vCheckBoxOrigin);
    ImGui::Checkbox(("##" + pBoolFeature->m_szTabName + pBoolFeature->m_szSectionName + pBoolFeature->m_szFeatureDisplayName).c_str(), &pBoolFeature->m_Data);
    

    // Keybind panel button.
    if(pBoolFeature->m_iFlags & FeatureFlags::FeatureFlag_SupportKeyBind)
    {
        ImVec2 vKeyBindPanelButton(vCheckBoxOrigin.x - FEATURE_PADDING_PXL - flCheckBoxSide, vCheckBoxOrigin.y);

        ImGuiIO& io = ImGui::GetIO();
        bool bButtonHovered = (
            (io.MousePos.x >= vKeyBindPanelButton.x && io.MousePos.x <= vKeyBindPanelButton.x + ImGui::GetFrameHeight()) &&
            (io.MousePos.y >= vKeyBindPanelButton.y && io.MousePos.y <= vKeyBindPanelButton.y + ImGui::GetFrameHeight())
        );

        ImVec4 vClrPrimary = m_clrPrimary.GetAsImVec4(); vClrPrimary.w = 1.0f;
        RGBA_t clrText; CalcTextClrForBg(clrText, m_clrSectionBox);
        ImGui::PushStyleColor(ImGuiCol_Text,    bButtonHovered == true ? m_clrTheme.GetAsImVec4() : clrText.GetAsImVec4());
        ImGui::PushStyleColor(ImGuiCol_Button,        bButtonHovered == true ? vClrPrimary : ImVec4(0.0f, 0.0f, 0.0f, 0.0f));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, bButtonHovered == true ? vClrPrimary : ImVec4(0.0f, 0.0f, 0.0f, 0.0f));
        ImGui::PushStyleColor(ImGuiCol_ButtonActive,  bButtonHovered == true ? vClrPrimary : ImVec4(0.0f, 0.0f, 0.0f, 0.0f));
        ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, WIDGET_ROUNDING);
        ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 0.0f);

        ImGui::SetCursorScreenPos(vKeyBindPanelButton);
        
        std::string szButtonText = reinterpret_cast<const char*>(u8"\uf013##"); // icon in UTF-8
        szButtonText += pBoolFeature->m_szTabName;
        szButtonText += pBoolFeature->m_szSectionName;
        szButtonText += pBoolFeature->m_szFeatureDisplayName;
        if(ImGui::Button(szButtonText.c_str(), ImVec2(flCheckBoxSide, flCheckBoxSide)) == true)
        {
            _TriggerPopup(pFeature);
        }

        _DrawBooleanPopup(pFeature, vWindowPos);
        ImGui::PopStyleColor(4); ImGui::PopStyleVar(2);
    }

    // Tool tip
    {
        ImVec4 vClrPopup = m_clrPrimary.GetAsImVec4(); vClrPopup.w = 1.0f;
        ImGui::PushStyleColor(ImGuiCol_PopupBg, vClrPopup);
        ImGui::PushStyleColor(ImGuiCol_Border, m_clrTheme.GetAsImVec4());
        ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, POPUP_ROUNDING);

        if (ImGui::IsItemHovered() == true && pBoolFeature->m_szToolTip.empty() == false)
            ImGui::SetTooltip(pBoolFeature->m_szToolTip.c_str());

        ImGui::PopStyleColor(2);
        ImGui::PopStyleVar();
    }

    // Pop all style vars.
    ImGui::PopStyleColor(5);
    ImGui::PopStyleVar(2);
}


///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
void MenuGUI_t::_DrawIntSlider(IFeature* pFeature, ImVec2 vMinWithPadding, ImVec2 vMaxWithPadding, ImVec2 vWindowPos)
{
    Feature<IntSlider_t>* pIntFeature = reinterpret_cast<Feature<IntSlider_t>*>(pFeature);

    float flFeatureWidth  = fabsf(vMaxWithPadding.x - vMinWithPadding.x);
    float flFeatureHeight = fabsf(vMinWithPadding.y - vMaxWithPadding.y);

    // Styling
    ImGui::PushStyleColor(ImGuiCol_FrameBg,          ImVec4(0.0f, 0.0f, 0.0f, 0.0f));
    ImGui::PushStyleColor(ImGuiCol_FrameBgActive,    ImVec4(0.0f, 0.0f, 0.0f, 0.0f));
    ImGui::PushStyleColor(ImGuiCol_FrameBgHovered,   ImVec4(0.0f, 0.0f, 0.0f, 0.0f));
    ImGui::PushStyleColor(ImGuiCol_SliderGrab,       ImVec4(0.0f, 0.0f, 0.0f, 0.0f));
    ImGui::PushStyleColor(ImGuiCol_SliderGrabActive, ImVec4(0.0f, 0.0f, 0.0f, 0.0f));
    ImGui::PushStyleColor(ImGuiCol_Text,             ImVec4(0.0f, 0.0f, 0.0f, 0.0f));

    // Drawing Feature's name
    ImGui::SetCursorScreenPos(ImVec2(vWindowPos.x + vMinWithPadding.x, vWindowPos.y + vMinWithPadding.y));
    constexpr float GAP_BEFORE_TRACK = 0.4f;
    ImGui::Text(pIntFeature->m_szFeatureDisplayName.c_str()); ImGui::SameLine(vMinWithPadding.x + (flFeatureWidth * GAP_BEFORE_TRACK));

    // Slider.
    const float flTrackWidth = flFeatureWidth * 0.4f/*/ 2.0f*/;
    ImGui::SetNextItemWidth(flTrackWidth);
    ImGui::SliderInt(
        ("##" + pIntFeature->m_szTabName + pIntFeature->m_szSectionName + pIntFeature->m_szFeatureDisplayName).c_str(),
        &pIntFeature->m_Data.m_iVal, pIntFeature->m_Data.m_iMin, pIntFeature->m_Data.m_iMax); 
    

    // Tool tip
    {
        ImVec4 vClrPopup = m_clrPrimary.GetAsImVec4(); vClrPopup.w = 1.0f;
        ImGui::PushStyleColor(ImGuiCol_PopupBg, vClrPopup);
        ImGui::PushStyleColor(ImGuiCol_Border, m_clrTheme.GetAsImVec4());
        ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, POPUP_ROUNDING);

        if (ImGui::IsItemHovered() == true && pIntFeature->m_szToolTip.empty() == false)
            ImGui::SetTooltip(pIntFeature->m_szToolTip.c_str());

        ImGui::PopStyleColor(2);
        ImGui::PopStyleVar();
    }

    ImDrawList* pDrawList = ImGui::GetWindowDrawList();

    // Drawing label for slider.
    RGBA_t clrText; CalcTextClrForBg(clrText, m_clrSectionBox);
    pDrawList->AddText(
        ImVec2(vWindowPos.x + vMinWithPadding.x, vWindowPos.y + vMinWithPadding.y + (flFeatureHeight - ImGui::GetTextLineHeight()) / 2.0f),
        ImColor(clrText.GetAsImVec4()), pIntFeature->m_szFeatureDisplayName.c_str()
    );


    // Drawing the slider manually
    constexpr float TRACK_THICKNESS_PXL = 4.0f;
    ImVec2 vTrackStart(vWindowPos.x + vMinWithPadding.x + (flFeatureWidth * GAP_BEFORE_TRACK), vWindowPos.y + vMinWithPadding.y + (flFeatureHeight / 2.0f) - TRACK_THICKNESS_PXL);
    ImVec2 vTrackEnd(vWindowPos.x + vMinWithPadding.x + (flFeatureWidth * GAP_BEFORE_TRACK) + flTrackWidth, vWindowPos.y + vMinWithPadding.y + (flFeatureHeight / 2.0f) + TRACK_THICKNESS_PXL);
    
    pDrawList->AddRectFilled(vTrackStart, vTrackEnd,ImColor(255, 255, 255, 255), 10.0f);
        
    // Drawing Keybind popup.
    if(pIntFeature->m_iFlags & FeatureFlags::FeatureFlag_SupportKeyBind)
    {
        float flFrameHeight = ImGui::GetFrameHeight();
        ImVec2 vKeyBindPanelButton(
            vTrackStart.x - (FEATURE_PADDING_PXL * 2.0f) - flFrameHeight,
            vWindowPos.y + vMinWithPadding.y + (flFeatureHeight - flFrameHeight) / 2.0f);

        ImGuiIO& io = ImGui::GetIO();
        bool bButtonHovered = (
            (io.MousePos.x >= vKeyBindPanelButton.x && io.MousePos.x <= vKeyBindPanelButton.x + ImGui::GetFrameHeight()) &&
            (io.MousePos.y >= vKeyBindPanelButton.y && io.MousePos.y <= vKeyBindPanelButton.y + ImGui::GetFrameHeight())
        );

        ImVec4 vClrPrimary = m_clrPrimary.GetAsImVec4(); vClrPrimary.w = 1.0f;
        RGBA_t clrText; CalcTextClrForBg(clrText, m_clrSectionBox);
        ImGui::PushStyleColor(ImGuiCol_Text,    bButtonHovered == true ? m_clrTheme.GetAsImVec4() : clrText.GetAsImVec4());
        ImGui::PushStyleColor(ImGuiCol_Button,        bButtonHovered == true ? vClrPrimary : ImVec4(0.0f, 0.0f, 0.0f, 0.0f));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, bButtonHovered == true ? vClrPrimary : ImVec4(0.0f, 0.0f, 0.0f, 0.0f));
        ImGui::PushStyleColor(ImGuiCol_ButtonActive,  bButtonHovered == true ? vClrPrimary : ImVec4(0.0f, 0.0f, 0.0f, 0.0f));
        ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, WIDGET_ROUNDING);
        ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 0.0f);

        ImGui::SetCursorScreenPos(vKeyBindPanelButton);
        std::string szButtonText = reinterpret_cast<const char*>(u8"\uf013##"); // icon in UTF-8
        szButtonText += pIntFeature->m_szTabName;
        szButtonText += pIntFeature->m_szSectionName;
        szButtonText += pIntFeature->m_szFeatureDisplayName;
        if(ImGui::Button(szButtonText.c_str(), ImVec2(flFrameHeight, flFrameHeight)) == true)
        {
            _TriggerPopup(pFeature);
            ResetAnimation(m_popupOpenTime, m_flPopupAnimation);
        }

        _DrawIntSliderPopup(pFeature, vWindowPos);

        ImGui::PopStyleColor(4); ImGui::PopStyleVar(2);
    }


    // Drawing knob.
    constexpr float KNOB_SIZE_PXL = 10.0f;
    float flKnobPos = static_cast<float>(pIntFeature->GetData().m_iVal - pIntFeature->m_Data.m_iMin) / static_cast<float>(pIntFeature->m_Data.m_iMax - pIntFeature->m_Data.m_iMin);
    flKnobPos      *= m_flAnimation; // Animating :)
    flKnobPos       = std::clamp<float>(flKnobPos, 0.0f, 1.0f);

    // NOTE : We are removing 2 * knobsize from the x coordinate, to the knob stays strictly on the track & don't overlap over other features.
    ImVec2 vKnobOriginScreen(
        vWindowPos.x + vMinWithPadding.x + ((flFeatureWidth * GAP_BEFORE_TRACK) + KNOB_SIZE_PXL) + ((flTrackWidth - (2.0f * KNOB_SIZE_PXL)) * flKnobPos),
        vWindowPos.y + ((vMinWithPadding.y + vMaxWithPadding.y) / 2.0f)
    );
    pDrawList->AddRectFilled(
        ImVec2(vKnobOriginScreen.x - KNOB_SIZE_PXL, vKnobOriginScreen.y - KNOB_SIZE_PXL),
        ImVec2(vKnobOriginScreen.x + KNOB_SIZE_PXL, vKnobOriginScreen.y + KNOB_SIZE_PXL),
        ImColor(m_clrTheme.GetAsImVec4()), 20.0f
    );

    ImVec2 vFloatInputScreenPos(
        vWindowPos.x + vMinWithPadding.x + (flFeatureWidth * GAP_BEFORE_TRACK) + flTrackWidth + FEATURE_PADDING_PXL,
        vWindowPos.y + vMinWithPadding.y + ((flFeatureHeight - ImGui::GetFrameHeight()) / 2.0f)
    );

    // Styling & drawing float input.
    ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize,  WIDGET_BORDER_THICKNESS);
    ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding,    5.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_ButtonTextAlign,  ImVec2(0.5f, 0.5f));
    ImGui::PushStyleColor(ImGuiCol_Border,        m_clrTheme.GetAsImVec4());
    ImGui::PushStyleColor(ImGuiCol_Text,          ImVec4(0.0f, 0.0f, 0.0f, 0.0f));
    ImGui::PushStyleColor(ImGuiCol_FrameBg,       m_clrSecondary.GetAsImVec4());
    ImGui::PushStyleColor(ImGuiCol_FrameBgActive, m_clrTheme.GetAsImVec4());

    ImGui::SetCursorScreenPos(vFloatInputScreenPos);
    float flIntInputWidth = (vWindowPos.x + vMaxWithPadding.x) - vFloatInputScreenPos.x;
    ImGui::SetNextItemWidth(flIntInputWidth);

    ImGui::InputInt(
        ("##IntInput" + pIntFeature->m_szTabName + pIntFeature->m_szSectionName + pIntFeature->m_szFeatureDisplayName).c_str(),
        &pIntFeature->m_Data.m_iVal, 0, 0);

    ImGui::PopStyleVar(3);
    ImGui::PopStyleColor(4);

    std::string szSliderValue = std::format("{:.0f}", static_cast<float>(pIntFeature->GetData().m_iVal) * m_flAnimation);
    size_t      nDigits       = szSliderValue.size();
    float       flCharWidth   = Resources::Fonts::JetBrains_SemiBold_NL_Small->GetCharAdvance(' '); // Assuming using a mono font.
    pDrawList->AddText(
        ImVec2(vFloatInputScreenPos.x + ((flIntInputWidth - (static_cast<float>(nDigits) * flCharWidth)) / 2.0f), vFloatInputScreenPos.y + ((ImGui::GetFrameHeight() - ImGui::GetTextLineHeight()) / 2.0f)),
        ImColor(clrText.GetAsImVec4()), szSliderValue.c_str()
    );

    // Pop all style vars.
    ImGui::PopStyleColor(6);
}


///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
void MenuGUI_t::_DrawFloatSlider(IFeature* pFeature, ImVec2 vMinWithPadding, ImVec2 vMaxWithPadding, ImVec2 vWindowPos) 
{
    Feature<FloatSlider_t>* pFloatFeature = reinterpret_cast<Feature<FloatSlider_t>*>(pFeature);

    float flFeatureWidth  = fabsf(vMaxWithPadding.x - vMinWithPadding.x);
    float flFeatureHeight = fabsf(vMinWithPadding.y - vMaxWithPadding.y);

    // Styling
    ImGui::PushStyleColor(ImGuiCol_FrameBg,          ImVec4(0.0f, 0.0f, 0.0f, 0.0f));
    ImGui::PushStyleColor(ImGuiCol_FrameBgActive,    ImVec4(0.0f, 0.0f, 0.0f, 0.0f));
    ImGui::PushStyleColor(ImGuiCol_FrameBgHovered,   ImVec4(0.0f, 0.0f, 0.0f, 0.0f));
    ImGui::PushStyleColor(ImGuiCol_SliderGrab,       ImVec4(0.0f, 0.0f, 0.0f, 0.0f));
    ImGui::PushStyleColor(ImGuiCol_SliderGrabActive, ImVec4(0.0f, 0.0f, 0.0f, 0.0f));
    ImGui::PushStyleColor(ImGuiCol_Text,             ImVec4(0.0f, 0.0f, 0.0f, 0.0f));

    // Drawing Feature's name
    ImGui::SetCursorScreenPos(ImVec2(vWindowPos.x + vMinWithPadding.x, vWindowPos.y + vMinWithPadding.y));
    constexpr float GAP_BEFORE_TRACK = 0.4f;
    ImGui::Text(pFloatFeature->m_szFeatureDisplayName.c_str()); ImGui::SameLine(vMinWithPadding.x + (flFeatureWidth * GAP_BEFORE_TRACK));

    // Slider.
    const float flTrackWidth = flFeatureWidth * 0.4f/*/ 2.0f*/;
    ImGui::SetNextItemWidth(flTrackWidth);
    ImGui::SliderFloat(
        ("##" + pFloatFeature->m_szTabName + pFloatFeature->m_szSectionName + pFloatFeature->m_szFeatureDisplayName).c_str(),
        &pFloatFeature->m_Data.m_flVal, pFloatFeature->m_Data.m_flMin, pFloatFeature->m_Data.m_flMax); 
    

    // Tool tip
    {
        ImVec4 vClrPopup = m_clrPrimary.GetAsImVec4(); vClrPopup.w = 1.0f;
        ImGui::PushStyleColor(ImGuiCol_PopupBg, vClrPopup);
        ImGui::PushStyleColor(ImGuiCol_Border, m_clrTheme.GetAsImVec4());
        ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, POPUP_ROUNDING);

        if (ImGui::IsItemHovered() == true && pFloatFeature->m_szToolTip.empty() == false)
            ImGui::SetTooltip(pFloatFeature->m_szToolTip.c_str());

        ImGui::PopStyleColor(2);
        ImGui::PopStyleVar();
    }


    ImDrawList* pDrawList = ImGui::GetWindowDrawList();

    // Drawing label for slider.
    RGBA_t clrText; CalcTextClrForBg(clrText, m_clrSectionBox);
    pDrawList->AddText(
        ImVec2(vWindowPos.x + vMinWithPadding.x, vWindowPos.y + vMinWithPadding.y + (flFeatureHeight - ImGui::GetTextLineHeight()) / 2.0f),
        ImColor(clrText.GetAsImVec4()), pFloatFeature->m_szFeatureDisplayName.c_str()
    );


    // Drawing the slider manually
    constexpr float TRACK_THICKNESS_PXL = 4.0f;
    ImVec2 vTrackStart(vWindowPos.x + vMinWithPadding.x + (flFeatureWidth * GAP_BEFORE_TRACK), vWindowPos.y + vMinWithPadding.y + (flFeatureHeight / 2.0f) - TRACK_THICKNESS_PXL);
    ImVec2 vTrackEnd(vWindowPos.x + vMinWithPadding.x + (flFeatureWidth * GAP_BEFORE_TRACK) + flTrackWidth, vWindowPos.y + vMinWithPadding.y + (flFeatureHeight / 2.0f) + TRACK_THICKNESS_PXL);
    
    pDrawList->AddRectFilled(vTrackStart, vTrackEnd, ImColor(255, 255, 255, 255), 10.0f);
        
    // Drawing Keybind popup.
    if(pFloatFeature->m_iFlags & FeatureFlags::FeatureFlag_SupportKeyBind)
    {
        float flFrameHeight = ImGui::GetFrameHeight();
        ImVec2 vKeyBindPanelButton(
            vTrackStart.x - (FEATURE_PADDING_PXL * 2.0f) - flFrameHeight,
            vWindowPos.y + vMinWithPadding.y + (flFeatureHeight - flFrameHeight) / 2.0f);

        ImGuiIO& io = ImGui::GetIO();
        bool bButtonHovered = (
            (io.MousePos.x >= vKeyBindPanelButton.x && io.MousePos.x <= vKeyBindPanelButton.x + ImGui::GetFrameHeight()) &&
            (io.MousePos.y >= vKeyBindPanelButton.y && io.MousePos.y <= vKeyBindPanelButton.y + ImGui::GetFrameHeight())
        );

        ImVec4 vClrPrimary = m_clrPrimary.GetAsImVec4(); vClrPrimary.w = 1.0f;
        RGBA_t clrText; CalcTextClrForBg(clrText, m_clrSectionBox);
        ImGui::PushStyleColor(ImGuiCol_Text,    bButtonHovered == true ? m_clrTheme.GetAsImVec4() : clrText.GetAsImVec4());
        ImGui::PushStyleColor(ImGuiCol_Button,        bButtonHovered == true ? vClrPrimary : ImVec4(0.0f, 0.0f, 0.0f, 0.0f));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, bButtonHovered == true ? vClrPrimary : ImVec4(0.0f, 0.0f, 0.0f, 0.0f));
        ImGui::PushStyleColor(ImGuiCol_ButtonActive,  bButtonHovered == true ? vClrPrimary : ImVec4(0.0f, 0.0f, 0.0f, 0.0f));
        ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, WIDGET_ROUNDING);
        ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 0.0f);

        ImGui::SetCursorScreenPos(vKeyBindPanelButton);
        std::string szButtonText = reinterpret_cast<const char*>(u8"\uf013##"); // icon in UTF-8
        szButtonText += pFloatFeature->m_szTabName;
        szButtonText += pFloatFeature->m_szSectionName;
        szButtonText += pFloatFeature->m_szFeatureDisplayName;
        if(ImGui::Button(szButtonText.c_str(), ImVec2(flFrameHeight, flFrameHeight)) == true)
        {
            _TriggerPopup(pFeature);
            ResetAnimation(m_popupOpenTime, m_flPopupAnimation);
        }

        _DrawFloatSliderPopup(pFeature, vWindowPos);
        ImGui::PopStyleColor(4); ImGui::PopStyleVar(2);
    }


    // Drawing knob.
    constexpr float KNOB_SIZE_PXL = 10.0f;
    float flKnobPos = (pFloatFeature->GetData().m_flVal - pFloatFeature->m_Data.m_flMin) / (pFloatFeature->m_Data.m_flMax - pFloatFeature->m_Data.m_flMin);
    flKnobPos      *= m_flAnimation; // Animating :)
    flKnobPos       = std::clamp<float>(flKnobPos, 0.0f, 1.0f);

    // NOTE : We are removing 2 * knobsize from the x coordinate, to the knob stays strictly on the track & don't overlap over other features.
    ImVec2 vKnobOriginScreen(
        vWindowPos.x + vMinWithPadding.x + ((flFeatureWidth * GAP_BEFORE_TRACK) + KNOB_SIZE_PXL) + ((flTrackWidth - (2.0f * KNOB_SIZE_PXL)) * flKnobPos),
        vWindowPos.y + ((vMinWithPadding.y + vMaxWithPadding.y) / 2.0f)
    );
    pDrawList->AddRectFilled(
        ImVec2(vKnobOriginScreen.x - KNOB_SIZE_PXL, vKnobOriginScreen.y - KNOB_SIZE_PXL),
        ImVec2(vKnobOriginScreen.x + KNOB_SIZE_PXL, vKnobOriginScreen.y + KNOB_SIZE_PXL),
        ImColor(m_clrTheme.GetAsImVec4()), 20.0f
    );

    ImVec2 vFloatInputScreenPos(
        vWindowPos.x + vMinWithPadding.x + (flFeatureWidth * GAP_BEFORE_TRACK) + flTrackWidth + FEATURE_PADDING_PXL,
        vWindowPos.y + vMinWithPadding.y + ((flFeatureHeight - ImGui::GetFrameHeight()) / 2.0f)
    );

    // Styling & drawing float input.
    ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize,  WIDGET_BORDER_THICKNESS);
    ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding,    5.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_ButtonTextAlign,  ImVec2(0.5f, 0.5f));
    ImGui::PushStyleColor(ImGuiCol_Border,        m_clrTheme.GetAsImVec4());
    ImGui::PushStyleColor(ImGuiCol_Text,          ImVec4(0.0f, 0.0f, 0.0f, 0.0f));
    ImGui::PushStyleColor(ImGuiCol_FrameBg,       m_clrSecondary.GetAsImVec4());
    ImGui::PushStyleColor(ImGuiCol_FrameBgActive, m_clrTheme.GetAsImVec4());

    ImGui::SetCursorScreenPos(vFloatInputScreenPos);
    float flIntInputWidth = (vWindowPos.x + vMaxWithPadding.x) - vFloatInputScreenPos.x;
    ImGui::SetNextItemWidth(flIntInputWidth);

    ImGui::InputFloat(
        ("##FloatInput" + pFloatFeature->m_szTabName + pFloatFeature->m_szSectionName + pFloatFeature->m_szFeatureDisplayName).c_str(),
        &pFloatFeature->m_Data.m_flVal);

    ImGui::PopStyleVar(3);
    ImGui::PopStyleColor(4);

    std::string szSliderValue = std::format("{:.1f}", pFloatFeature->GetData().m_flVal * m_flAnimation);
    size_t      nDigits       = szSliderValue.size();
    float       flCharWidth   = Resources::Fonts::JetBrains_SemiBold_NL_Small->GetCharAdvance(' '); // Assuming using a mono font.
    pDrawList->AddText(
        ImVec2(vFloatInputScreenPos.x + ((flIntInputWidth - (static_cast<float>(nDigits) * flCharWidth)) / 2.0f), vFloatInputScreenPos.y + ((ImGui::GetFrameHeight() - ImGui::GetTextLineHeight()) / 2.0f)),
        ImColor(clrText.GetAsImVec4()), szSliderValue.c_str()
    );

    // Pop all style vars.
    ImGui::PopStyleColor(6);
}


///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
void MenuGUI_t::_DrawDropDown(IFeature* pFeature, ImVec2 vMinWithPadding, ImVec2 vMaxWithPadding, ImVec2 vWindowPos)
{
    // Styling
    ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, WIDGET_BORDER_THICKNESS);
    ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding,   5.0f);
    ImGui::PushStyleColor(ImGuiCol_Border,         m_clrTheme.GetAsImVec4());
    ImGui::PushStyleColor(ImGuiCol_FrameBg,        m_clrSecondary.GetAsImVec4());
    ImGui::PushStyleColor(ImGuiCol_FrameBgHovered, m_clrSecondary.GetAsImVec4());
    ImGui::PushStyleColor(ImGuiCol_FrameBgActive,  m_clrSecondary.GetAsImVec4());
    ImGui::PushStyleColor(ImGuiCol_Button,         m_clrSecondary.GetAsImVec4());
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered,  m_clrSecondary.GetAsImVec4());
    ImGui::PushStyleColor(ImGuiCol_ButtonActive,   m_clrSecondary.GetAsImVec4());
                                                   
    Feature<DropDown_t>* pDropDownFeature = static_cast<Feature<DropDown_t>*>(pFeature);

    float  flWidgetHeight     = ImGui::GetFrameHeight();
    float  flFeatureHeight    = fabsf(vMinWithPadding.y - vMaxWithPadding.y);
    float  flFeatureWidth     = fabsf(vMaxWithPadding.x - vMinWithPadding.x);

    // Feature's name
    ImGui::SetCursorScreenPos(ImVec2(vWindowPos.x + vMinWithPadding.x, vWindowPos.y + vMinWithPadding.y + (flFeatureHeight - ImGui::GetTextLineHeight()) / 2.0f));
    ImGui::Text(pDropDownFeature->m_szFeatureDisplayName.c_str()); ImGui::SameLine();

    // Drop Down Feature
    RGBA_t clrText; CalcTextClrForBg(clrText, m_clrPrimary);
    {
        // Make sure drop down menu is completely opaque, we don't want no translucent ass drop down menu.
        ImVec4 vDropdownClr  = m_clrPrimary.GetAsImVec4();    vDropdownClr.w  = 1.0f;
        ImVec4 vHighLightClr = m_clrSectionBox.GetAsImVec4(); vHighLightClr.w = 1.0f;

        ImGui::PushStyleColor(ImGuiCol_Text,    clrText.GetAsImVec4());
        ImGui::PushStyleColor(ImGuiCol_PopupBg, vDropdownClr);
        ImGui::PushStyleColor(ImGuiCol_Border,  vDropdownClr);

        // Styling selected shit
        ImGui::PushStyleColor(ImGuiCol_HeaderActive,  vHighLightClr);
        ImGui::PushStyleColor(ImGuiCol_HeaderHovered, vHighLightClr);
        ImGui::PushStyleColor(ImGuiCol_Header,        vHighLightClr);
        ImGui::PushStyleVar(ImGuiStyleVar_PopupRounding, POPUP_ROUNDING); // a little bit of hardcoded rounding.

        ImGui::SetNextItemWidth(flFeatureWidth / 2.0f);
        ImGui::SetCursorScreenPos(ImVec2(vWindowPos.x + vMaxWithPadding.x - flFeatureWidth / 2.0f, vWindowPos.y + vMinWithPadding.y + (flFeatureHeight - flWidgetHeight) / 2.0f));
        ImGui::Combo(
            ("##" + pDropDownFeature->m_szTabName + pDropDownFeature->m_szSectionName + pDropDownFeature->m_szFeatureDisplayName).c_str(),
            &pDropDownFeature->m_iActiveData, pDropDownFeature->m_data.m_pItems, pDropDownFeature->m_data.m_nItems);

        ImGui::PopStyleColor(6);
        ImGui::PopStyleVar();
    }

    // Tool tip
    {
        ImVec4 vClrPopup = m_clrPrimary.GetAsImVec4(); vClrPopup.w = 1.0f;
        ImGui::PushStyleColor(ImGuiCol_PopupBg, vClrPopup);
        ImGui::PushStyleColor(ImGuiCol_Border, m_clrTheme.GetAsImVec4());
        ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, POPUP_ROUNDING);

        if (ImGui::IsItemHovered() == true && pDropDownFeature->m_szToolTip.empty() == false)
            ImGui::SetTooltip(pDropDownFeature->m_szToolTip.c_str());

        ImGui::PopStyleColor(2);
        ImGui::PopStyleVar();
    }

    // Pop all style vars.
    ImGui::PopStyleColor(7);
    ImGui::PopStyleVar(2);
}


///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
void MenuGUI_t::_DrawColor(IFeature* pFeature, ImVec2 vMinWithPadding, ImVec2 vMaxWithPadding, ImVec2 vWindowPos)
{
    Feature<ColorData_t>* pColorFeature = reinterpret_cast<Feature<ColorData_t>*>(pFeature);

    float  flColorPreviewSide = ImGui::GetFrameHeight();
    float  flFeatureHeight    = fabsf(vMinWithPadding.y - vMaxWithPadding.y);
    float  flFeatureWidth     = fabsf(vMaxWithPadding.x - vMinWithPadding.x);
    
    ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding,   WIDGET_ROUNDING);
    ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, WIDGET_BORDER_THICKNESS);
    ImGui::PushStyleColor(ImGuiCol_Border, m_clrTheme.GetAsImVec4());

    // Label
    ImGui::SetCursorScreenPos(ImVec2(vWindowPos.x + vMinWithPadding.x, vWindowPos.y + vMinWithPadding.y + (flFeatureHeight - ImGui::GetTextLineHeight()) / 2.0f));
    ImGui::Text(pColorFeature->m_szFeatureDisplayName.c_str()); ImGui::SameLine();

    // Color preview widget.
    ImVec2 vColorPreviewOrigin(vWindowPos.x + vMaxWithPadding.x - flColorPreviewSide, vWindowPos.y + vMinWithPadding.y + (flFeatureHeight - flColorPreviewSide) / 2.0f);
    ImGui::SetCursorScreenPos(vColorPreviewOrigin);
    {
        ImVec4 vPopclr       = m_clrPrimary.GetAsImVec4();    vPopclr.w       = 1.0f;
        ImVec4 vSecondaryClr = m_clrSectionBox.GetAsImVec4(); vSecondaryClr.w = 1.0f;
        RGBA_t clrText; CalcTextClrForBg(clrText, m_clrPrimary);

        ImGui::PushStyleColor(ImGuiCol_PopupBg,        vPopclr);
        ImGui::PushStyleColor(ImGuiCol_FrameBg,        vSecondaryClr);
        ImGui::PushStyleColor(ImGuiCol_FrameBgActive,  vSecondaryClr);
        ImGui::PushStyleColor(ImGuiCol_FrameBgHovered, vSecondaryClr);
        ImGui::PushStyleColor(ImGuiCol_Border,         ImVec4(0.0f, 0.0f, 0.0f, 0.0f));
        ImGui::PushStyleColor(ImGuiCol_Text,           clrText.GetAsImVec4());

        ImGui::PushStyleVar(ImGuiStyleVar_PopupBorderSize, POPUP_BORDER_THICKNESS);
        ImGui::PushStyleVar(ImGuiStyleVar_PopupRounding,   POPUP_ROUNDING);
        ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing,     ImVec2(2.0f, 2.0f));

        std::string szColorPickerID("##" + pColorFeature->m_szTabName + pColorFeature->m_szSectionName + pColorFeature->m_szFeatureDisplayName);


        float* pClrAnimated = nullptr;
        if(ImGui::IsPopupOpen((szColorPickerID + "picker").c_str()) == true)
        {
            auto AnimateClr = [&](float x) -> float
            {
                x = std::clamp<float>(x, 0.0f, 1.0f);
                float y = (x < 0.5f) ? x + 0.5f : x - 0.5f; // chose something @ 0.5f diffrence from orignal.

                return y + ((x - y) * m_flColorPickerAnim);
            };
            if(fabsf(ANIM_COMPLETE - m_flColorPickerAnim) <= ANIM_COMPLETION_TOLERANCE)
            {
                pClrAnimated = &pColorFeature->m_Data.r;
            }
            else 
            {
                HSVA_t hsv = pColorFeature->m_Data.GetAsBytes().ToHSVA();
                 // hsv.s = AnimateClr(hsv.s);
                hsv.v = AnimateClr(hsv.v);
                hsv.h = AnimateClr(hsv.h / 360.0f) * 360.0f;

               // LOG("Animated hue : %.2f", hsv.h);

               ImVec4 vClrAnimated = hsv.ToRGBA().GetAsImVec4();
               pClrAnimated = &vClrAnimated.x;
            }
        }
        else
        {
            pClrAnimated = &pColorFeature->m_Data.r;
        }

        // Color Picker
        ImGui::ColorEdit4(
            szColorPickerID.c_str(), pClrAnimated,
            ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoLabel | ImGuiColorEditFlags_PickerHueWheel);


        // Resetting color picker animation time, whenever the color picker widget is opened. 
        static bool bClrPickerAnimResetted = false;
        if(ImGui::IsPopupOpen((szColorPickerID + "_ColorPicker4").c_str(), ImGuiPopupFlags_AnyPopupId) == true)
        {
            if(bClrPickerAnimResetted == false)
            {
                ResetAnimation(m_colorPickerOpenTime, m_flColorPickerAnim);
                bClrPickerAnimResetted = true;
            }
        }
        else
        {
            bClrPickerAnimResetted = false;
        }

        // Drawing custom color preview border, cause it won't let me change it properly.
        ImDrawList* pDrawList = ImGui::GetWindowDrawList();
        pDrawList->AddRect(
            vColorPreviewOrigin, 
            ImVec2(vColorPreviewOrigin.x + flColorPreviewSide, vColorPreviewOrigin.y + flColorPreviewSide), 
            ImColor(m_clrTheme.GetAsImVec4()), WIDGET_ROUNDING, 0, WIDGET_BORDER_THICKNESS);


        ImGui::PopStyleColor(6); ImGui::PopStyleVar(3);
    }

    // Tool tip
    {
        ImVec4 vClrPopup = m_clrPrimary.GetAsImVec4(); vClrPopup.w = 1.0f;
        ImGui::PushStyleColor(ImGuiCol_PopupBg, vClrPopup);
        ImGui::PushStyleColor(ImGuiCol_Border, m_clrTheme.GetAsImVec4());
        ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, POPUP_ROUNDING);

        if (ImGui::IsItemHovered() == true && pColorFeature->m_szToolTip.empty() == false)
            ImGui::SetTooltip(pColorFeature->m_szToolTip.c_str());

        ImGui::PopStyleColor(2);
        ImGui::PopStyleVar();
    }

    // Drawing Keybind widget
    if(pColorFeature->m_iFlags & FeatureFlags::FeatureFlag_SupportKeyBind)
    {
        float flFrameHeight = ImGui::GetFrameHeight();
        ImVec2 vKeyBindPanelButton(vColorPreviewOrigin.x - FEATURE_PADDING_PXL - flColorPreviewSide, vColorPreviewOrigin.y);

        ImGuiIO& io = ImGui::GetIO();
        bool bButtonHovered = (
            (io.MousePos.x >= vKeyBindPanelButton.x && io.MousePos.x <= vKeyBindPanelButton.x + ImGui::GetFrameHeight()) &&
            (io.MousePos.y >= vKeyBindPanelButton.y && io.MousePos.y <= vKeyBindPanelButton.y + ImGui::GetFrameHeight())
        );

        ImVec4 vClrPrimary = m_clrPrimary.GetAsImVec4(); vClrPrimary.w = 1.0f;
        RGBA_t clrText; CalcTextClrForBg(clrText, m_clrSectionBox);
        ImGui::PushStyleColor(ImGuiCol_Text,    bButtonHovered == true ? m_clrTheme.GetAsImVec4() : clrText.GetAsImVec4());
        ImGui::PushStyleColor(ImGuiCol_Button,        bButtonHovered == true ? vClrPrimary : ImVec4(0.0f, 0.0f, 0.0f, 0.0f));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, bButtonHovered == true ? vClrPrimary : ImVec4(0.0f, 0.0f, 0.0f, 0.0f));
        ImGui::PushStyleColor(ImGuiCol_ButtonActive,  bButtonHovered == true ? vClrPrimary : ImVec4(0.0f, 0.0f, 0.0f, 0.0f));

        ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 0.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 0.0f);

        ImGui::SetCursorScreenPos(vKeyBindPanelButton);

        std::string szButtonText = reinterpret_cast<const char*>(u8"\uf013##"); // icon in UTF-8
        szButtonText += pColorFeature->m_szTabName;
        szButtonText += pColorFeature->m_szSectionName;
        szButtonText += pColorFeature->m_szFeatureDisplayName;
        if(ImGui::Button(szButtonText.c_str(), ImVec2(flFrameHeight, flFrameHeight)) == true)
        {
            _TriggerPopup(pFeature);
            ResetAnimation(m_popupOpenTime, m_flPopupAnimation);
        }
        _DrawColorPopup(pFeature, vWindowPos);

        ImGui::PopStyleColor(4); ImGui::PopStyleVar(2);
    }

    ImGui::PopStyleColor();
    ImGui::PopStyleVar(2);
}


///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
void MenuGUI_t::_TriggerPopup(IFeature* pFeature) const
{
    ImGui::OpenPopup(("##Popup" + pFeature->m_szTabName + pFeature->m_szSectionName + pFeature->m_szFeatureDisplayName).c_str());
}


///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
char VkToChar(int vk)
{
    if (vk >= 'A' && vk <= 'Z') 
        return static_cast<char>(vk);  // base lowercase
    
    if (vk >= '0' && vk <= '9')
        return static_cast<char>(vk);

    switch (vk)
    {
        case VK_SPACE:     return ' ';
        case VK_OEM_MINUS: return '-';
        case VK_OEM_PLUS:  return '=';
        case VK_OEM_COMMA: return ',';
        case VK_OEM_PERIOD:return '.';
        case VK_OEM_1:     return ';';
        case VK_OEM_2:     return '/';
        case VK_OEM_3:     return '`';
        case VK_OEM_4:     return '[';
        case VK_OEM_5:     return '\\';
        case VK_OEM_6:     return ']';
        case VK_OEM_7:     return '\'';
    }
    return 0;
}


///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
void MenuGUI_t::_DrawColorPopup(IFeature* pFeature, ImVec2 vWindowPos)
{
    ImGui::PushFont(m_pPopupFont);

    constexpr float nRows         = 3.0f;
    float           flPopupHeight = (SECTION_PADDING_PXL * (nRows + 1.0f)) + (nRows * ImGui::GetFrameHeight());
    ImGui::SetNextWindowSizeConstraints(ImVec2(100.0f, flPopupHeight), ImVec2(400.0f, flPopupHeight));

    // Formatting popup window
    ImVec4 vPopClr(m_clrPrimary.GetAsImVec4()); vPopClr.w = max(vPopClr.w, 0.8f);
    RGBA_t vTextClr; CalcTextClrForBg(vTextClr, m_clrPrimary);
    {
        ImGui::PushStyleColor(ImGuiCol_PopupBg, vPopClr);
        ImGui::PushStyleColor(ImGuiCol_Border,  m_clrTheme.GetAsImVec4());
        ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing,     ImVec2(0.0f, 0.0f));
        ImGui::PushStyleVar(ImGuiStyleVar_PopupBorderSize, POPUP_BORDER_THICKNESS);
        ImGui::PushStyleVar(ImGuiStyleVar_PopupRounding,   POPUP_ROUNDING);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding,  POPUP_ROUNDING);
    }
    

    ImGuiWindowFlags iPopFlags = ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoMove;
    if(ImGui::BeginPopup(("##Popup" + pFeature->m_szTabName + pFeature->m_szSectionName + pFeature->m_szFeatureDisplayName).c_str(), iPopFlags) == true)
    {
        float flFrameHeight = ImGui::GetFrameHeight();

        ImVec2 vKeyBindButtonPos(vWindowPos.x + SECTION_PADDING_PXL, vWindowPos.y + SECTION_PADDING_PXL);
        ImVec2 vCharacterSize(m_pFontFeatures->GetCharAdvance(' '), ImGui::GetTextLineHeight());
        ImVec2 vKeyBindScreenPos(vKeyBindButtonPos.x + (flFrameHeight - vCharacterSize.x) / 2.0f, vKeyBindButtonPos.y + (flFrameHeight - vCharacterSize.y) / 2.0f);

        ImVec4 vClrSectionBoxFullAlpha(m_clrSectionBox.GetAsImVec4()); vClrSectionBoxFullAlpha.w = 1.0f;
        
        // Debugging drawing
        if(Features::Menu::Menu::Draw_Guides.IsActive() == true)
        {
            ImDrawList* pDrawList = ImGui::GetForegroundDrawList();
            pDrawList->AddRect(vKeyBindButtonPos, ImVec2(vKeyBindButtonPos.x + 100.0f, vKeyBindButtonPos.y + ImGui::GetFrameHeight()), ImColor(m_clrTheme.GetAsImVec4()));
            pDrawList->AddRect(
                ImVec2(vKeyBindButtonPos.x, vKeyBindButtonPos.y + ImGui::GetFrameHeight() + SECTION_PADDING_PXL), 
                ImVec2(vKeyBindButtonPos.x + 100.0f, vKeyBindButtonPos.y + ImGui::GetFrameHeight() + (ImGui::GetFrameHeight() + SECTION_PADDING_PXL)), 
                ImColor(m_clrTheme.GetAsImVec4()));
            pDrawList->AddRect(vWindowPos, ImVec2(vWindowPos.x + 100.0f, vWindowPos.y + flPopupHeight), ImColor(m_clrTheme.GetAsImVec4()));
        }


        // Text
        {
            std::string szKeyBind = "KeyBind  : ";
            ImGui::SetCursorScreenPos(ImVec2(vKeyBindButtonPos.x, vKeyBindButtonPos.y + (flFrameHeight - ImGui::GetTextLineHeight()) / 2.0f));
            vKeyBindButtonPos.x += m_pFontFeatures->GetCharAdvance(' ') * szKeyBind.size();

            ImGui::TextColored(vTextClr.GetAsImVec4(), szKeyBind.c_str());
        }


        // Drawing & formatting Keybind button.
        {
            {
                ImVec4 vClrButton = (m_bRecordingKey == true ? m_clrTheme.GetAsImVec4() : vClrSectionBoxFullAlpha);
                RGBA_t clrText; CalcTextClrForBg(clrText, RGBA_t(vClrButton));

                ImGui::PushStyleColor(ImGuiCol_Button,        vClrButton);
                ImGui::PushStyleColor(ImGuiCol_ButtonHovered, vClrButton);
                ImGui::PushStyleColor(ImGuiCol_ButtonActive,  vClrButton);
                ImGui::PushStyleColor(ImGuiCol_Text,          clrText.GetAsImVec4());
            }

            ImGui::SetCursorScreenPos(vKeyBindButtonPos);
            if(ImGui::Button(
                (std::format("{}", VkToChar(pFeature->m_iKey) != 0 ? VkToChar(pFeature->m_iKey) : '~') + 
                "##PopupButton" + pFeature->m_szTabName + pFeature->m_szSectionName + pFeature->m_szFeatureDisplayName).c_str(), 
                ImVec2(flFrameHeight, flFrameHeight)) == true)
            {
                _StartRecordingKey();
            }

            ImGui::PushStyleColor(ImGuiCol_Text, vTextClr.GetAsImVec4());
            if(ImGui::IsItemHovered() == true)
                ImGui::SetTooltip("Current keybind for this feature. ( Click to change )");
            ImGui::PopStyleColor();

            {
                ImGui::PopStyleColor(4);
            }
        }
        

        // Drawing & formatting "Empty Keybind button".
        ImVec2 vClearKeybindButtonPos(vKeyBindButtonPos.x + flFrameHeight + INTER_FEATURE_PADDING_PXL, vKeyBindButtonPos.y);
        {
            {
                ImVec4 vkeyBindButtonClr(m_clrSectionBox.GetAsImVec4()); vkeyBindButtonClr.w = 1.0f;
                RGBA_t clrText; CalcTextClrForBg(clrText, m_clrTheme);

                ImGui::PushStyleColor(ImGuiCol_Button,        m_clrTheme.GetAsImVec4());
                ImGui::PushStyleColor(ImGuiCol_ButtonActive,  m_clrTheme.GetAsImVec4());
                ImGui::PushStyleColor(ImGuiCol_ButtonHovered, m_clrTheme.GetAsImVec4());
                ImGui::PushStyleColor(ImGuiCol_Text,          clrText.GetAsImVec4());
            }

            ImGui::SetCursorScreenPos(vClearKeybindButtonPos);
            if(ImGui::Button("X", ImVec2(flFrameHeight, flFrameHeight)) == true)
            {
                pFeature->m_iKey = 0;
            }

            ImGui::PushStyleColor(ImGuiCol_Text, vTextClr.GetAsImVec4());
            if(ImGui::IsItemHovered() == true)
                ImGui::SetTooltip("Clear keybind for this feature.");
            ImGui::PopStyleColor();


            {
                ImGui::PopStyleColor(4);
            }
        }

        // Did we finish recording a key.
        if(m_bRecordingKey == false && m_iRecordedKey != 0)
        {
            pFeature->m_iKey = m_iRecordedKey;
            m_iRecordedKey   = 0;
        }

        // Displaying hold / toggle setting.
        ImVec2 vHoldSettingTextPos(vWindowPos.x + SECTION_PADDING_PXL, vKeyBindButtonPos.y + ImGui::GetFrameHeight() + SECTION_PADDING_PXL);
        {
            ImVec2 vHoldSettingTextPosCentered(vHoldSettingTextPos.x, vHoldSettingTextPos.y + (ImGui::GetFrameHeight() - ImGui::GetTextLineHeight()) / 2.0f);
            ImGui::SetCursorScreenPos(vHoldSettingTextPosCentered);

            // Current Hold Setting.
            std::string szHoldStyle = std::format("Style    : [ {} ]", pFeature->m_iOverrideType == IFeature::OverrideType::OVERRIDE_HOLD ? " Hold " : "Toggle");
            ImGui::TextColored(vTextClr.GetAsImVec4(), szHoldStyle.c_str());
            
            // Hold setting toggle button.
            bool bSupportSingleStyle = (pFeature->m_iFlags & FeatureFlag_HoldOnlyKeyBind) || (pFeature->m_iFlags & FeatureFlag_ToggleOnlyKeyBind);
            if(bSupportSingleStyle == false)
            {
                ImGui::SetCursorScreenPos(ImVec2(vHoldSettingTextPos.x + INTER_FEATURE_PADDING_PXL + (szHoldStyle.size() * m_pFontFeatures->GetCharAdvance(' ')), vHoldSettingTextPos.y));

                // Styling Toggle button
                bool bIsHold = pFeature->m_iOverrideType == IFeature::OverrideType::OVERRIDE_HOLD;
                {
                    ImVec4 vClrButton = (bIsHold == true ? vClrSectionBoxFullAlpha : m_clrTheme.GetAsImVec4());
                    RGBA_t clrText; CalcTextClrForBg(clrText, RGBA_t(vClrButton));

                    ImGui::PushStyleColor(ImGuiCol_Button,        vClrButton);
                    ImGui::PushStyleColor(ImGuiCol_ButtonActive,  vClrButton);
                    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, vClrButton);
                    ImGui::PushStyleColor(ImGuiCol_Text,          clrText.GetAsImVec4());
                }


                if(ImGui::Button(
                    (std::string(bIsHold == true ? "H" : "T") + "##HoldStyleToggle" + pFeature->m_szTabName + pFeature->m_szSectionName + pFeature->m_szFeatureDisplayName).c_str(), 
                    ImVec2(flFrameHeight, flFrameHeight)) == true)
                {
                    pFeature->m_iOverrideType = 
                        bIsHold == true ? IFeature::OverrideType::OVERRIDE_TOGGLE : IFeature::OverrideType::OVERRIDE_HOLD; 
                }


                // Pop toggle button styles.
                {
                    ImGui::PopStyleColor(4);
                }
            }
        }
        
        // Drawing Override Data
        ImVec2 vOverrideDataPos(vWindowPos.x + SECTION_PADDING_PXL, vWindowPos.y + (ImGui::GetFrameHeight() * 2.0f) + (3.0f * SECTION_PADDING_PXL));

        Feature<ColorData_t>* pClrFeature = static_cast<Feature<ColorData_t>*>(pFeature);
        ImGui::SetCursorScreenPos(ImVec2(vOverrideDataPos.x, vOverrideDataPos.y + (flFrameHeight - ImGui::GetTextLineHeight()) / 2.0f));

        std::string szOverrideData("Override :");
        ImGui::TextColored(vTextClr.GetAsImVec4(), szOverrideData.c_str());


        ImGui::PushStyleColor(ImGuiCol_Border, m_clrTheme.GetAsImVec4());
        ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(2.0f, 2.0f));
        
        ImGui::SetCursorScreenPos(ImVec2(vOverrideDataPos.x + INTER_FEATURE_PADDING_PXL + (m_pFontFeatures->GetCharAdvance(' ') * szOverrideData.size()), vOverrideDataPos.y));
        ImGui::ColorEdit4(
            ("##Override" + pFeature->m_szTabName + pFeature->m_szSectionName + pFeature->m_szFeatureDisplayName).c_str(), 
            &pClrFeature->m_OverrideData.r, ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoLabel | ImGuiColorEditFlags_PickerHueWheel);

        ImGui::PopStyleColor(); ImGui::PopStyleVar();

        ImGui::EndPopup(); 
    }
    
    // Removing popup formatting.
    ImGui::PopStyleColor(2);
    ImGui::PopStyleVar(4);

    ImGui::PopFont();
}


///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
void MenuGUI_t::_DrawFloatSliderPopup(IFeature* pFeature, ImVec2 vWindowPos)
{
    ImGui::PushFont(m_pPopupFont);

    constexpr float nRows         = 3.0f;
    float           flPopupHeight = (SECTION_PADDING_PXL * (nRows + 1.0f)) + (nRows * ImGui::GetFrameHeight());
    ImGui::SetNextWindowSizeConstraints(ImVec2(100.0f, flPopupHeight), ImVec2(400.0f, flPopupHeight));

    // Formatting popup window
    ImVec4 vPopClr(m_clrPrimary.GetAsImVec4()); vPopClr.w = max(vPopClr.w, 0.8f);
    RGBA_t vTextClr; CalcTextClrForBg(vTextClr, m_clrPrimary);
    {
        ImGui::PushStyleColor(ImGuiCol_PopupBg, vPopClr);
        ImGui::PushStyleColor(ImGuiCol_Border,  m_clrTheme.GetAsImVec4());
        ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing,     ImVec2(0.0f, 0.0f));
        ImGui::PushStyleVar(ImGuiStyleVar_PopupBorderSize, POPUP_BORDER_THICKNESS);
        ImGui::PushStyleVar(ImGuiStyleVar_PopupRounding,   POPUP_ROUNDING);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding,  POPUP_ROUNDING);
    }
    

    ImGuiWindowFlags iPopFlags = ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoMove;
    if(ImGui::BeginPopup(("##Popup" + pFeature->m_szTabName + pFeature->m_szSectionName + pFeature->m_szFeatureDisplayName).c_str(), iPopFlags) == true)
    {
        float flFrameHeight = ImGui::GetFrameHeight();

        ImVec2 vKeyBindButtonPos(vWindowPos.x + SECTION_PADDING_PXL, vWindowPos.y + SECTION_PADDING_PXL);
        ImVec2 vCharacterSize(m_pFontFeatures->GetCharAdvance(' '), ImGui::GetTextLineHeight());
        ImVec2 vKeyBindScreenPos(vKeyBindButtonPos.x + (flFrameHeight - vCharacterSize.x) / 2.0f, vKeyBindButtonPos.y + (flFrameHeight - vCharacterSize.y) / 2.0f);

        ImVec4 vClrSectionBoxFullAlpha(m_clrSectionBox.GetAsImVec4()); vClrSectionBoxFullAlpha.w = 1.0f;
        
        // Debugging drawing
        if(Features::Menu::Menu::Draw_Guides.IsActive() == true)
        {
            ImDrawList* pDrawList = ImGui::GetForegroundDrawList();
            pDrawList->AddRect(vKeyBindButtonPos, ImVec2(vKeyBindButtonPos.x + 100.0f, vKeyBindButtonPos.y + ImGui::GetFrameHeight()), ImColor(m_clrTheme.GetAsImVec4()));
            pDrawList->AddRect(
                ImVec2(vKeyBindButtonPos.x, vKeyBindButtonPos.y + ImGui::GetFrameHeight() + SECTION_PADDING_PXL), 
                ImVec2(vKeyBindButtonPos.x + 100.0f, vKeyBindButtonPos.y + ImGui::GetFrameHeight() + (ImGui::GetFrameHeight() + SECTION_PADDING_PXL)), 
                ImColor(m_clrTheme.GetAsImVec4()));
            pDrawList->AddRect(vWindowPos, ImVec2(vWindowPos.x + 100.0f, vWindowPos.y + flPopupHeight), ImColor(m_clrTheme.GetAsImVec4()));
        }


        // Text
        {
            std::string szKeyBind = "KeyBind  : ";
            ImGui::SetCursorScreenPos(ImVec2(vKeyBindButtonPos.x, vKeyBindButtonPos.y + (flFrameHeight - ImGui::GetTextLineHeight()) / 2.0f));
            vKeyBindButtonPos.x += m_pFontFeatures->GetCharAdvance(' ') * szKeyBind.size();

            ImGui::TextColored(vTextClr.GetAsImVec4(), szKeyBind.c_str());
        }


        // Drawing & formatting Keybind button.
        {
            {
                ImVec4 vClrButton = (m_bRecordingKey == true ? m_clrTheme.GetAsImVec4() : vClrSectionBoxFullAlpha);
                RGBA_t clrText; CalcTextClrForBg(clrText, RGBA_t(vClrButton));

                ImGui::PushStyleColor(ImGuiCol_Button,        vClrButton);
                ImGui::PushStyleColor(ImGuiCol_ButtonHovered, vClrButton);
                ImGui::PushStyleColor(ImGuiCol_ButtonActive,  vClrButton);
                ImGui::PushStyleColor(ImGuiCol_Text,          clrText.GetAsImVec4());
            }

            ImGui::SetCursorScreenPos(vKeyBindButtonPos);
            if(ImGui::Button(
                (std::format("{}", VkToChar(pFeature->m_iKey) != 0 ? VkToChar(pFeature->m_iKey) : '~') + 
                "##PopupButton" + pFeature->m_szTabName + pFeature->m_szSectionName + pFeature->m_szFeatureDisplayName).c_str(), 
                ImVec2(flFrameHeight, flFrameHeight)) == true)
            {
                _StartRecordingKey();
            }

            ImGui::PushStyleColor(ImGuiCol_Text, vTextClr.GetAsImVec4());
            if(ImGui::IsItemHovered() == true)
                ImGui::SetTooltip("Current keybind for this feature. ( Click to change )");
            ImGui::PopStyleColor();

            {
                ImGui::PopStyleColor(4);
            }
        }
        

        // Drawing & formatting "Empty Keybind button".
        ImVec2 vClearKeybindButtonPos(vKeyBindButtonPos.x + flFrameHeight + INTER_FEATURE_PADDING_PXL, vKeyBindButtonPos.y);
        {
            {
                ImVec4 vkeyBindButtonClr(m_clrSectionBox.GetAsImVec4()); vkeyBindButtonClr.w = 1.0f;
                RGBA_t clrText; CalcTextClrForBg(clrText, m_clrTheme);

                ImGui::PushStyleColor(ImGuiCol_Button,        m_clrTheme.GetAsImVec4());
                ImGui::PushStyleColor(ImGuiCol_ButtonActive,  m_clrTheme.GetAsImVec4());
                ImGui::PushStyleColor(ImGuiCol_ButtonHovered, m_clrTheme.GetAsImVec4());
                ImGui::PushStyleColor(ImGuiCol_Text,          clrText.GetAsImVec4());
            }

            ImGui::SetCursorScreenPos(vClearKeybindButtonPos);
            if(ImGui::Button("X", ImVec2(flFrameHeight, flFrameHeight)) == true)
            {
                pFeature->m_iKey = 0;
            }

            ImGui::PushStyleColor(ImGuiCol_Text, vTextClr.GetAsImVec4());
            if(ImGui::IsItemHovered() == true)
                ImGui::SetTooltip("Clear keybind for this feature.");
            ImGui::PopStyleColor();


            {
                ImGui::PopStyleColor(4);
            }
        }

        // Did we finish recording a key.
        if(m_bRecordingKey == false && m_iRecordedKey != 0)
        {
            pFeature->m_iKey = m_iRecordedKey;
            m_iRecordedKey   = 0;
        }

        // Displaying hold / toggle setting.
        ImVec2 vHoldSettingTextPos(vWindowPos.x + SECTION_PADDING_PXL, vKeyBindButtonPos.y + ImGui::GetFrameHeight() + SECTION_PADDING_PXL);
        {
            ImVec2 vHoldSettingTextPosCentered(vHoldSettingTextPos.x, vHoldSettingTextPos.y + (ImGui::GetFrameHeight() - ImGui::GetTextLineHeight()) / 2.0f);
            ImGui::SetCursorScreenPos(vHoldSettingTextPosCentered);

            // Current Hold Setting.
            std::string szHoldStyle = std::format("Style    : [ {} ]", pFeature->m_iOverrideType == IFeature::OverrideType::OVERRIDE_HOLD ? " Hold " : "Toggle");
            ImGui::TextColored(vTextClr.GetAsImVec4(), szHoldStyle.c_str());
            
            // Hold setting toggle button.
            bool bSupportSingleStyle = (pFeature->m_iFlags & FeatureFlag_HoldOnlyKeyBind) || (pFeature->m_iFlags & FeatureFlag_ToggleOnlyKeyBind);
            if(bSupportSingleStyle == false)
            {
                ImGui::SetCursorScreenPos(ImVec2(vHoldSettingTextPos.x + INTER_FEATURE_PADDING_PXL + (szHoldStyle.size() * m_pFontFeatures->GetCharAdvance(' ')), vHoldSettingTextPos.y));

                // Styling Toggle button
                bool bIsHold = pFeature->m_iOverrideType == IFeature::OverrideType::OVERRIDE_HOLD;
                {
                    ImVec4 vClrButton = (bIsHold == true ? vClrSectionBoxFullAlpha : m_clrTheme.GetAsImVec4());
                    RGBA_t clrText; CalcTextClrForBg(clrText, RGBA_t(vClrButton));

                    ImGui::PushStyleColor(ImGuiCol_Button,        vClrButton);
                    ImGui::PushStyleColor(ImGuiCol_ButtonActive,  vClrButton);
                    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, vClrButton);
                    ImGui::PushStyleColor(ImGuiCol_Text,          clrText.GetAsImVec4());
                }


                if(ImGui::Button(
                    (std::string(bIsHold == true ? "H" : "T") + "##HoldStyleToggle" + pFeature->m_szTabName + pFeature->m_szSectionName + pFeature->m_szFeatureDisplayName).c_str(), 
                    ImVec2(flFrameHeight, flFrameHeight)) == true)
                {
                    pFeature->m_iOverrideType = 
                        bIsHold == true ? IFeature::OverrideType::OVERRIDE_TOGGLE : IFeature::OverrideType::OVERRIDE_HOLD; 
                }


                // Pop toggle button styles.
                {
                    ImGui::PopStyleColor(4);
                }
            }
        }
        
        // Drawing Override Data
        ImVec2 vOverrideDataPos(vWindowPos.x + SECTION_PADDING_PXL, vWindowPos.y + (ImGui::GetFrameHeight() * 2.0f) + (3.0f * SECTION_PADDING_PXL));

        // Make float slider transparent.
        ImGui::PushStyleColor(ImGuiCol_FrameBg,          ImVec4(0.0f, 0.0f, 0.0f, 0.0f));
        ImGui::PushStyleColor(ImGuiCol_FrameBgActive,    ImVec4(0.0f, 0.0f, 0.0f, 0.0f));
        ImGui::PushStyleColor(ImGuiCol_FrameBgHovered,   ImVec4(0.0f, 0.0f, 0.0f, 0.0f));
        ImGui::PushStyleColor(ImGuiCol_SliderGrab,       ImVec4(0.0f, 0.0f, 0.0f, 0.0f));
        ImGui::PushStyleColor(ImGuiCol_SliderGrabActive, ImVec4(0.0f, 0.0f, 0.0f, 0.0f));
        ImGui::PushStyleColor(ImGuiCol_Text,             ImVec4(0.0f, 0.0f, 0.0f, 0.0f));

        Feature<FloatSlider_t>* pFloatFeature = reinterpret_cast<Feature<FloatSlider_t>*>(pFeature);
        ImGui::SetCursorScreenPos(vOverrideDataPos);
        constexpr float TRACK_WIDTH = 200.0f; 
        ImGui::SetNextItemWidth(TRACK_WIDTH);
        ImGui::SliderFloat(
            ("##FloatSliderOverride" + pFeature->m_szTabName + pFeature->m_szSectionName + pFeature->m_szFeatureDisplayName).c_str(),
            &pFloatFeature->m_OverrideData.m_flVal, pFloatFeature->m_Data.m_flMin, pFloatFeature->m_Data.m_flMax);


        ImDrawList* pDrawList = ImGui::GetForegroundDrawList();


        // Drawing Track manually for better visuals.
        constexpr float TRACK_THICKNESS = 4.0f;
        ImVec2 vTrackOrigin(vOverrideDataPos.x, vOverrideDataPos.y + (ImGui::GetFrameHeight() / 2.0f));
        pDrawList->AddRectFilled(ImVec2(vTrackOrigin.x, vTrackOrigin.y - TRACK_THICKNESS), ImVec2(vTrackOrigin.x + TRACK_WIDTH, vTrackOrigin.y + TRACK_THICKNESS), ImColor(vClrSectionBoxFullAlpha), 5.0f);


        // Drawing Knob
        constexpr float KNOB_SIZE_PXL = 10.0f;
        float flKnobPos = (pFloatFeature->m_OverrideData.m_flVal - pFloatFeature->m_Data.m_flMin) / (pFloatFeature->m_Data.m_flMax - pFloatFeature->m_Data.m_flMin);
        flKnobPos      *= m_flPopupAnimation;
        flKnobPos       = std::clamp<float>(flKnobPos, 0.0f, 1.0f);

        // We are removing 2 KNOB_SIZE_PXL from x coordinates to keep the knob strictly on the track. It was overlapping onto other shit previously.
        ImVec2 vKnobOrigin(vTrackOrigin.x + KNOB_SIZE_PXL + ((TRACK_WIDTH - (2.0f * KNOB_SIZE_PXL)) * flKnobPos), vTrackOrigin.y);

        pDrawList->AddRectFilled(ImVec2(vKnobOrigin.x - KNOB_SIZE_PXL, vKnobOrigin.y - KNOB_SIZE_PXL), ImVec2(vKnobOrigin.x + KNOB_SIZE_PXL, vKnobOrigin.y + KNOB_SIZE_PXL), ImColor(m_clrTheme.GetAsImVec4()), 10000.0f);


        
        // Styling & drawing float input.
        ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, WIDGET_BORDER_THICKNESS);
        ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding,   5.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_ButtonTextAlign, ImVec2(0.5f, 0.5f));
        ImGui::PushStyleColor(ImGuiCol_Border,        m_clrTheme.GetAsImVec4());
        ImGui::PushStyleColor(ImGuiCol_Text,          ImVec4(0.0f, 0.0f, 0.0f, 0.0f));
        ImGui::PushStyleColor(ImGuiCol_FrameBg,       vClrSectionBoxFullAlpha);
        ImGui::PushStyleColor(ImGuiCol_FrameBgActive, m_clrTheme.GetAsImVec4());
        RGBA_t clrText; CalcTextClrForBg(clrText, vClrSectionBoxFullAlpha);


        ImVec2 vFloatInputScreenPos(vOverrideDataPos.x + TRACK_WIDTH + INTER_FEATURE_PADDING_PXL, vOverrideDataPos.y);
        ImGui::SetCursorScreenPos(vFloatInputScreenPos);
        constexpr float flFloatInputWidth = 50.0f;
        ImGui::SetNextItemWidth(flFloatInputWidth);

        ImGui::InputFloat(
            ("##PopupFloatInput" + pFloatFeature->m_szTabName + pFloatFeature->m_szSectionName + pFloatFeature->m_szFeatureDisplayName).c_str(),
            &pFloatFeature->m_OverrideData.m_flVal, 0.0f, 0.0f, "%.1f");

        ImGui::PopStyleVar(3);
        ImGui::PopStyleColor(4);

        std::string szSliderValue = std::format("{:.1f}", pFloatFeature->m_OverrideData.m_flVal * m_flPopupAnimation);
        size_t      nDigits       = szSliderValue.size();
        float       flCharWidth   = m_pFontFeatures->GetCharAdvance(' '); // Assuming using a mono font.
        pDrawList->AddText(
        ImVec2(vFloatInputScreenPos.x + ((flFloatInputWidth - (static_cast<float>(nDigits) * flCharWidth)) / 2.0f), vFloatInputScreenPos.y + ((ImGui::GetFrameHeight() - ImGui::GetTextLineHeight()) / 2.0f)),
        ImColor(clrText.GetAsImVec4()), szSliderValue.c_str());
        

        ImGui::PopStyleColor(6);
        ImGui::EndPopup(); 
 
    }
    
    // Removing popup formatting.
    ImGui::PopStyleColor(2);
    ImGui::PopStyleVar(4);

    ImGui::PopFont();
}


///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
void MenuGUI_t::_DrawIntSliderPopup(IFeature* pFeature, ImVec2 vWindowPos)
{
    ImGui::PushFont(m_pPopupFont);

    constexpr float nRows         = 3.0f;
    float           flPopupHeight = (SECTION_PADDING_PXL * (nRows + 1.0f)) + (nRows * ImGui::GetFrameHeight());
    ImGui::SetNextWindowSizeConstraints(ImVec2(100.0f, flPopupHeight), ImVec2(400.0f, flPopupHeight));

    // Formatting popup window
    ImVec4 vPopClr(m_clrPrimary.GetAsImVec4()); vPopClr.w = max(vPopClr.w, 0.8f);
    RGBA_t vTextClr; CalcTextClrForBg(vTextClr, m_clrPrimary);
    {
        ImGui::PushStyleColor(ImGuiCol_PopupBg, vPopClr);
        ImGui::PushStyleColor(ImGuiCol_Border,  m_clrTheme.GetAsImVec4());
        ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing,     ImVec2(0.0f, 0.0f));
        ImGui::PushStyleVar(ImGuiStyleVar_PopupBorderSize, POPUP_BORDER_THICKNESS);
        ImGui::PushStyleVar(ImGuiStyleVar_PopupRounding,   POPUP_ROUNDING);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding,  POPUP_ROUNDING);
    }
    

    ImGuiWindowFlags iPopFlags = ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoMove;
    if(ImGui::BeginPopup(("##Popup" + pFeature->m_szTabName + pFeature->m_szSectionName + pFeature->m_szFeatureDisplayName).c_str(), iPopFlags) == true)
    {
        float flFrameHeight = ImGui::GetFrameHeight();

        ImVec2 vKeyBindButtonPos(vWindowPos.x + SECTION_PADDING_PXL, vWindowPos.y + SECTION_PADDING_PXL);
        ImVec2 vCharacterSize(m_pFontFeatures->GetCharAdvance(' '), ImGui::GetTextLineHeight());
        ImVec2 vKeyBindScreenPos(vKeyBindButtonPos.x + (flFrameHeight - vCharacterSize.x) / 2.0f, vKeyBindButtonPos.y + (flFrameHeight - vCharacterSize.y) / 2.0f);

        ImVec4 vClrSectionBoxFullAlpha(m_clrSectionBox.GetAsImVec4()); vClrSectionBoxFullAlpha.w = 1.0f;
        
        // Debugging drawing
        if(Features::Menu::Menu::Draw_Guides.IsActive() == true)
        {
            ImDrawList* pDrawList = ImGui::GetForegroundDrawList();
            pDrawList->AddRect(vKeyBindButtonPos, ImVec2(vKeyBindButtonPos.x + 100.0f, vKeyBindButtonPos.y + ImGui::GetFrameHeight()), ImColor(m_clrTheme.GetAsImVec4()));
            pDrawList->AddRect(
                ImVec2(vKeyBindButtonPos.x, vKeyBindButtonPos.y + ImGui::GetFrameHeight() + SECTION_PADDING_PXL), 
                ImVec2(vKeyBindButtonPos.x + 100.0f, vKeyBindButtonPos.y + ImGui::GetFrameHeight() + (ImGui::GetFrameHeight() + SECTION_PADDING_PXL)), 
                ImColor(m_clrTheme.GetAsImVec4()));
            pDrawList->AddRect(vWindowPos, ImVec2(vWindowPos.x + 100.0f, vWindowPos.y + flPopupHeight), ImColor(m_clrTheme.GetAsImVec4()));
        }


        // Text
        {
            std::string szKeyBind = "KeyBind  : ";
            ImGui::SetCursorScreenPos(ImVec2(vKeyBindButtonPos.x, vKeyBindButtonPos.y + (flFrameHeight - ImGui::GetTextLineHeight()) / 2.0f));
            vKeyBindButtonPos.x += m_pFontFeatures->GetCharAdvance(' ') * szKeyBind.size();

            ImGui::TextColored(vTextClr.GetAsImVec4(), szKeyBind.c_str());
        }


        // Drawing & formatting Keybind button.
        {
            {
                ImVec4 vClrButton = (m_bRecordingKey == true ? m_clrTheme.GetAsImVec4() : vClrSectionBoxFullAlpha);
                RGBA_t clrText; CalcTextClrForBg(clrText, RGBA_t(vClrButton));

                ImGui::PushStyleColor(ImGuiCol_Button,        vClrButton);
                ImGui::PushStyleColor(ImGuiCol_ButtonHovered, vClrButton);
                ImGui::PushStyleColor(ImGuiCol_ButtonActive,  vClrButton);
                ImGui::PushStyleColor(ImGuiCol_Text,          clrText.GetAsImVec4());
            }

            ImGui::SetCursorScreenPos(vKeyBindButtonPos);
            if(ImGui::Button(
                (std::format("{}", VkToChar(pFeature->m_iKey) != 0 ? VkToChar(pFeature->m_iKey) : '~') + 
                "##PopupButton" + pFeature->m_szTabName + pFeature->m_szSectionName + pFeature->m_szFeatureDisplayName).c_str(), 
                ImVec2(flFrameHeight, flFrameHeight)) == true)
            {
                _StartRecordingKey();
            }

            ImGui::PushStyleColor(ImGuiCol_Text, vTextClr.GetAsImVec4());
            if(ImGui::IsItemHovered() == true)
                ImGui::SetTooltip("Current keybind for this feature. ( Click to change )");
            ImGui::PopStyleColor();

            {
                ImGui::PopStyleColor(4);
            }
        }
        

        // Drawing & formatting "Empty Keybind button".
        ImVec2 vClearKeybindButtonPos(vKeyBindButtonPos.x + flFrameHeight + INTER_FEATURE_PADDING_PXL, vKeyBindButtonPos.y);
        {
            {
                ImVec4 vkeyBindButtonClr(m_clrSectionBox.GetAsImVec4()); vkeyBindButtonClr.w = 1.0f;
                RGBA_t clrText; CalcTextClrForBg(clrText, m_clrTheme);
                ImGui::PushStyleColor(ImGuiCol_Button,        m_clrTheme.GetAsImVec4());
                ImGui::PushStyleColor(ImGuiCol_ButtonActive,  m_clrTheme.GetAsImVec4());
                ImGui::PushStyleColor(ImGuiCol_ButtonHovered, m_clrTheme.GetAsImVec4());
                ImGui::PushStyleColor(ImGuiCol_Text,          clrText.GetAsImVec4());
            }

            ImGui::SetCursorScreenPos(vClearKeybindButtonPos);
            if(ImGui::Button("X", ImVec2(flFrameHeight, flFrameHeight)) == true)
            {
                pFeature->m_iKey = 0;
            }

            ImGui::PushStyleColor(ImGuiCol_Text, vTextClr.GetAsImVec4());
            if(ImGui::IsItemHovered() == true)
                ImGui::SetTooltip("Clear keybind for this feature.");
            ImGui::PopStyleColor();


            {
                ImGui::PopStyleColor(4);
            }
        }

        // Did we finish recording a key.
        if(m_bRecordingKey == false && m_iRecordedKey != 0)
        {
            pFeature->m_iKey = m_iRecordedKey;
            m_iRecordedKey   = 0;
        }

        // Displaying hold / toggle setting.
        ImVec2 vHoldSettingTextPos(vWindowPos.x + SECTION_PADDING_PXL, vKeyBindButtonPos.y + ImGui::GetFrameHeight() + SECTION_PADDING_PXL);
        {
            ImVec2 vHoldSettingTextPosCentered(vHoldSettingTextPos.x, vHoldSettingTextPos.y + (ImGui::GetFrameHeight() - ImGui::GetTextLineHeight()) / 2.0f);
            ImGui::SetCursorScreenPos(vHoldSettingTextPosCentered);

            // Current Hold Setting.
            std::string szHoldStyle = std::format("Style    : [ {} ]", pFeature->m_iOverrideType == IFeature::OverrideType::OVERRIDE_HOLD ? " Hold " : "Toggle");
            ImGui::TextColored(vTextClr.GetAsImVec4(), szHoldStyle.c_str());
            
            // Hold setting toggle button.
            bool bSupportSingleStyle = (pFeature->m_iFlags & FeatureFlag_HoldOnlyKeyBind) || (pFeature->m_iFlags & FeatureFlag_ToggleOnlyKeyBind);
            if(bSupportSingleStyle == false)
            {
                ImGui::SetCursorScreenPos(ImVec2(vHoldSettingTextPos.x + INTER_FEATURE_PADDING_PXL + (szHoldStyle.size() * m_pFontFeatures->GetCharAdvance(' ')), vHoldSettingTextPos.y));

                // Styling Toggle button
                bool bIsHold = pFeature->m_iOverrideType == IFeature::OverrideType::OVERRIDE_HOLD;
                {
                    ImVec4 vClrButton = (bIsHold == true ? vClrSectionBoxFullAlpha : m_clrTheme.GetAsImVec4());
                    RGBA_t clrText; CalcTextClrForBg(clrText, RGBA_t(vClrButton));

                    ImGui::PushStyleColor(ImGuiCol_Button,        vClrButton);
                    ImGui::PushStyleColor(ImGuiCol_ButtonActive,  vClrButton);
                    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, vClrButton);
                    ImGui::PushStyleColor(ImGuiCol_Text,          clrText.GetAsImVec4());
                }


                if(ImGui::Button(
                    (std::string(bIsHold == true ? "H" : "T") + "##HoldStyleToggle" + pFeature->m_szTabName + pFeature->m_szSectionName + pFeature->m_szFeatureDisplayName).c_str(), 
                    ImVec2(flFrameHeight, flFrameHeight)) == true)
                {
                    pFeature->m_iOverrideType = 
                        bIsHold == true ? IFeature::OverrideType::OVERRIDE_TOGGLE : IFeature::OverrideType::OVERRIDE_HOLD; 
                }


                // Pop toggle button styles.
                {
                    ImGui::PopStyleColor(4);
                }
            }
        }
        
        // Drawing Override Data
        ImVec2 vOverrideDataPos(vWindowPos.x + SECTION_PADDING_PXL, vWindowPos.y + (ImGui::GetFrameHeight() * 2.0f) + (3.0f * SECTION_PADDING_PXL));

        // Make float slider transparent.
        ImGui::PushStyleColor(ImGuiCol_FrameBg,          ImVec4(0.0f, 0.0f, 0.0f, 0.0f));
        ImGui::PushStyleColor(ImGuiCol_FrameBgActive,    ImVec4(0.0f, 0.0f, 0.0f, 0.0f));
        ImGui::PushStyleColor(ImGuiCol_FrameBgHovered,   ImVec4(0.0f, 0.0f, 0.0f, 0.0f));
        ImGui::PushStyleColor(ImGuiCol_SliderGrab,       ImVec4(0.0f, 0.0f, 0.0f, 0.0f));
        ImGui::PushStyleColor(ImGuiCol_SliderGrabActive, ImVec4(0.0f, 0.0f, 0.0f, 0.0f));
        ImGui::PushStyleColor(ImGuiCol_Text,             ImVec4(0.0f, 0.0f, 0.0f, 0.0f));

        Feature<IntSlider_t>* pIntFeature = reinterpret_cast<Feature<IntSlider_t>*>(pFeature);
        ImGui::SetCursorScreenPos(vOverrideDataPos);
        constexpr float TRACK_WIDTH = 200.0f; 
        ImGui::SetNextItemWidth(TRACK_WIDTH);
        ImGui::SliderInt(
            ("##FloatSliderOverride" + pFeature->m_szTabName + pFeature->m_szSectionName + pFeature->m_szFeatureDisplayName).c_str(),
            &pIntFeature->m_OverrideData.m_iVal, pIntFeature->m_Data.m_iMin, pIntFeature->m_Data.m_iMax);


        ImDrawList* pDrawList = ImGui::GetForegroundDrawList();


        // Drawing Track manually for better visuals.
        constexpr float TRACK_THICKNESS = 4.0f;
        ImVec2 vTrackOrigin(vOverrideDataPos.x, vOverrideDataPos.y + (ImGui::GetFrameHeight() / 2.0f));
        pDrawList->AddRectFilled(ImVec2(vTrackOrigin.x, vTrackOrigin.y - TRACK_THICKNESS), ImVec2(vTrackOrigin.x + TRACK_WIDTH, vTrackOrigin.y + TRACK_THICKNESS), ImColor(vClrSectionBoxFullAlpha), 5.0f);


        // Drawing Knob
        constexpr float KNOB_SIZE_PXL = 10.0f;
        float flKnobPos = static_cast<float>(pIntFeature->m_OverrideData.m_iVal - pIntFeature->m_Data.m_iMin) / static_cast<float>(pIntFeature->m_Data.m_iMax - pIntFeature->m_Data.m_iMin);
        flKnobPos      *= m_flPopupAnimation;
        flKnobPos       = std::clamp<float>(flKnobPos, 0.0f, 1.0f);
        
        // We are removing 2 KNOB_SIZE_PXL from x coordinates to keep the knob strictly on the track. It was overlapping onto other shit previously.
        ImVec2 vKnobOrigin(vTrackOrigin.x + KNOB_SIZE_PXL + ((TRACK_WIDTH - (2.0f * KNOB_SIZE_PXL)) * flKnobPos), vTrackOrigin.y);

        pDrawList->AddRectFilled(ImVec2(vKnobOrigin.x - KNOB_SIZE_PXL, vKnobOrigin.y - KNOB_SIZE_PXL), ImVec2(vKnobOrigin.x + KNOB_SIZE_PXL, vKnobOrigin.y + KNOB_SIZE_PXL), ImColor(m_clrTheme.GetAsImVec4()), 10000.0f);


        
        // Styling & drawing float input.
        ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, WIDGET_BORDER_THICKNESS);
        ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding,   5.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_ButtonTextAlign, ImVec2(0.5f, 0.5f));
        ImGui::PushStyleColor(ImGuiCol_Border,        m_clrTheme.GetAsImVec4());
        ImGui::PushStyleColor(ImGuiCol_Text,          ImVec4(0.0f, 0.0f, 0.0f, 0.0f));
        ImGui::PushStyleColor(ImGuiCol_FrameBg,       vClrSectionBoxFullAlpha);
        ImGui::PushStyleColor(ImGuiCol_FrameBgActive, m_clrTheme.GetAsImVec4());
        RGBA_t clrText; CalcTextClrForBg(clrText, vClrSectionBoxFullAlpha);


        ImVec2 vFloatInputScreenPos(vOverrideDataPos.x + TRACK_WIDTH + INTER_FEATURE_PADDING_PXL, vOverrideDataPos.y);
        ImGui::SetCursorScreenPos(vFloatInputScreenPos);
        constexpr float flFloatInputWidth = 50.0f;
        ImGui::SetNextItemWidth(flFloatInputWidth);

        ImGui::InputInt(
            ("##PopupFloatInput" + pIntFeature->m_szTabName + pIntFeature->m_szSectionName + pIntFeature->m_szFeatureDisplayName).c_str(),
            &pIntFeature->m_OverrideData.m_iVal, 0, 0);

        ImGui::PopStyleVar(3);
        ImGui::PopStyleColor(4);

        std::string szSliderValue = std::format("{:.0f}", static_cast<float>(pIntFeature->m_OverrideData.m_iVal) * m_flPopupAnimation);
        size_t      nDigits       = szSliderValue.size();
        float       flCharWidth   = m_pFontFeatures->GetCharAdvance(' '); // Assuming using a mono font.
        pDrawList->AddText(
        ImVec2(vFloatInputScreenPos.x + ((flFloatInputWidth - (static_cast<float>(nDigits) * flCharWidth)) / 2.0f), vFloatInputScreenPos.y + ((ImGui::GetFrameHeight() - ImGui::GetTextLineHeight()) / 2.0f)),
        ImColor(clrText.GetAsImVec4()), szSliderValue.c_str());
        

        ImGui::PopStyleColor(6);
        ImGui::EndPopup(); 
 
    }
    
    // Removing popup formatting.
    ImGui::PopStyleColor(2);
    ImGui::PopStyleVar(4);

    ImGui::PopFont();
}


///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
void MenuGUI_t::_DrawBooleanPopup(IFeature* pFeature, ImVec2 vWindowPos)
{
    ImGui::PushFont(m_pPopupFont);

    constexpr float nRows         = 2.0f;
    float           flPopupHeight = (SECTION_PADDING_PXL * (nRows + 1.0f)) + (nRows * ImGui::GetFrameHeight());
    ImGui::SetNextWindowSizeConstraints(ImVec2(100.0f, flPopupHeight), ImVec2(400.0f, flPopupHeight));

    // Formatting popup window
    ImVec4 vPopClr(m_clrPrimary.GetAsImVec4()); vPopClr.w = max(vPopClr.w, 0.8f);
    RGBA_t vTextClr; CalcTextClrForBg(vTextClr, m_clrPrimary);
    {
        ImGui::PushStyleColor(ImGuiCol_PopupBg, vPopClr);
        ImGui::PushStyleColor(ImGuiCol_Border,  m_clrTheme.GetAsImVec4());
        ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing,     ImVec2(0.0f, 0.0f));
        ImGui::PushStyleVar(ImGuiStyleVar_PopupBorderSize, POPUP_BORDER_THICKNESS);
        ImGui::PushStyleVar(ImGuiStyleVar_PopupRounding,   POPUP_ROUNDING);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding,  POPUP_ROUNDING);
    }
    

    ImGuiWindowFlags iPopFlags = ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoMove;
    if(ImGui::BeginPopup(("##Popup" + pFeature->m_szTabName + pFeature->m_szSectionName + pFeature->m_szFeatureDisplayName).c_str(), iPopFlags) == true)
    {
        float flFrameHeight = ImGui::GetFrameHeight();

        ImVec2 vKeyBindButtonPos(vWindowPos.x + SECTION_PADDING_PXL, vWindowPos.y + SECTION_PADDING_PXL);
        ImVec2 vCharacterSize(m_pFontFeatures->GetCharAdvance(' '), ImGui::GetTextLineHeight());
        ImVec2 vKeyBindScreenPos(vKeyBindButtonPos.x + (flFrameHeight - vCharacterSize.x) / 2.0f, vKeyBindButtonPos.y + (flFrameHeight - vCharacterSize.y) / 2.0f);

        ImVec4 vClrSectionBoxFullAlpha(m_clrSectionBox.GetAsImVec4()); vClrSectionBoxFullAlpha.w = 1.0f;
        
        // Debugging drawing
        if(Features::Menu::Menu::Draw_Guides.IsActive() == true)
        {
            ImDrawList* pDrawList = ImGui::GetForegroundDrawList();
            pDrawList->AddRect(vKeyBindButtonPos, ImVec2(vKeyBindButtonPos.x + 100.0f, vKeyBindButtonPos.y + ImGui::GetFrameHeight()), ImColor(m_clrTheme.GetAsImVec4()));
            pDrawList->AddRect(
                ImVec2(vKeyBindButtonPos.x, vKeyBindButtonPos.y + ImGui::GetFrameHeight() + SECTION_PADDING_PXL), 
                ImVec2(vKeyBindButtonPos.x + 100.0f, vKeyBindButtonPos.y + ImGui::GetFrameHeight() + (ImGui::GetFrameHeight() + SECTION_PADDING_PXL)), 
                ImColor(m_clrTheme.GetAsImVec4()));
            pDrawList->AddRect(vWindowPos, ImVec2(vWindowPos.x + 100.0f, vWindowPos.y + flPopupHeight), ImColor(m_clrTheme.GetAsImVec4()));
        }


        // Text
        {
            std::string szKeyBind = "KeyBind  : ";
            ImGui::SetCursorScreenPos(ImVec2(vKeyBindButtonPos.x, vKeyBindButtonPos.y + (flFrameHeight - ImGui::GetTextLineHeight()) / 2.0f));
            vKeyBindButtonPos.x += m_pFontFeatures->GetCharAdvance(' ') * szKeyBind.size();

            ImGui::TextColored(vTextClr.GetAsImVec4(), szKeyBind.c_str());
        }


        // Drawing & formatting Keybind button.
        {
            {
                ImVec4 vButtonClr = (m_bRecordingKey == true ? m_clrTheme.GetAsImVec4() : vClrSectionBoxFullAlpha);
                RGBA_t clrText; CalcTextClrForBg(clrText, RGBA_t(vButtonClr));

                ImGui::PushStyleColor(ImGuiCol_Button,        vButtonClr);
                ImGui::PushStyleColor(ImGuiCol_ButtonHovered, vButtonClr);
                ImGui::PushStyleColor(ImGuiCol_ButtonActive,  vButtonClr);
                ImGui::PushStyleColor(ImGuiCol_Text,          clrText.GetAsImVec4());
            }

            ImGui::SetCursorScreenPos(vKeyBindButtonPos);
            if(ImGui::Button(
                (std::format("{}", VkToChar(pFeature->m_iKey) != 0 ? VkToChar(pFeature->m_iKey) : '~') + 
                "##PopupButton" + pFeature->m_szTabName + pFeature->m_szSectionName + pFeature->m_szFeatureDisplayName).c_str(), 
                ImVec2(flFrameHeight, flFrameHeight)) == true)
            {
                _StartRecordingKey();
            }

            ImGui::PushStyleColor(ImGuiCol_Text, vTextClr.GetAsImVec4());
            if(ImGui::IsItemHovered() == true)
                ImGui::SetTooltip("Current keybind for this feature. ( Click to change )");
            ImGui::PopStyleColor();

            {
                ImGui::PopStyleColor(4);
            }
        }
        

        // Drawing & formatting "Empty Keybind button".
        ImVec2 vClearKeybindButtonPos(vKeyBindButtonPos.x + flFrameHeight + INTER_FEATURE_PADDING_PXL, vKeyBindButtonPos.y);
        {
            {
                ImVec4 vkeyBindButtonClr(m_clrSectionBox.GetAsImVec4()); vkeyBindButtonClr.w = 1.0f;
                RGBA_t clrButton; CalcTextClrForBg(clrButton, m_clrPrimary);

                ImGui::PushStyleColor(ImGuiCol_Button,        m_clrTheme.GetAsImVec4());
                ImGui::PushStyleColor(ImGuiCol_ButtonActive,  m_clrTheme.GetAsImVec4());
                ImGui::PushStyleColor(ImGuiCol_ButtonHovered, m_clrTheme.GetAsImVec4());
                ImGui::PushStyleColor(ImGuiCol_Text,          clrButton.GetAsImVec4());
            }

            ImGui::SetCursorScreenPos(vClearKeybindButtonPos);
            if(ImGui::Button("X", ImVec2(flFrameHeight, flFrameHeight)) == true)
            {
                pFeature->m_iKey = 0;
            }

            ImGui::PushStyleColor(ImGuiCol_Text, vTextClr.GetAsImVec4());
            if(ImGui::IsItemHovered() == true)
                ImGui::SetTooltip("Clear keybind for this feature.");
            ImGui::PopStyleColor();


            {
                ImGui::PopStyleColor(4);
            }
        }

        // Did we finish recording a key.
        if(m_bRecordingKey == false && m_iRecordedKey != 0)
        {
            pFeature->m_iKey = m_iRecordedKey;
            m_iRecordedKey   = 0;
        }

        // Displaying hold / toggle setting.
        ImVec2 vHoldSettingTextPos(vWindowPos.x + SECTION_PADDING_PXL, vKeyBindButtonPos.y + ImGui::GetFrameHeight() + SECTION_PADDING_PXL);
        {
            ImVec2 vHoldSettingTextPosCentered(vHoldSettingTextPos.x, vHoldSettingTextPos.y + (ImGui::GetFrameHeight() - ImGui::GetTextLineHeight()) / 2.0f);
            ImGui::SetCursorScreenPos(vHoldSettingTextPosCentered);

            // Current Hold Setting.
            std::string szHoldStyle = std::format("Style    : [ {} ]", pFeature->m_iOverrideType == IFeature::OverrideType::OVERRIDE_HOLD ? " Hold " : "Toggle");
            ImGui::TextColored(vTextClr.GetAsImVec4(), szHoldStyle.c_str());
            
            // Hold setting toggle button.
            bool bSupportSingleStyle = (pFeature->m_iFlags & FeatureFlag_HoldOnlyKeyBind) || (pFeature->m_iFlags & FeatureFlag_ToggleOnlyKeyBind);
            if(bSupportSingleStyle == false)
            {
                ImGui::SetCursorScreenPos(ImVec2(vHoldSettingTextPos.x + INTER_FEATURE_PADDING_PXL + (szHoldStyle.size() * m_pFontFeatures->GetCharAdvance(' ')), vHoldSettingTextPos.y));

                // Styling Toggle button
                bool bIsHold = pFeature->m_iOverrideType == IFeature::OverrideType::OVERRIDE_HOLD;
                {
                    ImVec4 vClrActive = (bIsHold == true ? vClrSectionBoxFullAlpha : m_clrTheme.GetAsImVec4());
                    RGBA_t clrButton; CalcTextClrForBg(clrButton, RGBA_t(vClrActive));

                    ImGui::PushStyleColor(ImGuiCol_Button,        vClrActive);
                    ImGui::PushStyleColor(ImGuiCol_ButtonActive,  vClrActive);
                    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, vClrActive);
                    ImGui::PushStyleColor(ImGuiCol_Text,          clrButton.GetAsImVec4());
                }


                if(ImGui::Button(
                    (std::string(bIsHold == true ? "H" : "T") + "##HoldStyleToggle" + pFeature->m_szTabName + pFeature->m_szSectionName + pFeature->m_szFeatureDisplayName).c_str(), 
                    ImVec2(flFrameHeight, flFrameHeight)) == true)
                {
                    pFeature->m_iOverrideType = 
                        bIsHold == true ? IFeature::OverrideType::OVERRIDE_TOGGLE : IFeature::OverrideType::OVERRIDE_HOLD; 
                }


                // Pop toggle button styles.
                {
                    ImGui::PopStyleColor(4);
                }
            }
        }
        
        ImGui::EndPopup(); 
 
    }
    
    // Removing popup formatting.
    ImGui::PopStyleColor(2);
    ImGui::PopStyleVar(4);

    ImGui::PopFont();
}


///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
ImVec2 MenuGUI_t::_ClampWindowPos(const ImVec2 vWindowPos)
{
    // Purpose : If the complete window is visible in the frame, then the vWindowPos is returned as it is. 
    // else its clamped such that the complete window is visible in the frame.

    int iModelPreviewWidth = 0, iModelPreviewHeight = 0; F::modelPreview.GetRenderViewSize(iModelPreviewHeight, iModelPreviewWidth);

    ImGuiIO& io = ImGui::GetIO();
    
    float flMaxX = io.DisplaySize.x - (m_vMenuSize.x + iModelPreviewWidth + MENU_PADDING_IN_PXL);
    float flMaxY = io.DisplaySize.y - m_vMenuSize.y;

    ImVec2 vWindowPosClamped(vWindowPos);

    // Only clamp if initialized ( else assertion fail )
    if(flMaxX > 0.0f && flMaxY > 0.0f)
    {
        vWindowPosClamped.x = std::clamp<float>(vWindowPos.x, 0.0f, flMaxX);
        vWindowPosClamped.y = std::clamp<float>(vWindowPos.y, 0.0f, flMaxY);
    }

    return vWindowPosClamped;
}


///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
void MenuGUI_t::_StartRecordingKey()
{
    m_iRecordedKey  = 0;
    m_bRecordingKey = true;
}


///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
bool MenuGUI_t::ShouldRecordKey() const
{
    return m_bRecordingKey;
}


///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
void MenuGUI_t::ReturnRecordedKey(int64_t iKey)
{
    m_iRecordedKey  = iKey;
    m_bRecordingKey = false;
}


///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
void MenuGUI_t::GetPos(float& x, float& y) const
{
    x = m_vMenuPos.x; y = m_vMenuPos.y;
}


///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
void MenuGUI_t::GetSize(float& flWidth, float& flHeight) const
{
    flWidth = m_vMenuSize.x; flHeight = m_vMenuSize.y;
}


///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
RGBA_t MenuGUI_t::GetPrimaryClr() const
{
    return m_clrPrimary;
}


///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
RGBA_t MenuGUI_t::GetSecondaryClr() const
{
    return m_clrSecondary;
}


///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
RGBA_t MenuGUI_t::GetThemeClr() const
{
    return m_clrTheme;
}


///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
void MenuGUI_t::_CalculateColors()
{
    m_clrFeatureText = RGBA_t((unsigned char)255, 255, 255, 255);

    // Primary color
    {
        ColorData_t clrBottomLeft  = Features::Menu::Menu::ColorBottomLeft.GetData();
        ColorData_t clrBottomRight = Features::Menu::Menu::ColorBottomRight.GetData();
        ColorData_t clrTopLeft     = Features::Menu::Menu::ColorTopLeft.GetData();
        ColorData_t clrTopRight    = Features::Menu::Menu::ColorTopRight.GetData();

        // Averaging out the color for each corner.
        m_clrPrimary.r = static_cast<unsigned char>(((clrBottomLeft.r + clrBottomRight.r + clrTopLeft.r + clrTopRight.r) / 4.0f) * 255.0f);
        m_clrPrimary.g = static_cast<unsigned char>(((clrBottomLeft.g + clrBottomRight.g + clrTopLeft.g + clrTopRight.g) / 4.0f) * 255.0f);
        m_clrPrimary.b = static_cast<unsigned char>(((clrBottomLeft.b + clrBottomRight.b + clrTopLeft.b + clrTopRight.b) / 4.0f) * 255.0f);
        m_clrPrimary.a = static_cast<unsigned char>(((clrBottomLeft.a + clrBottomRight.a + clrTopLeft.a + clrTopRight.a) / 4.0f) * 255.0f);
    }

    // Secondary color
    {
        ColorData_t clrBottomLeft  = Features::Menu::SideMenu::ColorBottomLeft.GetData();
        ColorData_t clrBottomRight = Features::Menu::SideMenu::ColorBottomRight.GetData();
        ColorData_t clrTopLeft     = Features::Menu::SideMenu::ColorTopLeft.GetData();
        ColorData_t clrTopRight    = Features::Menu::SideMenu::ColorTopRight.GetData();

        // Averaging out the color for each corner.
        m_clrSecondary.r = static_cast<unsigned char>(((clrBottomLeft.r + clrBottomRight.r + clrTopLeft.r + clrTopRight.r) / 4.0f) * 255.0f);
        m_clrSecondary.g = static_cast<unsigned char>(((clrBottomLeft.g + clrBottomRight.g + clrTopLeft.g + clrTopRight.g) / 4.0f) * 255.0f);
        m_clrSecondary.b = static_cast<unsigned char>(((clrBottomLeft.b + clrBottomRight.b + clrTopLeft.b + clrTopRight.b) / 4.0f) * 255.0f);
        m_clrSecondary.a = static_cast<unsigned char>(((clrBottomLeft.a + clrBottomRight.a + clrTopLeft.a + clrTopRight.a) / 4.0f) * 255.0f);
    }

    // Section box color.
    {
        ColorData_t clrBottomLeft  = Features::Menu::SectionBoxes::ColorBottomLeft.GetData();
        ColorData_t clrBottomRight = Features::Menu::SectionBoxes::ColorBottomRight.GetData();
        ColorData_t clrTopLeft     = Features::Menu::SectionBoxes::ColorTopLeft.GetData();
        ColorData_t clrTopRight    = Features::Menu::SectionBoxes::ColorTopRight.GetData();

        // Averaging out the color for each corner.
        m_clrSectionBox.r = static_cast<unsigned char>(((clrBottomLeft.r + clrBottomRight.r + clrTopLeft.r + clrTopRight.r) / 4.0f) * 255.0f);
        m_clrSectionBox.g = static_cast<unsigned char>(((clrBottomLeft.g + clrBottomRight.g + clrTopLeft.g + clrTopRight.g) / 4.0f) * 255.0f);
        m_clrSectionBox.b = static_cast<unsigned char>(((clrBottomLeft.b + clrBottomRight.b + clrTopLeft.b + clrTopRight.b) / 4.0f) * 255.0f);
        m_clrSectionBox.a = static_cast<unsigned char>(((clrBottomLeft.a + clrBottomRight.a + clrTopLeft.a + clrTopRight.a) / 4.0f) * 255.0f);
    }

    // Theme
    m_clrTheme = Features::Menu::Theme::Theme.GetData().GetAsBytes();

    // Side menu buttom color
    {
        HSVA_t hsv = m_clrSecondary.ToHSVA();
        hsv.h = fmodf(hsv.h + 180.0f, 360.0f);
        //hsv.s = fmodf(hsv.s * 1.2f,   1.0f);
        //hsv.v = fmodf(hsv.v * 1.2f,   1.0f);
        // is there enough room that we can make the color distinct by making it brighter.
        if (hsv.s <= 0.85f && hsv.v <= 0.85f)
        {
            hsv.s += 0.15f; hsv.v += 0.15f;
        }
        else // Else change the gotdayem color. AAAAAAAHHHHHHH!!!!!!!!!!!!!!!!!!
        {
            hsv.s = 1.0f; hsv.v = 1.0f;
            hsv.h += 10.0f;
            if (hsv.h >= 360.0f)
            {
                hsv.h = fmodf(hsv.h, 360.0f);
            }
        }
        m_clrSideMenuButtons = hsv.ToRGBA();
    }
}


///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
void MenuGUI_t::CalcTextClrForBg(RGBA_t& vTextClrOut, const RGBA_t& vBgClr) const
{
    HSVA_t hsv = vBgClr.ToHSVA();

    // If color is bright enough, we use black, else white.
    if(hsv.v <= 0.5f)
        vTextClrOut.Set(255, 255, 255, 255);
    else
        vTextClrOut.Set(0, 0, 0, 255);
}


///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
void MenuGUI_t::_FindElevatedClr(RGBA_t& vClrOut, const RGBA_t& vBGClr) const
{
    HSVA_t hsv = vBGClr.ToHSVA();

    hsv.v = hsv.v >= 0.6f ? hsv.v - 0.3f : hsv.v + 0.3f;

    vClrOut = hsv.ToRGBA();
}


///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
void MenuGUI_t::CalculateAnim(const std::chrono::high_resolution_clock::time_point& animStartTime, float& flAnimationOut, const float flAnimCompleteTime)
{
    // If nearly complete, let it go.
    if(fabsf(ANIM_COMPLETE - flAnimationOut) < ANIM_COMPLETION_TOLERANCE)
    {
        flAnimationOut = ANIM_COMPLETE;
        return;
    }

    // Current time
    std::chrono::high_resolution_clock::time_point timeNow = std::chrono::high_resolution_clock::now();

    double flTimeSinceReset = static_cast<double>(std::chrono::duration_cast<std::chrono::milliseconds>(timeNow - animStartTime).count()) / 1000.0f;
    float  flTimeNormalized = static_cast<float>(flTimeSinceReset) / flAnimCompleteTime;

    flAnimationOut = 1.0f - powf(1.0f - flTimeNormalized, 3.0f); // animation = 1 - (1 - time)^3
    flAnimationOut = std::clamp<float>(flAnimationOut, ANIM_ZERO, ANIM_COMPLETE);
}


///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
void MenuGUI_t::ResetAnimation(std::chrono::high_resolution_clock::time_point& animStartTime, float& flAnimationOut)
{
    flAnimationOut = 0.0f;
    animStartTime  = std::chrono::high_resolution_clock::now();
}
