//=========================================================================
//                      MODEL PREVIEW
//=========================================================================
// by      : INSANE
// created : 31/07/2025
// 
// purpose : Renders any model. Anywhere.
//-------------------------------------------------------------------------
#include "ModelPreview.h"

// SDK
#include "../../SDK/class/BaseEntity.h"
#include "../../SDK/class/IModelLoader.h"
#include "../../SDK/class/IVModelRender.h"
#include "../../SDK/class/IVEngineClient.h"
#include "../../SDK/class/IMaterialSystem.h"
#include "../../SDK/class/IVRenderView.h"
#include "../../SDK/class/IMatRenderContext.h"
#include "../../SDK/class/IVEngineServer.h"
#include "../../SDK/class/INetworkStringTable.h"
#include "../../SDK/class/IEngineTool.h"
#include "../../SDK/class/ISurface.h"
#include "../../SDK/class/IEngineVGui.h"
#include "../../SDK/class/VGui_Panel.h"
#include "../../SDK/class/IPanel.h"
#include "../../SDK/class/VPanel.h"
#include "../../SDK/class/IViewPort.h"
#include "../../SDK/class/CMDL.h"
#include "../../SDK/class/IStudioRender.h"
#include "../../SDK/class/CClientState.h"
#include "../../SDK/class/IVModelInfo.h"
#include "../../SDK/class/CClientLeafSystem.h"

// UTILITY
#include "../../SDK/TF object manager/TFOjectManager.h"
#include "../../Extra/math.h"
#include "../../Utility/ConsoleLogging.h"
#include "../../Utility/Hook Handler/Hook_t.h"
#include "../../Utility/Interface Handler/Interface.h"
#include "../../Utility/Signature Handler/signatures.h"
#include "../../Utility/PullFromAssembly.h"

// Debugging macros
#define ENABLE_DEBUGGING_HOOKS  false
#define DISABLE_ESSENTIAL_HOOKS false

// FUNCTIONS
MAKE_SIG(VGui_Panel_Constructor,      "48 89 5C 24 ? 48 89 74 24 ? 57 48 83 EC ? 48 8B F2 48 8B D9 45 85 C0",                          CLIENT_DLL, void*, Panel*, void*, const char*) // arguments : VTable, parent panel pointer ( Get form engineVGui ), name.
MAKE_SIG(CBaseFlex_Constructor,       "48 89 5C 24 ? 48 89 6C 24 ? 48 89 74 24 ? 57 41 56 41 57 48 83 EC ? 48 8B D9 E8 ? ? ? ? 33 ED", CLIENT_DLL, void*, void*)
MAKE_SIG(CBaseEntity_SetModelByIndex, "48 89 5C 24 ? 57 48 83 EC ? 66 89 91",                                                          CLIENT_DLL, void*, BaseEntity*, unsigned short)
//MAKE_SIG(CMatRenderContext_SetLightingOrigin, "48 83 EC ? 8B 42 ? F2 0F 10 02", MATERIALSYSTEM_DLL, void*, void*, vec*)

// Should be 0x410.
GET_ADRS_FROM_ASSEMBLY(PaintFnOffset,            int, "FF 90 ? ? ? ? 48 8B 0D ? ? ? ? 49 8B D7 48 8B 01 FF 50 ? 48 8B 0D ? ? ? ? 49 8B D7", 2, 6, CLIENT_DLL)
GET_ADRS_FROM_ASSEMBLY(ModelPrecacheTableOffset, int, "48 89 BB ? ? ? ? 48 89 BB ? ? ? ? 48 89 BB ? ? ? ? 8D 57", 3, 7, ENGINE_DLL)


#if (DISABLE_ESSENTIAL_HOOKS == false)

MAKE_HOOK(CModelInfoClient_GetModel, "83 FA ? 0F 8C ? ? ? ? 48 8D 0D ? ? ? ? E9 ? ? ? ? CC CC CC CC CC CC CC CC CC CC CC 48 89 5C 24", __fastcall, ENGINE_DLL, void*, void* a1, int iModelIndex)
{
    // If in main menu, then just always return. it doesn't matter, nothing's gonna call that shit.
    if (I::iEngine->IsConnected() == false)
        return F::modelPreview.GetActiveModel();

    return Hook::CModelInfoClient_GetModel::O_CModelInfoClient_GetModel(a1, iModelIndex);
}
#endif


