#include <ppltasks.h>
#define _CRT_SECURE_NO_WARNINGS
#include <format>
#include "EndScene.h"
#include <cwchar>

// SDK
#include "../../SDK/class/IVEngineClient.h"
#include "../../SDK/class/IRender.h"
#include "../../SDK/class/ISurface.h"
#include "../../SDK/class/IPanel.h"

#include "../../Utility/Hook Handler/Hook_t.h"

// Resource Handlers
#include "../../Resources/Fonts/FontManager.h"

#include "../../Features/Graphics Engine V2/Draw Objects/Line/Line.h"
#include "../../Features/Graphics Engine V2/Draw Objects/Box/Box.h"
#include "../../Features/Graphics Engine V2/Draw Objects/Circle/Circle.h"
#include "../../Features/Graphics Engine V2/Draw Objects/Cube/Cube.h"

// To render here.
#include "../../Features/ImGui/KeybindPanel/KeybindPanel.h"
#include "../../Features/ImGui/NotificationSystem/NotificationSystem.h"
#include "../../Features/ImGui/PlayerList/PlayerListV2.h"
#include "../../Features/ImGui/MenuV2/MenuV2.h"
#include "../../Features/Graphics Engine V2/Graphics.h"
#include "../../Features/Graphics Engine/Graphics Engine/GraphicsEngine.h"
#include "../../Features/ImGui/InfoWindow/InfoWindow_t.h"
#include "../../Features/ModelPreview/ModelPreview.h"
#include "../../Features/Material Gen/MaterialGen.h"
#include "../../Utility/Insane Profiler/InsaneProfiler.h"
#include "../../Features/ESP/ESPV2.h"


namespace directX {
    namespace UI
    {
        bool UI_initialized_DX9   = false;
        bool shutdown_UI          = false;
        bool UI_has_been_shutdown = false;
        bool UI_visble            = true;
        bool WIN32_initialized    = false;
    };

    ImGuiIO*      IO      = nullptr;
    ImGuiContext* context = nullptr;

};


///////////////////////////////////////////////////////////////////////////

// Render Target copy call purpous. 
// ( 3 Copies from back buffer to some other place ( maybe some offscreen Render Target ) is done.
enum RTCopyCallIndex_t : int
{
    RTCopyCallIndex_BaseWorld           = 0,
    RTCopyCallIndex_ProjectilesAndDoors = 1,
    RTCopyCallIndex_ViewModel           = 2 // world, doors & projectiles and view model are all drawn till this call. so we can copy here.
};
int g_rtCopyCallIndex = RTCopyCallIndex_BaseWorld;
///////////////////////////////////////////////////////////////////////////



///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
HRESULT directX::H_StretchRect(LPDIRECT3DDEVICE9 pDevice, IDirect3DSurface9* pSrcSurface, void* pSrcRect, IDirect3DSurface9* pDestSurface, void* pDestRect, int StretchRectType)
{
    D3DSurface* pBackBuffer = nullptr;
    pDevice->GetBackBuffer(0, 0, D3DBACKBUFFER_TYPE_MONO, &pBackBuffer);

    // Only copy if somethings being written from the back buffer. 
    if (pSrcSurface == pBackBuffer)
    {
        if(g_rtCopyCallIndex >= RTCopyCallIndex_ProjectilesAndDoors && g_rtCopyCallIndex <= RTCopyCallIndex_ViewModel)
        {
            O_stretchRect(pDevice, pSrcSurface, nullptr, F::graphics.GetBlurSample(), nullptr, D3DTEXF_NONE);
        }

        // NOTE : This gets resetted to 0 after each frame at the end of the EndScene hook.
        g_rtCopyCallIndex++;
    }

    pBackBuffer->Release();
    return O_stretchRect(pDevice, pSrcSurface, pSrcRect, pDestSurface, pDestRect, StretchRectType);
}


///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
HRESULT directX::H_present(LPDIRECT3DDEVICE9 pDevice, void* a1, void* a2, void* a3, void* a4)
{
    HRESULT iResult = O_present(pDevice, a1, a2, a3, a4);


    if (I::iEngine->IsInGame() == false && Render::menuGUI.IsVisible() == true)
    {
        F::graphics.m_renderTargetDup0.StartCapture(pDevice);
    }

    return iResult;
}


