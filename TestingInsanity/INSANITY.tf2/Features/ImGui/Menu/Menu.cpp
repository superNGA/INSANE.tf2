#include "Menu.h"
#include <algorithm>

// UI Data
#include "../../../Hooks/EndScene/EndScene.h"

// Feature map
#include "../../FeatureHandler.h"

// GUI library
#include "../../../External Libraries/ImGui/imgui.h"

constexpr ImVec2 MENU_DIMENSIONS(300.0f, 500.0f);
constexpr ImVec2 BACK_BUTTON_DIMENSIONS(100.0f, 35.0f);

void UIMenu::Draw()
{
    // If don't want to render, then return
    if (directX::UI::UI_visble == false)
        return;

    // Begin Menu
    ImGui::SetNextWindowSize(MENU_DIMENSIONS);
    ImGui::Begin("INSANE.tf2", &directX::UI::UI_visble, ImGuiWindowFlags_::ImGuiWindowFlags_AlwaysAutoResize);

    // Aquiring Feature map
    const auto& vecFeatureMap = featureHandler.GetFeatureMap();
    
    // Drawing Tabs
    if (m_iViewState == UIViewState::TAB_VIEW)
    {
        for (const auto* tab : vecFeatureMap)
        {
            constexpr ImVec2 TAB_BUTTON_DIMENSIONS(280.0f, 30.0f);
            if (ImGui::Button((tab->m_szTabDisplayName + "##Tab").c_str(), TAB_BUTTON_DIMENSIONS) == true)
            {
                m_iViewState      = UIViewState::FEATURE_VIEW;
                m_iActiveTabIndex = tab->m_iIndex - 1;

                // Clamping so out-of-index doesn't cause crash
                m_iActiveTabIndex = std::clamp(m_iActiveTabIndex, 0, static_cast<int>(vecFeatureMap.size() - 1));
            }
        }
    }
    else
    {
        _DrawSection(vecFeatureMap[m_iActiveTabIndex]);
    }


    // Drawing Unload button only if in TabView
    if(m_iViewState == UIViewState::TAB_VIEW)
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
                _DrawFeature(pFeature);
            }
        }
    }
}

void UIMenu::_DrawFeature(IFeature* pFeature)
{
    switch (pFeature->m_iDataType)
    {
    case IFeature::DataType::DT_BOOLEAN:
        _DrawCheckBoxFeature(pFeature);
        return;
    case IFeature::DataType::DT_INTSLIDER:
        _DrawIntSliderFeature(pFeature);
        return;
    case IFeature::DataType::DT_FLOATSLIDER:
        _DrawFloatSlidereature(pFeature);
        return;
    case IFeature::DataType::DT_COLORDATA:
        _DrawColorSelectorFeature(pFeature);
        return;
    default:
        return;
    }
}

void UIMenu::_DrawCheckBoxFeature(IFeature* pFeature)
{
    // Casting into correct type
    Feature<bool>* pBoolFeature = static_cast<Feature<bool>*>(pFeature);

    // Rendering CheckBox
    ImGui::Checkbox((pBoolFeature->m_szFeatureDisplayName + "##Feature").c_str(), &pBoolFeature->m_Data);
    
    _DrawFeatureOptionWindow(pFeature);
}

void UIMenu::_DrawIntSliderFeature(IFeature* pFeature) const
{
    // Casting into correct type
    Feature<IntSlider_t>* pIntFeature = static_cast<Feature<IntSlider_t>*>(pFeature);

    // Rendering Int Slider
    ImGui::SliderInt((pIntFeature->m_szFeatureDisplayName + "##Feature").c_str(), &pIntFeature->m_Data.m_iVal, pIntFeature->m_Data.m_iMin, pIntFeature->m_Data.m_iMax);
}

void UIMenu::_DrawFloatSlidereature(IFeature* pFeature) const
{
    // Casting into correct type
    Feature<FloatSlider_t>* pFloatFeature = static_cast<Feature<FloatSlider_t>*>(pFeature);

    // Rendering Float Slider
    ImGui::SliderFloat((pFloatFeature->m_szFeatureDisplayName + "##Feature").c_str(), &pFloatFeature->m_Data.m_flVal, pFloatFeature->m_Data.m_flMin, pFloatFeature->m_Data.m_flMax, "%.2f");
}

void UIMenu::_DrawColorSelectorFeature(IFeature* pFeature) const
{
    // Casting into correct type
    Feature<ColorData_t>* pColorFeature = static_cast<Feature<ColorData_t>*>(pFeature);

    // Rendring Color Selector
    ImGui::ColorEdit4((pColorFeature->m_szFeatureDisplayName + "##Feature").c_str(), &pColorFeature->m_Data.r, ImGuiColorEditFlags_AlphaBar);
}

void UIMenu::_DrawFeatureOptionWindow(IFeature* pFeature)
{
    if (pFeature->m_iFlags == FeatureFlags::FeatureFlag_None)
        return;

    // Must be on same line as feature.
    ImGui::SameLine();

    // Open popup when clicking on '?'
    constexpr ImVec2 OPTION_BUTTON_DIMENSIONS(20.0f, 20.0f);
    if (ImGui::Button((std::string("?") + "##" + pFeature->m_szFeatureDisplayName).c_str(), OPTION_BUTTON_DIMENSIONS) == true)
    {
        ImGui::OpenPopup("Feature-Options");
    }

    // Designing Popup menu
    if (ImGui::BeginPopup("Feature-Options") == true)
    {
        m_bRecordingKey == true ?
            ImGui::Text("Key : [ Recording... ]") :
            ImGui::Text(std::format("Key : 0x{}", pFeature->m_iKey).c_str());
        
        ImGui::SameLine();

        if (ImGui::Button("Change?") == true && m_bRecordingKey == false)
        {
            m_bRecordingKey = true;
        }

        // Did we recorded a key?
        if (m_iRecordedKey != 0 && m_bRecordingKey == false)
        {
            pFeature->m_iKey = m_iRecordedKey;
            ResetKeyRecorder();
        }

        ImGui::Checkbox("Overide type", reinterpret_cast<bool*>(&pFeature->m_iOverrideType));
        pFeature->m_iOverrideType == IFeature::OverrideType::OVERRIDE_HOLD ?
            ImGui::Text("[ HOLD ]") :
            ImGui::Text("[ TOGGLE ]");
         
        ImGui::EndPopup();
    }
}