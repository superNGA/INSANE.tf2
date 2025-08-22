#include "Menu.h"
#include <algorithm>

// UI Data
#include "../../../Hooks/EndScene/EndScene.h"

// Feature map
#include "../../FeatureHandler.h"
#include "../../Config/ConfigHandler.h"

// GUI library
#include "../../../External Libraries/ImGui/imgui.h"
#include "../../../External Libraries/ImGui/imgui_internal.h"

#include "../../../Utility/ConsoleLogging.h"

constexpr ImVec2 MENU_DIMENSIONS(400.0f, 500.0f);
constexpr ImVec2 BACK_BUTTON_DIMENSIONS(100.0f, 35.0f);

void UIMenu::Draw()
{
    // If don't want to render, then return
    if (directX::UI::UI_visble == false)
        return;

    // Begin Menu
    ImGui::SetNextWindowSize(MENU_DIMENSIONS);
    if (ImGui::Begin(m_szMenuWindowName, &directX::UI::UI_visble, ImGuiWindowFlags_::ImGuiWindowFlags_AlwaysAutoResize) == true)
    {

        m_vLastWindowPos = { ImGui::GetWindowPos().x, ImGui::GetWindowPos().y };

        // Aquiring Feature map
        const auto& vecFeatureMap = featureHandler.GetFeatureMap();

        // Drawing Tabs
        if (m_iViewState == UIViewState::TAB_VIEW)
        {
            constexpr ImVec2 TAB_BUTTON_DIMENSIONS(280.0f, 30.0f);
            for (const auto* tab : vecFeatureMap)
            {
                if (ImGui::Button((tab->m_szTabDisplayName + "##Tab").c_str(), TAB_BUTTON_DIMENSIONS) == true)
                {
                    m_iViewState = UIViewState::FEATURE_VIEW;
                    m_iActiveTabIndex = tab->m_iIndex - 1;

                    // Clamping so out-of-index doesn't cause crash
                    m_iActiveTabIndex = std::clamp(m_iActiveTabIndex, 0, static_cast<int>(vecFeatureMap.size() - 1));
                }
            }

            if (ImGui::Button("Config", TAB_BUTTON_DIMENSIONS))
                m_iViewState = UIViewState::CONFIG_VIEW;
        }
        else if (m_iViewState == UIViewState::CONFIG_VIEW)
        {
            _DrawConfigView();
        }
        else
        {
            _DrawSection(vecFeatureMap[m_iActiveTabIndex]);
        }


        // Drawing Unload button only if in TabView
        if (m_iViewState == UIViewState::TAB_VIEW)
        {
            if (ImGui::Button("UnLoad", BACK_BUTTON_DIMENSIONS) == true)
                directX::UI::shutdown_UI = true;
        }
        // Else drawing "Back-To-TabView" button
        else
        {
            if (ImGui::Button("Back", BACK_BUTTON_DIMENSIONS) == true)
                m_iViewState = UIViewState::TAB_VIEW;
        }

        // End Menu
        ImGui::End();
    }
}


///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
void UIMenu::GetWindowSize(float& flHeight, float& flWidth) const
{
    flWidth = MENU_DIMENSIONS.x; flHeight = MENU_DIMENSIONS.y;
}


///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
void UIMenu::GetWindowPos(float& x, float& y) const
{
    x = m_vLastWindowPos.x; y = m_vLastWindowPos.y;
}


//=========================================================================
//                     Private methods
//=========================================================================
void UIMenu::_DrawSection(Tab_t* pTab)
{
    for (const auto* pSection : pTab->m_vecSections)
    {
        if (ImGui::CollapsingHeader((pSection->m_szSectionDisplayName + "##Section").c_str()))
        {
            for (auto* pFeature : pSection->m_vecFeatures)
            {
                _DrawFeature(pFeature, false);
            }
        }
    }
}

void UIMenu::_DrawFeature(IFeature* pFeature, bool bOverride)
{
    switch (pFeature->m_iDataType)
    {
    case IFeature::DataType::DT_BOOLEAN:
        _DrawCheckBoxFeature(pFeature, bOverride);
        return;
    case IFeature::DataType::DT_INTSLIDER:
        _DrawIntSliderFeature(pFeature, bOverride);
        return;
    case IFeature::DataType::DT_FLOATSLIDER:
        _DrawFloatSlidereature(pFeature, bOverride);
        return;
    case IFeature::DataType::DT_COLORDATA:
        _DrawColorSelectorFeature(pFeature, bOverride);
        return;
    default:
        return;
    }
}

