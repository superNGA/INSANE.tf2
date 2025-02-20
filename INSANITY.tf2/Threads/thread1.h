/* NOTE : 
* This thread will manage all the hooking,
* the single Utility object is made in this file
* also hold the single netvars object. Dont make another one :)
*/

#pragma once
#define _CRT_SECURE_NO_WARNINGS
#include <Windows.h>
#include <thread>
#include <mutex>
#include <unordered_map>

/* global data */
#include "../GlobalVars.h"

/* hooks */
#include "../Hooks/Createmove/CreateMove.h"
#include "../Hooks/FrameStageNotify/FrameStageNotify.h"
#include "../Hooks/RenderGlowEffect/renderGlowEffect.h"
#include "../Hooks/TraceRay/TraceRay.h"
#include "../Hooks/OverrideView/overrideView.h"
#include "../Hooks/PaintTraverse/PaintTraverse.h"

#include "../Hooks/EndScene/EndScene.h" // <- this has console_system included init
#include "../Hooks/WinProc/WinProc.h"
#include "../Hooks/DirectX Hook/DirectX_hook.h"
#include "../Libraries/Utility/Utility.h"

/* game classes */
#include "../SDK/class/I_BaseEntityDLL.h"

/* Runtime FN index finding mechanism */
#include "../SDK/FN index manager.h"

/* NetVar managment */
#include "../SDK/offsets/offsets.h"
extern local_netvars netvar; // <- this holds all the netvars

#ifdef _DEBUG
	extern Console_System cons;
#endif // _DEBUG

extern Utility util;

void execute_thread1(HINSTANCE instance);

namespace fn_runtime_adrs
{
	extern uintptr_t fn_createmove;
	extern uintptr_t fn_frame_stage_notify;
	extern uintptr_t fn_renderGlowEffect;
	extern uintptr_t fn_traceRay;
	extern uintptr_t fn_overrideView;
};