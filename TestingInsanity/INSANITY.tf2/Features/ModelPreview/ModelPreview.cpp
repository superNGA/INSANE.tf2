#include "ModelPreview.h"

// SDK
#include "../../SDK/class/BaseEntity.h"
#include "../../SDK/class/IModelLoader.h"
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

// UTILITY
#include "../../Extra/math.h"
#include "../../Utility/ConsoleLogging.h"
#include "../../Utility/Hook Handler/Hook_t.h"
#include "../../Utility/Interface Handler/Interface.h"
#include "../../Utility/Signature Handler/signatures.h"

// Debugging macros
#define ENABLE_DEBUGGING_HOOKS false

// FUNCTIONS
MAKE_SIG(VGui_Panel_Constructor, "48 89 5C 24 ? 48 89 74 24 ? 57 48 83 EC ? 48 8B F2 48 8B D9 45 85 C0", CLIENT_DLL, void*, Panel*, void*, const char*) // arguments : VTable, parent panel pointer ( Get form engineVGui ), name.
MAKE_SIG(SetModelAnglePos, "48 89 5C 24 ? 48 89 74 24 ? 57 48 83 EC ? 49 8B D8 48 8B FA 48 8B F1 E8 ? ? ? ? 8B 03", CLIENT_DLL, void*, void*, qangle, vec)
MAKE_SIG(SetClr, "8B 02 89 81 ? ? ? ? C3 CC CC CC CC CC CC CC F3 0F 11 89", CLIENT_DLL, void*, void*, uint32_t*)
MAKE_SIG(CTFPlayerModelPanel_Constructor, "48 89 5C 24 ? 48 89 74 24 ? 57 48 83 EC ? 48 8B F9 45 85 C9 74 ? 48 8D 05 ? ? ? ? 48 89 41 ? 45 33 C9 E8 ? ? ? ? 48 8D 05 ? ? ? ? 48 89 07 48 8D 35 ? ? ? ? 48 8D 05", CLIENT_DLL, void*, Panel*, Panel*, const char*, int)
MAKE_SIG(CTFPlayerModelPanel_SetPlayerClass, "48 89 5C 24 ? 48 89 6C 24 ? 48 89 74 24 ? 48 89 7C 24 ? 41 56 48 83 EC ? 44 8B F2", CLIENT_DLL, void*, Panel*, int, bool, bool)
MAKE_SIG(CTFPlayerModelPanel_ClearCarriedItem, "48 89 5C 24 ? 57 48 83 EC ? 48 8B D9 E8 ? ? ? ? 48 8B 0D ? ? ? ? 4C 8D 83", CLIENT_DLL, void*, Panel*)
MAKE_SIG(CTFPlayerModelPanel_UpdatePreviewVisuals, "40 53 57 41 54 41 56 48 81 EC ? ? ? ? 45 33 E4", CLIENT_DLL, void*, Panel*)

MAKE_SIG(InitializeAsClientEntity, "48 89 5C 24 ? 48 89 74 24 ? 57 48 83 EC ? 41 8B F0 48 8B D9 48 85 D2", CLIENT_DLL, bool, void*, const char*, int)
MAKE_SIG(CBaseFlex_Constructor, "48 89 5C 24 ? 48 89 6C 24 ? 48 89 74 24 ? 57 41 56 41 57 48 83 EC ? 48 8B D9 E8 ? ? ? ? 33 ED", CLIENT_DLL, void*, void*)

class IViewPortPanel;
MAKE_SIG(VGui_Panel_FindChildByName, "48 8B C4 53 48 81 EC ? ? ? ? 48 89 70 ? 33 F6", CLIENT_DLL, Panel*, Panel*, const char*, bool)
MAKE_SIG(IViewPort_GetPanelByName, "48 89 5C 24 ? 48 89 6C 24 ? 48 89 74 24 ? 48 89 7C 24 ? 41 56 48 83 EC ? 48 63 71 ? 33 FF", CLIENT_DLL, IViewPortPanel*, void*, const char*)
MAKE_SIG(GetClientModeNormal, "40 53 48 83 EC ? 8B 05 ? ? ? ? A8 ? 0F 85 ? ? ? ? 83 C8 ? 48 8D 1D ? ? ? ? 48 8B CB 89 05 ? ? ? ? E8 ? ? ? ? 33 C9", CLIENT_DLL, void*, bool)

MAKE_SIG(CClassLoadoutPanel_UpdateModelPanels, "40 53 48 83 EC ? 48 89 74 24 ? 48 8B D9 8B 89", CLIENT_DLL, void*, void*)
MAKE_SIG(CMDLPanel_Paint, "40 55 48 81 EC ? ? ? ? 48 8B E9 E8 ? ? ? ? F6 85", CLIENT_DLL, void*, Panel*)

/*
THEORY :
    Flow :
           Panel::PaintTraverse() --> CMDLPanel::Paint() --> PotteryWheel::Paint() -->
           IMatSystemSurface::Begin3DPaint() --> CMDLPaint::OnPaint3D() --> IMatSystemSurface::End3DPaint()

*/

