#include "thread1.h"

#ifdef _DEBUG
	Console_System cons(FG_CYAN, BOLD, BG_BLACK);
#endif // _DEBUG

void execute_thread1(HINSTANCE instance)
{
	// Initializing MinHook and Console_System
	MH_Initialize();
#ifdef _DEBUG
	cons.CreateNewConsole();
	cons.DoIntroduction();
	cons.DoDevider();
	cons.FastLog("MinHook initialized");
#endif

	MH_CreateHook((LPVOID*)get_endscene(), (LPVOID)directX::H_endscene, (LPVOID*)&directX::O_endscene); //End scene hook
	MH_EnableHook(MH_ALL_HOOKS); //enabling all hooks
	winproc::hook_winproc(); // Hooking WinProc

	while (!GetAsyncKeyState(VK_END))
	{
		std::this_thread::sleep_for(std::chrono::milliseconds(100));
	}

	directX::UI::shutdown_UI = true;
	winproc::unhook_winproc(); // Unhooking WinProc

	//waiting until ImGui has been shutdown properly.
	while (!directX::UI::UI_has_been_shutdown)
	{
		#ifdef _DEBUG
		cons.Log("Waiting for ImGui to shutdown properly", FG_YELLOW);
		#endif // _DEBUG
		std::this_thread::sleep_for(std::chrono::milliseconds(50));
	}
	MH_DisableHook(MH_ALL_HOOKS); // disabling hooks
	MH_Uninitialize(); // Uninitializing Minhook

#ifdef _DEBUG
	cons.Log("Removed all hooks", FG_RED);
	cons.FreeConsoleInstance();
#endif

	FreeLibraryAndExitThread(instance, 0);
}