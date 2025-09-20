#define _CRT_SECURE_NO_WARNINGS
#include <format>
#include "EndScene.h"

// SDK
#include "../../SDK/class/IVEngineClient.h"
#include "../../SDK/class/IRender.h"

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
///////////////////////////////////////////////////////////////////////////
HRESULT directX::H_beginScene(LPDIRECT3DDEVICE9 pDevice, void* a1, void* a2, void* a3, void* a4)
{
    HRESULT iResult = O_BeginScene(pDevice, a1, a2, a3, a4);
    
    F::graphics.m_renderTargetDup0.StartCapture(pDevice);

    return iResult;
}


///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
HRESULT directX::H_endscene(LPDIRECT3DDEVICE9 pDevice)
{
    if (!device)
    {
        device = pDevice;
    }

    /* Doing the backend stuff */
    if (!UI::UI_initialized_DX9 || !UI::WIN32_initialized)
    {
        initialize_backends();
    }


    if (UI::UI_initialized_DX9 && UI::WIN32_initialized)
    {
        // Font init must succed
        if (Resources::Fonts::fontManager.Initialize() == false)
            return O_endscene(pDevice);
    }

    /* skipping all if already ended */
    if (UI::UI_has_been_shutdown) return O_endscene(pDevice);

    F::graphics.m_renderTargetDup0.EndCapture(pDevice);
    F::graphics.Run(pDevice);

    // ImGui drawing here.
    ImGuiIO& io = ImGui::GetIO();
    //io.DisplaySize = ImVec2(1920.0f, 1080.0f); // Replace with actual screen resolution
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

    return O_endscene(pDevice);
}