//=========================================================================
//                     PUBLIC METHODS
//=========================================================================
void ModelPreview_t::Run()
{
    if (m_bVisible == false)
        return;

    // Make sure to not create string tables while in game or loading in a game.
    if (_ShouldCreateStringTable() == true)
    {
        _VerifyOrCreateStringTable();
    }

    // Precache models.
    if (m_bModelPrecached == false)
    {
        m_bModelPrecached = _PrecacheModels();
    }

    // Inititalize entity & panel object
    if (_Initialize() == false)
        return;
}


///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
bool ModelPreview_t::_Initialize()
{
    if (m_bPanelInitilized == false)
    {
        if (_InitializePanel() == false)
        {
            FAIL_LOG("panel init failed");
            return false;
        }
    }


    if (m_bEntInit == false)
    {
        if (_InitializeEntity() == false)
        {
            FAIL_LOG("Entity init failed");
            return false;
        }
    }


    return true;
}


///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
bool ModelPreview_t::_ShouldCreateStringTable()
{
    // Only when we are joining a match, do we need to create string tables ourselves.
    if (m_bJoiningMatch == false)
        return I::iEngine->IsConnected() == false;

    // else never create string tables, the engine will create it for us.
    return false;
}


///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
void ModelPreview_t::DiscardStringTables() const
{
    I::iNetworkStringTableContainer->RemoveAllTables();
    _SetTablePointer(nullptr);

    FAIL_LOG("Deleted MODEL_PRECACHE_TABLE and cleanup refrences.");
}


///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
void ModelPreview_t::SetActiveModel(int iIndex)
{
    // Don't set model pointer if model name not added to string table.
    if (m_bModelPrecached == false)
    {
        return;
    }

    if (iIndex == m_iActiveModelIndex)
        return;

    iIndex = std::clamp<int>(iIndex, 0, m_vecModels.size() - 1);

    m_iActiveModelIndex = iIndex;
    m_pActiveModel = I::iModelLoader->GetModelForName(m_vecModels[iIndex].c_str(), IModelLoader::FMODELLOADER_CLIENT);

    if (m_pActiveModel == nullptr)
    {
        FAIL_LOG("Failed to load model \"%s\". Setting model to default ( %s )", m_vecModels[iIndex].c_str(), m_vecModels[0].c_str());
        SetActiveModel(0);
    }

    WIN_LOG("Set active model to \"%s\" @ index : %d. [ %p ]", m_vecModels[m_iActiveModelIndex].c_str(), m_iActiveModelIndex, m_pActiveModel);
}


///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
bool ModelPreview_t::_InitializeEntity()
{
    assert(m_bEntInit == false);

    if (m_bEntInit == true)
    {
        FAIL_LOG("Entity already intitialized!!");
        return true;
    }


    constexpr uint64_t iEntSize = 0x400ull * 10ull;
    if (m_pEnt == nullptr)
    {
        m_pEnt = reinterpret_cast<BaseEntity*>(malloc(iEntSize));
        if (m_pEnt == nullptr)
        {
            FAIL_LOG("Bad malloc");
            return false;
        }

        memset(m_pEnt, 0, iEntSize);
        WIN_LOG("Model Entity allcoated @ %p", m_pEnt);
    }


    Sig::CBaseFlex_Constructor(m_pEnt);

    // Model we set here doesn't matter, We can change it later.
    auto* pTable = I::iNetworkStringTableContainer->FindTable(MODEL_PRECACHE_TABLENAME);
    if (pTable)
    {
        int iIndex = pTable->FindStringIndex(m_vecModels[0].c_str());
        Sig::CBaseEntity_SetModelByIndex(m_pEnt, iIndex);
        LOG("Set default model to \"%s\" @ index [ %d ]", m_vecModels[0].c_str(), 0);
    }

    m_bEntInit = true;
    return true;
}


///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
void ModelPreview_t::_FreeEntity()
{
    if (m_pEnt == nullptr || m_bEntInit == false)
        return;

    free(m_pEnt);
    m_pEnt = nullptr;
    m_bEntInit = false;
    LOG("Free entity successfully");
}


