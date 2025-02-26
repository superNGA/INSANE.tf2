//=========================================================================
//                      THREAD 1
//=========================================================================
// by      : INSANE
// created : 26/02/2025
// 
// purpose : responsible for initializing, updating, and proper shutdown
//           of the software.
//-------------------------------------------------------------------------

#pragma once
#define _CRT_SECURE_NO_WARNINGS
#include <Windows.h>
#include <thread>
#include <mutex>
#include <unordered_map>

// global data
#include "../GlobalVars.h"

//======================= GAME HOOKS =======================
#include "../Hooks/Createmove/CreateMove.h"
#include "../Hooks/FrameStageNotify/FrameStageNotify.h"
#include "../Hooks/RenderGlowEffect/renderGlowEffect.h"
#include "../Hooks/TraceRay/TraceRay.h"
#include "../Hooks/OverrideView/overrideView.h"
#include "../Hooks/PaintTraverse/PaintTraverse.h"
#include "../Hooks/ShouldDrawViewModel/ShouldDrawViewModel.h"

//======================= UI / RENDERING HOOKS =======================
#include "../Hooks/EndScene/EndScene.h" // <- this has console_system included init
#include "../Hooks/WinProc/WinProc.h"
#include "../Hooks/DirectX Hook/DirectX_hook.h"
#include "../Libraries/Utility/Utility.h"

/* game classes */
#include "../SDK/class/I_BaseEntityDLL.h"

//======================= HELPER SYSTEMS =======================
#include "../SDK/TF object manager/TFOjectManager.h"
#include "../SDK/FN index Manager/FN index manager.h"
#include "../SDK/offsets/offsets.h"

extern local_netvars netvar; // <- this holds all the netvars
extern Utility util;
#ifdef _DEBUG
extern Console_System cons;
#endif

class thread1_t
{
public:
	void execute_thread1(HINSTANCE instance);

private:
	bool _initializeNetvars();
	bool _initializeHooks();
	void _terminate(HINSTANCE instance);

	// if hooks are not enabled then can exit the software
	// without any problems, just exit the thread.
	bool _hooksEnabled = false;
};
inline thread1_t thread1;