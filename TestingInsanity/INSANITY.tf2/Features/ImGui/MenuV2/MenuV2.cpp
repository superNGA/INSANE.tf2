#include "MenuV2.h"

#include <algorithm>
#include <chrono>
#include <cmath>
#include <cstdint>
#include <filesystem>

#include "../../../External Libraries/ImGui/imgui_internal.h"
#include "../../../Extra/math.h"
#include "../../../External Libraries/ImGui/imgui.h"
#include "../../../Hooks/DirectX Hook/DirectX_hook.h"
#include "../../../Resources/Fonts/FontManager.h"
#include "../../../SDK/class/IPanel.h"
#include "../../../SDK/class/IVEngineClient.h"
#include "../../../SDK/class/BaseEntity.h"
#include "../../FeatureHandler.h"
#include "../NotificationSystem/NotificationSystem.h"
#include "../../Config/ConfigHandler.h"
#include "../../ModelPreview/ModelPreview.h"
#include "../KeybindPanel/KeybindPanel.h"
#include "../../../Utility/Profiler/Profiler.h"


// Renderer
#include "../../Graphics Engine V2/Graphics.h"
#include "../../Graphics Engine V2/Draw Objects/Box/Box.h"
#include "../../Graphics Engine V2/Draw Objects/Line/Line.h"
#include "../../Graphics Engine V2/Draw Objects/Circle/Circle.h"



///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
MenuGUI_t::MenuGUI_t() : m_menuAnim(), m_popupAnim(), m_colorPickerAnim(), m_modelAnim(0.7f)
{
    m_clrPrimary.Init(); m_clrSecondary.Init(); m_clrTheme.Init();

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
    PROFILER_RECORD_FUNCTION(EndScene);

    if (m_bVisible == false)
        return;

    if (_Initialize() == false)
        return;
    {
        ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0.0f, 0.0f, 0.0f, 0.0f));
    }
    
    _CalculateColors();
    
    // Animation calculations.
    m_menuAnim.CalculateAnim();
    m_popupAnim.CalculateAnim();
    m_colorPickerAnim.CalculateAnim();
    m_modelAnim.CalculateAnim();
    m_configButtonAnim.CalculateAnim();
    m_configLoadAnim.CalculateAnim();
    m_newFileAnim.CalculateAnim();
    _AnimateModel();

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
    // Whenever trying to close menu, iterate over all features and find all features with keybinds.
    if (bVisible == false && m_bVisible == true)
    {
        Render::KeybindPanel.RefreshFeatureList();
    }

    m_bVisible = bVisible;

    if(m_pMainMenu != nullptr)
        m_pMainMenu->SetVisible(m_bVisible);

    if (m_pSideMenu != nullptr)
        m_pSideMenu->SetVisible(m_bVisible);

    for(IDrawObj_t* pDrawObjs : m_vecSectionBoxes)
        pDrawObjs->SetVisible(bVisible);

    if(bVisible == false)
    {
        m_menuAnim.Reset(); m_popupAnim.Reset();
        m_modelAnim.Reset();
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
    PROFILER_RECORD_FUNCTION(EndScene);

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

        float flAnimatedScroll = (ANIM_COMPLETE - m_menuAnim.GetAnimation()) * 50.0f;
        ImVec2 vWindowPosAnimated(vWindowPos.x + (flWidth * SIDEMENU_SCALE), vWindowPos.y - ImGui::GetScrollY() + flAnimatedScroll);
        
        if (m_pActiveTab != nullptr)
        {
            _DrawSections(m_pActiveTab, flWidth * (1.0f - SIDEMENU_SCALE), flHeight, vWindowPosAnimated.x, vWindowPosAnimated.y, vWindowPos);
        }
        else if (m_pActiveTab == nullptr && m_bConfigPanelActive == true)
        {
            _DrawConfigPanel(vWindowPosAnimated.x, vWindowPosAnimated.y, flWidth * (1.0f - SIDEMENU_SCALE), flHeight);
        }

        ImGui::End();
    }

    return vWindowPos;
}