void UIMenu::_DrawCheckBoxFeature(IFeature* pFeature, bool bOverride)
{
    // CheckBoxes don't support override
    if (bOverride == true)
        return;

    // Casting into correct type
    Feature<bool>* pBoolFeature = static_cast<Feature<bool>*>(pFeature);

    // Rendering CheckBox
    ImGui::Checkbox((pBoolFeature->m_szFeatureDisplayName + "##Feature").c_str(), &pBoolFeature->m_Data);
    if (ImGui::IsItemHovered() == true && pFeature->m_szToolTip[0] != '\0')
        ImGui::SetTooltip(pFeature->m_szToolTip.c_str());

    _DrawFeatureOptionWindow(pFeature);
}

void UIMenu::_DrawIntSliderFeature(IFeature* pFeature, bool bOverride)
{
    // Casting into correct type
    Feature<IntSlider_t>* pIntFeature = static_cast<Feature<IntSlider_t>*>(pFeature);

    if (bOverride == true)
    {
        ImGui::SliderInt(
            (pIntFeature->m_szFeatureDisplayName + "##Feature").c_str(), 
            &pIntFeature->m_OverrideData.m_iVal, 
            pIntFeature->m_OverrideData.m_iMin, 
            pIntFeature->m_OverrideData.m_iMax);
        return;
    }

    // Rendering Int Slider
    ImGui::SliderInt(
        (pIntFeature->m_szFeatureDisplayName + "##Feature").c_str(), 
        pIntFeature->m_bIsOverrideActive == true ? &pIntFeature->m_OverrideData.m_iVal : &pIntFeature->m_Data.m_iVal,
        pIntFeature->m_Data.m_iMin, 
        pIntFeature->m_Data.m_iMax);

    _DrawFeatureOptionWindow(pFeature);
}

void UIMenu::_DrawFloatSlidereature(IFeature* pFeature, bool bOverride) 
{
    // Casting into correct type
    Feature<FloatSlider_t>* pFloatFeature = static_cast<Feature<FloatSlider_t>*>(pFeature);

    if (bOverride == true)
    {
        ImGui::SliderFloat((pFloatFeature->m_szFeatureDisplayName + "##Feature").c_str(), 
            &pFloatFeature->m_OverrideData.m_flVal, 
            pFloatFeature->m_OverrideData.m_flMin,
            pFloatFeature->m_OverrideData.m_flMax, "%.6f");
        return;
    }

    // Rendering Float Slider
    ImGui::SliderFloat((pFloatFeature->m_szFeatureDisplayName + "##Feature").c_str(), 
        pFloatFeature->m_bIsOverrideActive == true ? &pFloatFeature->m_OverrideData.m_flVal : &pFloatFeature->m_Data.m_flVal, 
        pFloatFeature->m_Data.m_flMin, 
        pFloatFeature->m_Data.m_flMax, "%.6f");

    _DrawFeatureOptionWindow(pFeature);
}

void UIMenu::_DrawColorSelectorFeature(IFeature* pFeature, bool bOverride) 
{
    // Casting into correct type
    Feature<ColorData_t>* pColorFeature = static_cast<Feature<ColorData_t>*>(pFeature);

    if (bOverride == true)
    {
        ImGui::ColorEdit4(
            (pColorFeature->m_szFeatureDisplayName + "##Feature").c_str(),
            &pColorFeature->m_OverrideData.r, 
            ImGuiColorEditFlags_AlphaBar);
        return;
    }

    ImGui::ColorEdit4(
        (pColorFeature->m_szFeatureDisplayName + "##Feature").c_str(),
        pColorFeature->m_bIsOverrideActive == true ? &pColorFeature->m_OverrideData.r : &pColorFeature->m_Data.r, 
        ImGuiColorEditFlags_AlphaBar);

    _DrawFeatureOptionWindow(pFeature);
}

void UIMenu::_DrawFeatureOptionWindow(IFeature* pFeature)
{
    if ((pFeature->m_iFlags & FeatureFlag_SupportKeyBind) == false)
        return;

    // Must be on same line as feature.
    ImGui::SameLine();

    // Open popup when clicking on '?'
    constexpr ImVec2 OPTION_BUTTON_DIMENSIONS(20.0f, 20.0f);
    if (ImGui::Button((std::string("?") + "##" + pFeature->m_szFeatureDisplayName).c_str(), OPTION_BUTTON_DIMENSIONS) == true)
    {
        ImGui::OpenPopup(std::string("Feature-Options##" + pFeature->m_szFeatureDisplayName).c_str());
    }

    _DrawFeatureOptionPopup(pFeature);
}

