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
    void        DiscardStringTables() const;
    inline void InvalidateModelPrecache() { m_bModelPrecached = false; }
    inline void JoiningMatch(bool bJoining) { m_bJoiningMatch = bJoining; }

    // Models
    void SetActiveModel(int iIndex);
    inline model_t*    GetActiveModel() const { return m_pActiveModel; }

    inline BaseEntity* GetModelEntity() const { return m_pEnt; }

    // Panel Pos & size
    inline void SetPanelSize(int  iHeight, int  iWidth)       { m_iPanelHeight = iHeight; m_iPanelWidth = iWidth; }
    inline void GetPanelSize(int& iHeight, int& iWidth) const { iHeight = m_iPanelHeight; iWidth = m_iPanelWidth; }

    inline void SetPanelPos(int  x, int  y)       { m_iPanelX = x; m_iPanelY = y; }
    inline void GetPanelPos(int& x, int& y) const { x = m_iPanelX; y = m_iPanelY; }

    // Render View Pos & Size
    inline void SetRenderViewSize(int  iHeight, int  iWidth)       { m_iRenderViewHeight = iHeight; m_iRenderViewWidth = iWidth; }
    inline void GetRenderViewSize(int& iHeight, int& iWidth) const { iHeight = m_iRenderViewHeight; iWidth = m_iRenderViewWidth; }

    inline void SetRenderViewPos(int  x, int  y)       { m_iRenderViewX = x; m_iRenderViewY = y; }
    inline void GetRenderViewPos(int& x, int& y) const { x = m_iRenderViewX; y = m_iRenderViewY; }

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

    // Entity init...
    bool _InitializeEntity();    
    BaseEntity* m_pEnt      = nullptr;
    bool        m_bEntInit  = false;

    // Panel init...
    bool _InitializePanel();
    void _FreePanel();
    Panel*      m_pPanel           = nullptr;
    bool        m_bPanelInitilized = false;
    const char* m_szPanelName      = "INSANE.TF2 Showcase Panel";

    int m_iPanelHeight = 800;
    int m_iPanelWidth  = 800;
    int m_iPanelX = 0, m_iPanelY = 0;

    int m_iRenderViewHeight = 700;
    int m_iRenderViewWidth  = 700;
    int m_iRenderViewX = 0, m_iRenderViewY = 0;

    // Vtable for panel object
    bool  _SpoofVTable();
    void  _FreeVTable();
    void* m_pSpoofedVTable = nullptr;
    void* m_pOriginalPaint = nullptr;
};
DECLARE_FEATURE_OBJECT(modelPreview, ModelPreview_t)


DEFINE_TAB(ModelPreview, 11)
DEFINE_SECTION(ModelPreview, "ModelPreview", 1)

DEFINE_FEATURE(Enable, bool, ModelPreview, ModelPreview, 1, false)
DEFINE_FEATURE(HardRespawn, bool, ModelPreview, ModelPreview, 2, false)