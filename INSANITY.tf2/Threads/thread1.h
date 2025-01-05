/* NOTE : 
* This thread will manage all the hooking,
* the single console_system & Utility object is made in this file
*/

#pragma once
#define _CRT_SECURE_NO_WARNINGS
#include <Windows.h>
#include <thread>
#include <unordered_map>

#include "../Hooks/EndScene/EndScene.h" // <- this has console_system included init
#include "../Hooks/WinProc/WinProc.h"
#include "../Hooks/DirectX Hook/DirectX_hook.h"
#include "../Libraries/Utility/Utility.h"

/* game classes */
#include "../SDK/class/I_BaseEntityDLL.h"

/* offsets managment */
#include "../SDK/offsets/offsets.h"

#ifdef _DEBUG
	extern Console_System cons;
#endif // _DEBUG
	extern Utility util;

void execute_thread1(HINSTANCE instance);

bool initialize_netvars();