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
//#include "../../Hooks/ProcessMovement/ProcessMovement.h"
//#include "../../Hooks/DrawModelExecute/DrawModelExecute.h"
//#include "../class/IMaterial.h"
#include "../class/IVRenderView.h"

class IVModelInfo;
enum OverrideType_t;
struct KeyValues;
class IMaterial;
class IEngineTrace;
struct global_var_base;
enum renderGroup_t;
struct FileWeaponInfo_t;


/* TODO : 
* -> Make it cleaner and more scalable. Signature scan and call these function from their
*		respective classes only and don't write out all of these fucking bullshit in the 
*		open with no fucking encapsulation.
* -> Pro-Tip : do the latest ones first and the older ones later. and check it frequently
*		to ensure no seveare breakabe occuring form fucking nowhere.
*/


// GENERAL
typedef int64_t(__fastcall* T_lookUpBone)(void* pEnt, const char* boneName);
typedef char* (__fastcall* T_getName)(void*, int64_t);
typedef void(__fastcall* T_addToLeafSystem)	(void*, renderGroup_t);
typedef int64_t(__fastcall* T_MD5_PseudoRandom)(int);
typedef IMaterial* (__fastcall* T_findMaterial)(void*, const char*, const char*, bool, const char*);
typedef void(__fastcall* T_forcedMaterialOverride)(void*, IMaterial*, OverrideType_t);
typedef IMaterial* (__fastcall* T_createMaterial)(void*, const char*, KeyValues*);

// CHAMS
typedef KeyValues* (__fastcall* T_initKeyValue)(void*, const char*);
typedef void (__fastcall* T_KVsetInt)(KeyValues*, const char*, int64_t);
typedef void (__fastcall* T_KVSetFloat)(KeyValues*, const char*, float);
typedef void (__fastcall* T_KVSetString)(KeyValues*, const char*, const char*);
typedef void(__fastcall* T_KVSetColor)(KeyValues*, const char*, TFclr_t);

// NO SPREAD
typedef float(__fastcall* T_GetWeaponSpread)(void*);
typedef float(__fastcall* T_RandomGausianFloat)(float, float);
typedef char* (__fastcall* T_WeaponIDToAlias)(int32_t);
typedef int16_t(__fastcall* T_LookUpWeaponInfoSlot)(char*);
typedef FileWeaponInfo_t* (__fastcall* T_GetWeaponFileHandle)(int16_t);
typedef int64_t(__fastcall* T_RandomSeed)(int64_t);
typedef float(__fastcall* T_RandomFloat)(float, float);
typedef void(__fastcall* T_SendStringCommand2)(void*, const char*);

class TFObjectManager_t
{
public:
	TFObjectManager_t();

    bool initializeFns();
    void update();
	bool initializeModuleHandles();

	std::atomic<bool> bIsInitialized;

//=========================================================================
//                     CALL-ABLE FUNCTIONS
//=========================================================================
	T_lookUpBone			 lookUpBones				= nullptr;
	T_addToLeafSystem		 addToLeafSystem			= nullptr;
	T_MD5_PseudoRandom		 MD5_PseudoRandom			= nullptr;

	//delete this
	std::atomic<void*> pCBaseClientState;

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


//=========================================================================
//                     OBJECTS :)
//=========================================================================
};
extern TFObjectManager_t tfObject;