#pragma once
#define _CRT_SECURE_NO_WARNINGS
#include <Windows.h>
#include <thread>

#include "../../GlobalVars.h"
#include "../EndScene/EndScene.h"

#ifdef _DEBUG
#include "../../Libraries/Console System/Console_System.h"
extern Console_System cons;
#endif // _DEBUG


namespace winproc
{
	static HWND target_window_handle = nullptr; //shall hold the handle to target windows
	static WNDPROC O_winproc = nullptr; // original WinProc address, to be used when unHooking

	bool hook_winproc();
	void unhook_winproc();

	LRESULT __stdcall H_winproc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
};