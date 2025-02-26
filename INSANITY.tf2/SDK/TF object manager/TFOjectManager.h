//=========================================================================
//                      TF GAME OBJECT MANAGMENT SYSTEM
//=========================================================================
// by      : INSANE
// created : 25/02/2025
// 
// purpose : updates, manages, provides tf2 objects safely.

#pragma once
#include <cstdint>
#include "../../GlobalVars.h"

class IEngineTrace;
struct global_var_base;
enum renderGroup_t;

typedef int64_t(__fastcall* T_lookUpBone)(void* pEnt, const char* boneName);
typedef char* (__fastcall* T_getName)(void*, int64_t);
typedef void(__fastcall* T_addToLeafSystem)	(void*, renderGroup_t);

class TFObjectManager_t
{
public:
    bool initializeFns();
    void update();
	bool initializeModuleHandles();
	bool initializeInterfaces();

//=========================================================================
//                     CALL-ABLE FUNCTIONS
//=========================================================================
	T_lookUpBone			lookUpBones	= nullptr;
	T_getName				getName = nullptr;
	T_addToLeafSystem		addToLeafSystem	= nullptr;
	global_var_base*		pGlobalVar = nullptr;

//=========================================================================
//                     MODULE ADRS
//=========================================================================
	uintptr_t				clientDll = 0;
	uintptr_t				engineDll = 0;
	uintptr_t				vguiDll	= 0;

//=========================================================================
//                     INTERFACES
//=========================================================================
	I_client_entity_list*	entityList	= nullptr;
	IBaseClientDLL*			baseClientDll = nullptr;
	IVEngineClient013*		engine	= nullptr;
	I_engine_client_replay* engineReplay = nullptr;
	IEngineTrace*			engineTrace = nullptr;
	IVDebugOverlay*			debugOverlay = nullptr;
	void*					iPanel = nullptr;
};
extern TFObjectManager_t tfObject;