void UIMenu::_DrawFeatureOptionPopup(IFeature* pFeature)
{
    if (ImGui::BeginPopup(std::string("Feature-Options##" + pFeature->m_szFeatureDisplayName).c_str()) == true)
    {
        m_bRecordingKey == true ?
            ImGui::Text("Key : [ Recording... ]") :
            ImGui::Text(std::format("Key : 0x{}", pFeature->m_iKey).c_str());

        ImGui::SameLine();

        // Change key bind button
        if (ImGui::Button("Change?") == true && m_bRecordingKey == false)
        {
            m_bRecordingKey = true;
        } ImGui::SameLine();
        if (ImGui::Button("Clear") == true)
        {
            pFeature->m_iKey = NULL;
        }

        // Did we recorded a key?
        if (m_iRecordedKey != 0 && m_bRecordingKey == false)
        {
            pFeature->m_iKey = m_iRecordedKey;
            ResetKeyRecorder();
        }

        // Does this feature has any specific Override option?
        if ((pFeature->m_iFlags & (FeatureFlag_HoldOnlyKeyBind | FeatureFlag_ToggleOnlyKeyBind)) == false)
        {
            ImGui::Checkbox("Overide type", reinterpret_cast<bool*>(&pFeature->m_iOverrideType));
        }

        // Print current override option
        pFeature->m_iOverrideType == IFeature::OverrideType::OVERRIDE_HOLD ?
            ImGui::Text("[ HOLD ]") :
            ImGui::Text("[ TOGGLE ]");

        _DrawFeature(pFeature, true);

        ImGui::EndPopup();
    }
}

void UIMenu::_DrawConfigView()
{
    //----------------------- Drop Down list for avilable files -----------------------
    // Getting all configs avialable
    const auto& vecConfigFiles = configHandler.GetAllConfigFile();

    std::vector<const char*> vecConfigNames = {};
    for (auto& configName : vecConfigFiles)
        vecConfigNames.push_back(configName.c_str());

    // Drop down menu
    static int iSelectedIndex = 0;
    static std::string szSelectedConfig = "";
    static bool bDeletedSelectedFile = false;
    static bool bCreatedFileRecently = false;
    if (ImGui::Combo("Select Item", &iSelectedIndex, vecConfigNames.data(), static_cast<int>(vecConfigNames.size())) || 
        bDeletedSelectedFile == true || bCreatedFileRecently == true || (szSelectedConfig == "" && vecConfigFiles.size() != NULL))
    {
        if (vecConfigFiles.size() != NULL)
            szSelectedConfig = vecConfigFiles[iSelectedIndex];
        else
            szSelectedConfig = "NO CONFIG FILE FOUND";

        bDeletedSelectedFile = false;
        bCreatedFileRecently = false;
    }

    //----------------------- Create new file button -----------------------
    static char szTextInput[126];
    ImGui::InputText("config name", szTextInput, sizeof(szTextInput));
    if (ImGui::Button("Create New file"))
    {
        if (configHandler.IsFileNameAvialable(std::string(szTextInput)) == true)
        {
            configHandler.CreateConfigFile(std::string(szTextInput));
            bCreatedFileRecently = true;
            szTextInput[0] = '\0';
        }
        else
            ImGui::OpenPopup("Override Warning");
    }

    if (ImGui::BeginPopupModal("Override Warning"))
    {
        ImGui::Text("file [ %s ] already exists. you sure you wanna override", szTextInput);

        if (ImGui::Button("over-write"))
        {
            configHandler.CreateConfigFile(std::string(szTextInput));
            bCreatedFileRecently = true;
            szTextInput[0] = '\0';
            ImGui::CloseCurrentPopup();
        }
        if (ImGui::Button("No"))
        {
            ImGui::CloseCurrentPopup();
        }
        ImGui::EndPopup();
    }

    //----------------------- Delete button -----------------------
    if (ImGui::Button("Delete"))
        ImGui::OpenPopup("Delete Warning");

    if (ImGui::BeginPopupModal("Delete Warning"))
    {
        if (ImGui::Button("YES"))
        {
            configHandler.DeleteConfigFile(szSelectedConfig);
            bDeletedSelectedFile = true;
            ImGui::CloseCurrentPopup();
        }
        
        if (ImGui::Button("NO"))
        {
            ImGui::CloseCurrentPopup();
        }
        
        ImGui::EndPopup();
    }

    //----------------------- Save to File -----------------------
    if (ImGui::Button("Save"))
    {
        if (configHandler.WriteToConfigFile(szSelectedConfig) == false)
            FAIL_LOG("Failed to write to file");
        else
            WIN_LOG("Successfully wrote config to file!");
    }

    //----------------------- Load from file -----------------------
    if (ImGui::Button("Load"))
    {
        if (configHandler.ReadConfigFile(szSelectedConfig) == true)
            WIN_LOG("Successfully loaded config");
        else
            FAIL_LOG("Failed to load config");
    }
}