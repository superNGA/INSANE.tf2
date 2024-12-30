#include "EndScene.h"

namespace directX {
    namespace UI
    {
        bool UI_initialized_DX9 = false;
        bool shutdown_UI = false;
        bool UI_has_been_shutdown = false;
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
    static ImGuiContext* context;
    if (!UI::UI_initialized_DX9)
    {
        context = ImGui::CreateContext();
        ImGui::SetCurrentContext(context);

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
    if (!UI::UI_visble || UI::UI_has_been_shutdown)
    {
        return O_endscene(P_DEVICE);
    }

    ImGuiIO& io = ImGui::GetIO();
    io.DisplaySize = ImVec2(1920.0f, 1080.0f); // Replace with actual screen resolution
    ImGui_ImplDX9_NewFrame();
    ImGui::NewFrame();

    ImGui::SetNextWindowSize(ImVec2(700, 400));
    ImGui::Begin("INSANITY", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse);
    ImGui::Text("Hello, World!");
    ImGui::End();

    ImGui::EndFrame();
    ImGui::Render();
    ImGui_ImplDX9_RenderDrawData(ImGui::GetDrawData());

    // Shuting down ImGui backends
    if (UI::shutdown_UI && !UI::UI_has_been_shutdown)
    {

        #ifdef _DEBUG
        if (ImGui::GetCurrentContext() != context)
        {
            cons.Log("[ Error ] current context is null before destroying it", FG_RED);  
        }
        #endif
        if(UI::UI_initialized_DX9) ImGui_ImplDX9_Shutdown();
        if(UI::WIN32_initialized) ImGui_ImplWin32_Shutdown();
        ImGui::DestroyContext();
        UI::shutdown_UI = false;
        UI::UI_has_been_shutdown = true;
        
        #ifdef _DEBUG
        cons.Log("ImGui has been shutdown", FG_RED);
        #endif // _DEBUG

    }

	return O_endscene(P_DEVICE);
}
