//=========================================================================
//                      THREAD 1
//=========================================================================
// by      : INSANE
// created : 26/02/2025
// 
// purpose : responsible for initializing, updating, and proper shutdown
//           of the software.
//-------------------------------------------------------------------------

#include "thread1.h"
Utility util;

//=========================================================================
//                     PUBLIC METHODS
//=========================================================================


//=========================================================================
// void thread1_t::execute_thread1(HINSTANCE instance)
//=========================================================================
/**
* initialized software and runs main cheat loop
*
* @param instance : basic DLL stuff
*/
//-------------------------------------------------------------------------
void thread1_t::execute_thread1(HINSTANCE instance)
{
	if (tfObject.initializeModuleHandles() == false)
	{
		_terminate(instance);
	}

	if (tfObject.initializeInterfaces() == false)
	{
		_terminate(instance);
	}

	if(_initializeNetvars() == false) // requies interfaces to be initialized
	{
		_terminate(instance);
	}	
	
	if (_initializeHooks() == false)
	{
		_terminate(instance);
	}

	if (tfObject.initializeFns() == false)
	{
		_terminate(instance);
	}

	//=======================MAIN CHEAT LOOP=======================
	LOG("thread 1", "initialized thread 1");
	while (!directX::UI::UI_has_been_shutdown)
	{
		thread_termination_status::thread1_primed = true;
		tfObject.update();
		std::this_thread::sleep_for(std::chrono::milliseconds(100));
	}

	_terminate(instance);
}


//=========================================================================
//                     PRIVATE METHODS
//=========================================================================


//=========================================================================
// bool thread1_t::_initializeNetvars()
//=========================================================================
/**
* intializes netvars safely
*/
//-------------------------------------------------------------------------
bool thread1_t::_initializeNetvars()
{
	/* intializing netvars */
	if (offsets::netvar_initialized == false && offsets::initialize() == false)
	{
		ERROR("NETVAR", "failed to intialize netvars");
		return false;
	}
	LOG("NETVAR", "succesfully initialized netvars");
	return true;
}


