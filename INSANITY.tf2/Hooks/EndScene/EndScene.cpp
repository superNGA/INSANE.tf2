#include "EndScene.h"

namespace directX {
    namespace UI
    {
        bool UI_initialized_DX9 = false;
        bool shutdown_UI = false;
        bool UI_visble = true;
        bool WIN32_initialized = false;
    };
};

HRESULT directX::H_endscene(LPDIRECT3DDEVICE9 P_DEVICE)
{
	if (!device)
	{
		device = P_DEVICE;
	}

    // Initializin DX9 imgui
    if (!UI::UI_initialized_DX9) {
        ImGui::CreateContext();
        ImGui_ImplDX9_Init(device);
        ImGui_ImplDX9_CreateDeviceObjects();
        ImGuiIO& io = ImGui::GetIO();
        io.ConfigFlags = ImGuiConfigFlags_NoMouseCursorChange;
        
        UI::UI_initialized_DX9 = true;

        #ifdef _DEBUG
        cons.Log("initialized ImGui DX9", FG_GREEN);
        #endif // _DEBUG
    }

    // initializing WIN32 imgui
    if (!UI::WIN32_initialized)
    {
        if (global::target_hwnd != nullptr)
        {
            ImGui_ImplWin32_Init(global::target_hwnd);
            UI::WIN32_initialized = true;

            #ifdef _DEBUG
            cons.Log("initialized ImGui WIN32", FG_GREEN);
            #endif // _DEBUG
        }
        #ifdef _DEBUG
        else
        {
            cons.Log("No window handle yet", FG_YELLOW);
        }
        #endif // _DEBUG
    }

    //skipping rendering if menu not visible
    if (!UI::UI_visble)
    {
        return O_endscene(P_DEVICE);
    }

    ImGuiIO& io = ImGui::GetIO();
    io.DisplaySize = ImVec2(1920.0f, 1080.0f); // Replace with actual screen resolution

    // Start a new ImGui frame
    ImGui_ImplDX9_NewFrame();
    ImGui::NewFrame();

    // Render menu
    ImGui::Begin("Test Menu");
    ImGui::Text("Hello, World!");
    ImGui::End();

    // Render ImGui
    ImGui::EndFrame();
    ImGui::Render();
    ImGui_ImplDX9_RenderDrawData(ImGui::GetDrawData());

    // Shuting down ImGui backends
    if (UI::shutdown_UI)
    {
        ImGui_ImplDX9_Shutdown();
        ImGui_ImplWin32_Shutdown();
        ImGui::DestroyContext();
    }

	return O_endscene(P_DEVICE);
}

