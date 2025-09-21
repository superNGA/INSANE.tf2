#define _CRT_SECURE_NO_WARNINGS
#include <format>
#include "EndScene.h"

// SDK
#include "../../SDK/class/IVEngineClient.h"
#include "../../SDK/class/IRender.h"

#include "../../Utility/Hook Handler/Hook_t.h"

// Resource Handlers
#include "../../Resources/Fonts/FontManager.h"

#include "../../Features/Graphics Engine V2/Draw Objects/Line/Line.h"
#include "../../Features/Graphics Engine V2/Draw Objects/Box/Box.h"
#include "../../Features/Graphics Engine V2/Draw Objects/Circle/Circle.h"
#include "../../Features/Graphics Engine V2/Draw Objects/Cube/Cube.h"

// To render here.
#include "../../Features/ImGui/MenuV2/MenuV2.h"
#include "../../Features/Graphics Engine V2/Graphics.h"
#include "../../Features/Graphics Engine/Graphics Engine/GraphicsEngine.h"
#include "../../Features/ImGui/PlayerList/PlayerList.h"
#include "../../Features/ImGui/InfoWindow/InfoWindow_t.h"
#include "../../Features/ImGui/Menu/Menu.h"
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

    if (pSrcSurface == pBackBuffer)
    {
        if(g_rtCopyCallIndex >= RTCopyCallIndex_ProjectilesAndDoors && g_rtCopyCallIndex <= RTCopyCallIndex_ViewModel)
        {
            O_stretchRect(pDevice, pSrcSurface, nullptr, F::graphics.GetBlurSample(), nullptr, D3DTEXF_NONE);
        }

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


    if (I::iEngine->IsInGame() == false)
    {
        F::graphics.m_renderTargetDup0.StartCapture(pDevice);
    }

    return iResult;
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


    if(I::iEngine->IsInGame() == false)
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
    ImGui::PushFont(Resources::Fonts::JetBrains_SemiBold_NL_Small);

    // Drawing graphics features.
    {
        if (Features::MaterialGen::MaterialGen::Enable.IsActive() == false)
        {
            F::graphicsEngine.Run(pDevice);

            Render::playerList.Draw();
            Render::InfoWindow.Draw();
            insaneProfiler.Render();
        }

        Render::uiMenu.Draw();
        Render::menuGUI.Draw();
        F::materialGen.Run();
        
        // Model Rendering.
        {
            F::modelPreview.Run();

            // if mat gen is disabled, sync the model panel to our menu.
            if(Features::MaterialGen::MaterialGen::Enable.IsActive() == false)
            {
                F::modelPreview.SetActiveModel(0);
                F::modelPreview.SetVisible(UI::UI_visble);
                F::modelPreview.SetPanelClr(255, 255, 255, 255);
                F::modelPreview.SetRenderViewClr(0, 0, 0, 255);

                float flHeight = 0.0f, flWidth = 0.0f; Render::uiMenu.GetWindowSize(flHeight, flWidth);
                F::modelPreview.SetRenderViewSize(static_cast<int>(flHeight), static_cast<int>(flWidth));
                F::modelPreview.SetPanelSize(static_cast<int>(flHeight), static_cast<int>(flWidth));

                float x = 0.0f, y = 0.0f; Render::uiMenu.GetWindowPos(x, y); x += flWidth;
                F::modelPreview.SetRenderViewPos(static_cast<int>(x), static_cast<int>(y));
                F::modelPreview.SetPanelPos(static_cast<int>(x), static_cast<int>(y));
            }
            else
            {
                F::modelPreview.SetVisible(true);
            }
        }

        if (F::graphics.GetBlurTexture() != nullptr)
        {
            ImGui::Begin("Texture Preview");

            // You can also tint or add border colors:
            ImGui::Image(reinterpret_cast<ImTextureID>(F::graphics.GetBlurTexture()), ImVec2(256, 256));

            ImGui::End();
        }

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