///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
static void HandleModelPreviewPanel()
{
    // don't mess if matGen is active.
    if(F::materialGen.IsVisible() == true)
    {
        F::modelPreview.SetVisible(true);
        return;
    }
    
    F::modelPreview.SetVisible(Render::menuGUI.IsVisible());

    if(F::modelPreview.IsVisible() == true)
    {
        F::modelPreview.SetActiveModel(0);
        F::modelPreview.SetPanelClr(Render::menuGUI.GetPrimaryClr());
        F::modelPreview.SetRenderViewClr(Render::menuGUI.GetPrimaryClr());
      
        // Model preview panel's size.
        float flHeight = 0.0f, flMenuWidth = 0.0f; Render::menuGUI.GetSize(flMenuWidth, flHeight); 
        const int iModelPreviewWidth = F::modelPreview.GetDefaultWidth();
        F::modelPreview.SetRenderViewSize(static_cast<int>(flHeight), iModelPreviewWidth);
        F::modelPreview.SetPanelSize     (static_cast<int>(flHeight), iModelPreviewWidth);
     
        // Model preview panel's pos.
        float x = 0.0f, y = 0.0f; Render::menuGUI.GetPos(x, y); x += (flMenuWidth + MENU_PADDING_IN_PXL);
        F::modelPreview.SetRenderViewPos(static_cast<int>(x), static_cast<int>(y));
        F::modelPreview.SetPanelPos     (static_cast<int>(x), static_cast<int>(y));
     
        F::modelPreview.DrawOverlay(Features::Menu::Menu::Rounding.GetData().m_flVal);
    }
}


///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
static void HandleDevelopersConsole()
{
    // if we our stuff is not open, then let developers console open.
    if(Render::menuGUI.IsVisible() == false && F::materialGen.IsVisible() == false)
        return;

    static vgui::VPANEL hGameConsole = NULL;
    if (hGameConsole == NULL)
    {
        hGameConsole = I::iPanel->FindChildByName(I::iSurface->GetEmbeddedPanel(), "GameConsole", true);

        if (hGameConsole == NULL)
        {
            FAIL_LOG("Failed to find console panel");
            return;
        }
        
        WIN_LOG("Found panel \"GameConsole\" @ ID : %llu", hGameConsole);
    }

    // Is console open ?
    if (I::iPanel->IsVisible(hGameConsole) == true)
    {
        I::iPanel->SetVisible(hGameConsole, false);
        LOG("Stopped GameConsole from opening");
    }
}


///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
HRESULT directX::H_endscene(LPDIRECT3DDEVICE9 pDevice)
{
    if (device == nullptr)
        device = pDevice;

    if (UI::UI_initialized_DX9 == false || UI::WIN32_initialized == false)
    {
        initialize_backends();
    }

    if (UI::UI_initialized_DX9 == true && UI::WIN32_initialized == true)
    {
        if (Resources::Fonts::fontManager.Initialize() == false)
            return O_endscene(pDevice);
    }

    if (UI::UI_has_been_shutdown) 
    {
        return O_endscene(pDevice);
    }


    if(I::iEngine->IsInGame() == false && Render::menuGUI.IsVisible() == true)
    {
        F::graphics.m_renderTargetDup0.EndCapture(pDevice);
    }
    F::graphics.Run(pDevice);

    // ImGui drawing here.
    ImGuiIO& io = ImGui::GetIO();
    ImGui_ImplWin32_NewFrame();
    ImGui_ImplDX9_NewFrame();
    ImGui::NewFrame();

    // Just set one decent font for now.
    ImGui::PushFont(Resources::Fonts::JetBrainsMonoNerd_Small);  
   
    // Drawing graphics features.
    {
        if (Features::MaterialGen::MaterialGen::Enable.IsActive() == false)
        {
            F::graphicsEngine.Run(pDevice);

            Render::InfoWindow.Draw();
            insaneProfiler.Render();
        }

        //Render::uiMenu.Draw();
        F::materialGen.Run();
        Render::notificationSystem.Draw();
        Render::KeybindPanel.Draw(); // Note : Keybind panel is given more priority than notifications, to prevent it from hidding shit in intense moments?
        Render::playerListV2.SetVisible(UI::UI_visble); Render::playerListV2.Draw();
        Render::menuGUI.SetVisible(UI::UI_visble);      Render::menuGUI.Draw();
        
        // Model Rendering.
        F::modelPreview.Run();
        HandleModelPreviewPanel();

        // Don't let InGame console come in the way.
        HandleDevelopersConsole();
    }

    ImGui::PopFont();

    // Ending ImGui drawing.
    ImGui::EndFrame();
    ImGui::Render();

    // Real drawing & pushing to vertex buffer is done here.
    ImGui_ImplDX9_RenderDrawData(ImGui::GetDrawData());

    if (UI::UI_visble)
    {
        if (UI::shutdown_UI && !UI::UI_has_been_shutdown)
        {
            shutdown_imgui();
        }
    }

    g_rtCopyCallIndex = 0;
    return O_endscene(pDevice);
}
