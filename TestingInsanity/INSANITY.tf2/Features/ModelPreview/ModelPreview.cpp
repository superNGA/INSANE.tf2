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

// UTILITY
#include "../../SDK/TF object manager/TFOjectManager.h"
#include "../../Extra/math.h"
#include "../../Utility/ConsoleLogging.h"
#include "../../Utility/Hook Handler/Hook_t.h"
#include "../../Utility/Interface Handler/Interface.h"
#include "../../Utility/Signature Handler/signatures.h"

// Debugging macros
#define ENABLE_DEBUGGING_HOOKS false
#define DISABLE_ESSENTIAL_HOOKS false

// FUNCTIONS
MAKE_SIG(VGui_Panel_Constructor, "48 89 5C 24 ? 48 89 74 24 ? 57 48 83 EC ? 48 8B F2 48 8B D9 45 85 C0", CLIENT_DLL, void*, Panel*, void*, const char*) // arguments : VTable, parent panel pointer ( Get form engineVGui ), name.
MAKE_SIG(InitializeAsClientEntity, "48 89 5C 24 ? 48 89 74 24 ? 57 48 83 EC ? 41 8B F0 48 8B D9 48 85 D2", CLIENT_DLL, bool, void*, const char*, int)
MAKE_SIG(CBaseFlex_Constructor, "48 89 5C 24 ? 48 89 6C 24 ? 48 89 74 24 ? 57 41 56 41 57 48 83 EC ? 48 8B D9 E8 ? ? ? ? 33 ED", CLIENT_DLL, void*, void*)


/*
THEORY :
    Flow :
           Panel::PaintTraverse() --> CMDLPanel::Paint() --> PotteryWheel::Paint() -->
           IMatSystemSurface::Begin3DPaint() --> CMDLPaint::OnPaint3D() --> IMatSystemSurface::End3DPaint()

    Crash is occuring at "CBaseAnimating::SetupBones()" and its calling a fn exclusive to it & noone else calls that funciton.
    DAYEM!, now I gotta rename the entire fucking SetupBones.
*/

MAKE_INTERFACE_SIGNATURE(CGameServer, "48 8D 0D ? ? ? ? F3 0F 10 3D", void*, ENGINE_DLL, 3, 7)


static bool m_bOurEntityIncoming = false;
#if (DISABLE_ESSENTIAL_HOOKS == false)


MAKE_HOOK(GetModel, "83 FA ? 0F 8C ? ? ? ? 48 8D 0D ? ? ? ? E9 ? ? ? ? CC CC CC CC CC CC CC CC CC CC CC 48 89 5C 24", __fastcall, ENGINE_DLL, void*, void* a1, int iModelIndex)
{
    auto result = Hook::GetModel::O_GetModel(a1, iModelIndex);
    
    // If in main menu, then just always return. it doesn't matter, nothing's gonna call that shit.
    if (I::iEngine->IsConnected() == false)
        return F::modelPreview.m_pModel;

    // return result;

    //If in game, don't return our model until entity fully initialized.
    if (F::modelPreview.m_bEntInit == false || m_bOurEntityIncoming == false)
        return result;

    WIN_LOG("Returned our special model :)");
    return F::modelPreview.m_pModel;
}
#endif


//=========================================================================
//                     PUBLIC METHODS
//=========================================================================
void ModelPreview_t::Run()
{
    if (Features::ModelPreview::ModelPreview::Enable.IsActive() == false)
        return;

    if (_InitializeEntity() == false)
        return;

    if (_InitializePanel() == false)
        return;
}


