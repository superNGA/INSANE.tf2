/*dll entry point is in this file*/
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <thread>

//threads
#include "Threads/thread1.h"

BOOL WINAPI DllMain(HINSTANCE instance, DWORD call_reason, LPVOID reserved)
{
	if (call_reason == DLL_PROCESS_ATTACH)
	{
		DisableThreadLibraryCalls(instance);
		
		//creating threads
		auto thread1 = CreateThread(nullptr, 0, reinterpret_cast<LPTHREAD_START_ROUTINE>(execute_thread1), instance, 0, nullptr);

		//closing thread handles
		if (thread1) CloseHandle(thread1);
	}
}