///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
void MenuGUI_t::_DrawTabBar(float flWidth, float flHeight, float x, float y)
{
    PROFILER_RECORD_FUNCTION(EndScene);

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
                        ImVec2 vButtonAnimationMin((vCursorScreenPos.x + flWidth - (flWidth * m_menuAnim.GetAnimation())), vCursorScreenPos.y);
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
                    m_menuAnim.Reset();
                }
     
                ImGui::PushFont(Resources::Fonts::JetBrainsMonoNerd_Mid);
                float flIconWidth = ImGui::GetFont()->GetCharAdvance(' ');
                ImGui::GetWindowDrawList()->AddText(
                    ImVec2(vCursorScreenPos.x + flIconWidth, vCursorScreenPos.y + (vButtonSize.y - ImGui::GetTextLineHeight()) / 2.0f),
                    ImColor(m_pActiveTab == pTab ? m_clrTheme.GetAsImVec4() : clrCatagoryText.GetAsImVec4()),
                    reinterpret_cast<const char*>(vecIconList[iTabIndex])
                );
                ImGui::PopFont();

                // if we have drawn highlighting for this button ourselves, then we must remove it now.
                if(bNoHighlighting == true)
                    ImGui::PopStyleColor(3);

                vCursorScreenPos.y += vButtonSize.y;
            }

            {
                ImGui::PopStyleColor(4); ImGui::PopStyleVar(2);
            }
        }

        
        // Drawing the config panel button.
        constexpr float CONFIG_BUTTON_PADDING_IN_PXL = 5.0f;
        constexpr float CONFIG_BUTTON_HEIGHT_IN_PXL = 30.0f;
        ImVec2 vConfigButtonPos (x + CONFIG_BUTTON_PADDING_IN_PXL, y + flHeight - (CONFIG_BUTTON_PADDING_IN_PXL + CONFIG_BUTTON_HEIGHT_IN_PXL));
        ImVec2 vConfigButtonSize(flWidth - (2.0f * CONFIG_BUTTON_PADDING_IN_PXL), CONFIG_BUTTON_HEIGHT_IN_PXL);
        ImGui::SetNextWindowPos(vConfigButtonPos);

        ImGui::BeginChild("##ConfigButtonChild", vConfigButtonSize);
        {
            RGBA_t clrText; CalcTextClrForBg(clrText, m_clrSecondary);
            {
                ImGui::PushFont(Resources::Fonts::JetBrainsMonoNerd_Small);
                ImGui::PushStyleColor(ImGuiCol_Button, m_clrSecondary.GetAsImVec4());
                ImGui::PushStyleColor(ImGuiCol_ButtonActive, m_clrPrimary.GetAsImVec4());
                ImGui::PushStyleColor(ImGuiCol_ButtonHovered, m_clrPrimary.GetAsImVec4());
                ImGui::PushStyleColor(ImGuiCol_Text, clrText.GetAsImVec4());

                ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, WIDGET_ROUNDING);
            }

            ImGui::SetCursorScreenPos(vConfigButtonPos);

            // Drawing text & button
            if (ImGui::Button("##Config", ImVec2(flWidth - (2.0f * CONFIG_BUTTON_PADDING_IN_PXL), CONFIG_BUTTON_HEIGHT_IN_PXL)) == true)
            {
                m_bConfigPanelActive = true;
                m_pActiveTab         = nullptr;
                m_menuAnim.Reset();
            }

            const ImGuiIO& io = ImGui::GetIO();
            static bool bLastHoverState = false;
            bool bButtonHovered =
                (io.MousePos.x >= vConfigButtonPos.x && io.MousePos.x <= vConfigButtonPos.x + vConfigButtonSize.x) &&
                (io.MousePos.y >= vConfigButtonPos.y && io.MousePos.y <= vConfigButtonPos.y + vConfigButtonSize.y);

            if (bLastHoverState != bButtonHovered)
            {
                m_configButtonAnim.Reset();
                bLastHoverState = bButtonHovered;
            }

            // Drawing logo
            ImGui::PushFont(Resources::Fonts::JetBrainsMonoNerd_Mid);
            ImDrawList* pDrawList = ImGui::GetWindowDrawList();

            float flIconWidth = ImGui::GetFont()->GetCharAdvance(' ');
            float flHeightOffsetAnimated = (bButtonHovered == true ? m_configButtonAnim.GetAnimation() : (1.0f - m_configButtonAnim.GetAnimation()));
            flHeightOffsetAnimated *= vConfigButtonSize.y;

            pDrawList->AddText(
                ImVec2(vConfigButtonPos.x + (vConfigButtonSize.x - flIconWidth) / 2.0f, vConfigButtonPos.y - flHeightOffsetAnimated + (vConfigButtonSize.y - ImGui::GetTextLineHeight()) / 2.0f),
                ImColor(clrText.GetAsImVec4()), reinterpret_cast<const char*>(u8"\uf15c"));
            ImGui::PopFont();
            
            std::string szText("Config");
            float flTextWidth = szText.size() * ImGui::GetFont()->GetCharAdvance(' ');
            pDrawList->AddText(
                ImVec2(vConfigButtonPos.x + (vConfigButtonSize.x - flTextWidth) / 2.0f, vConfigButtonPos.y - flHeightOffsetAnimated + vConfigButtonSize.y + (vConfigButtonSize.y - ImGui::GetTextLineHeight()) / 2.0f),
                ImColor(m_clrTheme.GetAsImVec4()), szText.c_str());

            {
                ImGui::PopStyleVar();
                ImGui::PopStyleColor(4);
                ImGui::PopFont();
            }

        }
        ImGui::EndChild();


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
void MenuGUI_t::_DrawConfigPanel(float x, float y, float flWidth, float flHeight)
{
    // First of all, disable all section boxes.
    for (BoxFilled2D_t* pSectionBox : m_vecSectionBoxes)
    {
        pSectionBox->SetVisible(false);
    }

    // Constants.
    constexpr float PADDING_FROM_WALLS        = 20.0f;
    constexpr float CONFIG_INFO_HEIGHT_IN_PXL = 100.0f;
    constexpr float CONFIG_FILES_WIDTH_PERC   = 0.8f;


    // Config Info Panel
    ImVec2 vConfigInfoPos (x + PADDING_FROM_WALLS, y + PADDING_FROM_WALLS);
    ImVec2 vConfigInfoSize((flWidth * CONFIG_FILES_WIDTH_PERC) - (2.0f * PADDING_FROM_WALLS), CONFIG_INFO_HEIGHT_IN_PXL);
    
    const float flConfigInfoRounding = Features::Menu::SectionBoxes::Rounding.GetData().m_flVal;
    ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(1.0f, 1.0f, 1.0f, 0.0f));
    ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, flConfigInfoRounding);
    ImGui::SetCursorScreenPos(vConfigInfoPos);
    ImGui::BeginChild("##ConfigFileInfoChild", vConfigInfoSize);
    {
        _DrawConfigInfo(vConfigInfoPos, vConfigInfoSize, flConfigInfoRounding);
    }
    ImGui::EndChild();
    ImGui::PopStyleVar();
    ImGui::PopStyleColor();


    // Config List Window
    ImVec2 vConfigListPos (vConfigInfoPos.x, vConfigInfoPos.y + CONFIG_INFO_HEIGHT_IN_PXL + PADDING_FROM_WALLS);
    ImVec2 vConfigListSize(
        vConfigInfoSize.x,
        (y + flHeight - PADDING_FROM_WALLS) - vConfigListPos.y);

    {
        ImGui::PushStyleColor(ImGuiCol_WindowBg, m_clrSecondary.GetAsImVec4());
        ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, POPUP_ROUNDING);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
        ImGui::PushStyleVar(ImGuiStyleVar_FramePadding,  ImVec2(0.0f, 0.0f));
        ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing,   ImVec2(0.0f, 0.0f));
    }

    ImGui::SetNextWindowPos(vConfigListPos);
    ImGui::SetNextWindowSize(vConfigListSize);
    
    if (ImGui::Begin("##AllConfigFiles", nullptr, ImGuiWindowFlags_NoDecoration) == true)
    {
        _DrawConfigList(vConfigListSize);
    }

    ImGui::PopStyleVar(4); ImGui::PopStyleColor();

    // Config Buttons ( save, set-as-default, etc... )
    ImVec2 vConfigButtonPos (vConfigListPos.x + vConfigListSize.x + (PADDING_FROM_WALLS / 2.0f), vConfigListPos.y);
    ImVec2 vConfigButtonSize((x + flWidth - (PADDING_FROM_WALLS / 2.0f)) - vConfigButtonPos.x, 30.0f);
    _DrawConfigButtons(vConfigButtonPos, vConfigButtonSize);

    // Unload button.
    {
        ImVec2 vDeloadButtonPos(vConfigButtonPos.x, vConfigListPos.y + vConfigListSize.y - vConfigButtonSize.y);
        ImGui::SetCursorScreenPos(vDeloadButtonPos);
        ImGui::PushFont(Resources::Fonts::JetBrainsMonoNerd_Mid);

        RGBA_t clrButtonActive; _FindElevatedClr(clrButtonActive, m_clrPrimary);
        ImGui::PushStyleColor(ImGuiCol_Button,          m_clrSecondary.GetAsImVec4());
        ImGui::PushStyleColor(ImGuiCol_ButtonActive,    clrButtonActive.GetAsImVec4());
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered,   clrButtonActive.GetAsImVec4());
        ImGui::PushStyleColor(ImGuiCol_Border,          m_clrTheme.GetAsImVec4());
        ImGui::PushStyleColor(ImGuiCol_Text,            m_clrTheme.GetAsImVec4());

        ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, WIDGET_BORDER_THICKNESS);
        ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding,   WIDGET_ROUNDING);

        if (ImGui::Button(reinterpret_cast<const char*>(u8"\uf011"), vConfigButtonSize) == true)
        {
            directX::UI::shutdown_UI = true; // bye my nigga :)
        }
        ImGui::PopStyleColor(5); ImGui::PopStyleVar(2);
        ImGui::PopFont();
    }
}


