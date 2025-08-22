#pragma once

#include <Windows.h>

/*this holds imformation about the target process*/
namespace global
{
	extern const char* target_window_name;
	extern const char* target_proc_name;
	extern HWND		   target_hwnd;
};