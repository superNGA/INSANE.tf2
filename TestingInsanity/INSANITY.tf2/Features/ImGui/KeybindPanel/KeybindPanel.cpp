#include "KeybindPanel.h"

#include <string>
#include <windows.h>

#include "../../../SDK/class/IVEngineClient.h"
#include "../MenuV2/MenuV2.h"
#include "../../../External Libraries/ImGui/imgui.h"
#include "../../../Resources/Fonts/FontManager.h"
#include "../../../Utility/ConsoleLogging.h"


std::string VkToString(DWORD vk);


///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
void KeybindPanel_t::Draw()
{
    if (Features::Menu::KeybindPanel::KeybindPanel.IsActive() == false)
        return;

    if (Features::Menu::KeybindPanel::KeybindPanel_ShowInLobby.IsActive() == false && I::iEngine->IsInGame() == false)
        return;

    ImGui::PushFont(Resources::Fonts::JetBrainsMonoNerd_Mid);

    ImGuiWindowFlags iWindowFlags =
        ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_NoCollapse |
        ImGuiWindowFlags_NoResize;

    // Don't let user move this shit if menu isn't open.
    if (Render::menuGUI.IsVisible() == false)
    {
        iWindowFlags |= ImGuiWindowFlags_NoMove;
    }
    else
    {
        iWindowFlags &= ~ImGuiWindowFlags_NoMove;
    }

    // Styling the main window.
    {
        ImGui::PushStyleColor(ImGuiCol_TitleBg,          Render::menuGUI.GetSecondaryClr().GetAsImVec4());
        ImGui::PushStyleColor(ImGuiCol_TitleBgActive,    Render::menuGUI.GetSecondaryClr().GetAsImVec4());
        ImGui::PushStyleColor(ImGuiCol_TitleBgCollapsed, Render::menuGUI.GetSecondaryClr().GetAsImVec4());
        ImGui::PushStyleColor(ImGuiCol_WindowBg,         Render::menuGUI.GetPrimaryClr().GetAsImVec4());
        ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding,   Features::Menu::KeybindPanel::KeybindPanel_Rounding.GetData().m_flVal);
    }


    ImGui::PushFont(Resources::Fonts::JetBrainsMonoNerd_Small);
    constexpr float PADDING_IN_PXL               = 4.0f;
    constexpr float KEYBIND_SECTION_SIZE_IN_CHAR = 8.0f;
    constexpr float FEATURE_VALUE_SIZE_IN_CHAR   = 8.0f;

    // Setting window width.
    {
        float flWindowWidthInChar = (m_iLongestFeatureName + 2) + FEATURE_VALUE_SIZE_IN_CHAR;
        if (Features::Menu::KeybindPanel::KeybindPanel_ShowSectionName.IsActive() == true)
        {
            flWindowWidthInChar += (m_iLongestSectionName + 2);
        }
        if (Features::Menu::KeybindPanel::KeybindPanel_ShowKeybind.IsActive() == true)
        {
            flWindowWidthInChar += KEYBIND_SECTION_SIZE_IN_CHAR; // Keybinds section has hardcoded width of 10 characters.
        }

        ImGui::SetNextWindowSize(ImVec2(
            ImGui::GetFont()->GetCharAdvance(' ') * flWindowWidthInChar,
            (m_vecKeybindFeatures.size() * ImGui::GetTextLineHeight()) + ((m_vecKeybindFeatures.size() + 1) * PADDING_IN_PXL))
        );
    }

    // Keyboard icon for title here.
    std::string szWindowTitle = reinterpret_cast<const char*>(u8"\uf11c");
    szWindowTitle += "##KeyBindPanel";
    if (ImGui::Begin(szWindowTitle.c_str(), nullptr, iWindowFlags) == true)
    {
        ImVec2 vWindowSize = ImGui::GetWindowSize();
        ImVec2 vWindowPos  = ImGui::GetWindowPos();
        ImVec2 vCursorPos  = ImGui::GetWindowPos();

        // We need it to move one line worth down, else the title bar will hide one entry.
        vCursorPos.x += PADDING_IN_PXL; vCursorPos.y += (2.0f * PADDING_IN_PXL) + ImGui::GetTextLineHeight();

        RGBA_t clrText; Render::menuGUI.CalcTextClrForBg(clrText, Render::menuGUI.GetPrimaryClr());
        
        ImDrawList* pDrawList   = ImGui::GetWindowDrawList();
        float       flCharWidth = ImGui::GetFont()->GetCharAdvance(' ');

        std::string szFeatureInfo("");
        for (IFeature* pFeature : m_vecKeybindFeatures)
        {
            ImGui::SetCursorScreenPos(vCursorPos);

            bool bOverrideStatus = false;
            _GetFeatureOverrideInfo(pFeature, szFeatureInfo, bOverrideStatus);
            ImColor featureTextClr = bOverrideStatus == true ? Render::menuGUI.GetThemeClr().GetAsImVec4() : clrText.GetAsImVec4();

            
            // Drawing feature's name...
            pDrawList->AddText(vCursorPos, featureTextClr, pFeature->m_szFeatureDisplayName.c_str());
            vCursorPos.x += (m_iLongestFeatureName + 1) * flCharWidth;


            // Drawing section names...
            if (Features::Menu::KeybindPanel::KeybindPanel_ShowSectionName.IsActive() == true && m_iLongestFeatureName > 0)
            {
                pDrawList->AddText(vCursorPos, featureTextClr, pFeature->m_szSectionName.c_str());
                vCursorPos.x += (m_iLongestSectionName + 1) * flCharWidth;
            }


            // Drawing Keybinds...
            if (Features::Menu::KeybindPanel::KeybindPanel_ShowKeybind.IsActive() == true)
            {
                // Center aligning this text.
                std::string szKeybind(VkToString(pFeature->m_iKey));
                float       flKeybindTextWidth = static_cast<float>(szKeybind.size());
                ImVec2      vKeybindTextPos(vCursorPos.x + ((KEYBIND_SECTION_SIZE_IN_CHAR - flKeybindTextWidth) / 2.0f) * flCharWidth, vCursorPos.y);

                pDrawList->AddText(vKeybindTextPos, featureTextClr, szKeybind.c_str());
                vCursorPos.x += KEYBIND_SECTION_SIZE_IN_CHAR * flCharWidth;
            }


            // Drawing feature's value... ( Center aligned )
            float   flTextWidth = static_cast<float>(szFeatureInfo.size());
            ImVec2  vFeatureInfoPos(vCursorPos.x + ((FEATURE_VALUE_SIZE_IN_CHAR - flTextWidth) / 2.0f) * flCharWidth, vCursorPos.y);
            pDrawList->AddText(vFeatureInfoPos, featureTextClr, szFeatureInfo.c_str());

            // Setup cursor pos for next entry.
            vCursorPos = ImVec2(vWindowPos.x + PADDING_IN_PXL, vCursorPos.y + ImGui::GetTextLineHeight() + PADDING_IN_PXL);
        }

        ImGui::End();
    }
    ImGui::PopFont();


    ImGui::PopStyleVar(2);
    ImGui::PopStyleColor(4);
    ImGui::PopFont();
}


