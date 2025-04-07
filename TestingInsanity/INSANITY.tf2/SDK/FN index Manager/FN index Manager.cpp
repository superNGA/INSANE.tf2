//=========================================================================
//                         FUNCTION INDEX MANAGER
//=========================================================================
// by      : INSANE
// created : 24/02/2025
// 
// purpose : this is supposed to signature scan the VTable and get the function indexs
// at runtime, so I don't have to hardcode them.

#include "FN index manager.h"

//=========================================================================
//                     PUBLIC METHODS
//=========================================================================

//=========================================================================
// uintptr_t getFnAdrs(FN_name_t FnName, void* pVTable)
//=========================================================================
/**
* wrapper for getFnIndex, directly retreives the function adrs from given 
* vtable & index
*
* @param FnName : input a index of desired fn from the enum
* @param pVTable : input pointer to vtable and cast it to void* too
*/
uintptr_t FNindexManager_t::getFnAdrs(FN_name_t FnName, void* pObject)
{
	return (uintptr_t)util.GetVirtualTable(pObject)[getFnIndex(FnName, pObject)];
}


//=========================================================================
// uint16_t getFnIndex(FN_name_t FnName, void* pVTable)
//=========================================================================
/**
* retrieves function index in a vtable
*
* @param FnName : input a index of desired fn from the enum
* @param pVTable : input pointer to vtable and cast it to void* too
*/
uint16_t FNindexManager_t::getFnIndex(FN_name_t FnName, void* pObject)
{
	auto it = MAP_FnIndex.find(FnName);
	if (it != MAP_FnIndex.end()) { // function already cached
		return it->second; // return FN index
	}

	uint16_t index = 0;
	switch (FnName)
	{
	case FN_GET_TRACER_TYPE:
		index = searchPatter(pObject, GET_TRACER_TYPE,		SEARCH_THRESHOLD::THRESHOLD_250);
		break;
	case FN_GET_WPN_INFO:
		index = searchPatter(pObject, GET_WPN_INFO,			SEARCH_THRESHOLD::THRESHOLD_250);
		break;
	case FN_GET_PANEL_NAME:
		index = searchPatter(pObject, GET_PANEL_NAME,		SEARCH_THRESHOLD::THRESHOLD_50);
		break;
	case FN_PAINT_TRAVERSE:
		index = searchPatter(pObject, PAINT_TRAVERSE,		SEARCH_THRESHOLD::THRESHOLD_50);
		break;
	case FN_FRAME_STAGE_NOTIFY:
		index = searchPatter(pObject, FRAME_STAGE_NOTIFY,	SEARCH_THRESHOLD::THRESHOLD_50);
		break;
	case FN_IS_ATTACK_CRIT:
		index = searchPatter(pObject, IS_ATTACK_CRIT,		SEARCH_THRESHOLD::THRESHOLD_400);
		break;
	case FN_FIND_MATERIAL:
		index = searchPatter(pObject, FIND_MATERIAL,		SEARCH_THRESHOLD::THRESHOLD_100);
		break;
	default:
		return 0;
	}

	if (!index) { // if failed to find signature
		#ifdef _DEBUG
		cons.Log(FG_RED, "FN INDEX MANAGER", "FN [%d] doesn't exist for : %p", FnName, pObject);
		#endif // _DEBUG
		return 0;
	}
	#ifdef _DEBUG
	cons.Log(FG_GREEN, "FN INDEX MANAGER", "succesfully got index for [%d] @ [%d]", FnName, index);
	#endif

	MAP_FnIndex[FnName] = index;
	return index;
}

uint32_t FNindexManager_t::getFnIndex(const char* signature, void* pObject, SEARCH_THRESHOLD iSearchThreshold)
{
	uint32_t hashSignature = FNV1A32(signature);
	
	auto it = MAP_FnIndex2.find(hashSignature);
	if (it != MAP_FnIndex2.end()) { // function already cached
		return it->second; // return FN index
	}

	auto index = searchPatter(pObject, signature, iSearchThreshold);
	if (index == 0)
	{
		#ifdef _DEBUG
		cons.Log(FG_RED, "Fn Index managment", "Failed to find funcition with signature [ %s ]", signature);
		#endif
		return 0;
	}

	#ifdef _DEBUG
	cons.Log(FG_GREEN, "Fn Index Managment", "Found signature [ %s ] @ index [ %d ]", signature, index);
	#endif

	MAP_FnIndex2[hashSignature] = index;
	return index;
}


//=========================================================================
//                     PRIVATE METHODS
//=========================================================================

//=========================================================================
// int16_t searchPatter(void* pVTable, const char* signature, uint16_t seachingThreadHold)
//=========================================================================
/**
* loops through the given Vtable and finds fn index in vtable
*
* @param pVTable : pointer to vtable casted as void*
* @param signature : signature to find in vtable
* @param searchingThreadhold : number of signatures to seach before giving up
*/
int16_t FNindexManager_t::searchPatter(void* pObject, const char* signature, uint16_t seachingThreadHold)
{
	void** VTable = util.GetVirtualTable(pObject);
	for (int i = 0; i < seachingThreadHold; i++) {
		if (util.FindPattern(signature, (uintptr_t)VTable[i])) {
			return i;
		}
	}
	return 0;
}