///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
void MenuGUI_t::_DrawConfigInfo(ImVec2 vConfigInfoPos, ImVec2 vConfigInfoSize, const float flConfigInfoRounding)
{
    ImDrawList* pDrawList = ImGui::GetWindowDrawList();

    // Drawing Config info box
    pDrawList->AddRectFilled(
        vConfigInfoPos, ImVec2(vConfigInfoPos.x + vConfigInfoSize.x, vConfigInfoPos.y + vConfigInfoSize.y), // Min & Max
        ImColor(m_clrSecondary.GetAsImVec4()),
        flConfigInfoRounding);

    // outline for config info box
    pDrawList->AddRect(
        vConfigInfoPos, ImVec2(vConfigInfoPos.x + vConfigInfoSize.x, vConfigInfoPos.y + vConfigInfoSize.y), // Min & Max
        ImColor(m_clrTheme.GetAsImVec4()),
        flConfigInfoRounding, 0, WIDGET_BORDER_THICKNESS);


    // Drawing file config imfo
    if (m_iLoadedConfigIndex >= 0)
    {
        constexpr float ITEM_PADDING = 5.0f;

        ImVec2 vCursorPos(vConfigInfoPos.x + ITEM_PADDING, vConfigInfoPos.y + vConfigInfoSize.y - ITEM_PADDING);
        vCursorPos.x += ITEM_PADDING;
        vCursorPos.y -= ImGui::GetTextLineHeight();
        
        RGBA_t clrText; CalcTextClrForBg(clrText, m_clrSecondary);

        pDrawList->AddText(
            vCursorPos, ImColor(clrText.GetAsImVec4()), std::format("Date : {}", m_szLoadedConfigLastModifyTime).c_str()
        );

        vCursorPos.y -= ImGui::GetTextLineHeight();
        pDrawList->AddText(
            vCursorPos, ImColor(clrText.GetAsImVec4()), std::format("Size : {} bytes", m_iLoadedConfigSize).c_str()
        );

        vCursorPos.y -= ImGui::GetTextLineHeight();
        pDrawList->AddText(
            vCursorPos, ImColor(clrText.GetAsImVec4()), std::format("Path : {}", m_szLoadedConfigPath).c_str()
        );


        ImGui::PushFont(Resources::Fonts::JetBrainsMonoNerd_Large);
        ImVec2 vIconPos(vCursorPos);
        vIconPos.x += flConfigInfoRounding;
        vIconPos.y = vConfigInfoPos.y + ((vCursorPos.y - vConfigInfoPos.y) - ImGui::GetTextLineHeight()) / 2.0f;
        float flIconWidth = ImGui::GetFont()->GetCharAdvance(' ');
        pDrawList->AddText(
            vIconPos, ImColor(clrText.GetAsImVec4()), reinterpret_cast<const char*>(u8"\uf15c")
        );
        ImGui::PopFont();


        ImGui::PushFont(Resources::Fonts::MontserratBlack);
        std::string szFileName = m_szLoadedConfigName;
        const std::string szExtension(".INSANE");
        if (szFileName.ends_with(szExtension)) 
        {
            szFileName.erase(szFileName.size() - szExtension.size());
        }
        ImVec2 vTextSize = ImGui::CalcTextSize(szFileName.c_str());

        ImGui::PushFont(Resources::Fonts::ShadowIntoLight);
        ImVec2 vFileExtensionSize = ImGui::CalcTextSize(szExtension.c_str());
        ImGui::PopFont();

        ImVec2 vConfigNamePos(
            vConfigInfoPos.x + (vConfigInfoSize.x - (vTextSize.x + vFileExtensionSize.x)) / 2.0f,
            vConfigInfoPos.y + ((vCursorPos.y - vConfigInfoPos.y) - ImGui::GetTextLineHeight()) / 2.0f
        );


        pDrawList->AddText(
            vConfigNamePos, ImColor(clrText.GetAsImVec4()), szFileName.c_str()
        );
        ImGui::PushFont(Resources::Fonts::ShadowIntoLight);
        pDrawList->AddText(
            ImVec2(vConfigNamePos.x + vTextSize.x, vConfigNamePos.y + (vTextSize.y - vFileExtensionSize.y) + 2.0f), ImColor(m_clrTheme.GetAsImVec4()), szExtension.c_str()
        );
        ImGui::PopFont();
        ImGui::PopFont();
    }

    // NOTE : we draw the flash in the end so it draws over everything. 
    //        and yes, you can notice the text over the flash & it doesn't look nice.
    // Config info box's animation
    if (m_iLoadedConfigIndex >= 0)
    {
        //const float flFlashWidth = Features::Menu::SideMenu::AnimAccentSize.GetData().m_flVal * 10.0f;
        const float flFlashWidth = 300.0f;
        ImVec2 vAnimMax(
            vConfigInfoPos.x + vConfigInfoSize.x + flFlashWidth - ((vConfigInfoSize.x + flFlashWidth) * m_configLoadAnim.GetAnimation()),
            vConfigInfoPos.y + vConfigInfoSize.y);
        ImVec2 vAnimMin(vAnimMax.x - flFlashWidth, vConfigInfoPos.y);
        pDrawList->AddRectFilled(vAnimMin, vAnimMax, ImColor(m_clrTheme.GetAsImVec4()));
    }
}


///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
void MenuGUI_t::_DrawConfigList(ImVec2 vConfigListSize)
{
    ImDrawList* pDrawList = ImGui::GetWindowDrawList();

    ImVec2 vWindowPos(ImGui::GetWindowPos());
    ImVec2 vCursorPos(vWindowPos.x + SECTION_PADDING_PXL, vWindowPos.y + SECTION_PADDING_PXL);

    constexpr float PADDING_FROM_WALLS = SECTION_PADDING_PXL; // Space between config entry and surrounding n shit.
    constexpr float PADDING_BETWEEN_ENTIRES = 5.0f; // Space between entires.
    constexpr float CONFIG_ENTRY_HEIGHT = 50.0f; // Height of each config entry.
    constexpr float CONFIG_ENTRY_ITEM_SPACING = 5.0f; // space between config entry walls & its contents.
    ImVec2 vConfigEntrySize(vConfigListSize.x - (2.0f * PADDING_FROM_WALLS), CONFIG_ENTRY_HEIGHT);


    const std::vector<std::string>& vecAllConfigs = configHandler.GetAllConfigFile();
    for (int iConfigIndex = 0; iConfigIndex < vecAllConfigs.size(); iConfigIndex++)
    {
        const std::string& szConfigName = vecAllConfigs[iConfigIndex];

        // Drawing a button behind the config entry ( we gonna draw ) so can do some logic.
        ImGui::SetCursorScreenPos(vCursorPos);
        {
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.0f, 0.0f, 0.0f, 0.0f));
            ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.0f, 0.0f, 0.0f, 0.0f));
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.0f, 0.0f, 0.0f, 0.0f));
            ImGui::PushItemFlag(ImGuiItemFlags_AllowOverlap, true);
            if (ImGui::Button(("##" + szConfigName).c_str(), vConfigEntrySize) == true)
            {
                m_iActiveConfigIndex = iConfigIndex;
            }
            ImGui::PopItemFlag();
            ImGui::PopStyleColor(3);
        }

        pDrawList->AddRectFilled(
            vCursorPos, ImVec2(vCursorPos.x + vConfigEntrySize.x, vCursorPos.y + vConfigEntrySize.y),
            ImColor(m_clrPrimary.GetAsImVec4()), POPUP_ROUNDING
        );

        if (iConfigIndex == m_iActiveConfigIndex)
        {
            pDrawList->AddRect(
                vCursorPos, ImVec2(vCursorPos.x + vConfigEntrySize.x, vCursorPos.y + vConfigEntrySize.y),
                ImColor(m_clrTheme.GetAsImVec4()), POPUP_ROUNDING, 0, WIDGET_BORDER_THICKNESS
            );
        }

        // NOTE : entry is of primary color, draw on a secondary color window, which is draw on a primary color window.
        RGBA_t clrText; CalcTextClrForBg(clrText, m_clrPrimary);

        ImGui::PushFont(Resources::Fonts::JetBrainsMonoNerd_Mid);
        ImVec2 vFileIconPos(vCursorPos.x + CONFIG_ENTRY_ITEM_SPACING, vCursorPos.y + (vConfigEntrySize.y / 2.0f) - ImGui::GetTextLineHeight());
        ImColor clrConfigName(m_iLoadedConfigIndex == iConfigIndex ? m_clrTheme.GetAsImVec4() : clrText.GetAsImVec4());
        pDrawList->AddText(vFileIconPos, clrConfigName, reinterpret_cast<const char*>(u8"\uf15c"));

        ImGui::PopFont();

        vFileIconPos.y = vCursorPos.y + vConfigEntrySize.y - CONFIG_ENTRY_ITEM_SPACING - ImGui::GetTextLineHeight();
        pDrawList->AddText(vFileIconPos, clrConfigName, szConfigName.c_str());

        // Styling buttons.
        {
            RGBA_t clrButtonActive; _FindElevatedClr(clrButtonActive, m_clrPrimary);
            ImGui::PushStyleColor(ImGuiCol_Button, m_clrSecondary.GetAsImVec4());
            ImGui::PushStyleColor(ImGuiCol_ButtonActive, clrButtonActive.GetAsImVec4());
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, clrButtonActive.GetAsImVec4());

            ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, POPUP_ROUNDING);
        }


        ImGui::PushID(szConfigName.c_str());
        ImGui::PushFont(Resources::Fonts::JetBrainsMonoNerd_Mid);
        ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0.0f, 0.0f));
        constexpr float CONFIG_BUTTON_PERCENTAGE = 0.2f;
        ImVec2 vConfigButtonSize(
            vConfigEntrySize.y - (2.0f * CONFIG_ENTRY_ITEM_SPACING),
            vConfigEntrySize.y - (2.0f * CONFIG_ENTRY_ITEM_SPACING));

        ImVec2 vConfigButtonPos(
            vCursorPos.x + vConfigEntrySize.x - CONFIG_ENTRY_ITEM_SPACING - vConfigButtonSize.x,
            vCursorPos.y + vConfigEntrySize.y - CONFIG_ENTRY_ITEM_SPACING - vConfigButtonSize.y);
        ImGui::SetCursorScreenPos(vConfigButtonPos);
        // Load config button.
        if (ImGui::Button(reinterpret_cast<const char*>(u8"\uf409"), vConfigButtonSize) == true)
        {
            configHandler.ReadConfigFile(szConfigName);
            Render::notificationSystem.PushBack("successfully loaded config %s", szConfigName.c_str());

            m_iLoadedConfigIndex = iConfigIndex;
            m_iActiveConfigIndex = -1; // Don't highlight the config once loaded. ( this would give a "consumed" sort of feeling )
            m_configLoadAnim.Reset();

            _GetFileInfo(szConfigName);
        }
        ImGui::PopStyleVar();
        ImGui::PopFont();
        ImGui::PopID();

        {
            ImGui::PopStyleColor(3); ImGui::PopStyleVar();
        }

        // Draw animation if this config is loaded or this file is newly created.
        bool bNewlyAddedFile = m_newFileAnim.IsComplete() == false && iConfigIndex == m_iNewFileIndex;
        if (m_iLoadedConfigIndex == iConfigIndex || bNewlyAddedFile == true)
        {
            // Choose whose animation we are doing here. ( new file or config loaded )
            float flAnimation = bNewlyAddedFile == true ? m_newFileAnim.GetAnimation() : m_configLoadAnim.GetAnimation();

            // Drawing animation.
            float flFlashWidth = Features::Menu::SideMenu::AnimAccentSize.GetData().m_flVal;
            ImVec2 vAnimMax(
                vCursorPos.x + vConfigEntrySize.x + flFlashWidth - ((vConfigEntrySize.x + flFlashWidth) * flAnimation),
                vCursorPos.y + vConfigEntrySize.y);
            ImVec2 vAnimMin(vAnimMax.x - flFlashWidth, vCursorPos.y);

            pDrawList->AddRectFilled(vAnimMin, vAnimMax, ImColor(m_clrTheme.GetAsImVec4()));
        }

        // Updating cursor pos for the next entry.
        vCursorPos.y += vConfigEntrySize.y + PADDING_BETWEEN_ENTIRES;
    }

    ImGui::End();
}


