//=========================================================================
//                      MODEL PREVIEW
//=========================================================================
// by      : INSANE
// created : 31/07/2025
// 
// purpose : Renders any model. Anywhere.
//-------------------------------------------------------------------------
#pragma once

#include <string>
#include <vector>

#include "../FeatureHandler.h"

class BaseEntity;
class Panel;
class INetworkStringTable;
class model_t;

class ModelPreview_t
{
public:
    void Run();
    void Free();

    inline void* GetOriginalPaintFn() const { return m_pOriginalPaint; }

    // String table
    void DiscardStringTables() const;
    void InvalidateModelPrecache();
    void JoiningMatch(bool bJoining);

    // Models
    void        SetActiveModel(int iIndex);
    model_t*    GetActiveModel() const;
    BaseEntity* GetModelEntity() const;

    void SetVisible(bool bVisible);

    // Panel Pos & size
    void SetPanelSize(int  iHeight, int  iWidth);
    void GetPanelSize(int& iHeight, int& iWidth) const;

    void SetPanelPos(int  x, int  y);
    void GetPanelPos(int& x, int& y) const;

    void SetPanelClr(unsigned char r, unsigned char g, unsigned char b, unsigned char a);
    RGBA_t GetPanelClr() const;

    // Render View Pos & Size
    void SetRenderViewSize(int  iHeight, int  iWidth);
    void GetRenderViewSize(int& iHeight, int& iWidth) const;

    void SetRenderViewPos(int  x, int  y);
    void GetRenderViewPos(int& x, int& y) const;

    void SetRenderViewClr(unsigned char r, unsigned char g, unsigned char b, unsigned char a);
    RGBA_t GetRenderViewClr() const;

    // Camera
    void SetCameraPos(const vec& vCameraPos);
    vec GetCameraPos() const;

private:
    bool _ShouldCreateStringTable();
    bool m_bJoiningMatch = false;

    void _SetTablePointer(INetworkStringTable* pTable) const;
    void _VerifyOrCreateStringTable()                  const;
    bool _PrecacheModels()                             const;

    int      m_iActiveModelIndex = -1;
    model_t* m_pActiveModel      = nullptr;
    bool     m_bModelPrecached   = false;
    std::vector<std::string> m_vecModels =
    {
        "models/player/spy.mdl",
        "models/player/heavy.mdl",
        "custom/CustomModels/models/johny.mdl",
        "models/weapons/w_models/w_toolbox.mdl",
        "models/props_island/crocodile/crocodile.mdl"
    };


    // init wrapper for Entity & panel.
    bool _Initialize();
    bool m_bVisible = true;

    // Entity init...
    bool _InitializeEntity();
    void _FreeEntity();
    BaseEntity* m_pEnt     = nullptr;
    bool        m_bEntInit = false;

    // Panel init...
    bool _InitializePanel();
    Panel*      m_pPanel           = nullptr;
    bool        m_bPanelInitilized = false;
    const char* m_szPanelName      = "INSANE.tf2 Showcase Panel";

    vec m_vCameraPos = { -180.0f, 0.0f, 75.0f };

    int m_iPanelHeight = 800;
    int m_iPanelWidth  = 800;
    int m_iPanelX = 0, m_iPanelY = 0;
    RGBA_t m_panelClr;

    int m_iRenderViewHeight = 700;
    int m_iRenderViewWidth  = 700;
    int m_iRenderViewX = 0, m_iRenderViewY = 0;
    RGBA_t m_renderViewClr;

    // Vtable for panel object
    uint64_t _FindChildByName(vgui::VPANEL parent, const std::string& szChildName, bool bRecurse);
    bool     _SpoofVTable();
    void     _FreeVTable();
    void* m_pSpoofedVTable  = nullptr;
    void* m_pOriginalVTable = nullptr;
    void* m_pOriginalPaint  = nullptr;
};
DECLARE_FEATURE_OBJECT(modelPreview, ModelPreview_t)