///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
void __fastcall PaintHijack(Panel* a1)
{
    // Setting panel's size
    int iWidth = 0, iHeight = 0;
    F::modelPreview.GetPanelSize(iHeight, iWidth);
    I::iPanel->SetSize(a1->GetVPanel(), iWidth, iHeight);
    RGBA_t panelClr = F::modelPreview.GetPanelClr(); I::iSurface->DrawSetint(panelClr.r, panelClr.g, panelClr.b, panelClr.a);
    I::iSurface->DrawFilledRect(0, 0, iWidth, iHeight);

    // Setting panel's pos
    int x = 0, y = 0; F::modelPreview.GetPanelPos(x, y);
    I::iPanel->SetPos(a1->GetVPanel(), x, y);

    BaseEntity* pEnt = F::modelPreview.GetModelEntity();

    // if model entity not initialized call original & leave.
    if (pEnt == nullptr || F::modelPreview.GetActiveModel() == nullptr || F::modelPreview.GetActiveModelIndex() == -1)
    {
        ((void(__fastcall*)(Panel*))(F::modelPreview.GetOriginalPaintFn()))(a1);
        return;
    }


    static void* pCubeMap{ nullptr };
    if (pCubeMap == nullptr)
    {
        pCubeMap = I::iMaterialSystem->FindTexture("editor/cubemap.hdr", TEXTURE_GROUP_CUBE_MAP, true);
        if (pCubeMap == nullptr) return;
        WIN_LOG("CubeMap good");
    }


    static CViewSetup view; static bool bViewInit = false;
    if (bViewInit == false)
    {
        bViewInit = true;
        memset(&view, 0, sizeof(CViewSetup));

        view.fov = 30.0f; view.m_bOrtho = false; view.zFar = 1000; view.zNear = 7;
        view.x = 100; view.y = 100;
        view.height = 100; view.width = 100;

        view.origin = vec(-220.0f, -4.0f, 35.0f); view.angles.Init();
        WIN_LOG("View good");
    }

    view.fov = F::modelPreview.GetBaseCameraFOV();

    // View setup origin
    view.origin = F::modelPreview.GetCameraPos();

    // Setting view setup pos & size.
    int iRenderViewHeight = 0, iRenderViewWidth = 0; F::modelPreview.GetRenderViewSize(iRenderViewHeight, iRenderViewWidth);
    int iRenderViewX      = 0, iRenderViewY     = 0; F::modelPreview.GetRenderViewPos(iRenderViewX, iRenderViewY);

    view.x      = iRenderViewX;      view.y     = iRenderViewY;
    view.height = iRenderViewHeight; view.width = iRenderViewWidth;

    // Animation fix
    {
        pEnt->m_flAnimTime(pEnt->m_flAnimTime() + tfObject.pGlobalVar->frametime / 2.0f);
        pEnt->m_flCycle(pEnt->m_flCycle() + 0.002f);
        pEnt->m_nSequence(0);
    }


    // Rendering model
    {
        IMatRenderContext* pRenderCtx = I::iMaterialSystem->GetRenderContext();
        pRenderCtx->BindLocalCubemap(pCubeMap);

        // Fix lighting n shit.
        vec vLightOrigin(0.0f); pRenderCtx->SetLightingOrigin(&vLightOrigin); // We also have signature for this function.
        pRenderCtx->SetAmbientLight(1.0f, 1.0f, 1.0f);
        I::iStudioRender->SetAmbientLightColors(F::modelPreview.GetAmbientLighting());
        I::iStudioRender->SetLocalLights(0, nullptr);


        Frustum dummyFrustum;
        I::iVRenderView->Push3DView(&view, 1 | 2 | 32, NULL, dummyFrustum);
        vec vClrModulation(1.0f); I::iVRenderView->SetColorModulation(&vClrModulation);
        I::iVRenderView->SetBlend(1.0f);

        pRenderCtx->ClearBuffers(true, false);
        RGBA_t clr = F::modelPreview.GetRenderViewClr(); pRenderCtx->ClearColor4ub(clr.r, clr.g, clr.b, clr.a);

        I::iVModelRender->SuppressEngineLighting(true);
        pEnt->DrawModel(STUDIO_RENDER);
        I::iVModelRender->SuppressEngineLighting(false);

        I::iVRenderView->PopView(dummyFrustum);

        pRenderCtx->BindLocalCubemap(nullptr);
    }


    // calling original paint.
    ((void(__fastcall*)(Panel*))(F::modelPreview.GetOriginalPaintFn()))(a1);
}


