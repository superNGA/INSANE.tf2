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
	
	//enabling all hooks
	MH_EnableHook(MH_ALL_HOOKS);

	while (!GetAsyncKeyState(VK_END))
	{
		std::this_thread::sleep_for(std::chrono::seconds(1));
	}

	// Uninitializing Minhook and freeing console
	MH_Uninitialize();
#ifdef _DEBUG
	cons.Log("Removed all hooks", FG_RED);
	cons.FreeConsoleInstance();
#endif
}