#include "thread1.h"

#ifdef _DEBUG
	Console_System cons(FG_CYAN, BOLD, BG_BLACK);
#endif // _DEBUG
Utility util;

void execute_thread1(HINSTANCE instance)
{
	/* Initializing MinHook and Console_System& utility */
	MH_Initialize();
	#ifdef _DEBUG
	cons.CreateNewConsole();
	cons.DoIntroduction();
	cons.DoDevider();
	cons.FastLog("MinHook initialized");
	#endif

	/* initializing module handles */
	if (!handle::initialize())
	{
		#ifdef _DEBUG
		cons.Log("[ error ] Failed to get moudle handles for one or all modules", FG_RED);
		#endif
	}
	#ifdef _DEBUG
	cons.Log("Successfully initialized module handles", FG_GREEN);
	#endif

	/* intializing netvars */
	if (!offsets::netvar_initialized && !offsets::initialize())
	{
		#ifdef _DEBUG
		cons.Log("Failed to intialize netvars", FG_RED);
		#endif
	}
	#ifdef _DEBUG
	cons.Log("Initialize netvars", FG_GREEN);
	#endif

	/* hooking and enabling hooks */
	MH_CreateHook((LPVOID*)get_endscene(), (LPVOID)directX::H_endscene, (LPVOID*)&directX::O_endscene); //End scene hook
	MH_EnableHook(MH_ALL_HOOKS); //enabling all hooks
	winproc::hook_winproc(); // Hooking WinProc

	/* main cheat loop */
	while (!directX::UI::UI_has_been_shutdown)
	{
		std::this_thread::sleep_for(std::chrono::milliseconds(100));
	}

	//waiting until ImGui has been shutdown properly, in case of some errors
	while (!directX::UI::UI_has_been_shutdown)
	{
		#ifdef _DEBUG
		cons.Log("Waiting for ImGui to shutdown properly", FG_YELLOW);
		#endif
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