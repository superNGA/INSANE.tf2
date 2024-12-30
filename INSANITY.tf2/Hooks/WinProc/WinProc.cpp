#include "WinProc.h"

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

bool winproc::hook_winproc() {
    //finding window by name here
    target_window_handle = FindWindow(nullptr, global::target_window_name); // Replace with your game's window title
    if (!target_window_handle)
    {
        #ifdef _DEBUG
        cons.Log("Failed to get window HWND", FG_RED);
        #endif 
        return false;
    }

    global::target_hwnd = target_window_handle;
    #ifdef _DEBUG
    cons.Log("found target window", FG_GREEN);
    printf("window handle -> 0x%p\n", target_window_handle);
    #endif

    #ifdef _DEBUG
    cons.Log("wating for win32 implimentation to be initialized", FG_YELLOW);
    #endif // _DEBUG

    while (!directX::UI::WIN32_initialized)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }

    O_winproc = (WNDPROC)SetWindowLongPtr(target_window_handle, GWLP_WNDPROC, (LONG_PTR)H_winproc);

    #ifdef _DEBUG
    cons.FastLog("Hooked WinProc");
    #endif
    return true;
}

void winproc::unhook_winproc()
{
    if (target_window_handle && O_winproc) {
        SetWindowLongPtr(target_window_handle, GWLP_WNDPROC, (LONG_PTR)O_winproc);
        #ifdef _DEBUG
        cons.FastLog("Unhooked WinProc");
        #endif
        return;
    }

    #ifdef _DEBUG
    cons.Log("Failed unhooking", FG_RED);
    #endif // _DEBUG

}

LRESULT __stdcall winproc::H_winproc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    if (!directX::UI::UI_has_been_shutdown)
    {
        if (directX::UI::UI_visble && ImGui_ImplWin32_WndProcHandler(hWnd, uMsg, wParam, lParam)) {
            return true; // Let ImGui handle the input
        }
    }
#ifdef _DEBUG
    else
    {
        cons.Log("Stoped WinProc from using ImGui after shutdown", FG_RED);
    }
#endif // _DEBUG


    // Handle custom input
    if (uMsg == WM_KEYDOWN)
    {
        if (wParam == VK_INSERT) 
        {
            #ifdef _DEBUG
            cons.FastLog("Toggled UI");
            #endif
            directX::UI::UI_visble = !directX::UI::UI_visble; // Toggle menu
        }
    }

    // Call the original WndProc
    return CallWindowProc(O_winproc, hWnd, uMsg, wParam, lParam);
}