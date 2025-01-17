#include "thread1.h"

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


	/* getting interfaces */
	int entity_list_code, ivengineclient_code, iengineclientreplay_code;
	interface_tf2::entity_list		= (I_client_entity_list*)util.GetInterface(ICLIENTENTITYLIST, CLIENT_DLL, &entity_list_code);
	interface_tf2::engine			= (IVEngineClient013*)util.GetInterface(IVENGIENCLIENT013, ENGINE_DLL, &ivengineclient_code);
	interface_tf2::engine_replay	= (I_engine_client_replay*)util.GetInterface(ENGINE_CLIENT_REPLAY, ENGINE_DLL, &iengineclientreplay_code);

	#ifdef _DEBUG
	entity_list_code			? cons.Log("Failed to get IClientEntityList", FG_RED)		: cons.Log("Successfully retrived IClientEntityList", FG_GREEN);
	ivengineclient_code			? cons.Log("Failed to get IVEngineClient014", FG_RED)		: cons.Log("Successfully retrived IVEngineClient014", FG_GREEN);
	iengineclientreplay_code	? cons.Log("Failed to get EngineClientReplay001", FG_RED)	: cons.Log("Successfully retrived EngineClientReplay001", FG_GREEN);
	#endif

	/* signature scanning */
	fn_runtime_adrs::fn_createmove = util.FindPattern("40 53 48 83 EC ? 0F 29 74 24 ? 49 8B D8", CLIENT_DLL);

	/* hooking and enabling hooks */
	MH_CreateHook((LPVOID*)get_endscene(), (LPVOID)directX::H_endscene, (LPVOID*)&directX::O_endscene); //End scene hook
	MH_CreateHook((LPVOID)fn_runtime_adrs::fn_createmove, (LPVOID)hook::createmove::hooked_createmove, (LPVOID*)&hook::createmove::original_createmove);

	MH_EnableHook(MH_ALL_HOOKS); //enabling all hooks
	winproc::hook_winproc(); // Hooking WinProc

	/* MAIN CHEAT LOOP, end this and whole cheat goes kaput! */
	while (!directX::UI::UI_has_been_shutdown)
	{
		thread_termination_status::thread1_primed = true;
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

	ExitThread(0);
	thread_termination_status::thread1 = true;
	return;
}

namespace fn_runtime_adrs
{
	uintptr_t fn_createmove = 0;
};