///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
void MenuGUI_t::_DrawConfigButtons(ImVec2 vConfigButtonPos, ImVec2 vConfigButtonSize)
{
    constexpr float PADDING_BETWEEN_CONFIG_BUTTONS = 5.0f; // Space between config setting buttons. ( 3 buttons on the right )
    constexpr float BUTTON_ITEM_SPACING_IN_PXL     = 5.0f; // Space between config setting buttons. ( 3 buttons on the right )

    RGBA_t clrText;         CalcTextClrForBg(clrText, m_clrSecondary);
    RGBA_t clrButtonActive; _FindElevatedClr(clrButtonActive, m_clrPrimary);
    {
        //RGBA_t clrButton = m_iActiveConfigIndex == -1 ? m_clrPrimary : m_clrSecondary;
        RGBA_t clrButton = m_clrSecondary;
        if (m_iActiveConfigIndex == -1) // This is so that the button seems unreactive when no config is selected.
            clrButtonActive = clrButton;

        ImGui::PushStyleColor(ImGuiCol_Button, clrButton.GetAsImVec4());
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, clrButtonActive.GetAsImVec4());
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, clrButtonActive.GetAsImVec4());
        ImGui::PushStyleColor(ImGuiCol_Text, clrText.GetAsImVec4());

        ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, WIDGET_ROUNDING);
    }
    
    ImDrawList* pDrawList = ImGui::GetWindowDrawList();

    // Drawing text input field for user to enter file name.
    static char s_szFileName[0xFF] = "";
    {
        ImGui::PushFont(Resources::Fonts::JetBrainsMonoNerd_Mid);
        ImVec2 vIconPos(vConfigButtonPos.x, vConfigButtonPos.y + (vConfigButtonSize.y - ImGui::GetTextLineHeight()) / 2.0f);
        float flIconWidth = ImGui::GetFont()->GetCharAdvance(' ');
        pDrawList->AddText( vIconPos, ImColor(clrText.GetAsImVec4()), reinterpret_cast<const char*>(u8"\uf11c"));
        ImGui::PopFont();

        // NOTE : for some reason, this keyboard icon is taking up space for 2 character in a mono font. so notice we are using 2 * icon width below.
        ImGui::SetNextItemWidth(vConfigButtonSize.x - ((flIconWidth * 2.0f) + BUTTON_ITEM_SPACING_IN_PXL));
        ImGui::SetCursorScreenPos(ImVec2(vConfigButtonPos.x + ((flIconWidth * 2.0f) + BUTTON_ITEM_SPACING_IN_PXL), vConfigButtonPos.y));
        ImGui::PushStyleColor(ImGuiCol_FrameBg, m_clrSecondary.GetAsImVec4());
        if (ImGui::InputText("##ConfigFileName", s_szFileName, sizeof(s_szFileName), ImGuiInputTextFlags_EnterReturnsTrue) == true)
        {
            LOG(s_szFileName);
        }
        ImGui::PopStyleColor();
    }


    // Drawing "Save Config" button.
    {
        vConfigButtonPos.y += ImGui::GetFrameHeight() + PADDING_BETWEEN_CONFIG_BUTTONS;
        RGBA_t clrCreateConfigHovered; _FindElevatedClr(clrCreateConfigHovered, m_clrPrimary);
        ImGui::PushStyleColor(ImGuiCol_ButtonActive,  clrCreateConfigHovered.GetAsImVec4());
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, clrCreateConfigHovered.GetAsImVec4());

        ImGui::SetCursorScreenPos(vConfigButtonPos);
        if (ImGui::Button(("##CreateConfig"), vConfigButtonSize) == true)
        {
            bool bFileCreated = false;

            {
                const std::vector<std::string>& vecAllConfigs = configHandler.GetAllConfigFile();

                std::string szLoadedConfigName("INVALID_FILE_NAME");
                if(m_iLoadedConfigIndex >= 0 && m_iLoadedConfigIndex < vecAllConfigs.size())
                {
                    szLoadedConfigName = vecAllConfigs[m_iLoadedConfigIndex];
                }

                std::string szActiveConfigName("INVALID_FILE_NAME");
                if (m_iActiveConfigIndex >= 0 && m_iActiveConfigIndex < vecAllConfigs.size())
                {
                    szActiveConfigName = vecAllConfigs[m_iActiveConfigIndex];
                }

                // set them as "Invalid" for now, try to find then again, so we have correct indexes. :)
                m_iLoadedConfigIndex = -1;
                m_iActiveConfigIndex = -1;

                
                bFileCreated = configHandler.CreateConfigFile(s_szFileName);


                // now try to find the config thats loaded.
                const std::vector<std::string>& vecUpdatedConfigs = configHandler.GetAllConfigFile();
                size_t nConfigs = vecUpdatedConfigs.size();
                std::string szExpectedNewFileName = s_szFileName + configHandler.GetExtension();
                for (int iConfigIndex = 0; iConfigIndex < nConfigs; iConfigIndex++)
                {
                    if (vecUpdatedConfigs[iConfigIndex] == szActiveConfigName)
                    {
                        m_iActiveConfigIndex = iConfigIndex;
                        LOG("Updated active config index to : %d", m_iLoadedConfigIndex);
                    }

                    if (vecUpdatedConfigs[iConfigIndex] == szLoadedConfigName)
                    {
                        m_iLoadedConfigIndex = iConfigIndex;
                        LOG("Updated loaded config index to : %d", m_iLoadedConfigIndex);
                    }

                    if (vecUpdatedConfigs[iConfigIndex] == szExpectedNewFileName)
                    {
                        m_iNewFileIndex = iConfigIndex;
                        LOG("New file allocated index : %d", iConfigIndex);
                    }
                }
            }

            if (bFileCreated == true)
            {
                LOG("Created config file [ %s ]", s_szFileName);
                Render::notificationSystem.PushBack("Created config file %s", s_szFileName);
                s_szFileName[0] = '\0';
                m_newFileAnim.Reset();
            }
            else
            {
                FAIL_LOG("Failed to create config file [ %s ]", s_szFileName);
                Render::notificationSystem.PushBack("Failed to Created config file %s", s_szFileName);
            }
        }

        ImGui::PopStyleColor(2);

        ImGui::PushFont(Resources::Fonts::JetBrainsMonoNerd_Mid);
        pDrawList->AddText(
            ImVec2(vConfigButtonPos.x + BUTTON_ITEM_SPACING_IN_PXL, vConfigButtonPos.y + (vConfigButtonSize.y - ImGui::GetTextLineHeight()) / 2.0f),
            ImColor(clrText.GetAsImVec4()), reinterpret_cast<const char*>(u8"\uf4d0"));
        ImGui::PopFont();

        pDrawList->AddText(
            ImVec2(vConfigButtonPos.x + BUTTON_ITEM_SPACING_IN_PXL, vConfigButtonPos.y + (vConfigButtonSize.y - ImGui::GetTextLineHeight()) / 2.0f),
            ImColor(clrText.GetAsImVec4()), "   Create");
    }


    // Drawing "Save Config" button.
    {
        vConfigButtonPos.y += PADDING_BETWEEN_CONFIG_BUTTONS + vConfigButtonSize.y;
        ImGui::SetCursorScreenPos(vConfigButtonPos);
        if (ImGui::Button(("##SaveConfig"), vConfigButtonSize) == true)
        {
            const std::vector<std::string>& vecAllConfigs = configHandler.GetAllConfigFile();

            // Must be a valid file
            if (m_iActiveConfigIndex >= 0 && m_iActiveConfigIndex < vecAllConfigs.size())
            {
                configHandler.WriteToConfigFile(vecAllConfigs[m_iActiveConfigIndex]);
                Render::notificationSystem.PushBack("Saved to config %s", vecAllConfigs[m_iActiveConfigIndex].c_str());
                LOG("saved config my nigga!");
            }
            else
            {
                FAIL_LOG("Selected a file first dumb ass.");
                Render::notificationSystem.PushBack("Select a file first, dumb ass.");
            }
        }

        ImGui::PushFont(Resources::Fonts::JetBrainsMonoNerd_Mid);
        pDrawList->AddText(
            ImVec2(vConfigButtonPos.x + BUTTON_ITEM_SPACING_IN_PXL, vConfigButtonPos.y + (vConfigButtonSize.y - ImGui::GetTextLineHeight()) / 2.0f),
            ImColor(clrText.GetAsImVec4()), reinterpret_cast<const char*>(u8"\uf0c7"));
        ImGui::PopFont();

        pDrawList->AddText(
            ImVec2(vConfigButtonPos.x + BUTTON_ITEM_SPACING_IN_PXL, vConfigButtonPos.y + (vConfigButtonSize.y - ImGui::GetTextLineHeight()) / 2.0f),
            ImColor(clrText.GetAsImVec4()), "   Save");
    }


    // Drawing "Set as default config" button
    {
        vConfigButtonPos.y += PADDING_BETWEEN_CONFIG_BUTTONS + vConfigButtonSize.y;
        ImGui::SetCursorScreenPos(vConfigButtonPos);
        if (ImGui::Button("##SetAsDefault", vConfigButtonSize) == true)
        {
            LOG("TODO : Make this load default config feautre");
        }

        ImGui::PushFont(Resources::Fonts::JetBrainsMonoNerd_Mid);
        pDrawList->AddText(
            ImVec2(vConfigButtonPos.x + BUTTON_ITEM_SPACING_IN_PXL, vConfigButtonPos.y + (vConfigButtonSize.y - ImGui::GetTextLineHeight()) / 2.0f),
            ImColor(clrText.GetAsImVec4()), reinterpret_cast<const char*>(u8"\uf005"));
        ImGui::PopFont();

        pDrawList->AddText(
            ImVec2(vConfigButtonPos.x + BUTTON_ITEM_SPACING_IN_PXL, vConfigButtonPos.y + (vConfigButtonSize.y - ImGui::GetTextLineHeight()) / 2.0f),
            ImColor(clrText.GetAsImVec4()), "   Set Default");
    }


    // Drawing "Delete config" button.
    {
        vConfigButtonPos.y += PADDING_BETWEEN_CONFIG_BUTTONS + vConfigButtonSize.y;
        ImGui::SetCursorScreenPos(vConfigButtonPos);
        if (ImGui::Button("##DeleteConfig", vConfigButtonSize) == true)
        {
            const std::vector<std::string>& vecAllConfigs = configHandler.GetAllConfigFile();

            // Must be a valid file
            if (m_iActiveConfigIndex >= 0 && m_iActiveConfigIndex < vecAllConfigs.size())
            {
                Render::notificationSystem.PushBack("Deleted config file %s", vecAllConfigs[m_iActiveConfigIndex].c_str());

                // Incase a config file is loaded we need to maintain a valid loaded config index, 
                // for that we need to re-aquire loaded config index after deleting current file.
                if(m_iLoadedConfigIndex >= 0 && m_iLoadedConfigIndex < vecAllConfigs.size())
                {
                    std::string szLoadedConfigName = vecAllConfigs[m_iLoadedConfigIndex];

                    configHandler.DeleteConfigFile(vecAllConfigs[m_iActiveConfigIndex]);

                    m_iLoadedConfigIndex = -1; // set "no config loaded" by default.

                    // now try to find the config thats loaded.
                    const std::vector<std::string>& vecUpdatedConfigsList = configHandler.GetAllConfigFile();
                    size_t nConfigs = vecUpdatedConfigsList.size();
                    for (int iConfigIndex = 0; iConfigIndex < nConfigs; iConfigIndex++)
                    {
                        if (vecUpdatedConfigsList[iConfigIndex] == szLoadedConfigName)
                        {
                            m_iLoadedConfigIndex = iConfigIndex;
                            LOG("Updated loaded config index to : %d", m_iLoadedConfigIndex);
                        }
                    }

                    WIN_LOG("Deleted file @ index [ %d ]", m_iActiveConfigIndex);
                    m_iActiveConfigIndex = -1; // since this files needs to be selected to be deleted, no file must now be selected.
                }
                else // if no config file is loaded, then just yank that shit.
                {
                    configHandler.DeleteConfigFile(vecAllConfigs[m_iActiveConfigIndex]);
                }
            }
            else
            {
                FAIL_LOG("Selected a file first dumb ass.");
                Render::notificationSystem.PushBack("No file selected");
            }
        }

        ImGui::PushFont(Resources::Fonts::JetBrainsMonoNerd_Mid);
        pDrawList->AddText(
            ImVec2(vConfigButtonPos.x + BUTTON_ITEM_SPACING_IN_PXL, vConfigButtonPos.y + (vConfigButtonSize.y - ImGui::GetTextLineHeight()) / 2.0f),
            ImColor(clrText.GetAsImVec4()), reinterpret_cast<const char*>(u8"\uf014"));
        ImGui::PopFont();

        pDrawList->AddText(
            ImVec2(vConfigButtonPos.x + BUTTON_ITEM_SPACING_IN_PXL, vConfigButtonPos.y + (vConfigButtonSize.y - ImGui::GetTextLineHeight()) / 2.0f),
            ImColor(clrText.GetAsImVec4()), "   Delete");
    }

    ImGui::PopStyleColor(4); ImGui::PopStyleVar();
}


