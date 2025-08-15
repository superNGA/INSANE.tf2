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
#include "../../SDK/class/IVModelInfo.h"
#include "../../SDK/class/CClientLeafSystem.h"

// UTILITY
#include "../../SDK/TF object manager/TFOjectManager.h"
#include "../../Extra/math.h"
#include "../../Utility/ConsoleLogging.h"
#include "../../Utility/Hook Handler/Hook_t.h"
#include "../../Utility/Interface Handler/Interface.h"
#include "../../Utility/Signature Handler/signatures.h"

// Debugging macros
#define ENABLE_DEBUGGING_HOOKS  false
#define DISABLE_ESSENTIAL_HOOKS false

// FUNCTIONS
MAKE_SIG(VGui_Panel_Constructor, "48 89 5C 24 ? 48 89 74 24 ? 57 48 83 EC ? 48 8B F2 48 8B D9 45 85 C0", CLIENT_DLL, void*, Panel*, void*, const char*) // arguments : VTable, parent panel pointer ( Get form engineVGui ), name.
MAKE_SIG(InitializeAsClientEntity, "48 89 5C 24 ? 48 89 74 24 ? 57 48 83 EC ? 41 8B F0 48 8B D9 48 85 D2", CLIENT_DLL, bool, void*, const char*, int)
MAKE_SIG(CBaseFlex_Constructor, "48 89 5C 24 ? 48 89 6C 24 ? 48 89 74 24 ? 57 41 56 41 57 48 83 EC ? 48 8B D9 E8 ? ? ? ? 33 ED", CLIENT_DLL, void*, void*)

MAKE_SIG(CClientState_SetModel, "48 89 5C 24 ? 56 48 83 EC ? 48 8B F1 48 63 DA 48 8B 89 ? ? ? ? 48 85 C9 0F 84 ? ? ? ? 81 FB ? ? ? ? 0F 87 ? ? ? ? 48 8B 01 FF 50 ? 3B D8 0F 8D ? ? ? ? 48 8B 8E ? ? ? ? 8B D3 48 89 6C 24 ? 4C 89 74 24",
    ENGINE_DLL, void, void*, int)


MAKE_INTERFACE_SIGNATURE(CGameServer, "48 8D 0D ? ? ? ? F3 0F 10 3D", void*, ENGINE_DLL, 3, 7)


#if (DISABLE_ESSENTIAL_HOOKS == false)

MAKE_HOOK(GetModel, "83 FA ? 0F 8C ? ? ? ? 48 8D 0D ? ? ? ? E9 ? ? ? ? CC CC CC CC CC CC CC CC CC CC CC 48 89 5C 24", __fastcall, ENGINE_DLL, void*, void* a1, int iModelIndex)
{
    // If in main menu, then just always return. it doesn't matter, nothing's gonna call that shit.
    if (I::iEngine->IsConnected() == false)
        return F::modelPreview.GetActiveModel();

    return Hook::GetModel::O_GetModel(a1, iModelIndex);
}
#endif


//=========================================================================
//                     PUBLIC METHODS
//=========================================================================
void ModelPreview_t::Run()
{
    if (Features::ModelPreview::ModelPreview::Enable.IsActive() == false)
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

    if (I::iEngine->IsInGame() == true && Features::ModelPreview::ModelPreview::HardRespawn.IsActive() == true)
    {
        static bool bHardRespawnDone = false;
        if (bHardRespawnDone == false)
        {
            bHardRespawnDone = true;
            Sig::InitializeAsClientEntity(m_pEnt, m_vecModels[m_iActiveModelIndex].c_str(), RENDER_GROUP_OPAQUE_ENTITY);
            WIN_LOG("Hard Respawn done");
        }
    }
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

    FAIL_LOG("removed & cleaned up model precache string tables.");
}