///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
void KeybindPanel_t::RefreshFeatureList()
{
    m_vecKeybindFeatures.clear();

    const std::vector<Tab_t*>& vTabs = Config::featureHandler.GetFeatureMap();
    
    // Iterate all Tabs->Sections->Features.
    for (const Tab_t* pTab : vTabs)
    {
        for (const Section_t* pSection : pTab->m_vecSections)
        {
            // Get the length of the longest section name.
            if (pSection->m_szSectionDisplayName.size() > m_iLongestSectionName)
            {
                m_iLongestSectionName = pSection->m_szSectionDisplayName.size();
            }

            for (IFeature* pFeature : pSection->m_vecFeatures)
            {
                if ((pFeature->m_iFlags & FeatureFlag_SupportKeyBind) == true && pFeature->m_iKey != 0)
                {
                    m_vecKeybindFeatures.push_back(pFeature);

                    // Get the length of the longest feature name.
                    if (pFeature->m_szFeatureDisplayName.size() > m_iLongestFeatureName)
                    {
                        m_iLongestFeatureName = pFeature->m_szFeatureDisplayName.size();
                    }
                }
            }
        }
    }
}


///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
void KeybindPanel_t::_GetFeatureOverrideInfo(IFeature* pFeature, std::string& szInfoOut, bool& bOverrideStatusOut)
{
    bOverrideStatusOut = false; 
    szInfoOut          = "NA";

    switch (pFeature->m_iDataType)
    {
    case IFeature::DataType::DT_BOOLEAN:
    {
        Feature<bool>* pBoolFeature = static_cast<Feature<bool>*>(pFeature);
        bOverrideStatusOut          = pBoolFeature->IsActive();
        szInfoOut                   = bOverrideStatusOut == false ? "False" : "True";
        break;
    }
    case IFeature::DataType::DT_INTSLIDER:
    {
        Feature<IntSlider_t>* pIntFeature = static_cast<Feature<IntSlider_t>*>(pFeature);
        bOverrideStatusOut = pIntFeature->GetData().m_iVal == pIntFeature->m_OverrideData.m_iVal;
        szInfoOut          = std::format("{}", pIntFeature->GetData().m_iVal);
        break;
    }
    case IFeature::DataType::DT_FLOATSLIDER:
    {
        Feature<FloatSlider_t>* pFloatFeature = static_cast<Feature<FloatSlider_t>*>(pFeature);
        bOverrideStatusOut = pFloatFeature->GetData().m_flVal == pFloatFeature->m_OverrideData.m_flVal;
        szInfoOut          = std::format("{:.1f}", pFloatFeature->GetData().m_flVal > 0.001f ? pFloatFeature->GetData().m_flVal : 0.0f);
        break;
    }

    // These features don't have a override mechanism ( except color, but Who the fuck needs color override imformation ? )
    case IFeature::DataType::DT_DROPDOWN:
    case IFeature::DataType::DT_COLORDATA:
    case IFeature::DataType::DT_INVALID:
    default: break;
    }
}