///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
void MenuGUI_t::_GetFileInfo(const std::string& szFilePath)
{
    try
    {
        m_szLoadedConfigName = szFilePath;
        std::filesystem::path p(szFilePath);

        m_szLoadedConfigPath = std::filesystem::absolute(p).string();
        m_iLoadedConfigSize  = std::filesystem::file_size(p);

        // Last modification time
        auto ftime = std::filesystem::last_write_time(p);
        auto sctp  = 
            std::chrono::time_point_cast<std::chrono::system_clock::duration>(
                ftime - std::filesystem::file_time_type::clock::now()
                + std::chrono::system_clock::now());

        std::time_t cftime = std::chrono::system_clock::to_time_t(sctp);
        std::tm* tmLocal = std::localtime(&cftime);

        std::ostringstream oss;
        oss << std::put_time(tmLocal, "%Y-%m-%d %H:%M:%S");
        m_szLoadedConfigLastModifyTime = oss.str();
    }
    catch (const std::exception& e) 
    {
        FAIL_LOG(e.what());
    }

    LOG("File name : %s", m_szLoadedConfigName.c_str());
    LOG("File path : %s", m_szLoadedConfigPath.c_str());
    LOG("Mod. time : %s", m_szLoadedConfigLastModifyTime.c_str());
    LOG("size      : %llu bytes", m_iLoadedConfigSize);
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
        RGBA_t clrText; CalcTextClrForBg(clrText, m_clrPrimary);
        ImGui::PushStyleColor(ImGuiCol_PopupBg, vClrPopup);
        ImGui::PushStyleColor(ImGuiCol_Border, m_clrTheme.GetAsImVec4());
        ImGui::PushStyleColor(ImGuiCol_Text, clrText.GetAsImVec4());
        ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, POPUP_ROUNDING);

        if (ImGui::IsItemHovered() == true && pBoolFeature->m_szToolTip.empty() == false)
            ImGui::SetTooltip(pBoolFeature->m_szToolTip.c_str());

        ImGui::PopStyleColor(3);
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
        RGBA_t clrText; CalcTextClrForBg(clrText, m_clrPrimary);
        ImGui::PushStyleColor(ImGuiCol_PopupBg, vClrPopup);
        ImGui::PushStyleColor(ImGuiCol_Border, m_clrTheme.GetAsImVec4());
        ImGui::PushStyleColor(ImGuiCol_Text, clrText.GetAsImVec4());
        ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, POPUP_ROUNDING);

        if (ImGui::IsItemHovered() == true && pIntFeature->m_szToolTip.empty() == false)
        {
            ImGui::SetTooltip(pIntFeature->m_szToolTip.c_str());
        }

        ImGui::PopStyleColor(3);
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
            m_popupAnim.Reset();
        }

        _DrawIntSliderPopup(pFeature, vWindowPos);

        ImGui::PopStyleColor(4); ImGui::PopStyleVar(2);
    }


    // Drawing knob.
    float flKnobPos = static_cast<float>(pIntFeature->GetData().m_iVal - pIntFeature->m_Data.m_iMin) / static_cast<float>(pIntFeature->m_Data.m_iMax - pIntFeature->m_Data.m_iMin);
    flKnobPos      *= m_menuAnim.GetAnimation(); // Animating :)
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

    std::string szSliderValue = std::format("{:.0f}", static_cast<float>(pIntFeature->GetData().m_iVal) * m_menuAnim.GetAnimation());
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
        RGBA_t clrText; CalcTextClrForBg(clrText, m_clrPrimary);
        ImGui::PushStyleColor(ImGuiCol_PopupBg, vClrPopup);
        ImGui::PushStyleColor(ImGuiCol_Border, m_clrTheme.GetAsImVec4());
        ImGui::PushStyleColor(ImGuiCol_Text,   clrText.GetAsImVec4());
        ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, POPUP_ROUNDING);

        if (ImGui::IsItemHovered() == true && pFloatFeature->m_szToolTip.empty() == false)
        {
            ImGui::SetTooltip(pFloatFeature->m_szToolTip.c_str());
        }

        ImGui::PopStyleColor(3);
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
            m_popupAnim.Reset();
        }

        _DrawFloatSliderPopup(pFeature, vWindowPos);
        ImGui::PopStyleColor(4); ImGui::PopStyleVar(2);
    }


    // Drawing knob.
    constexpr float KNOB_SIZE_PXL = 10.0f;
    float flKnobPos = (pFloatFeature->GetData().m_flVal - pFloatFeature->m_Data.m_flMin) / (pFloatFeature->m_Data.m_flMax - pFloatFeature->m_Data.m_flMin);
    flKnobPos      *= m_menuAnim.GetAnimation(); // Animating :)
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

    std::string szSliderValue = std::format("{:.1f}", pFloatFeature->GetData().m_flVal * m_menuAnim.GetAnimation());
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
        RGBA_t clrText; CalcTextClrForBg(clrText, m_clrPrimary);
        ImGui::PushStyleColor(ImGuiCol_PopupBg, vClrPopup);
        ImGui::PushStyleColor(ImGuiCol_Border, m_clrTheme.GetAsImVec4());
        ImGui::PushStyleColor(ImGuiCol_Text, clrText.GetAsImVec4());
        ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, POPUP_ROUNDING);

        if (ImGui::IsItemHovered() == true && pDropDownFeature->m_szToolTip.empty() == false)
            ImGui::SetTooltip(pDropDownFeature->m_szToolTip.c_str());

        ImGui::PopStyleColor(3);
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

                return y + ((x - y) * m_colorPickerAnim.GetAnimation());
            };
            if(fabsf(ANIM_COMPLETE - m_colorPickerAnim.GetAnimation()) <= ANIM_COMPLETION_TOLERANCE)
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
                m_colorPickerAnim.Reset();
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
        RGBA_t clrText; CalcTextClrForBg(clrText, m_clrPrimary);
        ImGui::PushStyleColor(ImGuiCol_PopupBg, vClrPopup);
        ImGui::PushStyleColor(ImGuiCol_Border, m_clrTheme.GetAsImVec4());
        ImGui::PushStyleColor(ImGuiCol_Text, clrText.GetAsImVec4());
        ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, POPUP_ROUNDING);

        if (ImGui::IsItemHovered() == true && pColorFeature->m_szToolTip.empty() == false)
            ImGui::SetTooltip(pColorFeature->m_szToolTip.c_str());

        ImGui::PopStyleColor(3);
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
            m_popupAnim.Reset();
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
        ImVec2 vWindowPos    = ImGui::GetWindowPos();
        float  flFrameHeight = ImGui::GetFrameHeight();

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
        ImVec2 vWindowPos    = ImGui::GetWindowPos();
        float  flFrameHeight = ImGui::GetFrameHeight();

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
        flKnobPos      *= m_popupAnim.GetAnimation();
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

        std::string szSliderValue = std::format("{:.1f}", pFloatFeature->m_OverrideData.m_flVal * m_popupAnim.GetAnimation());
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
        ImVec2 vWindowPos    = ImGui::GetWindowPos();
        float  flFrameHeight = ImGui::GetFrameHeight();

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
        flKnobPos      *= m_popupAnim.GetAnimation();
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

        std::string szSliderValue = std::format("{:.0f}", static_cast<float>(pIntFeature->m_OverrideData.m_iVal) * m_popupAnim.GetAnimation());
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
        ImVec2 vWindowPos    = ImGui::GetWindowPos();
        float  flFrameHeight = ImGui::GetFrameHeight();

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
void MenuGUI_t::_AnimateModel()
{
    BaseEntity* pModelEnt = F::modelPreview.GetModelEntity();
    if (pModelEnt == nullptr)
        return;

    bool bAnimationCompleted = fabsf(ANIM_COMPLETE - m_modelAnim.GetAnimation()) <= ANIM_COMPLETION_TOLERANCE;
    if (bAnimationCompleted == false)
    {
        pModelEnt->GetAbsAngles().yaw = Features::MaterialGen::ModelPreview::ModelAbsAngle.GetData().m_flVal * m_modelAnim.GetAnimation();
    }
}


