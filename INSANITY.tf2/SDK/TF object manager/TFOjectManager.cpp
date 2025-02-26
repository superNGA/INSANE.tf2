//=========================================================================
//                      TF GAME OBJECT MANAGMENT SYSTEM
//=========================================================================
// by      : INSANE
// created : 25/02/2025
// 
// purpose : updates, manages, provides tf2 objects safely.

#include "TFOjectManager.h"
#include "../FN index Manager/FN index manager.h"

TFObjectManager_t tfObject;

//=========================================================================
//                     PUBLIC METHODS
//=========================================================================

//=========================================================================
// bool initialize()
//=========================================================================
/**
* popullates required tf2 objects
*/
bool TFObjectManager_t::initializeFns()
{
	getName			= (T_getName)g_FNindexManager.getFnAdrs(FN_GET_PANEL_NAME, iPanel);
	lookUpBones		= (T_lookUpBone)util.FindPattern("40 53 48 83 EC ? 48 8B DA E8 ? ? ? ? 48 8B C8 48 8B D3 48 83 C4 ? 5B E9 ? ? ? ? CC CC 48 89 74 24", CLIENT_DLL);
	addToLeafSystem = (T_addToLeafSystem)util.FindPattern("40 53 48 83 EC ? B8 ? ? ? ? 44 8B C2", CLIENT_DLL);
	pGlobalVar		= engineReplay->GetClientGlobalVars();

	if (getName == nullptr || lookUpBones == nullptr || addToLeafSystem == nullptr || pGlobalVar == nullptr)
	{
		ERROR("TFObjectManager", "Failed intialization");
		return false;
	}
	LOG("TFObjectManager", "successfully initiazed Fns");
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
	pGlobalVar = engineReplay->GetClientGlobalVars();
}


//=========================================================================
// void TFObjectManager_t::initializeModuleHandles()
//=========================================================================
/**
* gets and stores modules adrs
*/
//-------------------------------------------------------------------------
bool TFObjectManager_t::initializeModuleHandles()
{
	clientDll	= (uintptr_t)GetModuleHandle(CLIENT_DLL);
	engineDll	= (uintptr_t)GetModuleHandle(ENGINE_DLL);
	vguiDll		= (uintptr_t)GetModuleHandle(VGUI2_DLL);

	if (clientDll == 0 || engineDll == 0 || vguiDll == 0)
	{
		ERROR("TFObjectManager", "Failed to retrieve module handles");
		return false;
	}
	LOG("TFObjectManager", "successfully retrieved module handles");
	return true;
}


//=========================================================================
// bool TFObjectManager_t::initializeInterfaces()
//=========================================================================
/**
* captues and stores interfaces from tha mf game niggaaaaa
*/
//-------------------------------------------------------------------------
bool TFObjectManager_t::initializeInterfaces()
{
	// error codes...
	int entityList_ = 0;
	int engineClient_ = 0;
	int engineReplay_ = 0;
	int engineTrace_ = 0;
	int debugOverlay_ = 0;
	int iPanel_ = 0;
	int baseClient_ = 0;

	entityList		= (I_client_entity_list*)util.GetInterface(ICLIENTENTITYLIST, CLIENT_DLL, &entityList_);
	engine			= (IVEngineClient013*)util.GetInterface(IVENGIENCLIENT013, ENGINE_DLL, &engineClient_);
	engineReplay	= (I_engine_client_replay*)util.GetInterface(ENGINE_CLIENT_REPLAY, ENGINE_DLL, &engineReplay_);
	engineTrace		= (IEngineTrace*)util.GetInterface(IENGINETRACE, ENGINE_DLL, &engineTrace_);
	debugOverlay	= (IVDebugOverlay*)util.GetInterface(IVDEBUGOVERLAY, ENGINE_DLL, &debugOverlay_);
	baseClientDll   = (IBaseClientDLL*)util.GetInterface(BASE_CLIENT_DLL, CLIENT_DLL, &baseClient_);
	iPanel			= util.GetInterface(VGUI_PANEL, VGUI2_DLL, &iPanel_);

	// if adding new interface, ADD IT HERE :)
	if (entityList_ != 0 || engineClient_ != 0 || engineReplay_ != 0 || engineTrace_ != 0 || debugOverlay_ != 0 || iPanel_ != 0 || baseClient_ != 0)
	{
		ERROR("TFObjectManager", "Failed to capture interfaces");
		return false;
	}
	LOG("TFObjectManager", "successfully captured interfaces");
	return true;
}