///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
void ModelPreview_t::_SetTablePointer(INetworkStringTable* pTable) const
{
    *reinterpret_cast<INetworkStringTable**>(reinterpret_cast<uintptr_t>(I::cClientState) + static_cast<uint64_t>(*ASM::ModelPrecacheTableOffset)) = pTable;
    //*reinterpret_cast<INetworkStringTable**>(reinterpret_cast<uintptr_t>(I::CGameServer) + 0x54298LLU)  = pTable;
}


///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
void ModelPreview_t::_VerifyOrCreateStringTable() const
{
    // If in game, then the table must be there.
    if (I::iEngine->IsInGame() == true)
        return;

    // already made
    INetworkStringTable* pTable = I::iNetworkStringTableContainer->FindTable(MODEL_PRECACHE_TABLENAME);
    if (pTable != nullptr)
        return;

    I::iNetworkStringTableContainer->m_bAllowCreation = true;
    pTable = I::iNetworkStringTableContainer->CreateStringTableEx(MODEL_PRECACHE_TABLENAME, (1 << 12), 1, 2, false);
    I::iNetworkStringTableContainer->m_bAllowCreation = false;

    _SetTablePointer(pTable);

    if (pTable == nullptr)
        FAIL_LOG("Failed to create string table");

    WIN_LOG("Created \"%s\" string table successfully", MODEL_PRECACHE_TABLENAME);
}


///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
bool ModelPreview_t::_PrecacheModels() const
{
    INetworkStringTable* pTable = I::iNetworkStringTableContainer->FindTable(MODEL_PRECACHE_TABLENAME);
    if (pTable == nullptr)
    {
        FAIL_LOG("MODEL_PRECACHE_TABLE not found");
        return false;
    }


    for (const std::string& szModelname : m_vecModels)
    {
        int iMDLIndex = pTable->FindStringIndex(szModelname.c_str());

        // already added to table
        if (iMDLIndex != INVALID_TABLE_INDEX)
        {
            LOG("Model \"%s\" is already precached", szModelname.c_str());
            continue;
        }

        pTable->AddString(false, szModelname.c_str());

        int iModelIndex = pTable->FindStringIndex(szModelname.c_str());

        LOG("Precached model \"%s\" @ index : %d", szModelname.c_str(), iModelIndex);
    }

    WIN_LOG("Model Precache done!");
    return true;
}


///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
bool ModelPreview_t::_InitializePanel()
{
    if (m_bPanelInitilized == true)
        return true;

    assert(m_pSpoofedVTable == nullptr && m_pOriginalPaint == nullptr && m_pOriginalVTable == nullptr && "Trying to reinitialize panel object.");

    // Checking if the panel already exists. ( i.e. if we are injecting. )
    // EDIT : It seems like the panel gets auto deleted, cause I am unable to find it in reinjection. I'm just gonna leave it here.
    vgui::VPANEL vDesiredPanel = I::iPanel->FindChildByName(I::iSurface->GetEmbeddedPanel(), std::string(m_szPanelName), true);
    if (vDesiredPanel != NULL)
    {
        if (_SpoofVTable() == false)
        {
            FAIL_LOG("Failed to spoof VTable");
            return false;
        }

        I::iPanel->SetParent(vDesiredPanel, I::iSurface->GetEmbeddedPanel());
        I::iPanel->SetEnabled(m_pPanel->GetVPanel(), true);
        I::iPanel->SetVisible(m_pPanel->GetVPanel(), true);
        I::iPanel->SetPos(m_pPanel->GetVPanel(), 0, 0);
        I::iPanel->SetSize(m_pPanel->GetVPanel(), m_iPanelWidth, m_iPanelHeight);

        LOG("Panel \"%s\" already exists, reusing it.", m_szPanelName);

        m_bPanelInitilized = true;
        return true;
    }
    LOG("Panel \"%s\" doesn't exist yet. Creating one.", m_szPanelName);


    // Allocate our panel object some memory.
    if (m_pPanel == nullptr)
    {
        constexpr size_t iPanelSize = 0x400ULL * 2; //  It only needed 400 bytes, but its valve we are talking about.
        m_pPanel = reinterpret_cast<Panel*>(malloc(iPanelSize));
        if (m_pPanel == nullptr)
        {
            FAIL_LOG("Failed memory allocation for panel");
            return false;
        }

        memset(m_pPanel, 0, iPanelSize);
    }

    // Check if the parent panel is good or not
    VPanel* pParentVPanel = reinterpret_cast<VPanel*>(I::iSurface->GetEmbeddedPanel());
    if (pParentVPanel == nullptr)
    {
        FAIL_LOG("Bad parent VPanel!");
        return false;
    }
    Panel* pParentPanel = pParentVPanel->Client();
    if (pParentPanel == nullptr)
    {
        FAIL_LOG("Parent panel* is invalid ( NULL )");
        return false;
    }


    // Run our panel object throught the constructor so VTables-n-shit gets setup properly
    Sig::VGui_Panel_Constructor(m_pPanel, pParentPanel, m_szPanelName);
    Sig::VGui_Panel_MakeReadyToUse(m_pPanel);

    // spoof the VTable, replace the paint function with ours.
    if (m_pSpoofedVTable == nullptr)
    {
        _SpoofVTable();
    }

    // Enable the panel.
    I::iPanel->SetEnabled(m_pPanel->GetVPanel(), true);
    I::iPanel->SetVisible(m_pPanel->GetVPanel(), true);
    I::iPanel->SetPos(m_pPanel->GetVPanel(), 0, 0);
    I::iPanel->SetSize(m_pPanel->GetVPanel(), m_iPanelWidth, m_iPanelHeight);

    m_bPanelInitilized = true;
    WIN_LOG("Showcase panel initialized! @[ %p ]", m_pPanel);
    return true;
}


