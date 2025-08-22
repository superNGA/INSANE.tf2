#pragma once
#include "EndScene.h"
#include "../WinProc/WinProc.h"
#include "../../Utility/ConsoleLogging.h"


void directX::initialize_backends()
{
    // Initializin DX9 imgui
    if (!UI::UI_initialized_DX9)
    {
        context = ImGui::CreateContext();
        ImGui::SetCurrentContext(context);

        ImGui_ImplDX9_Init(device);
        ImGui_ImplDX9_CreateDeviceObjects();
        ImGuiIO& io = ImGui::GetIO();
        io.ConfigFlags = ImGuiConfigFlags_NoMouseCursorChange;

        UI::UI_initialized_DX9 = true;

        LOG("initialized ImGui DX9");
    }

    // initializing WIN32 imgui
    if (!UI::WIN32_initialized)
    {
        if (global::target_hwnd != nullptr)
        {
            ImGui_ImplWin32_Init(global::target_hwnd);
            UI::WIN32_initialized = true;

            WIN_LOG("initialized ImGui WIN32");
        }
        else
            FAIL_LOG("No window handle yet"); 
    }

}


void directX::shutdown_imgui()
{
    // Shuting down ImGui backends
    if (ImGui::GetCurrentContext() != context)
    {
        FAIL_LOG("current context is null before destroying it");
    }

    /*Shuting down ImGui backends*/
    if (UI::UI_initialized_DX9) ImGui_ImplDX9_Shutdown();
    if (UI::WIN32_initialized) ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();

    /*Unhooking winProc*/
    winproc::unhook_winproc();

    UI::shutdown_UI = false;
    UI::UI_has_been_shutdown = true;
    WIN_LOG("ImGui has been shutdown");
}