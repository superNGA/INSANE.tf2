/*
* This thread will manage all the hooking,
* the single console_system object is made in this file
*/

#pragma once
#define _CRT_SECURE_NO_WARNINGS
#include <Windows.h>
#include <thread>

#include "../Hooks/EndScene/EndScene.h"
#include "../Hooks/WinProc/WinProc.h"
#include "../Hooks/DirectX Hook/DirectX_hook.h"

#ifdef _DEBUG
	extern Console_System cons;
#endif // _DEBUG

void execute_thread1(HINSTANCE instance);