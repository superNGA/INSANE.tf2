#pragma once
#define	_CRT_SECURE_NO_WARNINGS

#include <Windows.h>
#include <thread>

//DirectX sdk libraries
#include <d3d9.h>
#include <d3dx9.h>
#pragma comment(lib, "d3d9.lib")
#pragma comment(lib, "d3dx9.lib")

//Minhook
#include "../../External Libraries/MinHook/MinHook.h"

//colour full console
#ifdef _DEBUG
#include "../../Libraries/Console System/Console_System.h"
#endif

//global variables
extern int WINDOW_HEIGHT, WINDOW_WIDTH;
extern HWND WINDOW;
extern LPDIRECT3DDEVICE9 DEVICE;

/*returns endscene address.*/
void* get_endscene();

/*Gets the Directx devices virtual table*/
bool GET_DIRECTX_DEVICES_VTABLE(void** P_TABLE, size_t SIZE);

/*Gets the handle to the window or some shit, IDK*/
HWND GET_PROCESS_WINDOW();

/*This gets called by the EnumWindows and checks the window*/
BOOL CALLBACK CHECK_WINDOW(HWND CURRENT_WINDOW, LPARAM LP);
