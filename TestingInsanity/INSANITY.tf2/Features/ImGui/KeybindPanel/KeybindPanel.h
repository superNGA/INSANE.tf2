#pragma once

#include "../../FeatureHandler.h"
#include <vector>


///////////////////////////////////////////////////////////////////////////
class KeybindPanel_t
{
public:
    void Draw();

    void RefreshFeatureList();

private:
    void _GetFeatureOverrideInfo(IFeature* pFeature, std::string& szInfoOut, bool& bOverrideStatusOut);

    std::vector<IFeature*> m_vecKeybindFeatures;
    int                    m_iLongestFeatureName = 0;
    int                    m_iLongestSectionName = 0;

};
///////////////////////////////////////////////////////////////////////////

DECLARE_CUSTOM_OBJECT(KeybindPanel, KeybindPanel_t, Render)

DEFINE_SECTION(KeybindPanel, "Menu", 6)
DEFINE_FEATURE(KeybindPanel, "Enable", bool, KeybindPanel, Menu, 1, false, FeatureFlag_SupportKeyBind,
    "Open or close keybind window.")
DEFINE_FEATURE(KeybindPanel_Rounding,        "Rounding",          FloatSlider_t, KeybindPanel, Menu, 2, FloatSlider_t(5.0f, 0.0f, 100.0f))
DEFINE_FEATURE(KeybindPanel_ShowSectionName, "Show Section Name", bool, KeybindPanel, Menu, 3, false)
DEFINE_FEATURE(KeybindPanel_ShowKeybind,     "Show KeyBind",      bool, KeybindPanel, Menu, 4, false)
DEFINE_FEATURE(KeybindPanel_ShowInLobby,     "Show In Lobby",     bool, KeybindPanel, Menu, 5, false)