//=========================================================================
// bool thread1_t::_initializeHooks()
//=========================================================================
/**
* signature scan & hook all required fns
*/
//-------------------------------------------------------------------------
bool thread1_t::_initializeHooks()
{
	MH_Initialize();
	LOG("thread 1", "Minhook started");

	// signature scanning...
	uintptr_t pCreateMove_				= util.FindPattern("40 53 48 83 EC ? 0F 29 74 24 ? 49 8B D8", CLIENT_DLL);
	uintptr_t pRenderGlowEffect_		= util.FindPattern("48 89 6C 24 ? 48 89 74 24 ? 57 48 83 EC ? 48 8B E9 41 8B F8 48 8B 0D", CLIENT_DLL);
	uintptr_t pOverrideView_			= util.FindPattern("48 89 5C 24 ? 55 48 8D 6C 24 ? 48 81 EC ? ? ? ? 48 8B DA", CLIENT_DLL);
	uintptr_t pShouldDrawViewModel_		= util.FindPattern("48 83 EC ? E8 ? ? ? ? 48 85 C0 74 ? 48 8D 88 ? ? ? ? BA ? ? ? ? E8", CLIENT_DLL);
	uintptr_t pDrawModelExecute_		= util.FindPattern("4C 89 4C 24 ? 48 89 4C 24 ? 55 53 56 57 41 54", ENGINE_DLL);
	uintptr_t pFrameStageNotify_		= g_FNindexManager.getFnAdrs(FN_FRAME_STAGE_NOTIFY, (void*)tfObject.baseClientDll);
	uintptr_t pProcessMovement_			= (uintptr_t)(util.GetVirtualTable(tfObject.iGameMovement)[1]); // its the first one nigga
	//uintptr_t pSvPure_					= util.FindPattern("48 89 5C 24 ? 48 89 74 24 ? 48 89 7C 24 ? 55 48 8D AC 24 ? ? ? ? 48 81 EC", ENGINE_DLL);
	uintptr_t pSvPure_					= util.FindPattern("40 56 48 83 EC ? 83 3D ? ? ? ? ? 48 8B F1 0F 8E", ENGINE_DLL);

	if (pCreateMove_ == 0 || pRenderGlowEffect_ == 0 || pOverrideView_ == 0 || 
		pShouldDrawViewModel_ == 0 || pFrameStageNotify_ == 0 || pProcessMovement_ == 0 || 
		pDrawModelExecute_ == 0 || pSvPure_ == 0)
	{
		ERROR("thread 1", "Failed signature scanning");
		return false;
	}

	/* hooking FNs */
	MH_CreateHook((LPVOID*)get_endscene(),		(LPVOID)directX::H_endscene,								(LPVOID*)&directX::O_endscene);
	MH_CreateHook((LPVOID)pCreateMove_,			(LPVOID)hook::createmove::hooked_createmove,				(LPVOID*)&hook::createmove::original_createmove);
	MH_CreateHook((LPVOID)pRenderGlowEffect_,	(LPVOID)hook::renderGlowEffect::H_renderGlowEffect,			(LPVOID*)&hook::renderGlowEffect::O_renderGlowEffect); 
	MH_CreateHook((LPVOID)pOverrideView_,		(LPVOID)hook::overrideView::H_overrideView,					(LPVOID*)&hook::overrideView::O_overrideView); // override view from IClientMode, game place as Createmove
	MH_CreateHook((LPVOID)pDrawModelExecute_,	(LPVOID)hook::DME::H_DME,									(LPVOID*)&hook::DME::O_DME);
	MH_CreateHook((LPVOID)pSvPure_,				(LPVOID)hook::sv_pure::H_svPure,							(LPVOID*)&hook::sv_pure::O_svPure);

	/* hooking FNs by index */
	MH_CreateHook((LPVOID)pFrameStageNotify_,	(LPVOID)hook::frame_stage_notify::hook_frame_stage_notify,	(LPVOID*)&hook::frame_stage_notify::original_frame_stage_notify);
	MH_CreateHook((LPVOID)g_FNindexManager.getFnAdrs(FN_PAINT_TRAVERSE, tfObject.iPanel), (LPVOID)hook::paintTraverse::H_paintTraverse, (LPVOID*)&hook::paintTraverse::O_paintTraverse);

	MH_EnableHook(MH_ALL_HOOKS);
	winproc::hook_winproc();
	LOG("thread 1", "Enabled all hooks succesfully");
	_hooksEnabled = true;

	return true;
}


//=========================================================================
// void thread1_t::_terminate()
//=========================================================================
/**
* unhooks & uninitiazed everything and terminates the cheat.
*/
//-------------------------------------------------------------------------
void thread1_t::_terminate(HINSTANCE instance)
{
	// if hooks are not enabled then endscene/UI is not a concern
	// just exit thread and free library
	if (_hooksEnabled == false) 
	{
		ERROR("thread 1", "terminating without hooking (early exit) something went wrong");
		#ifdef _DEBUG
		cons.FreeConsoleInstance();
		#endif
		FreeLibraryAndExitThread(instance, 0);
		return;
	}
	// handling exit if hooks are intialized
	else
	{
		// exiting UI...
		directX::UI::shutdown_UI = true;
		while (!directX::UI::UI_has_been_shutdown)
		{
			WAIT_MSG("DIRECTX / UI", "exit");
			std::this_thread::sleep_for(std::chrono::milliseconds(50));
		}
		
		// unhooking...
		MH_DisableHook(MH_ALL_HOOKS);
		MH_Uninitialize();

		chams.FreeAllMaterial();

		// freeing terminal..
		LOG("thread 1", "uninitiazed everything, teminated software gracefully");
		#ifdef _DEBUG
		cons.FreeConsoleInstance();
		#endif
		FreeLibraryAndExitThread(instance, 0);
		return;
	}
}