///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
void ModelPreview_t::SetActiveModel(int iIndex)
{
    if (m_bModelPrecached == false)
    {
        if (Features::ModelPreview::ModelPreview::Enable.IsActive() == true)
            FAIL_LOG("Models are not precached yet.");
        return;
    }

    if (iIndex == m_iActiveModelIndex)
        return;

    iIndex = std::clamp<int>(iIndex, 0, m_vecModels.size() - 1);

    m_iActiveModelIndex = iIndex;
    m_pActiveModel      = I::iModelLoader->GetModelForName(m_vecModels[iIndex].c_str(), IModelLoader::FMODELLOADER_CLIENT);

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

    // Model name set here doesn't matter, We can change it later.
    if (Sig::InitializeAsClientEntity(m_pEnt, m_vecModels[0].c_str(), RENDER_GROUP_OPAQUE_ENTITY) == false)
    {
        FAIL_LOG("Failed to register entity as non_networked entity");
        return false;
    }


    m_bEntInit = true;
    return true;
}


///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
void __fastcall PaintHijack(Panel* a1)
{
    I::iSurface->DrawSetint(255, 255, 255, 255);
    int iWidth = 0, iHeight = 0;
    F::modelPreview.GetPanelSize(iHeight, iWidth);
    I::iSurface->DrawFilledRect(0, 0, iWidth, iHeight);

    BaseEntity* pEnt = F::modelPreview.GetModelEntity();
    
    // if model entity not initialized call original & leave.
    if(pEnt == nullptr) 
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

        view.fov = 20.0f; view.m_bOrtho = false; view.zFar = 1000; view.zNear = 7;
        view.x      = 100; view.y     = 100;
        view.height = 100; view.width = 100;

        view.origin = vec(-220.0f, -4.0f, 35.0f); view.angles.Init();
        WIN_LOG("View good");
    }

    // Setting view setup pos & size.
    int iRenderViewHeight = 0, iRenderViewWidth = 0; F::modelPreview.GetRenderViewSize(iRenderViewHeight, iRenderViewWidth);
    int iRenderViewX      = 0,  iRenderViewY    = 0; F::modelPreview.GetRenderViewPos(iRenderViewX, iRenderViewY);
    
    view.x      = iRenderViewX;      view.y     = iRenderViewY;
    view.height = iRenderViewHeight; view.width = iRenderViewWidth;

    static vec white[6] =
    {
        vec(0.4, 0.4, 0.4),
        vec(0.4, 0.4, 0.4),
        vec(0.4, 0.4, 0.4),
        vec(0.4, 0.4, 0.4),
        vec(0.4, 0.4, 0.4),
        vec(0.4, 0.4, 0.4),
    };

    // Animation fix
    {
        pEnt->m_flAnimTime(pEnt->m_flAnimTime() + tfObject.pGlobalVar->frametime / 2.0f);
        pEnt->m_flCycle(pEnt->m_flCycle() + 0.01f);
        pEnt->m_nSequence(86);
    }


    // Rendering model
    {
        auto* pRenderCtx = I::iMaterialSystem->GetRenderContext();
        pRenderCtx->BindLocalCubemap(pCubeMap);
        
        // Fix lighting n shit.
        vec vLightOrigin(0.0f); (*(void(__fastcall**)(void*, vec*))(*reinterpret_cast<uintptr_t*>(pRenderCtx) + 0x4E8ull))(pRenderCtx, &vLightOrigin);
        pRenderCtx->SetAmbientLight(1.0f, 1.0f, 1.0f);
        I::iStudioRender->SetAmbientLightColors(white);
        I::iStudioRender->SetLocalLights(0, nullptr);


        Frustum dummyFrustum; 
        I::iVRenderView->Push3DView(&view, 1 | 2 | 32, NULL, dummyFrustum);
        vec vClrModulation(1.0f); I::iVRenderView->SetColorModulation(&vClrModulation);
        I::iVRenderView->SetBlend(1.0f);
        
        pRenderCtx->ClearBuffers(true, false);

        // NOTE : After somewhere the call to Draw function, the bones gets setup & positioned nicely.
        pEnt->DrawModel(STUDIO_RENDER);
        printf("Model drawing done\n");
        
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
    *reinterpret_cast<INetworkStringTable**>(reinterpret_cast<uintptr_t>(I::cClientState) + 0x08EA8LLU) = pTable;
    *reinterpret_cast<INetworkStringTable**>(reinterpret_cast<uintptr_t>(I::CGameServer) + 0x54298LLU)  = pTable;
}


