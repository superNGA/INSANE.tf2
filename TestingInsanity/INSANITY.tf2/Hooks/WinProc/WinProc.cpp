#include "WinProc.h"
#include "../../Features/ImGui/Menu/Menu.h"

// Utility
#include "../../Utility/Hook_t.h"
#include "../../Utility/signatures.h"
#include "../../Libraries/Utility/Utility.h"

// SDK
#include "../../SDK/class/ISurface.h"
#include "../../SDK/class/IInputSystem.h"
#include "../../SDK/class/IInput.h"

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

// NOTE : gets called once for each input, not in batch.
LRESULT __stdcall winproc::H_winproc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) 
{
    // Handle custom input
    if (uMsg == WM_KEYDOWN && wParam == VK_INSERT)
    {
        LOG("Toggled UI");
        directX::UI::UI_visble = !directX::UI::UI_visble; // Toggle menu
    }

    // Recording key for Key-Bind
    if (Render::uiMenu.m_bRecordingKey == true)
    {
        if (uMsg == WM_XBUTTONDOWN)
        {
            WORD xButton = GET_XBUTTON_WPARAM(wParam);
            if(xButton == XBUTTON1 || xButton == XBUTTON2)
            {
                // we will be using the unofficial offset ( 0x05 & 0x06 ) for xbuttons.
                Render::uiMenu.m_iRecordedKey  = xButton + 0x04; 
                Render::uiMenu.m_bRecordingKey = false;
            }
        }
        else if (uMsg == WM_KEYDOWN || (uMsg >= WM_MOUSEFIRST && uMsg <= WM_MOUSELAST && uMsg != WM_MOUSEMOVE))
        {
            Render::uiMenu.m_iRecordedKey  = wParam;
            Render::uiMenu.m_bRecordingKey = false;
        }
    }

    // Checking if menu has been toggled
    static bool bLastMenuVisibility    = true;
    bool        bToggleMouseVisibility = (bLastMenuVisibility != directX::UI::UI_visble);
    bLastMenuVisibility                = directX::UI::UI_visble;
    
    // if toggled, then change mouse's visibility
    if (bToggleMouseVisibility == true)
    {
        I::iSurface->SetCursorAlwaysVisible(directX::UI::UI_visble);
        I::iSurface->ApplyChanges();

        // Drawing Cursor using ImGui, 
        // cause TF2 functions will do literally anything except what they are supposed to do!
        ImGui::GetIO().MouseDrawCursor = directX::UI::UI_visble;

        //LOG("Toggled mouse visibility to [ %s ]", directX::UI::UI_visble ? "VISIBLE" : "NOT_VISIBLE");
    }

    // No inputs to game if menu OPEN
    if (directX::UI::UI_visble == true) 
    {
        ImGui_ImplWin32_WndProcHandler(hWnd, uMsg, wParam, lParam);

        // Ditching keyBoard inputs
        if (WM_KEYFIRST <= uMsg && uMsg <= WM_KEYLAST)
        {
            I::iInputSystem->ResetInputState();
            return TRUE;
        }

        // Ditching mouse inputs
        if (WM_MOUSEFIRST <= uMsg && uMsg <= WM_MOUSELAST)
            return TRUE;
    }

    // Call the original WndProc
    return CallWindowProc(O_winproc, hWnd, uMsg, wParam, lParam);
}

// LockCursor @ 62th in ISurface interface.