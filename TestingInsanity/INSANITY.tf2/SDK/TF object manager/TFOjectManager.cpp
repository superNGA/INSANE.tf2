//=========================================================================
//                      TF GAME OBJECT MANAGMENT SYSTEM
//=========================================================================
// by      : INSANE
// created : 25/02/2025
// 
// purpose : updates, manages, provides tf2 objects safely.

#include "TFOjectManager.h"
#include "../FN index Manager/FN index manager.h"
#include "../../Utility/ConsoleLogging.h"

TFObjectManager_t tfObject;

//=========================================================================
//                     PUBLIC METHODS
//=========================================================================


//=========================================================================
// TFObjectManager_t::TFObjectManager_t()
//=========================================================================
/**
* CONSTRUCTOR : Fills in the default values for atomic variables
*/
//-------------------------------------------------------------------------
TFObjectManager_t::TFObjectManager_t()
{
	bIsInitialized.store(false);
}


//=========================================================================
// bool initialize()
//=========================================================================
/**
* popullates required tf2 objects
*/
bool TFObjectManager_t::initializeFns()
{
	lookUpBones				= (T_lookUpBone)util.FindPattern("40 53 48 83 EC ? 48 8B DA E8 ? ? ? ? 48 8B C8 48 8B D3 48 83 C4 ? 5B E9 ? ? ? ? CC CC 48 89 74 24", CLIENT_DLL);
	addToLeafSystem			= (T_addToLeafSystem)util.FindPattern("40 53 48 83 EC ? B8 ? ? ? ? 44 8B C2", CLIENT_DLL);
	
	pGlobalVar				= I::iEngineClientReplay->GetClientGlobalVars();

	if (lookUpBones == nullptr || addToLeafSystem == nullptr || pGlobalVar == nullptr)
	{
		FAIL_LOG("Failed intialization Fns");
		return false;
	}
	WIN_LOG("TFObjectManager", "successfully initiazed Fns");
	return true;
}


//=========================================================================
// void update()
//=========================================================================
/**
* update tf2 objects & pointers, to be called @ a leisurely pace
*/
void TFObjectManager_t::update()
{
	pGlobalVar = I::iEngineClientReplay->GetClientGlobalVars();
	bIsInitialized.store(true);
}