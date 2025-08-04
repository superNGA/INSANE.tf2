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
MAKE_SIG(CMDLPanel_Paint,                 "40 55 48 81 EC ? ? ? ? 48 8B E9 E8 ? ? ? ? F6 85",                                      CLIENT_DLL, void*, void*)
MAKE_SIG(SetModelAnglePos,                "48 89 5C 24 ? 48 89 74 24 ? 57 48 83 EC ? 49 8B D8 48 8B FA 48 8B F1 E8 ? ? ? ? 8B 03", CLIENT_DLL, void*, void*, qangle, vec)
MAKE_SIG(SetClr,                          "8B 02 89 81 ? ? ? ? C3 CC CC CC CC CC CC CC F3 0F 11 89",                               CLIENT_DLL, void*, void*, uint32_t*)
MAKE_SIG(CTFPlayerModelPanel_Constructor, "48 89 5C 24 ? 48 89 74 24 ? 57 48 83 EC ? 48 8B F9 45 85 C9 74 ? 48 8D 05 ? ? ? ? 48 89 41 ? 45 33 C9 E8 ? ? ? ? 48 8D 05 ? ? ? ? 48 89 07 48 8D 35 ? ? ? ? 48 8D 05", CLIENT_DLL, void*, Panel*, Panel*, const char*, int)
MAKE_SIG(CTFPlayerModelPanel_SetPlayerClass, "48 89 5C 24 ? 48 89 6C 24 ? 48 89 74 24 ? 48 89 7C 24 ? 41 56 48 83 EC ? 44 8B F2", CLIENT_DLL, void*, Panel*, int, bool, bool)
MAKE_SIG(CTFPlayerModelPanel_ClearCarriedItem, "48 89 5C 24 ? 57 48 83 EC ? 48 8B D9 E8 ? ? ? ? 48 8B 0D ? ? ? ? 4C 8D 83", CLIENT_DLL, void*, Panel*)
MAKE_SIG(CTFPlayerModelPanel_UpdatePreviewVisuals, "40 53 57 41 54 41 56 48 81 EC ? ? ? ? 45 33 E4", CLIENT_DLL, void*, Panel*)


MAKE_HOOK(ISurface_PaintTraverse, "48 89 5C 24 ? 48 89 6C 24 ? 48 89 74 24 ? 57 41 54 41 55 41 56 41 57 48 83 EC ? 0F 29 74 24", __fastcall, VGUIMATSURFACE_DLL, void*,
    void* a1, vgui::VPANEL panel, bool a3)
{
    //std::cout << "Painting panel [ " << I::iPanel->GetName(panel) << " ] \n";
    return Hook::ISurface_PaintTraverse::O_ISurface_PaintTraverse(a1, panel, a3);
}

MAKE_HOOK(IPanel_PaintTraverse, "48 89 5C 24 ? 57 48 83 EC ? 48 8B 01 41 0F B6 D9", __fastcall, VGUI2_DLL, void*,
    void* pVTable, vgui::VPANEL panel, bool bForceRepaint, bool bAllowForce)
{
    //LOG("Painting panel [ %s ]", I::iPanel->GetName(panel));
    
    
    /*if (std::string(I::iPanel->GetName(panel)) == std::string("INSANE_ShowcasePanel"))
    {
        printf("Drawing model panel\n");
    }*/


    /*if (F::modelPreview.m_bPanelInitilized == true)
        if (panel == F::modelPreview.m_pPanel->GetVPanel())
            WIN_LOG("Drawing our panel as [ %s ]", I::iPanel->GetName(panel));*/

    return Hook::IPanel_PaintTraverse::O_IPanel_PaintTraverse(pVTable, panel, bForceRepaint, bAllowForce);
}


