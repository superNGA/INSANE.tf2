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
#include "../../GlobalVars.h"
#include "../../Hooks/ProcessMovement/ProcessMovement.h"
#include "../../Hooks/DrawModelExecute/DrawModelExecute.h"
//#include "../class/IMaterial.h"
#include "../class/IVRenderView.h"

class IVModelInfo;
enum OverrideType_t;
struct KeyValues;
class IMaterial;
class IEngineTrace;
struct global_var_base;
enum renderGroup_t;

typedef int64_t(__fastcall* T_lookUpBone)(void* pEnt, const char* boneName);
typedef char* (__fastcall* T_getName)(void*, int64_t);
typedef void(__fastcall* T_addToLeafSystem)	(void*, renderGroup_t);
typedef int64_t(__fastcall* T_MD5_PseudoRandom)(int);
typedef IMaterial* (__fastcall* T_findMaterial)(void*, const char*, const char*, bool, const char*);
typedef void(__fastcall* T_forcedMaterialOverride)(void*, IMaterial*, OverrideType_t);
typedef IMaterial* (__fastcall* T_createMaterial)(void*, const char*, KeyValues*);

typedef KeyValues* (__fastcall* T_initKeyValue)(void*, const char*);
typedef void (__fastcall* T_KVsetInt)(KeyValues*, const char*, int64_t);
typedef void (__fastcall* T_KVSetFloat)(KeyValues*, const char*, float);
typedef void (__fastcall* T_KVSetString)(KeyValues*, const char*, const char*);

class TFObjectManager_t
{
public:
	TFObjectManager_t();

    bool initializeFns();
    void update();
	bool initializeModuleHandles();
	bool initializeInterfaces();

	std::atomic<bool> bIsInitialized;

//=========================================================================
//                     CALL-ABLE FUNCTIONS
//=========================================================================
	T_lookUpBone			 lookUpBones				= nullptr;
	T_getName				 getName					= nullptr;
	T_addToLeafSystem		 addToLeafSystem			= nullptr;
	T_MD5_PseudoRandom		 MD5_PseudoRandom			= nullptr;
	T_findMaterial			 FindMaterial				= nullptr;
	T_forcedMaterialOverride pForcedMaterialOverride	= nullptr;
	T_createMaterial		 pCreateMaterial			= nullptr;
	
	T_initKeyValue			 pInitKeyValue				= nullptr;
	T_KVsetInt				 pKVSetInt					= nullptr;
	T_KVSetFloat			 pKVSetFloat				= nullptr;
	T_KVSetString			 pKVSetString				= nullptr;

	global_var_base*		pGlobalVar					= nullptr;

//=========================================================================
//                     MODULE ADRS
//=========================================================================
	uintptr_t				clientDll		= 0;
	uintptr_t				engineDll		= 0;
	uintptr_t				vguiDll			= 0;

//=========================================================================
//                     INTERFACES
//=========================================================================
	I_client_entity_list*	entityList		= nullptr;
	IBaseClientDLL*			baseClientDll	= nullptr;
	IVEngineClient013*		engine			= nullptr;
	I_engine_client_replay* engineReplay	= nullptr;
	IEngineTrace*			engineTrace		= nullptr;
	IVDebugOverlay*			debugOverlay	= nullptr;
	void*					iPanel			= nullptr;
	void*					iGameMovement	= nullptr;
	void*					IMaterialSystem = nullptr;
	void*					IVRenderModel	= nullptr;
	void*					IStudioRender	= nullptr;
	IVRenderView*			iVRenderView	= nullptr;
	IVModelInfo*			iVModelInfo		= nullptr;

//=========================================================================
//                     OBJECTS :)
//=========================================================================
	std::atomic<CMoveData*> pMove; // <- this is not maintained, nullptr currently
};
extern TFObjectManager_t tfObject;