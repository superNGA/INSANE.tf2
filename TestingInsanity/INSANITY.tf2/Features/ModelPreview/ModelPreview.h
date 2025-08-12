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
    const int m_iHeight = 800;
    const int m_iWidth  = 800;

    bool _InitializeEntity();
    void* m_pBones = nullptr; // Yea I have do it myself, else setupbones will fail in mainmenu.
    void* m_pEnt = nullptr;
    void* m_pModel = nullptr;
    bool m_bEntInit = false;

private:
    bool   _InitializePanel();
    void   _FreePanel();
    Panel* m_pPanel = nullptr;
    bool   m_bPanelInitilized = false;

    bool   _SpoofVTable();
    void   _FreeVTable();
    void* m_pSpoofedVTable = nullptr;
    void* m_pOriginalPaint = nullptr;
};
DECLARE_FEATURE_OBJECT(modelPreview, ModelPreview_t)


DEFINE_TAB(ModelPreview, 11)
DEFINE_SECTION(ModelPreview, "ModelPreview", 1)

DEFINE_FEATURE(Enable, bool, ModelPreview, ModelPreview, 1, false)