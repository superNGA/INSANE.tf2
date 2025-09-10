#define _CRT_SECURE_NO_WARNINGS
#include <format>
#include "EndScene.h"

// Resource Handlers
#include "../../Resources/Fonts/FontManager.h"

#include "../../Features/Graphics Engine V2/Draw Objects/Line/Line.h"

// To render here.
#include "../../Features/Graphics Engine V2/Graphics.h"
#include "../../Features/Graphics Engine/Graphics Engine/GraphicsEngine.h"
#include "../../Features/ImGui/PlayerList/PlayerList.h"
#include "../../Features/ImGui/InfoWindow/InfoWindow_t.h"
#include "../../Features/ImGui/Menu/Menu.h"
#include "../../Features/ModelPreview/ModelPreview.h"
#include "../../Features/Material Gen/MaterialGen.h"
#include "../../Utility/Insane Profiler/InsaneProfiler.h"


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
HRESULT directX::H_endscene(LPDIRECT3DDEVICE9 P_DEVICE)
{
	if (!device)
	{
		device = P_DEVICE;
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
            return O_endscene(P_DEVICE);
    }

    /* skipping all if already ended */
    if (UI::UI_has_been_shutdown) return O_endscene(P_DEVICE);

    /* Starting ImGui new frame*/
    ImGuiIO& io = ImGui::GetIO();
    io.DisplaySize = ImVec2(1920.0f, 1080.0f); // Replace with actual screen resolution
    ImGui_ImplWin32_NewFrame();
    ImGui_ImplDX9_NewFrame();
    ImGui::NewFrame();

    // Just set one font decent font for now.
    ImGui::PushFont(Resources::Fonts::JetBrains_SemiBold_NL_Small);

    // Delete this
    {
        static Line_t* pLine = nullptr;
        if (pLine == nullptr)
        {
            pLine = new Line_t();
            pLine->SetAs2DObject(true);
            pLine->SetPoints(vec(0.0f, 0.0f, 0.0f), vec(500.0f, 500.0f, 0.0f));
            pLine->SetColor(255, 255, 255, 255);
            pLine->SetBlur(1);
        }
    }

    // Drawing graphics features.
    {
        if (Features::MaterialGen::MaterialGen::Enable.IsActive() == false)
        {
            F::graphicsEngine.Run(P_DEVICE);
            F::graphics.Run(P_DEVICE);

            Render::playerList.Draw();
            Render::InfoWindow.Draw();
            insaneProfiler.Render();
        }

        Render::uiMenu.Draw();
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

    /* Frame end */
    ImGui::EndFrame();
    ImGui::Render();

    ImGui_ImplDX9_RenderDrawData(ImGui::GetDrawData());

    if (UI::UI_visble)
    {
        /* managing shutdone once the animation is done */
        if (UI::shutdown_UI && !UI::UI_has_been_shutdown)
        {
            shutdown_imgui();
        }
    }

    /* calling original function */
	auto result = O_endscene(P_DEVICE);

    return result;
}