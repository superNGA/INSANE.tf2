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
#include "../Utility/signatures.h"
#include "../Utility/Interface.h"
#include "../Utility/Hook_t.h"
#include "../Utility/ExportFnHelper.h"
#include "../Features/features.h"

#include "../Features/ImGui/InfoWindow/InfoWindow_t.h"
#include "../SDK/class/IGameEventManager.h"
#include "../SDK/Entity Manager/entityManager.h"

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

	if (interfaceInitialize.Initialize() == false)
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

	if (allSignatures.Initialize() == false)
	{
		_terminate(instance);
	}

	if (allExportFns.Initialize() == false)
	{
		_terminate(instance);
	}

	if(hook_t.Initialize() == false)
	{
		_terminate(instance);
	}

	if (tfObject.initializeFns() == false)
	{
		_terminate(instance);
	}

	if (allFeatures.Initialize() == false)
	{
		_terminate(instance);
	}

	if (iEventListener.Initialize() == false)
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

		bool bInGame = I::iEngine->IsInGame();
		
		// Resetting features if not in game
		if(bInGame == false)
		{
			Features::critHack.Reset();
			entityManager.Reset();
		}

		Render::InfoWindow.AddToInfoWindow("connection status", std::format("{}", I::iEngine->IsConnected() ?
			( bInGame ? "Connected and in-game" : "Connected but not in-game") : "Not Connected"));
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

	/* hooking FNs */
	MH_CreateHook((LPVOID*)get_endscene(), (LPVOID)directX::H_endscene, (LPVOID*)&directX::O_endscene);

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