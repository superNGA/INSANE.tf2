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

// UTILITY
#include "../Utility/Signature Handler/signatures.h"
#include "../Utility/Interface Handler/Interface.h"
#include "../Utility/Hook Handler/Hook_t.h"
#include "../Utility/Export Fn Handler/ExportFnHelper.h"
#include "../Utility/PullFromAssembly.h"
#include "../Utility/Insane Profiler/InsaneProfiler.h"
#include "../Utility/ClassIDHandler/ClassIDHandler.h"
#include "../SDK/NetVars/NetVarHandler.h"
#include "../Features/FeatureHandler.h"
#include "../Features/ImGui/InfoWindow/InfoWindow_t.h"

// SDK
#include "../SDK/class/IGameEventManager.h"
#include "../SDK/class/ISurface.h"
#include "../SDK/Entity Manager/entityManager.h"

// FEATURES
#include "../Features/Aimbot/Aimbot Melee/AimbotMelee.h"
#include "../Features/Aimbot/Aimbot Projectile/AimbotProjectile.h"
#include "../Features/Movement/Movement.h"
#include "../Features/Projectile Engine/ProjectileEngine.h"
#include "../Features/Graphics Engine/Graphics Engine/GraphicsEngine.h"
#include "../Features/MovementSimulation/MovementSimulation.h"

Utility util;

//=========================================================================
//                     PUBLIC METHODS
//=========================================================================


//=========================================================================
// void thread1_t::execute_thread1(HINST
// ANCE instance)
//=========================================================================
/**
* initialized software and runs main cheat loop
*
* @param instance : basic DLL stuff
*/
//-------------------------------------------------------------------------
void thread1_t::execute_thread1(HINSTANCE instance)
{
	if (allASMData.Initialize() == false)
	{
		_terminate(instance);
	}

	if (interfaceInitialize.Initialize() == false)
	{
		_terminate(instance);
	}

	if (netVarHandler.Initialize() == false)
	{
		_terminate(instance);
	}

	if (allSignatures.Initialize() == false)
	{
		_terminate(instance);
	}

	if (_initializeHooks() == false)
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

	if (featureHandler.Initialize() == false)
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
			F::critHack.Reset();
			F::aimbotMelee.Reset();
			F::aimbotProjectile.Reset();
			F::aimbotProjectile.DeleteProjLUT(); // TODO : Maybe move it to _terminate() ?
			F::movement.Reset();
			F::projectileEngine.Reset();
			F::movementSimulation.ClearStrafeData();
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
	MH_CreateHook((LPVOID*)GetBeginScene(), (LPVOID)directX::H_BeginScene, (LPVOID*)&directX::O_BeginScene);

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
		FAIL_LOG("thread 1", "terminating without hooking (early exit) something went wrong");
		UNINITIALIZE_CONSOLE();
		FreeLibraryAndExitThread(instance, 0);
		F::graphicsEngine.FreeAllDrawObjs();
		return;
	}
	// handling exit if hooks are intialized
	else
	{
		// exiting UI...
		directX::UI::shutdown_UI = true;
		while (!directX::UI::UI_has_been_shutdown)
		{
			LOG("Waiting for UI to shutdown");
			std::this_thread::sleep_for(std::chrono::milliseconds(50));
		}
	
		// Deleting all dynamically allocated visual objects...
		F::graphicsEngine.FreeAllDrawObjs();

		// unhooking...
		MH_DisableHook(MH_ALL_HOOKS);
		MH_Uninitialize();

		// Resetting mouse, if in game else it 
		// completely breaks the mouse & won't 
		// let the user use the mouse at all
		if(I::iEngine->IsInGame() == true)
		{
			I::iSurface->SetCursorAlwaysVisible(false);
			I::iSurface->ApplyChanges();
		}

		chams.FreeAllMaterial();

		// freeing terminal..
		LOG("thread 1", "uninitiazed everything, teminated software gracefully");
		UNINITIALIZE_CONSOLE();
		FreeLibraryAndExitThread(instance, 0);
		return;
	}
}