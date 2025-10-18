#include "WinProc.h"
#include "../../Features/ImGui/MenuV2/MenuV2.h"
#include "../../Features/Material Gen/MaterialGen.h"
#include "../../Features/ImGui/NotificationSystem/NotificationSystem.h" // Delete this, just for testing

// Utility
#include "../../Utility/ConsoleLogging.h"
#include "../../Utility/Hook Handler/Hook_t.h"
#include "../../Utility/Signature Handler/signatures.h"
#include "../../Utility/Profiler/Profiler.h"
#include "../../Libraries/Utility/Utility.h"

// SDK
#include "../../SDK/class/ISurface.h"
#include "../../SDK/class/IInputSystem.h"
#include "../../SDK/class/IInput.h"
#include <winuser.h>

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);


///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
bool winproc::hook_winproc() {
    //finding window by name here
    target_window_handle = FindWindow(nullptr, global::target_window_name); // Replace with your game's window title
    if (!target_window_handle)
    {
        FAIL_LOG("Failed to get window HWND");
        return false;
    }

    global::target_hwnd = target_window_handle;
    WIN_LOG("found target window");
    LOG("window handle -> 0x%p\n", target_window_handle);

    LOG("wating for win32 implimentation to be initialized");

    while (!directX::UI::WIN32_initialized)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }

    O_winproc = (WNDPROC)SetWindowLongPtr(target_window_handle, GWLP_WNDPROC, (LONG_PTR)H_winproc);

    LOG("Hooking WinProc");
    return true;
}


///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
void winproc::unhook_winproc()
{
    if (target_window_handle && O_winproc) 
    {
        SetWindowLongPtr(target_window_handle, GWLP_WNDPROC, (LONG_PTR)O_winproc);
        
        WIN_LOG("Unhooked ImGui");
        return;
    }

    FAIL_LOG("Failed UnHooking WinProc");
}


///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
LRESULT __stdcall winproc::H_winproc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) 
{
    // NOTE : gets called once for each input, not in batch.
    
    // Handle custom input
    if (uMsg == WM_KEYDOWN && wParam == VK_DELETE) // TODO : Make this changable from menu!!!
    {
        directX::UI::UI_visble = !directX::UI::UI_visble; // Toggle menu
        LOG("Set UI : [ %s ]", directX::UI::UI_visble == true ? "VISIBLE" : "NOT_VISIBLE");
        Render::notificationSystem.PushBack("Set UI : [ %s ]", directX::UI::UI_visble == true ? "VISIBLE" : "NOT_VISIBLE");
    }


    // Recording key for Key-Bind
    if (Render::menuGUI.ShouldRecordKey() == true)
    {
        if (uMsg == WM_XBUTTONDOWN)
        {
            uint64_t xButton = static_cast<uint64_t>(GET_XBUTTON_WPARAM(wParam));
            if(xButton == XBUTTON1 || xButton == XBUTTON2)
            {
                // we will be using the unofficial offset ( 0x05 & 0x06 ) for xbuttons.
                Render::menuGUI.ReturnRecordedKey(xButton + 0x04llu);
                LOG("Recorded : %X", xButton);
                return TRUE;
            }
        }
        else if (uMsg == WM_KEYDOWN || (uMsg >= WM_MOUSEFIRST && uMsg <= WM_MOUSELAST && uMsg != WM_MOUSEMOVE))
        {
            Render::menuGUI.ReturnRecordedKey(wParam); 
            LOG("Recorded : %X", wParam);
            return TRUE;
        }
    }


    // Checking if menu has been toggled
    static bool bLastMenuVisibility    = true;
    bool        bNeedMouseVisible      = directX::UI::UI_visble == true || F::materialGen.IsVisible() == true || F::profiler.IsInfoPanelVisible() == true;
    bool        bToggleMouseVisibility = (bLastMenuVisibility != bNeedMouseVisible);
    bLastMenuVisibility                = bNeedMouseVisible;
    
    // if toggled, then change mouse's visibility
    if (bToggleMouseVisibility == true)
    {
        I::iSurface->SetCursorAlwaysVisible(bNeedMouseVisible);
        I::iSurface->ApplyChanges();

        // Drawing Cursor using ImGui, 
        // cause TF2 functions will do literally anything except what they are supposed to do!
        ImGui::GetIO().MouseDrawCursor = bNeedMouseVisible;

        //LOG("Toggled mouse visibility to [ %s ]", bNeedMouseVisible ? "VISIBLE" : "NOT_VISIBLE");
    }

    // No inputs to game if menu OPEN
    if (bNeedMouseVisible == true)
    {
        ImGui_ImplWin32_WndProcHandler(hWnd, uMsg, wParam, lParam);

        // Ditching keyBoard inputs
        if (WM_KEYFIRST <= uMsg && uMsg <= WM_KEYLAST)
        {
            I::iInputSystem->ResetInputState(); // I don't know if this makes a difference or not.
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
