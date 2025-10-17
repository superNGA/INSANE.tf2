/*dll entry point is in this file*/
#include <cassert>
#define _CRT_SECURE_NO_WARNINGS
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <thread>

// Globals
#include "GlobalVars.h"

#include "Utility/ConsoleLogging.h"

//threads
#include "Threads/thread1.h"

void runCheat(HINSTANCE instance)
{
	thread1.execute_thread1(instance);
}

BOOL WINAPI DllMain(HINSTANCE instance, DWORD call_reason, void* reserved)
{
	if (call_reason == DLL_PROCESS_ATTACH)
	{
		DisableThreadLibraryCalls(instance);
		
		INITIALIZE_CONSOLE();

		//creating threads
		auto pThread1 = CreateThread(nullptr, 0, reinterpret_cast<LPTHREAD_START_ROUTINE>(runCheat), instance, 0, nullptr);

		LOG("DllMain", "All threads created Successfully");
		//LOG("DllMain", "Loaded @ %p", reserved);

		//closing thread handles
		if (pThread1) CloseHandle(pThread1);

		LOG("DllMain", "Thread Handles Destroyed");
	}

	LOG("termination", "Exited DllMain");
	return TRUE;
}
