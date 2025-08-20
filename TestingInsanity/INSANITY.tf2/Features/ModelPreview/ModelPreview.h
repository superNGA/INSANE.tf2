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
#include "../../SDK/class/Basic Structures.h"

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
    int         GetActiveModelIndex() const;
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
    
    float GetBaseCameraFOV() const;
    float GetVerticalFOV()   const;
    float GetHorizontalFOV() const;
    void SetCameraFOV(const float flCameraFOV);

    // Lighting
    enum AmbientLight_t : int32_t
    {
        LIGHT_BACK = 0, LIGHT_FRONT,
        LIGHT_LEFT,     LIGHT_RIGHT,
        LIGHT_TOP,      LIGHT_BOTTON
    };
    vec* GetAmbientLighting();
    void SetAmbientLight(const vec& vLight, AmbientLight_t iLightIndex);
    void SetDefaultLight();

private:
    bool _ShouldCreateStringTable();
    bool m_bJoiningMatch = false;

    void _SetTablePointer(INetworkStringTable* pTable) const;
    void _VerifyOrCreateStringTable()                  const;
    bool _PrecacheModels()                             const;
    void _UpdateEntityModel(int iIndex);

    int      m_iActiveModelIndex = -1;
    model_t* m_pActiveModel      = nullptr;
    bool     m_bModelPrecached   = false;
    std::vector<std::string> m_vecModels =
    {
        "models/player/spy.mdl",
        "models/player/heavy.mdl",
        "models/weapons/w_models/w_toolbox.mdl",
        "models/props_gameplay/tombstone_specialdelivery.mdl",
        "models/props_gameplay/tombstone_crocostyle.mdl",
        "models/props_gameplay/tombstone_tankbuster.mdl",
        "models/props_gameplay/tombstone_gasjockey.mdl"
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

    // Camera...
    vec m_vCameraPos = { -180.0f, 0.0f, 75.0f };
    float m_flCameraFOV = 20.0f; // This most likely won't change ever.

    // Panel...
    int m_iPanelHeight = 800;
    int m_iPanelWidth  = 800;
    int m_iPanelX = 0, m_iPanelY = 0;
    RGBA_t m_panelClr;

    int m_iRenderViewHeight = 700;
    int m_iRenderViewWidth  = 700;
    int m_iRenderViewX = 0, m_iRenderViewY = 0;
    RGBA_t m_renderViewClr;

    // Vtable for panel object
    bool     _SpoofVTable();
    void     _FreeVTable();
    void* m_pSpoofedVTable  = nullptr;
    void* m_pOriginalVTable = nullptr;
    void* m_pOriginalPaint  = nullptr;


    vec m_vAmbientLight[6] =
    {
        vec(0.4f, 0.4f, 0.4f),
        vec(0.4f, 0.4f, 0.4f),
        vec(0.4f, 0.4f, 0.4f),
        vec(0.4f, 0.4f, 0.4f),
        vec(0.4f, 0.4f, 0.4f),
        vec(0.4f, 0.4f, 0.4f),
    };
};
DECLARE_FEATURE_OBJECT(modelPreview, ModelPreview_t)
DEFINE_SECTION(ModelPreview, "MaterialGen", 2)