///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
bool ModelPreview_t::_SpoofVTable()
{
    if (m_pSpoofedVTable != nullptr)
        return false;


    constexpr size_t iVTableSize = 213ULL * sizeof(void*); // Its 257 fn for CTFPlayerModelPanel
    m_pSpoofedVTable = malloc(iVTableSize);
    if (m_pSpoofedVTable == nullptr)
    {
        FAIL_LOG("Failed to allocate mem for spoofed vtable");
        return false;
    }

    // just to be safe, check if paint fn index gets changed.
    if (*ASM::PaintFnOffset != 0x410)
        FAIL_LOG("Panel::Paint fn index has been modified by devs. We are using index : %d offset : 0x%X", *ASM::PaintFnOffset / 8, *ASM::PaintFnOffset);

    // copy original vtable to our memory.
    m_pOriginalVTable = *reinterpret_cast<void***>(m_pPanel);
    memcpy(m_pSpoofedVTable, m_pOriginalVTable, iVTableSize);

    // Store the original paint fn adrs.
    m_pOriginalPaint = *reinterpret_cast<void**>(reinterpret_cast<uintptr_t>(m_pOriginalVTable) + *ASM::PaintFnOffset);

    // Sawp out the paint function.
    *reinterpret_cast<uintptr_t*>(reinterpret_cast<uintptr_t>(m_pSpoofedVTable) + *ASM::PaintFnOffset) = reinterpret_cast<uintptr_t>(PaintHijack);

    // Swap out the VTable with our modified VTable.
    *reinterpret_cast<void**>(m_pPanel) = m_pSpoofedVTable;

    WIN_LOG("Spoofed VTable successfully");
}


///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
void ModelPreview_t::Free()
{
    if(m_pPanel != nullptr)
    {
        I::iPanel->SetVisible(m_pPanel->GetVPanel(), false);
        I::iPanel->SetEnabled(m_pPanel->GetVPanel(), false);
        I::iPanel->SetSize(m_pPanel->GetVPanel(), 1, 1); // This seems to fix it.
    }

    _FreeVTable();
    _FreeEntity();
}


///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
void ModelPreview_t::_FreeVTable()
{
    if (m_pSpoofedVTable == nullptr || m_pOriginalVTable == nullptr)
        return;
    
    // Restore the original VTable for our panel object.
    *reinterpret_cast<uintptr_t*>(m_pPanel) = reinterpret_cast<uintptr_t>(m_pOriginalVTable);

    free(m_pSpoofedVTable);
    WIN_LOG("Freed & restored VTable succesfully");
}


/////////////////////// GETTERS & SETTERS /////////////////////////////////
///////////////////////////////////////////////////////////////////////////
void ModelPreview_t::SetPanelClr(unsigned char r, unsigned char g, unsigned char b, unsigned char a)
{
    m_panelClr.r = r; m_panelClr.g = g; m_panelClr.b = b; m_panelClr.a = a;
}


