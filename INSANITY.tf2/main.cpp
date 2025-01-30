/*dll entry point is in this file*/
#define _CRT_SECURE_NO_WARNINGS
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <thread>

// Globals
#include "GlobalVars.h"

#ifdef _DEBUG
#include "Libraries/Console System/Console_System.h"
Console_System cons(FG_CYAN, BOLD, BG_BLACK);
#endif

//threads
#include "Threads/thread1.h"
#include "Threads/thread2.h"

BOOL WINAPI DllMain(HINSTANCE instance, DWORD call_reason, LPVOID reserved)
{
	if (call_reason == DLL_PROCESS_ATTACH)
	{
		DisableThreadLibraryCalls(instance);
		
		#ifdef _DEBUG
		cons.CreateNewConsole();
		cons.DoIntroduction();
		cons.DoDevider();
		#endif

		//creating threads
		auto thread1 = CreateThread(nullptr, 0, reinterpret_cast<LPTHREAD_START_ROUTINE>(execute_thread1), instance, 0, nullptr);
		auto thread2 = CreateThread(nullptr, 0, reinterpret_cast<LPTHREAD_START_ROUTINE>(execute_thread2), instance, 0, nullptr);

		#ifdef _DEBUG
		cons.Log(FG_GREEN, "DllMain", "All threads created Successfully");
		#endif

		//closing thread handles
		if (thread1) CloseHandle(thread1);
		if (thread2) CloseHandle(thread2);

		#ifdef _DEBUG
		cons.Log(FG_GREEN, "DllMain", "Thread Handles Destroyed");
		#endif
	}
	#ifdef _DEBUG
	cons.Log(FG_RED, "termination", "Exited DllMain");
	#endif

	return TRUE;
}