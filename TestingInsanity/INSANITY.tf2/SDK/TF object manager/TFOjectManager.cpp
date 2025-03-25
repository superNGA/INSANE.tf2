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
// TFObjectManager_t::TFObjectManager_t()
//=========================================================================
/**
* CONSTRUCTOR : Fills in the default values for atomic variables
*/
//-------------------------------------------------------------------------
TFObjectManager_t::TFObjectManager_t()
{
	bIsInitialized.store(false);
	pMove.store(nullptr);
}


//=========================================================================
// bool initialize()
//=========================================================================
/**
* popullates required tf2 objects
*/
bool TFObjectManager_t::initializeFns()
{
	getName					= (T_getName)g_FNindexManager.getFnAdrs(FN_GET_PANEL_NAME, iPanel);
	lookUpBones				= (T_lookUpBone)util.FindPattern("40 53 48 83 EC ? 48 8B DA E8 ? ? ? ? 48 8B C8 48 8B D3 48 83 C4 ? 5B E9 ? ? ? ? CC CC 48 89 74 24", CLIENT_DLL);
	addToLeafSystem			= (T_addToLeafSystem)util.FindPattern("40 53 48 83 EC ? B8 ? ? ? ? 44 8B C2", CLIENT_DLL);
	MD5_PseudoRandom		= (T_MD5_PseudoRandom)util.FindPattern("89 4C 24 ? 55 48 8B EC 48 81 EC", CLIENT_DLL);
	FindMaterial			= (T_findMaterial)g_FNindexManager.getFnAdrs(FN_FIND_MATERIAL, IMaterialSystem);
	pForcedMaterialOverride = (T_forcedMaterialOverride)util.FindPattern("48 89 91 ? ? ? ? 44 89 81", STUDIORENDER_DLL); // <- this one is from IStudioRender interface
	pCreateMaterial			= (T_createMaterial)util.FindPattern("48 89 5C 24 ? 57 48 83 EC ? 48 8B C2", MATERIALSYSTEM_DLL);
	pGlobalVar				= engineReplay->GetClientGlobalVars();
	//pForcedMaterialOverride = (T_forcedMaterialOverride)util.FindPattern("4C 8B DC 49 89 5B ? 49 89 6B ? 49 89 73 ? 57 48 83 EC ? 48 8B 1D", ENGINE_DLL); // this is from IVModelRender or someshit like that

	if (getName == nullptr || lookUpBones == nullptr || addToLeafSystem == nullptr || pGlobalVar == nullptr || 
		MD5_PseudoRandom == nullptr || pForcedMaterialOverride == nullptr || pCreateMaterial == nullptr)
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
	bIsInitialized.store(true);
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
	int entityList_		 = 0;
	int engineClient_	 = 0;
	int engineReplay_	 = 0;
	int engineTrace_	 = 0;
	int debugOverlay_	 = 0;
	int iPanel_			 = 0;
	int baseClient_		 = 0;
	int gameMovement_	 = 0;
	int iMaterialSystem_ = 0;
	int ivRenderModel_	 = 0;
	int iStudioRender_	 = 0;

	entityList		= (I_client_entity_list*)util.GetInterface(ICLIENTENTITYLIST, CLIENT_DLL, &entityList_);
	engine			= (IVEngineClient013*)util.GetInterface(IVENGIENCLIENT013, ENGINE_DLL, &engineClient_);
	engineReplay	= (I_engine_client_replay*)util.GetInterface(ENGINE_CLIENT_REPLAY, ENGINE_DLL, &engineReplay_);
	engineTrace		= (IEngineTrace*)util.GetInterface(IENGINETRACE, ENGINE_DLL, &engineTrace_);
	debugOverlay	= (IVDebugOverlay*)util.GetInterface(IVDEBUGOVERLAY, ENGINE_DLL, &debugOverlay_);
	baseClientDll   = (IBaseClientDLL*)util.GetInterface(BASE_CLIENT_DLL, CLIENT_DLL, &baseClient_);
	iPanel			= util.GetInterface(VGUI_PANEL, VGUI2_DLL, &iPanel_);
	iGameMovement	= util.GetInterface(GAME_MOVEMENT, CLIENT_DLL, &gameMovement_);
	IMaterialSystem = util.GetInterface(IMATERIAL_SYSTEM, MATERIALSYSTEM_DLL, &iMaterialSystem_);
	IVRenderModel	= util.GetInterface(IVRENDER_MODEL, ENGINE_DLL, &ivRenderModel_);
	IStudioRender	= util.GetInterface(ISTUDIO_RENDER, STUDIORENDER_DLL, &iStudioRender_);


	// if adding new interface, ADD IT HERE :)
	if (entityList_ != 0  || engineClient_ != 0 || engineReplay_ != 0 || 
		engineTrace_ != 0 || debugOverlay_ != 0 || iPanel_ != 0		  || 
		baseClient_ != 0  || gameMovement_ != 0 || iMaterialSystem_ != 0||
		ivRenderModel_ != 0 || iStudioRender_ != 0)
	{
		ERROR("TFObjectManager", "Failed to capture interfaces");
		printf("%d\n", iMaterialSystem_);
		return false;
	}
	LOG("TFObjectManager", "successfully captured interfaces");
	return true;
}