//=========================================================================
//                     PUBLIC METHODS
//=========================================================================
void ModelPreview_t::Run()
{

    if (Features::ModelPreview::ModelPreview::Debug.IsActive() == true)
    {
        IViewPort* iViewPortMaxing = *reinterpret_cast<IViewPort**>(I::iViewPort);
        printf("Take this MF : %p\n", iViewPortMaxing);

        /*Panel* pParentPanel = I::iViewPort->FindPanelByName("scores");
        printf("pParentPanel -> [ %p ]  &&  Name [ %s ]\n", pParentPanel, I::iPanel->GetName(pParentPanel->GetVPanel()));*/
    }
    
    if (Features::ModelPreview::ModelPreview::Enable.IsActive() == false)
        return;

    if (_InitializePanel() == false)
        return;

    I::iPanel->SetTopmostPopup(m_pPanel->GetVPanel(), true);
    I::iPanel->SetPos(m_pPanel->GetVPanel(), 0, 0);
    I::iPanel->SetSize(m_pPanel->GetVPanel(), 500, 500);
}


void ModelPreview_t::Free()
{
    _FreePanel();
    _FreeVTable();
}


void __fastcall PaintHijack(void* a1)
{
    I::iSurface->DrawSetint(0, 255, 0, 255);
    I::iSurface->DrawFilledRect(0, 0, 100, 100);

    // Casting & calling original function.
    //typedef void(__fastcall* T_paint)(void* a1);
    //((T_paint)(F::modelPreview.GetOriginalPaintFn()))(a1);
}


bool ModelPreview_t::_InitializePanel()
{
    if (m_bPanelInitilized == true)
        return true;

    // Allocate our panel object some memory.
    if (m_pPanel == nullptr)
    {
        constexpr size_t iPanelSize = 0x400ULL * 6; // 6 KiBs
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
        FAIL_LOG("Bad parent panel!");
        return false;
    }
    Panel* pParentPanel = pParentVPanel->Client();
    if (pParentPanel == nullptr)
    {
        FAIL_LOG("Parent VPanel is valid but its corrosponding ClientPanel is invalid ( NULL )");
        return false;
    }


    // Run our panel object throught the constructor so VTables-n-shit gets setup properly
    Sig::CTFPlayerModelPanel_Constructor(m_pPanel, pParentPanel, "INSANE_ShowcasePanel", 1);
    // Sig::VGui_Panel_MakeReadyToUse(m_pPanel); // IDK if even does anything.

    // spoof the VTable, so we can fuck with the paint function :).
    if (m_pSpoofedVTable == nullptr)
    {
        _SpoofVTable();
    }

    printf("clearing carry item\n");
    Sig::CTFPlayerModelPanel_ClearCarriedItem(m_pPanel);
    printf("Gonna change class \n");
    Sig::CTFPlayerModelPanel_SetPlayerClass(m_pPanel, TF_SOLDIER, false, false);
    printf("Setting model team\n");
    *reinterpret_cast<int*>(reinterpret_cast<uintptr_t>(m_pPanel) + 0xF08) = 2; // Setting team
    Sig::CTFPlayerModelPanel_UpdatePreviewVisuals(m_pPanel);

    // Enable the panel. just-in-case.
    I::iPanel->SetEnabled(m_pPanel->GetVPanel(), true);
    I::iPanel->SetVisible(m_pPanel->GetVPanel(), true);

    m_bPanelInitilized = true;
    WIN_LOG("Showcase panel initialized! [ %p ]", m_pPanel);
    return true;
}


bool ModelPreview_t::_SpoofVTable()
{
    constexpr size_t iVTableSize = 257ULL * sizeof(void*);
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
    m_pOriginalPaint    = reinterpret_cast<void*>(pTargetFn);

    *reinterpret_cast<uintptr_t*>(pTargetFn) = reinterpret_cast<uintptr_t>(PaintHijack);

    // Replace our panel object's VTable with the spoofed VTable ( we just created above )
    *reinterpret_cast<void**>(m_pPanel) = m_pSpoofedVTable;
}



//=========================================================================
//                     DEBUB HOOKS
//=========================================================================


#if (ENABLE_DEBUGGING_HOOKS == true)


MAKE_HOOK(CMDLPanel_Paint, "40 55 48 81 EC ? ? ? ? 48 8B E9 E8 ? ? ? ? F6 85", __fastcall, CLIENT_DLL, void*, void* a1)
{
    // printf("panel object pointer : %p\n", a1); 
    return Hook::CMDLPanel_Paint::O_CMDLPanel_Paint(a1);
}


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