#include "ModelPreview.h"

// SDK
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

// UTILITY
#include "../../Extra/math.h"
#include "../../Utility/ConsoleLogging.h"
#include "../../Utility/Hook Handler/Hook_t.h"
#include "../../Utility/Interface Handler/Interface.h"
#include "../../Utility/Signature Handler/signatures.h"

// Debugging macros
#define ENABLE_DEBUGGING_HOOKS false

// FUNCTIONS
MAKE_SIG(VGui_Panel_Constructor,          "48 89 5C 24 ? 48 89 74 24 ? 57 48 83 EC ? 48 8B F2 48 8B D9 45 85 C0",                  CLIENT_DLL, void*, Panel*, void*, const char*) // arguments : VTable, parent panel pointer ( Get form engineVGui ), name.
MAKE_SIG(SetModelAnglePos,                "48 89 5C 24 ? 48 89 74 24 ? 57 48 83 EC ? 49 8B D8 48 8B FA 48 8B F1 E8 ? ? ? ? 8B 03", CLIENT_DLL, void*, void*, qangle, vec)
MAKE_SIG(SetClr,                          "8B 02 89 81 ? ? ? ? C3 CC CC CC CC CC CC CC F3 0F 11 89",                               CLIENT_DLL, void*, void*, uint32_t*)
MAKE_SIG(CTFPlayerModelPanel_Constructor, "48 89 5C 24 ? 48 89 74 24 ? 57 48 83 EC ? 48 8B F9 45 85 C9 74 ? 48 8D 05 ? ? ? ? 48 89 41 ? 45 33 C9 E8 ? ? ? ? 48 8D 05 ? ? ? ? 48 89 07 48 8D 35 ? ? ? ? 48 8D 05", CLIENT_DLL, void*, Panel*, Panel*, const char*, int)
MAKE_SIG(CTFPlayerModelPanel_SetPlayerClass, "48 89 5C 24 ? 48 89 6C 24 ? 48 89 74 24 ? 48 89 7C 24 ? 41 56 48 83 EC ? 44 8B F2", CLIENT_DLL, void*, Panel*, int, bool, bool)
MAKE_SIG(CTFPlayerModelPanel_ClearCarriedItem, "48 89 5C 24 ? 57 48 83 EC ? 48 8B D9 E8 ? ? ? ? 48 8B 0D ? ? ? ? 4C 8D 83", CLIENT_DLL, void*, Panel*)
MAKE_SIG(CTFPlayerModelPanel_UpdatePreviewVisuals, "40 53 57 41 54 41 56 48 81 EC ? ? ? ? 45 33 E4", CLIENT_DLL, void*, Panel*)


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
        for(int iChildIndex = 0; iChildIndex < nChildren; iChildIndex++)
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


Panel* g_pTargetPanel = nullptr;
bool   g_bRolling     = false;

void ModelPreview_t::Run()
{
    static vgui::VPANEL classmodelpanel = NULL;

    if(classmodelpanel == NULL)
    {
        classmodelpanel = FindChildByName(I::iSurface->GetEmbeddedPanel(), "classmodelpanel", true);
        if (classmodelpanel != NULL)
        {
            g_pTargetPanel = I::iPanel->Client(classmodelpanel);
            WIN_LOG("Found Target Panel \"classmodelpanel\" @ (%p)", g_pTargetPanel);
        }
    }

    if (Features::ModelPreview::ModelPreview::Debug.IsActive() == true && classmodelpanel != NULL)
    {
        // bind to root panel.
        I::iPanel->SetParent(classmodelpanel, I::iSurface->GetEmbeddedPanel());
        
        // size & pos
        I::iPanel->SetPos(classmodelpanel, 0, 0); I::iPanel->SetSize(classmodelpanel, 1600, 900);

        // Enable
        I::iPanel->SetVisible(classmodelpanel, true); I::iPanel->SetEnabled(classmodelpanel, true);
        // NOTE : if we use it as popup, it won't even render 1 bit. I::iPanel->SetPopup(classmodelpanel, true);   I::iPanel->SetTopmostPopup(classmodelpanel, true);

        g_bRolling = true;
    }
    
    if (Features::ModelPreview::ModelPreview::Enable.IsActive() == false)
        return;

    if (_InitializePanel() == false)
        return;
}



void __fastcall PaintHijack(Panel* a1)
{
    I::iSurface->DrawSetint(255, 255, 255, 255);
    I::iSurface->DrawFilledRect(0, 0, 500, 500);

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
        constexpr size_t iPanelSize = 0x400ULL; // 1024 bytes ( It only needed 400 bytes, but its valve we are talking about. )
        m_pPanel                    = reinterpret_cast<Panel*>(malloc(iPanelSize));
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
    I::iPanel->SetSize(m_pPanel->GetVPanel(), 1600, 900);

    m_bPanelInitilized = true;
    WIN_LOG("Showcase panel initialized! @[ %p ]", m_pPanel);
    return true;
}