inline void ConcatTransform(const matrix3x4_t& parent, const matrix3x4_t& child, matrix3x4_t& out)
{
    for (int iRow = 0; iRow < 3; iRow++)
    {
        for (int iCol = 0; iCol < 3; iCol++)
        {
            out.m[iRow][iCol] =
                parent.m[iRow][0] * child.m[0][iCol] +
                parent.m[iRow][1] * child.m[1][iCol] +
                parent.m[iRow][2] * child.m[2][iCol];
        }

        out.m[iRow][3] =
            parent.m[iRow][0] * child.m[0][3] +
            parent.m[iRow][1] * child.m[1][3] +
            parent.m[iRow][2] * child.m[2][3] +
            parent.m[iRow][3];
    }
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

        pTable->AddString(false, "models/player/spy.mdl");
        *reinterpret_cast<INetworkStringTable**>(reinterpret_cast<uintptr_t>(I::cClientState) + 0x8EA8ull)  = pTable;
        *reinterpret_cast<INetworkStringTable**>(reinterpret_cast<uintptr_t>(I::CGameServer)  + 0x54298ull) = pTable;
        WIN_LOG("Create table successfully @ { %p }", pTable);
    }


    constexpr uint64_t iEntSize = 0x400ull * 10ull;
    if (m_pEnt == nullptr)
    {
        m_pEnt = malloc(iEntSize);
        if (m_pEnt == nullptr)
        {
            FAIL_LOG("Bad malloc");
            return false;
        }
        memset(m_pEnt, 0, iEntSize);
        WIN_LOG("Allocated ourselves a nigga @ %p", m_pEnt);
    }



    if (m_pModel == nullptr)
    {
        m_pModel = I::iModelLoader->GetModelForName("models/player/spy.mdl", IModelLoader::FMODELLOADER_CLIENT);
        if (m_pModel == nullptr)
        {
            FAIL_LOG("Failed to find model");
            return false;
        }

        WIN_LOG("Found model @ %p | original model type : %d", m_pModel, reinterpret_cast<model_t*>(m_pModel)->type);
    }

    Sig::CBaseFlex_Constructor(m_pEnt);
    if (Sig::InitializeAsClientEntity(m_pEnt, "models/player/spy.mdl", RENDER_GROUP_OPAQUE_ENTITY) == false)
    {
        FAIL_LOG("Failed to create entity");
        return false;
    }


    if(false)//if (m_pBones == nullptr)
    {
        constexpr uint64_t iBoneMatrixSize = /*0x400ull * 10ull*/ sizeof(matrix3x4_t) * MAX_STUDIO_BONES;
        m_pBones = malloc(iBoneMatrixSize);
        if (m_pBones == nullptr)
        {
            FAIL_LOG("Allocation failed for bone matrix");
            return false;
        }

        memset(m_pBones, 0, iBoneMatrixSize);

        // Setting bone matrix pointer of our entity
        *reinterpret_cast<void**>(reinterpret_cast<uintptr_t>(reinterpret_cast<BaseEntity*>(m_pEnt)->GetClientRenderable()) + 2112ull) = m_pBones;
        WIN_LOG("Allocated bone matix for our nigga :)");
    }

    *reinterpret_cast<model_t**>(reinterpret_cast<uintptr_t>(m_pEnt) + 152ull)       = reinterpret_cast<model_t*>(m_pModel); // Setting model pointer
    *reinterpret_cast<unsigned short*>(reinterpret_cast<uintptr_t>(m_pEnt) + 102ull) = 1; // Setting model index.
    
    int* iSequence = reinterpret_cast<int*>(reinterpret_cast<uintptr_t>(reinterpret_cast<BaseEntity*>(m_pEnt)->GetClientRenderable()) + 2816ull);
    *iSequence = 1;

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
        pRenderCtx->SetAmbientLight(1.0f, 1.0f, 1.0f);
        I::iStudioRender->SetAmbientLightColors(white);
        I::iStudioRender->SetLocalLights(0, nullptr);


        Frustum dummyFrustum; 
        I::iVRenderView->Push3DView(&view, 1 | 2 | 32, NULL, dummyFrustum);
        vec vClrModulation(1.0f); I::iVRenderView->SetColorModulation(&vClrModulation);
        I::iVRenderView->SetBlend(1.0f);
        
        pRenderCtx->ClearBuffers(true, false);

        // NOTE : After somewhere the call to Draw function, the bones gets setup & positioned nicely.
        printf("Gonna draw\n");
        m_bOurEntityIncoming = true;
        reinterpret_cast<BaseEntity*>(F::modelPreview.m_pEnt)->DrawModel(STUDIO_RENDER);
        m_bOurEntityIncoming = false;
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