///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
void ModelPreview_t::_VerifyOrCreateStringTable() const
{
    // If in game, then the table must be there.
    if (I::iEngine->IsInGame() == true)
        return;

    INetworkStringTable* pTable = I::iNetworkStringTableContainer->FindTable(MODEL_PRECACHE_TABLENAME);
    if (pTable != nullptr)
        return;

    I::iNetworkStringTableContainer->m_bAllowCreation = true;
    pTable = I::iNetworkStringTableContainer->CreateStringTableEx(MODEL_PRECACHE_TABLENAME, (1 << 12), 1, 2, false);
    I::iNetworkStringTableContainer->m_bAllowCreation = false;

    _SetTablePointer(pTable);

    if (pTable == nullptr)
        FAIL_LOG("Failed to create string table");

    WIN_LOG("Create string successfully");
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
        //Sig::CClientState_SetModel(I::cClientState, iModelIndex); // Causing crashing? IDK

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

    // copy original vtable to our memory.
    memcpy(m_pSpoofedVTable, *reinterpret_cast<void**>(m_pPanel), iVTableSize);

    // Change Paint function ( via index ) with our Paint function & store the original fn adrs.
    uintptr_t pTargetFn = reinterpret_cast<uintptr_t>(m_pSpoofedVTable) + 0x410;
    m_pOriginalPaint    = *reinterpret_cast<void**>(pTargetFn); // 0x410 is pointer to fn pointer. de-ref it.

    *reinterpret_cast<uintptr_t*>(pTargetFn) = reinterpret_cast<uintptr_t>(PaintHijack);

    // Replace our panel object's VTable with the spoofed VTable ( we just created above )
    *reinterpret_cast<void**>(m_pPanel) = m_pSpoofedVTable;
}


///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
void ModelPreview_t::Free()
{
    _FreePanel();
    _FreeVTable();
}


///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
void ModelPreview_t::_FreePanel()
{
    if (m_pPanel == nullptr)
        return;

    free(m_pPanel);
    m_bPanelInitilized = false;
}


///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
void ModelPreview_t::_FreeVTable()
{
    if (m_pSpoofedVTable == nullptr)
        return;

    free(m_pSpoofedVTable);
}


//=========================================================================
//                     DEBUGGING
//=========================================================================

#if (DISABLE_ESSENTIAL_HOOKS == false)

//MAKE_HOOK(CNetworkStringTableContainer_CreateStringTableEx, "48 89 5C 24 ? 48 89 6C 24 ? 48 89 7C 24 ? 41 56 48 83 EC ? 80 79", __fastcall, ENGINE_DLL, bool,
//    void* pThis, const char* szTableName, int nMaxEntries, int iUserDataFixedSize, int iUserDataNetworkBits, bool bIsFile)
//{
//    // Checking if this string table already exists.
//    INetworkStringTable* pModelPrecacheTable = I::iNetworkStringTableContainer->FindTable(MODEL_PRECACHE_TABLENAME);
//    if (pModelPrecacheTable != nullptr && std::string(szTableName) == std::string(MODEL_PRECACHE_TABLENAME)) // if table already exists, return the existing one.
//    {
//        WIN_LOG("Stopped engine from making \"%s\" string table again", szTableName);
//        return pModelPrecacheTable;
//    }
//
//
//    // Let engine create its tables.
//    LOG("Letting engine create network string table \"%s\"", szTableName);
//
//    return Hook::CNetworkStringTableContainer_CreateStringTableEx::O_CNetworkStringTableContainer_CreateStringTableEx(
//        pThis, szTableName, nMaxEntries, iUserDataFixedSize, iUserDataNetworkBits, bIsFile);
//}


struct LightingState_t
{
    vec         r_boxcolor[6];		// ambient, and lights that aren't in locallight[]
    int			numlights;
    void*       locallight[4];
};

MAKE_HOOK(LightcacheGetDynamic, "48 89 5C 24 ? 44 89 4C 24 ? 4C 89 44 24 ? 55 56 57 41 54 41 55 41 56 41 57 48 81 EC ? ? ? ? 48 8B 05", __fastcall, ENGINE_DLL,
    void*, void* pThis, vec* vOrigin, LightingState_t* pLightState, void* a1, int a2, bool a3)
{
    if (I::iEngine->IsConnected() == true)
        return Hook::LightcacheGetDynamic::O_LightcacheGetDynamic(pThis, vOrigin, pLightState, a1, a2, a3);


    //WIN_LOG("Returned spoofed light state for Origin { %.2f %.2f %.2f }", vOrigin->x, vOrigin->y, vOrigin->z);
    for (int i = 0; i < 6; i++)
    {
        pLightState->r_boxcolor[i] = vec(0.5f, 0.5f, 0.5f);
        
        if (i < 4)
            pLightState->locallight[i] = nullptr;
    }
    pLightState->numlights = 0;
    return nullptr;
}