///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
bool MenuGUI_t::DrawIntSlider(const char* szLabel, ImVec2 vMin, ImVec2 vMax, int* pData, const int iMin, const int iMax, RGBA_t clrBackground, const float* pTrackThickness, const float* pKnowSize, const float* pAnimationState)
{
    {
        static ImVec4 vTransparent(0.0f, 0.0f, 0.0f, 0.0f);
        ImGui::PushStyleColor(ImGuiCol_FrameBg,          vTransparent);
        ImGui::PushStyleColor(ImGuiCol_FrameBgActive,    vTransparent);
        ImGui::PushStyleColor(ImGuiCol_FrameBgHovered,   vTransparent);
        ImGui::PushStyleColor(ImGuiCol_SliderGrab,       vTransparent);
        ImGui::PushStyleColor(ImGuiCol_SliderGrabActive, vTransparent);
        ImGui::PushStyleColor(ImGuiCol_Text,             vTransparent);
    }

    float flTrackThickness = (pTrackThickness == nullptr ? TRACK_THICKNESS_PXL : *pTrackThickness);
    float flKnobSize       = (pKnowSize       == nullptr ? KNOB_SIZE_PXL       : *pKnowSize);
    float flAnimationState = (pAnimationState == nullptr ? 1.0f                : *pAnimationState);

    ImVec2 vWidgetSize(vMax.x - vMin.x, vMax.y - vMin.y);
    float flFrameHeight  = ImGui::GetFrameHeight();

    ImVec2 vSliderOrigin(vMin.x, vWidgetSize.y <= flFrameHeight ? vMin.y : vMin.y + (vWidgetSize.y - flFrameHeight) / 2.0f);

    ImGui::SetCursorScreenPos(vSliderOrigin); ImGui::SetNextItemWidth(vWidgetSize.x);
    bool bDataModified = ImGui::SliderInt(szLabel, pData, iMin, iMax);

    // Calculating Clr :)
    // Don't just plain black if background color is really light, use secondary color ( assuming use is sane and sober and is not using same color 
    // primary and secondary. )
    RGBA_t clrTrack; CalcTextClrForBg(clrTrack, clrBackground);
    if(clrTrack.r == 0x0 && clrTrack.g == 0x0 && clrTrack.b == 0x0)
    {
        if(clrBackground == m_clrSecondary)
        {
            clrTrack = m_clrPrimary;
        }
        else 
        {
            clrTrack = m_clrSecondary;
        }
    }

    ImDrawList* pDrawList = ImGui::GetWindowDrawList();

    // Drawing tha track.
    ImVec2 vTrackOrigin(vMin.x, vMin.y + (vWidgetSize.y / 2.0f));
    pDrawList->AddRectFilled(
        ImVec2(vTrackOrigin.x, vTrackOrigin.y - flTrackThickness),
        ImVec2(vMax.x, vTrackOrigin.y + flTrackThickness),
        ImColor(clrTrack.GetAsImVec4()), 1000.0f
    );

    // Calculating knobs position ( with animation )
    float flKnobPos = static_cast<float>(*pData - iMin) / static_cast<float>(iMax - iMin);
    flKnobPos *= flAnimationState; // Animating.
    float flEffectiveTrackWidth = vWidgetSize.x - (2.0f * flKnobSize); // if we don't reduce the knob size we knob will hang over the ends.
    ImVec2 vKnowOrigin(vMin.x + flKnobSize + (flEffectiveTrackWidth * flKnobPos), vTrackOrigin.y);

    pDrawList->AddRectFilled(
        ImVec2(vKnowOrigin.x - flKnobSize, vKnowOrigin.y - flKnobSize), 
        ImVec2(vKnowOrigin.x + flKnobSize, vKnowOrigin.y + flKnobSize), 
        ImColor(m_clrTheme.GetAsImVec4()), 1000.0f
    );


    {
        ImGui::PopStyleColor(6);
    }

    return bDataModified;
}


