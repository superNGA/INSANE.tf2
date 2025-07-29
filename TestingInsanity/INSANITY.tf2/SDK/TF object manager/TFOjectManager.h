//=========================================================================
//                      TF GAME OBJECT MANAGMENT SYSTEM
//=========================================================================
// by      : INSANE
// created : 25/02/2025
// 
// purpose : updates, manages, provides tf2 objects safely.

#pragma once
#include <atomic>
#include <cstdint>

// SDK
#include "../../GlobalVars.h"
#include "../class/IVRenderView.h"

struct global_var_base;
enum renderGroup_t;

// Delete this
#define TICK_TO_TIME(x) (static_cast<float>(x) * tfObject.pGlobalVar->interval_per_tick)
#define TIME_TO_TICK(x) (static_cast<uint32_t>(static_cast<float>(x) / tfObject.pGlobalVar->interval_per_tick))
#define TICK_INTERVAL	(tfObject.pGlobalVar->interval_per_tick)
#define CUR_TIME		(tfObject.pGlobalVar->curtime)
#define GLOBAL_TICKCOUNT (tfObject.pGlobalVar->tickcount)

typedef int64_t(__fastcall* T_lookUpBone)(void* pEnt, const char* boneName);
typedef void(__fastcall* T_addToLeafSystem)	(void*, renderGroup_t);

class TFObjectManager_t
{
public:
	TFObjectManager_t();

    bool initializeFns();
    void update();
	
	std::atomic<bool> bIsInitialized;

//=========================================================================
//                     CALL-ABLE FUNCTIONS
//=========================================================================
	T_lookUpBone	  lookUpBones	  = nullptr;
	T_addToLeafSystem addToLeafSystem = nullptr;
	global_var_base*  pGlobalVar	  = nullptr;
};
extern TFObjectManager_t tfObject;