#include "thread1.h"

Utility util;

void execute_thread1(HINSTANCE instance)
{
	/* Initializing MinHook and Console_System& utility */
	MH_Initialize();
	#ifdef _DEBUG
	cons.Log(FG_GREEN, "MINHOOK", "MINHOOK initialized");
	#endif


	/* initializing module handles */
	if (!handle::initialize())
	{
		#ifdef _DEBUG
		cons.Log(FG_RED, "ERROR", "Failed to get module handle for one or more modules");
		#endif
	}
	#ifdef _DEBUG
	cons.Log(FG_GREEN, "THREAD 1", "Successfully intialized module handles");
	#endif


	/* intializing netvars */
	if (!offsets::netvar_initialized && !offsets::initialize())
	{
		#ifdef _DEBUG
		cons.Log(FG_RED, "ERROR", "Failed to intialize netvars");
		#endif
	}
	#ifdef _DEBUG
	cons.Log(FG_GREEN, "NETVARs", "Initialized NetVars");
	#endif


	/* getting interfaces */
	int entity_list_code, ivengineclient_code, iengineclientreplay_code;
	interface_tf2::entity_list		= (I_client_entity_list*)util.GetInterface(ICLIENTENTITYLIST, CLIENT_DLL, &entity_list_code);
	interface_tf2::engine			= (IVEngineClient013*)util.GetInterface(IVENGIENCLIENT013, ENGINE_DLL, &ivengineclient_code);
	interface_tf2::engine_replay	= (I_engine_client_replay*)util.GetInterface(ENGINE_CLIENT_REPLAY, ENGINE_DLL, &iengineclientreplay_code);

	#ifdef _DEBUG
	entity_list_code			? cons.Log(FG_RED, "ERROR","Failed to get IClientEntityList")		: cons.Log(FG_GREEN, "INTERFACE","Successfully retrived IClientEntityList");
	ivengineclient_code			? cons.Log(FG_RED, "ERROR","Failed to get IVEngineClient014")		: cons.Log(FG_GREEN, "INTERFACE","Successfully retrived IVEngineClient014");
	iengineclientreplay_code	? cons.Log(FG_RED, "ERROR","Failed to get EngineClientReplay001")	: cons.Log(FG_GREEN, "INTERFACE","Successfully retrived EngineClientReplay001");
	#endif

	/* getting fn runtime adrs */
	fn_runtime_adrs::fn_createmove			= util.FindPattern("40 53 48 83 EC ? 0F 29 74 24 ? 49 8B D8", CLIENT_DLL);
	fn_runtime_adrs::fn_renderGlowEffect	= util.FindPattern("48 89 6C 24 ? 48 89 74 24 ? 57 48 83 EC ? 48 8B E9 41 8B F8 48 8B 0D", CLIENT_DLL);
	fn_runtime_adrs::fn_frame_stage_notify	= (uintptr_t)util.GetVirtualTable((void*)interface_tf2::base_client)[35];
	
	/* storing manually calling functions */
	TF2_functions::lookUpBone = (TF2_functions::T_lookUpBone)util.FindPattern("40 53 48 83 EC ? 48 8B DA E8 ? ? ? ? 48 8B C8 48 8B D3 48 83 C4 ? 5B E9 ? ? ? ? CC CC 48 89 74 24", CLIENT_DLL);

	/* hooking FNs */
	MH_CreateHook((LPVOID*)get_endscene(),							(LPVOID)directX::H_endscene,								(LPVOID*)&directX::O_endscene); //End scene hook
	MH_CreateHook((LPVOID)fn_runtime_adrs::fn_createmove,			(LPVOID)hook::createmove::hooked_createmove,				(LPVOID*)&hook::createmove::original_createmove);
	MH_CreateHook((LPVOID)fn_runtime_adrs::fn_renderGlowEffect,		(LPVOID)hook::renderGlowEffect::H_renderGlowEffect,			(LPVOID*)&hook::renderGlowEffect::O_renderGlowEffect);
	/* hooking FNs by index */
	MH_CreateHook((LPVOID)fn_runtime_adrs::fn_frame_stage_notify,	(LPVOID)hook::frame_stage_notify::hook_frame_stage_notify,	(LPVOID*)&hook::frame_stage_notify::original_frame_stage_notify);

	/* Enabling all hooks */
	MH_EnableHook(MH_ALL_HOOKS);
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
		cons.Log(FG_YELLOW, "THREAD 1", "Waiting for ImGui to ShutDown properly");
		#endif
		std::this_thread::sleep_for(std::chrono::milliseconds(50));
	}
	MH_DisableHook(MH_ALL_HOOKS); // disabling hooks
	MH_Uninitialize(); // Uninitializing Minhook

	#ifdef _DEBUG
	cons.Log(FG_GREEN,"termination", "Removed all hooks");
	#endif

	while (!thread_termination_status::thread2) {
		#ifdef _DEBUG
		cons.Log(FG_YELLOW, "THREAD 1", "waiting for thread 2 to exit");
		#endif
		std::this_thread::sleep_for(std::chrono::milliseconds(500)); 
	}

	#ifdef _DEBUG
	cons.Log(FG_GREEN, "termination", "THREAD 1");
	cons.Log(FG_GREEN, "termination", "FREE'ed Library");
	cons.FreeConsoleInstance();
	#endif

	thread_termination_status::thread1 = true;
	FreeLibraryAndExitThread(instance, 0);
	return;
}

namespace fn_runtime_adrs
{
	uintptr_t fn_createmove			= 0;
	uintptr_t fn_frame_stage_notify = 0;
	uintptr_t fn_renderGlowEffect	= 0;
};