MAKE_INTERFACE_SIGNATURE(CGameServer, "48 8D 0D ? ? ? ? F3 0F 10 3D", void*, ENGINE_DLL, 3, 7)


MAKE_HOOK(GetModel, "83 FA ? 0F 8C ? ? ? ? 48 8D 0D ? ? ? ? E9 ? ? ? ? CC CC CC CC CC CC CC CC CC CC CC 48 89 5C 24", __fastcall, ENGINE_DLL, void*, void* a1, int a2)
{
    auto result = Hook::GetModel::O_GetModel(a1, a2);
    if (result == nullptr)
    {
        WIN_LOG("returned ( %p ) for index %d", F::modelPreview.m_pModel, a2);
        __debugbreak();

        return F::modelPreview.m_pModel;
    }

    return result;
}


//=========================================================================
//                     PUBLIC METHODS
//=========================================================================

vgui::VPANEL FindChildByName(vgui::VPANEL parent, const std::string& szChildName, bool bRecurse)
{
    // Sanity checks
    if (parent == NULL)
        return NULL;

    // parent is child ? ( LMAO )
    if (std::string(I::iPanel->GetName(parent)) == szChildName)
        return parent;

    // Child count
    int nChildren = I::iPanel->GetChildCount(parent);

    // No children ?
    if (nChildren == 0)
        return NULL;

    // Check all children for a match
    for (int iChildIndex = 0; iChildIndex < nChildren; iChildIndex++)
    {
        vgui::VPANEL child = I::iPanel->GetChild(parent, iChildIndex);

        // match found
        if (std::string(I::iPanel->GetName(child)) == szChildName)
            return child;
    }


    if (bRecurse == true)
    {
        for (int iChildIndex = 0; iChildIndex < nChildren; iChildIndex++)
        {
            vgui::VPANEL child = I::iPanel->GetChild(parent, iChildIndex);

            // recurse till we found match
            vgui::VPANEL childMatch = FindChildByName(child, szChildName, bRecurse);

            // if match found
            if (childMatch != NULL)
                return childMatch;
        }
    }

    return NULL;
}


void ModelPreview_t::Run()
{
    if (Features::ModelPreview::ModelPreview::Enable.IsActive() == false)
        return;

    if (_InitializeEntity() == false)
        return;

    if (_InitializePanel() == false)
        return;
}


bool ModelPreview_t::_InitializeEntity()
{
    if (m_bEntInit == true)
        return true;


    auto* pTable = I::iNetworkStringTableContainer->FindTable(MODEL_PRECACHE_TABLENAME);
    if (pTable == nullptr)
    {
        I::iNetworkStringTableContainer->m_bAllowCreation = true;

        const char szModelPrecacheTable[255] = MODEL_PRECACHE_TABLENAME;
        pTable = I::iNetworkStringTableContainer->CreateStringTableEx(szModelPrecacheTable, (1 << 12), 1, 2, false);

        I::iNetworkStringTableContainer->m_bAllowCreation = false;

        if (pTable == nullptr)
        {
            FAIL_LOG("Failed to create table");
            return false;
        }

        pTable->AddString(false, "models/player/soldier.mdl");
        pTable->AddString(false, "models/player/scout.mdl");
        *reinterpret_cast<INetworkStringTable**>(reinterpret_cast<uintptr_t>(I::cClientState) + 0x8EA8ull)  = pTable;
        *reinterpret_cast<INetworkStringTable**>(reinterpret_cast<uintptr_t>(I::CGameServer)  + 0x54298ull) = pTable;
        WIN_LOG("Create table successfully @ { %p }", pTable);
    }


    uint64_t iEntSize = 0x400ull * 10ull;
    if (m_pEnt == nullptr)
    {
        m_pEnt = malloc(iEntSize);
        if (m_pEnt == nullptr)
        {
            FAIL_LOG("Bad malloc");
            return false;
        }
    }

    if (m_pModel == nullptr)
    {
        m_pModel = I::iModelLoader->GetModelForName("models/player/scout.mdl", IModelLoader::FMODELLOADER_CLIENT);
        if (m_pModel == nullptr)
        {
            FAIL_LOG("Failed to find model");
            return false;
        }

        WIN_LOG("Found model @ %p", m_pModel);
    }

    memset(m_pEnt, 0, iEntSize);
    Sig::CBaseFlex_Constructor(m_pEnt);
    if (Sig::InitializeAsClientEntity(m_pEnt, "models/player/scout.mdl", RENDER_GROUP_OPAQUE_ENTITY) == false)
    {
        FAIL_LOG("Failed to create entity");
        return false;
    }

    *reinterpret_cast<model_t**>(reinterpret_cast<uintptr_t>(m_pEnt) + 152ull) = reinterpret_cast<model_t*>(m_pModel);
    m_bEntInit = true;
    return true;
}


static vec white[6] =
{
    vec(0.4, 0.4, 0.4),
    vec(0.4, 0.4, 0.4),
    vec(0.4, 0.4, 0.4),
    vec(0.4, 0.4, 0.4),
    vec(0.4, 0.4, 0.4),
    vec(0.4, 0.4, 0.4),
};

