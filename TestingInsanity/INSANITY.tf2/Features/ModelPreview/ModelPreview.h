#pragma once

#include <string>
#include <vector>

#include "../FeatureHandler.h"

class Panel;

class ModelPreview_t
{
public:
    void Run();
    void Free();

    inline void* GetOriginalPaintFn() const { return m_pOriginalPaint; }
private:
    bool   _InitializePanel();
    void   _FreePanel();
    Panel* m_pPanel           = nullptr;
    bool   m_bPanelInitilized = false;

    bool   _SpoofVTable();
    void   _FreeVTable();
    void*  m_pSpoofedVTable   = nullptr;
    void*  m_pOriginalPaint   = nullptr;
};
DECLARE_FEATURE_OBJECT(modelPreview, ModelPreview_t)


DEFINE_TAB(ModelPreview, 11)
DEFINE_SECTION(ModelPreview, "ModelPreview", 1)

DEFINE_FEATURE(Enable,          bool, ModelPreview, ModelPreview, 1, false)
DEFINE_FEATURE(Debug,           bool, ModelPreview, ModelPreview, 2, false)
DEFINE_FEATURE(DrawReplicate,   bool, ModelPreview, ModelPreview, 3, false)

DEFINE_FEATURE(X,     FloatSlider_t, ModelPreview, ModelPreview, 4, FloatSlider_t(0.0f, 0.0f, 1920.0f))
DEFINE_FEATURE(Y,     FloatSlider_t, ModelPreview, ModelPreview, 5, FloatSlider_t(0.0f, 0.0f, 1080.0f))
DEFINE_FEATURE(Z,     FloatSlider_t, ModelPreview, ModelPreview, 6, FloatSlider_t(0.0f, 0.0f, 1080.0f))
DEFINE_FEATURE(PITCH, FloatSlider_t, ModelPreview, ModelPreview, 7, FloatSlider_t(0.0f, -89.0f, 89.0f))
DEFINE_FEATURE(YAW,   FloatSlider_t, ModelPreview, ModelPreview, 8, FloatSlider_t(0.0f, -180.0f, 180.0f))
DEFINE_FEATURE(ROLL,  FloatSlider_t, ModelPreview, ModelPreview, 9, FloatSlider_t(0.0f, -180.0f, 180.0f))