#endif



#if (ENABLE_DEBUGGING_HOOKS == true)

MAKE_HOOK(CGameServer_CreateEngineStringTables, "48 89 5C 24 ? 48 89 74 24 ? 48 89 7C 24 ? 55 48 8D AC 24 ? ? ? ? 48 81 EC", __fastcall, ENGINE_DLL, void*,
    void* a1)
{
    // Before engine creates its string tables, remove all tables.
    I::iNetworkStringTableContainer->RemoveAllTables();
    F::modelPreview.SetModelPrecacheTable(nullptr);
    WIN_LOG("Cleared out table pointers from ClientClass & CGameServer objects");

    return Hook::CGameServer_CreateEngineStringTables::O_CGameServer_CreateEngineStringTables(a1);
}


// Call once when starting a server.
MAKE_HOOK(CGameServer_SpawnServer, "48 89 5C 24 ? 48 89 6C 24 ? 48 89 74 24 ? 57 41 54 41 55 41 56 41 57 48 81 EC ? ? ? ? 49 8B F1", __fastcall, ENGINE_DLL, bool,
    void* pThis, const char* szMapName, const char* szMapFile, const char* startSpot)
{
    LOG("Map Name : %s, Map File : %s, startSpot : %s", szMapName, szMapFile, startSpot);
    return Hook::CGameServer_SpawnServer::O_CGameServer_SpawnServer(pThis, szMapName, szMapFile, startSpot);
}


// Call once for each table. ( in 2nd table generation sprint )
MAKE_HOOK(CClientState_HookClientStringTable, "48 89 5C 24 ? 48 89 74 24 ? 57 48 83 EC ? 48 8B F2 48 8B D9 E8 ? ? ? ? 48 8B F8", __fastcall, ENGINE_DLL, bool,
    void* pThis, const char* szTableName)
{
    LOG("Setting Client Table \"%s\"", szTableName);
    return Hook::CClientState_HookClientStringTable::O_CClientState_HookClientStringTable(pThis, szTableName);
}


// Call once for each table. ( after the 2nd table generation sprint )
MAKE_HOOK(CClientState_InstallEngineStringTableCallback, "48 89 5C 24 ? 48 89 74 24 ? 57 48 83 EC ? 48 8B FA 48 8B F1 E8 ? ? ? ? 48 8B D8", __fastcall, ENGINE_DLL, bool,
    void* pThis, const char* szTableName)
{
    FAIL_LOG("StringTable -> \"%s\"", szTableName);
    return Hook::CClientState_InstallEngineStringTableCallback::O_CClientState_InstallEngineStringTableCallback(pThis, szTableName);
}


// Call once for each table. ( in the 2nd table generation sprint, this is the first call for each table. )
MAKE_HOOK(CBaseClientState_ProcessCreateStringTable, "48 89 5C 24 ? 48 89 4C 24 ? 55 56 57 41 54 41 55 41 56 41 57 48 83 EC ? 48 8B FA", __fastcall, ENGINE_DLL, bool,
    void* pThis, void* a1, void* a2)
{
    WIN_LOG("BaseClient creating string tables!!!");
    return Hook::CBaseClientState_ProcessCreateStringTable::O_CBaseClientState_ProcessCreateStringTable(pThis, a1, a2);
}

#endif

MAKE_HOOK(CClientLeafSystem_AddRenderable, "48 89 5C 24 ? 57 48 83 EC ? 45 33 C9", __fastcall, CLIENT_DLL, void*,
    void* pThis, void* pEnt, int iRenderGroup)
{
    WIN_LOG("Entity : %p ^^^ Render group : %d", pEnt, iRenderGroup);
    return Hook::CClientLeafSystem_AddRenderable::O_CClientLeafSystem_AddRenderable(pThis, pEnt, iRenderGroup);
}