///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
RGBA_t ModelPreview_t::GetPanelClr() const
{
    return m_panelClr;
}


///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
void ModelPreview_t::SetRenderViewClr(unsigned char r, unsigned char g, unsigned char b, unsigned char a)
{
    m_renderViewClr.r = r; m_renderViewClr.g = g; m_renderViewClr.b = b; m_renderViewClr.a = a;
}


///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
RGBA_t ModelPreview_t::GetRenderViewClr() const
{
    return m_renderViewClr;
}


///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
void ModelPreview_t::SetCameraPos(const vec& vCameraPos)
{
    m_vCameraPos = vCameraPos;
}


///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
vec ModelPreview_t::GetCameraPos() const
{
    return m_vCameraPos;
}


///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
float ModelPreview_t::GetBaseCameraFOV() const
{
    return m_flCameraFOV;
}


///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
float ModelPreview_t::GetVerticalFOV() const
{
    int iHeight = 0, iWidth = 0; GetRenderViewSize(iHeight, iWidth);

    float flScale = static_cast<float>(iHeight) / static_cast<float>(iWidth);

    float flHorizontalFOV = DEG2RAD(GetBaseCameraFOV());
    
    // Frustum width at 1 unit from camera. with correct horizontal FOV ofcourse.
    float flFrustumWidth = 2.0f * (tanf(flHorizontalFOV / 2.0f) * 1.0f);

    // Size the height according to how much bigger the height is from the width.
    float flFrustumHeight = flFrustumWidth * flScale;

    return RAD2DEG(2.0f * atanf((flFrustumHeight / 2.0f) / 1.0f));
}


///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
float ModelPreview_t::GetHorizontalFOV() const
{
    float flBaseFOV = DEG2RAD(GetBaseCameraFOV());

    int iHeight = 0, iWidth = 0; GetRenderViewSize(iHeight, iWidth);
    float flWidthCorrect = static_cast<float>(iHeight) * (4.0f / 3.0f);
    
    // This is the distance at which the frustum width will be equal to 4:3 of the height.
    // cause that is what the FOV is correct for.
    float flDist = (static_cast<float>(flWidthCorrect) / 2.0f) / tanf(flBaseFOV / 2.0f);

    // at the same distance ( flDist ) what angle will result in the frustum being equal to 
    // our renderview's size ( i.e. custom width )
    return RAD2DEG(2.0f * atanf((static_cast<float>(iWidth) / 2.0f) / flDist));
}


///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
void ModelPreview_t::SetCameraFOV(const float flCameraFOV)
{
    m_flCameraFOV = flCameraFOV;
}


///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
void ModelPreview_t::InvalidateModelPrecache()
{
    m_bModelPrecached = false;
}


///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
void ModelPreview_t::JoiningMatch(bool bJoining)
{
    m_bJoiningMatch = bJoining;
}


///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
model_t* ModelPreview_t::GetActiveModel() const
{
    return m_pActiveModel;
}


///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
int ModelPreview_t::GetActiveModelIndex() const
{
    return m_iActiveModelIndex;
}


///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
BaseEntity* ModelPreview_t::GetModelEntity() const
{
    return m_pEnt;
}


///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
void ModelPreview_t::SetVisible(bool bVisible)
{
    // Change panel visibility
    if (m_pPanel != nullptr)
    {
        I::iPanel->SetVisible(m_pPanel->GetVPanel(), bVisible);
    }

    bool bRefreshModel = bVisible == true && bVisible != m_bVisible;
    m_bVisible = bVisible;

    if (m_pEnt == nullptr || m_iActiveModelIndex == -1 || m_pActiveModel == nullptr)
        return;

    // Refresh model once if opened ( to prevent bullshit )
    if (bRefreshModel == true)
    {
        INetworkStringTable* pTable = I::iNetworkStringTableContainer->FindTable(MODEL_PRECACHE_TABLENAME);
        if (pTable == nullptr)
        {
            FAIL_LOG("String table not found!");
        }
        else
        {
            int iIndex = pTable->FindStringIndex(m_vecModels[m_iActiveModelIndex].c_str());
            Sig::CBaseEntity_SetModelByIndex(m_pEnt, iIndex);
            LOG("Updated model info for our model entity. Set to \"%s\"", m_vecModels[m_iActiveModelIndex].c_str());
        }
    }
}