///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
std::string VkToString(DWORD vk)
{
    switch (vk)
    {
    case VK_LBUTTON:    return "M1";
    case VK_RBUTTON:    return "M2";
    case VK_MBUTTON:    return "M3";
    case VK_XBUTTON1:   return "M4";
    case VK_XBUTTON2:   return "M5";

    case VK_BACK:       return "Back";
    case VK_TAB:        return "Tab";
    case VK_RETURN:     return "Enter";
    case VK_SHIFT:      return "Shift";
    case VK_CONTROL:    return "Ctrl";
    case VK_MENU:       return "Alt";
    case VK_PAUSE:      return "Pause";
    case VK_CAPITAL:    return "Caps";
    case VK_ESCAPE:     return "Esc";
    case VK_SPACE:      return "Space";
    case VK_PRIOR:      return "PgUp";
    case VK_NEXT:       return "PgDown";
    case VK_END:        return "End";
    case VK_HOME:       return "Home";
    case VK_LEFT:       return "Left";
    case VK_UP:         return "Up";
    case VK_RIGHT:      return "Right";
    case VK_DOWN:       return "Down";
    case VK_INSERT:     return "Ins";
    case VK_DELETE:     return "Del";
    case VK_LWIN:       return "LWin";
    case VK_RWIN:       return "RWin";
    case VK_APPS:       return "Apps";
    case VK_NUMLOCK:    return "NumLock";
    case VK_SCROLL:     return "ScrollLock";

        // Function keys
    case VK_F1:         return "F1";
    case VK_F2:         return "F2";
    case VK_F3:         return "F3";
    case VK_F4:         return "F4";
    case VK_F5:         return "F5";
    case VK_F6:         return "F6";
    case VK_F7:         return "F7";
    case VK_F8:         return "F8";
    case VK_F9:         return "F9";
    case VK_F10:        return "F10";
    case VK_F11:        return "F11";
    case VK_F12:        return "F12";

        // Numpad
    case VK_NUMPAD0:    return "Num0";
    case VK_NUMPAD1:    return "Num1";
    case VK_NUMPAD2:    return "Num2";
    case VK_NUMPAD3:    return "Num3";
    case VK_NUMPAD4:    return "Num4";
    case VK_NUMPAD5:    return "Num5";
    case VK_NUMPAD6:    return "Num6";
    case VK_NUMPAD7:    return "Num7";
    case VK_NUMPAD8:    return "Num8";
    case VK_NUMPAD9:    return "Num9";
    case VK_MULTIPLY:   return "Num*";
    case VK_ADD:        return "Num+";
    case VK_SUBTRACT:   return "Num-";
    case VK_DECIMAL:    return "Num.";
    case VK_DIVIDE:     return "Num/";

        // Common printable ASCII (0-9, A-Z)
    default:
        if (vk >= '0' && vk <= '9')
            return std::string(1, (char)vk);
        if (vk >= 'A' && vk <= 'Z')
            return std::string(1, (char)vk);
        if (vk >= 0xBA && vk <= 0xC0) // ; = , - . / `
            return std::string(1, (char)vk);
        if (vk >= 0xDB && vk <= 0xDE) // [ \ ] '
            return std::string(1, (char)vk);
        return "Niga";
    }
}