#if (DISABLE_ESSENTIAL_HOOKS == false)
MAKE_HOOK(BaseAnimating_SetupBones, "48 8B C4 44 89 40 ? 48 89 50 ? 55 53", __fastcall, CLIENT_DLL, int64_t,
    void* pEnt, void* a1, int a2, int a3, int a4)
{
    auto result = Hook::BaseAnimating_SetupBones::O_BaseAnimating_SetupBones(pEnt, a1, a2, a3, a4);
    
    if (result != 0)
        WIN_LOG("Setupbones SUCCEDDED");
    else
        FAIL_LOG("Setupbones FAILED");

    return result;
}


//MAKE_HOOK(CModelInfoClient_GetModelMaterialAndLighting, "48 8B C4 55 53 48 8D A8 ? ? ? ? 48 81 EC ? ? ? ? 8B 4A", __fastcall, ENGINE_DLL, void*,
//    void* pThis, model_t* pModel, vec* origin, qangle* angles, void* pTrace, vec* lighting, vec* matColor)
//{
//    LOG("Gettign model material n shit for model ( %s )", pModel->strName);
//    return Hook::CModelInfoClient_GetModelMaterialAndLighting::O_CModelInfoClient_GetModelMaterialAndLighting(
//        pThis, pModel, origin, angles, pTrace, lighting, matColor);
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


    WIN_LOG("Returned spoofed light state for Origin { %.2f %.2f %.2f }", vOrigin->x, vOrigin->y, vOrigin->z);
    for (int i = 0; i < 6; i++)
    {
        pLightState->r_boxcolor[i] = vec(0.5f, 0.5f, 0.5f);
        
        if (i < 4)
            pLightState->locallight[i] = nullptr;
    }
    pLightState->numlights = 0;
    return nullptr;
}


MAKE_HOOK(CModelRender_DrawModelSetup, "48 89 5C 24 ? 48 89 6C 24 ? 48 89 4C 24", __fastcall, ENGINE_DLL, bool,
    void* pThis, void* a1, void* a2, void* a3, void* a4)
{
    if(F::modelPreview.m_bEntInit == false || m_bOurEntityIncoming == false)
        return Hook::CModelRender_DrawModelSetup::O_CModelRender_DrawModelSetup(pThis, a1, a2, a3, a4);


    /*matrix3x4_t* pBoneMatrix = *reinterpret_cast<matrix3x4_t**>(reinterpret_cast<uintptr_t>(reinterpret_cast<BaseEntity*>(F::modelPreview.m_pEnt)->GetClientRenderable()) + 2112ull);
    for (int iBone = 0; iBone < 78; iBone++)
    {
        printf("# %d | %.2f %.2f %.2f\n", iBone, pBoneMatrix[iBone].m[0][3], pBoneMatrix[iBone].m[1][3], pBoneMatrix[iBone].m[2][3]);
    }*/

    bool bResult = Hook::CModelRender_DrawModelSetup::O_CModelRender_DrawModelSetup(pThis, a1, a2, a3, a4);
    
    matrix3x4_t* pBoneMatrix = reinterpret_cast<matrix3x4_t*>(a4);
    for (int iBone = 0; iBone < 78; iBone++)
    {
        printf("# %d | %.2f %.2f %.2f\n", iBone, pBoneMatrix[iBone].m[0][3], pBoneMatrix[iBone].m[1][3], pBoneMatrix[iBone].m[2][3]);
    }

    /*if(bResult == true)
        WIN_LOG("DRAW MODEL SETUP : SUCCEEDED :)");
    else
        FAIL_LOG("DRAW MODEL SETUP : failed :(");*/
    
    return bResult;
}

#endif