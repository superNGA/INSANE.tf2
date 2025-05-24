#pragma once
#include "../../FeatureHandler.h"

class Tab_t;
class Section_t;
class IFeature;

class UIMenu
{
public:
    void Draw();

    void ResetKeyRecorder() { m_bRecordingKey = false; m_iRecordedKey = 0; }
    bool    m_bRecordingKey = false;
    int64_t m_iRecordedKey  = 0;

private:
    void _DrawSection(Tab_t* pTab);
    void _DrawFeature(IFeature* pFeature);

    void _DrawCheckBoxFeature(IFeature* pFeature);
    void _DrawIntSliderFeature(IFeature* pFeature)const;
    void _DrawFloatSlidereature(IFeature* pFeature)const;
    void _DrawColorSelectorFeature(IFeature* pFeature)const;

    void _DrawFeatureOptionWindow(IFeature* pFeature);
    
    void _DrawConfigView();
    
    enum class UIViewState : int8_t
    {
        TAB_VIEW, FEATURE_VIEW, CONFIG_VIEW
    };
    UIViewState m_iViewState      = UIViewState::TAB_VIEW;
    int         m_iActiveTabIndex = 0;
};
DECLARE_CUSTOM_OBJECT(uiMenu, UIMenu, Render);