void __fastcall PaintHijack(Panel* a1)
{
    I::iSurface->DrawSetint(255, 255, 255, 255);
    I::iSurface->DrawFilledRect(0, 0, F::modelPreview.m_iWidth, F::modelPreview.m_iHeight);

    
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
        bViewInit = true; memset(&view, 0, sizeof(CViewSetup)); 
        view.fov = 70.0f; view.m_bOrtho = false; view.zFar = 1000; view.zNear = 7;
        view.x      = F::modelPreview.m_iWidth  / 4;  view.y    = F::modelPreview.m_iHeight / 4;
        view.height = F::modelPreview.m_iHeight / 2; view.width = F::modelPreview.m_iWidth  / 2;

        view.origin = vec(-220.0f, -4.0f, 35.0f); view.angles.Init();
        WIN_LOG("View good");
    }

    if(F::modelPreview.m_pEnt != nullptr)
    {
        auto* pRenderCtx = I::iMaterialSystem->GetRenderContext();
        pRenderCtx->BindLocalCubemap(pCubeMap);
        
        // Fix lighting n shit.
        vec vLightOrigin(0.0f); (*(void(__fastcall**)(void*, vec*))(*reinterpret_cast<uintptr_t*>(pRenderCtx) + 0x4E8ull))(pRenderCtx, &vLightOrigin);
        pRenderCtx->SetAmbientLight(0.0f, 1.0f, 0.0f);
        I::iStudioRender->SetAmbientLightColors(white);
        I::iStudioRender->SetLocalLights(0, nullptr);


        Frustum dummyFrustum; 
        I::iVRenderView->Push3DView(&view, 1 | 2 | 32, NULL, dummyFrustum);
        vec vClrModulation(1.0f); I::iVRenderView->SetColorModulation(&vClrModulation);
        I::iVRenderView->SetBlend(1.0f);
        
        pRenderCtx->ClearBuffers(true, false);

        printf("Gonna draw\n");
        reinterpret_cast<BaseEntity*>(F::modelPreview.m_pEnt)->DrawModel(STUDIO_RENDER);    
        printf("Done draw\n");
        
        I::iVRenderView->PopView(dummyFrustum);

        pRenderCtx->BindLocalCubemap(nullptr);
    }
    

    // calling original paint.
    ((void(__fastcall*)(Panel*))(F::modelPreview.GetOriginalPaintFn()))(a1);
}


bool ModelPreview_t::_InitializePanel()
{
    if (m_bPanelInitilized == true)
        return true;

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
    Sig::VGui_Panel_Constructor(m_pPanel, pParentPanel, "INSANE_ShowcasePanel");
    Sig::VGui_Panel_MakeReadyToUse(m_pPanel);

    // spoof the VTable, so we can fuck with the paint function :).
    if (m_pSpoofedVTable == nullptr)
    {
        _SpoofVTable();
    }

    // Enable the panel.
    I::iPanel->SetEnabled(m_pPanel->GetVPanel(), true);
    I::iPanel->SetVisible(m_pPanel->GetVPanel(), true);
    I::iPanel->SetPos(m_pPanel->GetVPanel(), 0, 0);
    I::iPanel->SetSize(m_pPanel->GetVPanel(), m_iWidth, m_iHeight);

    m_bPanelInitilized = true;
    WIN_LOG("Showcase panel initialized! @[ %p ]", m_pPanel);
    return true;
}


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

    // copy original vtable to our memory.
    memcpy(m_pSpoofedVTable, *reinterpret_cast<void**>(m_pPanel), iVTableSize);

    // Change Paint function ( via index ) with our Paint function & store the original fn adrs.
    uintptr_t pTargetFn = reinterpret_cast<uintptr_t>(m_pSpoofedVTable) + 0x410;
    m_pOriginalPaint    = *reinterpret_cast<void**>(pTargetFn); // 0x410 is pointer to fn pointer. de-ref it.

    *reinterpret_cast<uintptr_t*>(pTargetFn) = reinterpret_cast<uintptr_t>(PaintHijack);

    // Replace our panel object's VTable with the spoofed VTable ( we just created above )
    *reinterpret_cast<void**>(m_pPanel) = m_pSpoofedVTable;
}



void ModelPreview_t::Free()
{
    _FreePanel();
    _FreeVTable();
}

void ModelPreview_t::_FreePanel()
{
    if (m_pPanel == nullptr)
        return;

    free(m_pPanel);
    m_bPanelInitilized = false;
}

void ModelPreview_t::_FreeVTable()
{
    if (m_pSpoofedVTable == nullptr)
        return;

    free(m_pSpoofedVTable);
}


//=========================================================================
//                     DEBUGGING
//=========================================================================

MAKE_HOOK(CTraceEngine_GetBrushInfo, "4C 8B DC 57 41 54", __fastcall, ENGINE_DLL, bool,
    void* pThis, int iBrush, void* a1, void* a2)
{
    LOG("Calling CTraceEngine_GetBrushInfo");
    bool bResult = Hook::CTraceEngine_GetBrushInfo::O_CTraceEngine_GetBrushInfo(pThis, iBrush, a1, a2);
    LOG("Brush info : %d", bResult);

    return bResult;
}