///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
void ModelPreview_t::SetPanelSize(int iHeight, int iWidth)
{
    m_iPanelHeight = iHeight;
    m_iPanelWidth  = iWidth;
}


///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
void ModelPreview_t::GetPanelSize(int& iHeight, int& iWidth) const
{
    iHeight = m_iPanelHeight; 
    iWidth  = m_iPanelWidth;
}


///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
void ModelPreview_t::SetPanelPos(int x, int y)
{
    m_iPanelX = x; m_iPanelY = y;
}


///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
void ModelPreview_t::GetPanelPos(int& x, int& y) const
{
    x = m_iPanelX; y = m_iPanelY;
}


///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
void ModelPreview_t::SetRenderViewSize(int iHeight, int iWidth)
{
    m_iRenderViewHeight = iHeight; 
    m_iRenderViewWidth  = iWidth;
}


///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
void ModelPreview_t::GetRenderViewSize(int& iHeight, int& iWidth) const
{
    iHeight = m_iRenderViewHeight; 
    iWidth  = m_iRenderViewWidth;
}


///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
void ModelPreview_t::SetRenderViewPos(int x, int y)
{
    m_iRenderViewX = x; m_iRenderViewY = y;
}


///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
void ModelPreview_t::GetRenderViewPos(int& x, int& y) const
{
    x = m_iRenderViewX; y = m_iRenderViewY;
}


///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
vec* ModelPreview_t::GetAmbientLighting()
{
    return m_vAmbientLight;
}


///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
void ModelPreview_t::SetAmbientLight(const vec& vLight, AmbientLight_t iLightIndex)
{
    m_vAmbientLight[iLightIndex] = vLight;
}


///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
void ModelPreview_t::SetDefaultLight()
{
    vec vDefaultLight(0.4f, 0.4f, 0.4f);

    m_vAmbientLight[AmbientLight_t::LIGHT_TOP]    = vDefaultLight;
    m_vAmbientLight[AmbientLight_t::LIGHT_BOTTON] = vDefaultLight;
    m_vAmbientLight[AmbientLight_t::LIGHT_BACK]   = vDefaultLight;
    m_vAmbientLight[AmbientLight_t::LIGHT_FRONT]  = vDefaultLight;
    m_vAmbientLight[AmbientLight_t::LIGHT_RIGHT]  = vDefaultLight;
    m_vAmbientLight[AmbientLight_t::LIGHT_LEFT]   = vDefaultLight;
}




//=========================================================================
//                     DEBUGGING
//=========================================================================
#if (DISABLE_ESSENTIAL_HOOKS == false)

struct LightingState_t
{
    vec         r_boxcolor[6];		// ambient, and lights that aren't in locallight[]
    int			numlights;
    void* locallight[4];
};

MAKE_HOOK(LightcacheGetDynamic, "48 89 5C 24 ? 44 89 4C 24 ? 4C 89 44 24 ? 55 56 57 41 54 41 55 41 56 41 57 48 81 EC ? ? ? ? 48 8B 05", __fastcall, ENGINE_DLL,
    void*, void* pThis, vec* vOrigin, LightingState_t* pLightState, void* a1, int a2, bool a3)
{
    if (I::iEngine->IsInGame() == true)
        return Hook::LightcacheGetDynamic::O_LightcacheGetDynamic(pThis, vOrigin, pLightState, a1, a2, a3);


    //WIN_LOG("Returned spoofed light state for Origin { %.2f %.2f %.2f }", vOrigin->x, vOrigin->y, vOrigin->z);
    for (int i = 0; i < 6; i++)
    {
        pLightState->r_boxcolor[i] = vec(1.0f, 1.0f, 1.0f);

        if (i < 4)
            pLightState->locallight[i] = nullptr;
    }
    pLightState->numlights = 0;
    return nullptr;
}


MAKE_HOOK(CBaseEntity_UpdateVisibility, "48 89 5C 24 ? 48 89 74 24 ? 57 48 83 EC ? 48 8B D9 48 8B 0D", __fastcall, CLIENT_DLL, void*,
    void* pEnt)
{
    if (pEnt == F::modelPreview.GetModelEntity())
    {
        FAIL_LOG("Skipped visibility update for ModelEntity.");
        return nullptr;
    }

    return Hook::CBaseEntity_UpdateVisibility::O_CBaseEntity_UpdateVisibility(pEnt);
}

#endif