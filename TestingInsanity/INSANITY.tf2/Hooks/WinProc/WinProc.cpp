#include "WinProc.h"
#include "../../Features/ImGui/Menu/Menu.h"

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
    cons.Log(FG_RED, "initiatlizing", "Hooking WinProc");
    #endif
    return true;
}

void winproc::unhook_winproc()
{
    if (target_window_handle && O_winproc) {
        SetWindowLongPtr(target_window_handle, GWLP_WNDPROC, (LONG_PTR)O_winproc);
        #ifdef _DEBUG
        cons.Log(FG_GREEN, "termination", "Unhooked ImGui");
        #endif
        return;
    }

    #ifdef _DEBUG
    cons.Log(FG_RED, "ERROR", "Failed UnHooking WinProc");
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
        cons.Log(FG_YELLOW, "WARNING", "Stoped WinProc from using ImGui after shutdown");
    }
    #endif

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

    // Recording key for Key-Bind
    if (Render::uiMenu.m_bRecordingKey == true)
    {
        if (uMsg == WM_KEYDOWN)
        {
            Render::uiMenu.m_iRecordedKey  = wParam;
            Render::uiMenu.m_bRecordingKey = false;
        }
    }

    // Blocking mouse input
    if (directX::UI::UI_visble) {
        switch (uMsg) {
        case WM_MOUSEMOVE:
        case WM_LBUTTONDOWN:
        case WM_RBUTTONDOWN:
        case WM_LBUTTONUP:
        case WM_RBUTTONUP:
        case WM_MOUSEWHEEL:
            return false;
        }
    }

    // Call the original WndProc
    return CallWindowProc(O_winproc, hWnd, uMsg, wParam, lParam);
}