///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
bool MenuGUI_t::DrawFloatSlider(const char* szLabel, ImVec2 vMin, ImVec2 vMax, float* pData, const float flMin, const int flMax, RGBA_t clrBackground, const float* pTrackThickness, const float* pKnowSize, const float* pAnimationState)
{
    {
        static ImVec4 vTransparent(0.0f, 0.0f, 0.0f, 0.0f);
        ImGui::PushStyleColor(ImGuiCol_FrameBg,          vTransparent);
        ImGui::PushStyleColor(ImGuiCol_FrameBgActive,    vTransparent);
        ImGui::PushStyleColor(ImGuiCol_FrameBgHovered,   vTransparent);
        ImGui::PushStyleColor(ImGuiCol_SliderGrab,       vTransparent);
        ImGui::PushStyleColor(ImGuiCol_SliderGrabActive, vTransparent);
        ImGui::PushStyleColor(ImGuiCol_Text,             vTransparent);
    }

    float flTrackThickness = (pTrackThickness == nullptr ? TRACK_THICKNESS_PXL : *pTrackThickness);
    float flKnobSize       = (pKnowSize       == nullptr ? KNOB_SIZE_PXL       : *pKnowSize);
    float flAnimationState = (pAnimationState == nullptr ? 1.0f                : *pAnimationState);

    ImVec2 vWidgetSize(vMax.x - vMin.x, vMax.y - vMin.y);
    float flFrameHeight  = ImGui::GetFrameHeight();

    ImVec2 vSliderOrigin(vMin.x, vWidgetSize.y <= flFrameHeight ? vMin.y : vMin.y + (vWidgetSize.y - flFrameHeight) / 2.0f);

    ImGui::SetCursorScreenPos(vSliderOrigin); ImGui::SetNextItemWidth(vWidgetSize.x);
    bool bDataModified = ImGui::SliderFloat(szLabel, pData, flMin, flMax, "%.1f");

    // Calculating Clr :)
    // Don't just plain black if background color is really light, use secondary color ( assuming use is sane and sober and is not using same color 
    // primary and secondary. )
    RGBA_t clrTrack; CalcTextClrForBg(clrTrack, clrBackground);
    if(clrTrack.r == 0x0 && clrTrack.g == 0x0 && clrTrack.b == 0x0)
    {
        if(clrBackground == m_clrSecondary)
        {
            clrTrack = m_clrPrimary;
        }
        else 
        {
            clrTrack = m_clrSecondary;
        }
    }

    ImDrawList* pDrawList = ImGui::GetWindowDrawList();

    // Drawing tha track.
    ImVec2 vTrackOrigin(vMin.x, vMin.y + (vWidgetSize.y / 2.0f));
    pDrawList->AddRectFilled(
        ImVec2(vTrackOrigin.x, vTrackOrigin.y - flTrackThickness),
        ImVec2(vMax.x, vTrackOrigin.y + flTrackThickness),
        ImColor(clrTrack.GetAsImVec4()), 1000.0f
    );

    // Calculating knobs position ( with animation )
    float flKnobPos = static_cast<float>(*pData - flMin) / static_cast<float>(flMax - flMin);
    flKnobPos *= flAnimationState; // Animating.
    float flEffectiveTrackWidth = vWidgetSize.x - (2.0f * flKnobSize); // if we don't reduce the knob size we knob will hang over the ends.
    ImVec2 vKnowOrigin(vMin.x + flKnobSize + (flEffectiveTrackWidth * flKnobPos), vTrackOrigin.y);

    pDrawList->AddRectFilled(
        ImVec2(vKnowOrigin.x - flKnobSize, vKnowOrigin.y - flKnobSize), 
        ImVec2(vKnowOrigin.x + flKnobSize, vKnowOrigin.y + flKnobSize), 
        ImColor(m_clrTheme.GetAsImVec4()), 1000.0f
    );


    {
        ImGui::PopStyleColor(6);
    }

    return bDataModified;
}


///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
void MenuGUI_t::DrawIntInput(const char* szLabel, ImVec2 vMin, ImVec2 vMax, int* pData, const int* pMin, const int* pMax, RGBA_t* pFrameClr, const float* pAnimation)
{
    RGBA_t clrFrame = (pFrameClr == nullptr ? m_clrSecondary : *pFrameClr);
    float  flAnimation = (pAnimation == nullptr ? 1.0f : *pAnimation);
    RGBA_t clrText; CalcTextClrForBg(clrText, clrFrame);

    ImVec2 vWidgetSize(vMax.x - vMin.x, vMax.y - vMin.y);
    float flFrameHeight = ImGui::GetFrameHeight();
    
    // Styling & drawing float input.
    ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize,  WIDGET_BORDER_THICKNESS);
    ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding,    5.0f);
    ImGui::PushStyleColor(ImGuiCol_Border,              m_clrTheme.GetAsImVec4());
    ImGui::PushStyleColor(ImGuiCol_Text,                ImVec4(0.0f, 0.0f, 0.0f, 0.0f));
    ImGui::PushStyleColor(ImGuiCol_FrameBg,             clrFrame.GetAsImVec4());
    ImGui::PushStyleColor(ImGuiCol_FrameBgActive,       m_clrTheme.GetAsImVec4());

    ImGui::SetCursorScreenPos(ImVec2(vMin.x, vWidgetSize.y <= flFrameHeight ? vMin.y : vMin.y + (vWidgetSize.y - flFrameHeight) / 2.0f));
    ImGui::SetNextItemWidth(vWidgetSize.x);
    if (ImGui::InputInt(szLabel, pData, 0, 0) == true)
    {
        if (pMin != nullptr && pMax != nullptr)
        {
            *pData = std::clamp<int>(*pData, *pMin, *pMax);
        }
    }

    std::string szSliderValue = std::format("{:.0f}", static_cast<float>(*pData) * m_menuAnim.GetAnimation());
    size_t      nDigits       = szSliderValue.size();
    float       flCharWidth   = ImGui::GetFont()->GetCharAdvance(' '); // Assuming using a mono font.
    ImGui::GetWindowDrawList()->AddText(
        ImVec2(vMin.x + (vWidgetSize.x - (flCharWidth * static_cast<float>(nDigits))) / 2.0f, vMin.y + (vWidgetSize.y - ImGui::GetTextLineHeight()) / 2.0f),
        ImColor(clrText.GetAsImVec4()), szSliderValue.c_str()
    );

    ImGui::PopStyleVar(2);
    ImGui::PopStyleColor(4);
}