bool ModelPreview_t::_SpoofVTable()
{
    if (m_pSpoofedVTable != nullptr)
        return false;


    constexpr size_t iVTableSize = 213ULL * sizeof(void*); // Its 257 fn for CTFPlayerModelPanel
    m_pSpoofedVTable             = malloc(iVTableSize);
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
//                     DEBUB HOOKS
//=========================================================================

#if (ENABLE_DEBUGGING_HOOKS == true) 

// called for "classmodelpanel" even when its hidden. Nothing very useful
MAKE_HOOK(OnTick, "40 53 48 83 EC ? 83 B9 ? ? ? ? ? 48 8B D9 74 ? E8", __fastcall, CLIENT_DLL, void*, Panel* a1)
{
    return Hook::OnTick::O_OnTick(a1);
}

bool g_bCMDLDrawActive = false;
void* g_pClassLoadoutPanel = nullptr;
MAKE_HOOK(MatrixGetColoum, "4C 63 CA 42 8B 04 89", __fastcall, CLIENT_DLL, void*, void* a1, int a2, void* a3)
{
    /*if(g_bCMDLDrawActive == true)
        printf("MATRIX_GET_COLOUM()");*/

    return Hook::MatrixGetColoum::O_MatrixGetColoum(a1, a2, a3);
}

MAKE_HOOK(CMDL_Draw, "4C 8B DC 49 89 73 ? 55 57", __fastcall, CLIENT_DLL, void, short* a1, matrix3x4_t* pRootToWorld, matrix3x4_t* pBoneToWorld)
{
    g_bCMDLDrawActive = true;

    printf("Drawing MESH { %d | POS (%.2f) (%.2f) (%.2f) }", *a1, pRootToWorld->m[0][3], pRootToWorld->m[1][3], pRootToWorld->m[2][3]);

    /*printf("RootToWorld \n{%.2f %.2f %.2f %.2f}\n{%.2f %.2f %.2f %.2f}\n{%.2f %.2f %.2f %.2f}",
        pRootToWorld->m[0][0], pRootToWorld->m[0][1], pRootToWorld->m[0][2], pRootToWorld->m[0][3],
        pRootToWorld->m[1][0], pRootToWorld->m[1][1], pRootToWorld->m[1][2], pRootToWorld->m[1][3],
        pRootToWorld->m[2][0], pRootToWorld->m[2][1], pRootToWorld->m[2][2], pRootToWorld->m[2][3]);

    printf("\nBoneToWorld \n{%.2f %.2f %.2f %.2f}\n{%.2f %.2f %.2f %.2f}\n{%.2f %.2f %.2f %.2f}",
        pBoneToWorld->m[0][0], pBoneToWorld->m[0][1], pBoneToWorld->m[0][2], pBoneToWorld->m[0][3],
        pBoneToWorld->m[1][0], pBoneToWorld->m[1][1], pBoneToWorld->m[1][2], pBoneToWorld->m[1][3],
        pBoneToWorld->m[2][0], pBoneToWorld->m[2][1], pBoneToWorld->m[2][2], pBoneToWorld->m[2][3]);*/

    Hook::CMDL_Draw::O_CMDL_Draw(a1, pRootToWorld, pBoneToWorld);

    g_bCMDLDrawActive = false;
}

MAKE_HOOK(CClassLoadoutPanel_UpdateModelPanels, "40 53 48 83 EC ? 48 89 74 24 ? 48 8B D9 8B 89", __fastcall, CLIENT_DLL, void*, void* a1)
{
    static void* pThisPointerHistory = nullptr;
    if(a1 != nullptr)
    {
        LOG("Stored history");
        pThisPointerHistory = a1;
        g_pClassLoadoutPanel = a1;
    }
    else
    {
        LOG("Calling from reserver");
        a1 = pThisPointerHistory;
    }

    // Who the fuck called this shit?
    if(a1 == nullptr && pThisPointerHistory == nullptr)
    {
        FAIL_LOG("Who the fuck is calling this shit. Fuck you nigga!");
        return nullptr;
    }


    *reinterpret_cast<int*>(reinterpret_cast<uintptr_t>(a1) + 800ULL) = TF_SCOUT; // This works.
    *reinterpret_cast<int*>(reinterpret_cast<uintptr_t>(a1) + 804ULL) = 2;        // TF_RED.
    *reinterpret_cast<int*>(reinterpret_cast<uintptr_t>(a1) + 808ULL) = 0;        // Set slot. Secondary
    
    if(g_pTargetPanel != nullptr)
    {
        short iMDLHandle = *reinterpret_cast<short*>((reinterpret_cast<uintptr_t>(g_pTargetPanel) + 1456ULL));
        LOG("MDL handle : { %d }", iMDLHandle);
    }

    return Hook::CClassLoadoutPanel_UpdateModelPanels::O_CClassLoadoutPanel_UpdateModelPanels(a1);
}


//=========================================================================
//                     SET MDLs
//=========================================================================
MAKE_HOOK(CMDLPanel_SetMDL_Handle, "48 89 5C 24 ? 48 89 6C 24 ? 48 89 74 24 ? 48 89 7C 24 ? 41 56 48 83 EC ? 48 8B D9 4D 8B F0", __fastcall, CLIENT_DLL, void*,
    void* a1, __int64 a2, void* a3)
{
    LOG("Set MDL Handle { %d }", static_cast<unsigned short>(a2));
    return Hook::CMDLPanel_SetMDL_Handle::O_CMDLPanel_SetMDL_Handle(a1, a2, a3);
}

MAKE_HOOK(CMDLPanel_SetMDL_String, "48 89 5C 24 ? 48 89 6C 24 ? 48 89 74 24 ? 57 41 56 41 57 48 83 EC ? 4D 8B F0", __fastcall, CLIENT_DLL, void*,
    void* pPanel, const char* szMDLName, void* pProxyData)
{
    LOG("Set MDL { %s }", szMDLName);
    return Hook::CMDLPanel_SetMDL_String::O_CMDLPanel_SetMDL_String(pPanel, szMDLName, pProxyData);
}



MAKE_HOOK(CClassLoadoutPanel_OnCommand, "48 89 5C 24 ? 57 48 83 EC ? 48 8B DA 48 8B F9 48 8D 15 ? ? ? ? 48 3B DA 0F 84 ? ? ? ? 48 8B CB E8 ? ? ? ? 85 C0 0F 84 ? ? ? ? 48 8D 15 ? ? ? ? 48 3B DA 0F 84 ? ? ? ? 48 8B CB E8 ? ? ? ? 85 C0 0F 84 ? ? ? ? 41 B8", 
    __fastcall, CLIENT_DLL, void*, void* a1, const char* szCommand)
{
    LOG("loadout panel commanded  [ %s ]", szCommand);
    return Hook::CClassLoadoutPanel_OnCommand::O_CClassLoadoutPanel_OnCommand(a1, szCommand);
}


// Draws black background for confirmation.
MAKE_HOOK(CMDLPanel_Paint, "40 55 48 81 EC ? ? ? ? 48 8B E9 E8 ? ? ? ? F6 85", __fastcall, CLIENT_DLL, void*, Panel* a1)
{
    I::iSurface->DrawSetint(0, 0, 0, 100);
    I::iSurface->DrawFilledRect(0, 0, 1920, 1080);


    vec vMDLPos = *reinterpret_cast<vec*>(reinterpret_cast<uintptr_t>(a1) + 3660);
    printf("_POS : %.2f %.2f %.2f _", vMDLPos.x, vMDLPos.y, vMDLPos.z);
    
    Sig::SetModelAnglePos(a1, qangle(0.0f), vec(0.0f));

    // Forcefully refreshing & setting model to scout.
    if(a1->GetMDLHandle() == 0xFFFFUL && g_bRolling == true && I::iPanel->GetParent(a1->GetVPanel()) == I::iSurface->GetEmbeddedPanel())
    {
        Sig::CTFPlayerModelPanel_SetPlayerClass(a1, TF_SCOUT, true, false);
        LOG("Force refreshed model to \"SCOUT\"");
    }

    return Hook::CMDLPanel_Paint::O_CMDLPanel_Paint(a1);
}


MAKE_HOOK(CMDLPanel_OnPaint3D, "40 55 56 B8 ? ? ? ? E8 ? ? ? ? 48 2B E0 48 8D A9", __fastcall, CLIENT_DLL, void*, Panel* pPanel)
{
    CMDL* pMDL = reinterpret_cast<CMDL*>(reinterpret_cast<uintptr_t>(pPanel) + 1456ULL);
    printf("Beginning 3D paint. MDL [ %d ] { ", pMDL->m_MDLHandle);
    pMDL->m_Color = { 255, 255, 255, 255 };

    auto result = Hook::CMDLPanel_OnPaint3D::O_CMDLPanel_OnPaint3D(pPanel);
    printf(" } Ending 3D paint\n");
    return result;
}

// #if (ENABLE_DEBUGGING_HOOKS == true)

MAKE_HOOK(ISurface_PaintTraverse, "48 89 5C 24 ? 48 89 6C 24 ? 48 89 74 24 ? 57 41 54 41 55 41 56 41 57 48 83 EC ? 0F 29 74 24", __fastcall, VGUIMATSURFACE_DLL, void*,
    void* a1, vgui::VPANEL panel, bool a3)
{
    return Hook::ISurface_PaintTraverse::O_ISurface_PaintTraverse(a1, panel, a3);
}

MAKE_HOOK(IPanel_PaintTraverse, "48 89 5C 24 ? 57 48 83 EC ? 48 8B 01 41 0F B6 D9", __fastcall, VGUI2_DLL, void*,
    void* pVTable, vgui::VPANEL panel, bool bForceRepaint, bool bAllowForce)
{
    //if (std::string(I::iPanel->GetName(panel)) == std::string("classmodelpanel")) // MatSystemTopPanel -> staticPanel -> GameUI Panel -> character_info -> Sheet -> CharInfoLoadoutSubPanel -> class_loadout_panel -> classmodelpanel
    //{
    //}

    return Hook::IPanel_PaintTraverse::O_IPanel_PaintTraverse(pVTable, panel, bForceRepaint, bAllowForce);
}


MAKE_HOOK(CMatSystemSurface_Begin3DPaint, "48 89 5C 24 ? 48 89 6C 24 ? 56 57 41 54 41 56 41 57 48 83 EC ? 41 8B F1", __fastcall, VGUIMATSURFACE_DLL, void,
    void* a1, int a2, int a3, int a4, int a5, char a6)
{
    Hook::CMatSystemSurface_Begin3DPaint::O_CMatSystemSurface_Begin3DPaint(a1, a2, a3, a4, a5, a6);
}

// #if (ENABLE_DEBUGGING_HOOKS == true) 


MAKE_HOOK(Constructor_ModelPanel, "48 89 5C 24 ? 48 89 74 24 ? 57 48 83 EC ? 48 8B F9 45 85 C9 74 ? 48 8D 05 ? ? ? ? 48 89 41 ? 45 33 C9 E8 ? ? ? ? 48 8D 05 ? ? ? ? 48 89 07 48 8D 35 ? ? ? ? 48 8D 05",
    __fastcall, CLIENT_DLL, void*, __int64 a1, __int64 a2, __int64 a3, int a4)
{
    WIN_LOG("||||||||||||||||||  modelpanel is constructed  |||||||||||||||||");
    return Hook::Constructor_ModelPanel::O_Constructor_ModelPanel(a1, a2, a3, a4);
}

MAKE_HOOK(SetHeldItemTo, "41 56 41 57 48 81 EC ? ? ? ? 4C 8B F2 4C 8B F9", __fastcall, CLIENT_DLL, void*, void* a1, void* a2)
{
    LOG("Changed held item to [ %p ]", a2);
    return Hook::SetHeldItemTo::O_SetHeldItemTo(a1, a2);
}

MAKE_HOOK(SetEyeClr, "48 89 5C 24 ? 48 89 74 24 ? 57 48 83 EC ? 80 7C 24", __fastcall, CLIENT_DLL, void*,
    void* pVTable, const char* pEffectName, vec vColor1, vec vColor2, bool bForceUpdate, bool bPlaySparks)
{
    // LOG("Drawing eye effect [ %s ]", pEffectName);
    return Hook::SetEyeClr::O_SetEyeClr(pVTable, "unusual_zap_green", vec(1.0f, 0.0f, 0.0f), vec(0.0f, 0.0f, 1.0f), true, true);
}

// Doesn't get called at all in the main menu.
MAKE_HOOK(ModelImagePanel_Paint, "40 55 48 81 EC ? ? ? ? 48 8B E9 48 81 C1", __fastcall, CLIENT_DLL, void*, void* a1)
{
    printf("Model Image Panel\n");
    return Hook::ModelImagePanel_Paint::O_ModelImagePanel_Paint(a1);
}

MAKE_HOOK(test1, "48 89 4C 24 ? 55 41 56", __fastcall, CLIENT_DLL, void, void* a1)
{
    LOG("Test [ 1 ] being called");
    Hook::test1::O_test1(a1);
}

MAKE_HOOK(SetModelAnglePos, "48 89 5C 24 ? 48 89 74 24 ? 57 48 83 EC ? 49 8B D8 48 8B FA 48 8B F1 E8 ? ? ? ? 8B 03", __fastcall, CLIENT_DLL, void*,
    void* a1, qangle qAngles, vec vPos)
{
    return Hook::SetModelAnglePos::O_SetModelAnglePos(a1, qAngles, vPos);
}

#endif