///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
void MenuGUI_t::DrawFloatInput(const char* szLabel, ImVec2 vMin, ImVec2 vMax, float* pData, const float* pMin, const float* pMax, RGBA_t* pFrameClr, const float* pAnimation)
{
    RGBA_t clrFrame = (pFrameClr == nullptr ? m_clrSecondary : *pFrameClr);
    float  flAnimation = (pAnimation == nullptr ? 1.0f : *pAnimation);
    RGBA_t clrText; CalcTextClrForBg(clrText, clrFrame);

    ImVec2 vWidgetSize(vMax.x - vMin.x, vMax.y - vMin.y);
    float flFrameHeight = ImGui::GetFrameHeight();
    
    // Styling & drawing float input.
    ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize,  WIDGET_BORDER_THICKNESS);
    ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding,    5.0f);
    ImGui::PushStyleColor(ImGuiCol_Border,              m_clrTheme.GetAsImVec4());
    ImGui::PushStyleColor(ImGuiCol_Text,                ImVec4(0.0f, 0.0f, 0.0f, 0.0f));
    ImGui::PushStyleColor(ImGuiCol_FrameBg,             clrFrame.GetAsImVec4());
    ImGui::PushStyleColor(ImGuiCol_FrameBgActive,       m_clrTheme.GetAsImVec4());

    ImGui::SetCursorScreenPos(ImVec2(vMin.x, vWidgetSize.y <= flFrameHeight ? vMin.y : vMin.y + (vWidgetSize.y - flFrameHeight) / 2.0f));
    ImGui::SetNextItemWidth(vWidgetSize.x);
    if (ImGui::InputFloat(szLabel, pData, 0.0f, 0.0f, "%.1f") == true)
    {
        if(pMin != nullptr && pMax != nullptr)
        {
            *pData = std::clamp<float>(*pData, *pMin, *pMax);
        }
    }

    std::string szSliderValue = std::format("{:.1f}", static_cast<float>(*pData) * m_menuAnim.GetAnimation());
    size_t      nDigits       = szSliderValue.size();
    float       flCharWidth   = ImGui::GetFont()->GetCharAdvance(' '); // Assuming using a mono font.
    
    float flTextWidth = static_cast<float>(nDigits) * flCharWidth;
    ImVec2 vTextOrigin(flTextWidth >= vWidgetSize.x ? vMin.x : vMin.x + (vWidgetSize.x - flTextWidth) / 2.0f, vMin.y + (vWidgetSize.y - ImGui::GetTextLineHeight()) / 2.0f);
    ImGui::GetWindowDrawList()->AddText(
        vTextOrigin,
        ImColor(clrText.GetAsImVec4()), szSliderValue.c_str()
    );

    ImGui::PopStyleVar(2);
    ImGui::PopStyleColor(4);

}


///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
bool MenuGUI_t::DrawIntInputWidget(
    const char* szText, const char* szLabel, 
    ImVec2 vMin, ImVec2 vMax, 
    int* pData, const int iMin, const int iMax, 
    RGBA_t clrBackground, 
    float flSliderPercentage, float flIntInputPercentage, 
    const float* pTrackThickness, const float* pKnobSize, const float* pAnimationState)
{
    flSliderPercentage   = std::clamp<float>(flSliderPercentage,   0.0f, 1.0f);
    flIntInputPercentage = std::clamp<float>(flIntInputPercentage, 0.0f, 1.0f);

    constexpr float flPaddingFromWalls      = FEATURE_PADDING_PXL;
    constexpr float flPaddingBetweenWidgets = FEATURE_PADDING_PXL;

    // Adding padding to dimensions
    vMin.x += flPaddingFromWalls; vMin.y += flPaddingFromWalls;
    vMax.x -= flPaddingFromWalls; vMax.y -= flPaddingFromWalls;

    ImVec2 vWidgetSize(vMax.x - vMin.x, vMax.y - vMin.y);

    RGBA_t clrText; CalcTextClrForBg(clrText, clrBackground);
    ImGui::PushStyleColor(ImGuiCol_Text, clrText.GetAsImVec4());

    // Text.
    ImVec2 vTextPos(vMin.x, vMin.y + (vWidgetSize.y - ImGui::GetTextLineHeight()) / 2.0f);
    ImGui::SetCursorScreenPos(vTextPos);
    ImGui::Text(szText);


    bool bDataModified = DrawIntSlider(
        std::string("##Slider" + std::string(szLabel)).c_str(), 
        ImVec2(vMin.x + (1.0f - (flSliderPercentage + flIntInputPercentage)) * vWidgetSize.x, vMin.y), 
        ImVec2(vMin.x + (1.0f - flIntInputPercentage)                        * vWidgetSize.x, vMax.y), 
        pData, iMin, iMax, clrBackground, pTrackThickness, pKnobSize, pAnimationState
    );


    DrawIntInput(
        std::string("##IntInput" + std::string(szLabel)).c_str(),
        ImVec2(vMin.x + (1.0f - flIntInputPercentage) * vWidgetSize.x + flPaddingBetweenWidgets, vMin.y),
        vMax, pData, &iMin, &iMax, nullptr, pAnimationState);


    ImGui::PopStyleColor();
    return bDataModified;
}


///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
bool MenuGUI_t::DrawFloatInputWidget(
    const char* szText, const char* szLabel, 
    ImVec2 vMin, ImVec2 vMax, 
    float* pData, const float flMin, const float flMax, 
    RGBA_t clrBackground, 
    float flSliderPercentage, float flIntInputPercentage, 
    const float* pTrackThickness, const float* pKnobSize, const float* pAnimationState)
{
    flSliderPercentage   = std::clamp<float>(flSliderPercentage,   0.0f, 1.0f);
    flIntInputPercentage = std::clamp<float>(flIntInputPercentage, 0.0f, 1.0f);

    constexpr float flPaddingFromWalls      = FEATURE_PADDING_PXL;
    constexpr float flPaddingBetweenWidgets = FEATURE_PADDING_PXL;

    // Adding padding to dimensions
    vMin.x += flPaddingFromWalls; vMin.y += flPaddingFromWalls;
    vMax.x -= flPaddingFromWalls; vMax.y -= flPaddingFromWalls;

    ImVec2 vWidgetSize(vMax.x - vMin.x, vMax.y - vMin.y);

    RGBA_t clrText; CalcTextClrForBg(clrText, clrBackground);
    ImGui::PushStyleColor(ImGuiCol_Text, clrText.GetAsImVec4());

    // Text.
    ImVec2 vTextPos(vMin.x, vMin.y + (vWidgetSize.y - ImGui::GetTextLineHeight()) / 2.0f);
    ImGui::SetCursorScreenPos(vTextPos);
    ImGui::Text(szText);


    bool bDataModified = DrawFloatSlider(
        std::string("##Slider" + std::string(szLabel)).c_str(), 
        ImVec2(vMin.x + (1.0f - (flSliderPercentage + flIntInputPercentage)) * vWidgetSize.x, vMin.y), 
        ImVec2(vMin.x + (1.0f - flIntInputPercentage)                        * vWidgetSize.x, vMax.y), 
        pData, flMin, flMax, clrBackground, pTrackThickness, pKnobSize, pAnimationState
    );


    DrawFloatInput(
        std::string("##IntInput" + std::string(szLabel)).c_str(),
        ImVec2(vMin.x + (1.0f - flIntInputPercentage) * vWidgetSize.x + flPaddingBetweenWidgets, vMin.y),
        vMax, pData, &flMin, &flMax, nullptr, pAnimationState);


    ImGui::PopStyleColor();
    return bDataModified;
}
