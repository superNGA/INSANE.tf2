/* this is supposed to signature scan the VTable and get the function indexs
at runtime, so I don't have to hardcode them.*/

#pragma once
#include "unordered_map"

#include "../Libraries/Utility/Utility.h"
#include "../Libraries/Console System/Console_System.h"
extern Utility util;
extern Console_System cons;

// FN SIGNATURES
#define GET_TRACER_TYPE "48 89 5C 24 ? 48 89 6C 24 ? 48 89 74 24 ? 57 48 83 EC ? 48 8B D9 E8 ? ? ? ? 4C 8D 0D"
#define GET_PANEL_NAME	"48 8B 02 48 8B CA 48 FF A0 ? ? ? ? CC CC CC 48 8B 02 48 8B CA 48 FF A0 ? ? ? ? CC CC CC 48 8B 02 48 8B CA 48 FF A0 ? ? ? ? CC CC CC 48 83 EC"
#define PAINT_TRAVERSE	"48 89 5C 24 ? 57 48 83 EC ? 48 8B 01 41 0F B6 D9"
#define GET_WPN_INFO	"66 3B 0D"

// giving index for function names
enum FN_name_t {
	FN_GET_TRACER_TYPE=0,
	FN_GET_WPN_INFO,
	FN_GET_PANEL_NAME,
	FN_PAINT_TRAVERSE
};

// this is the threshold value for seaching in VTable, i.e. we only check this many functions in given VTable
// before returning NULL, passing a SEARCH_THRESHOLD is mandatory :)
enum SEARCH_THRESHOLD {
	THRESHOLD_25 = 25,
	THRESHOLD_50 = 50,
	THRESHOLD_75 = 75,
	THRESHOLD_100 = 100,
	THRESHOLD_150 = 150,
	THRESHOLD_200 = 200,
	THRESHOLD_250 = 250,
	THRESHOLD_300 = 300,
	THRESHOLD_350 = 350,
	THRESHOLD_400 = 400,
	THRESHOLD_450 = 450,
	THRESHOLD_500 = 500
};


class FNindexManager_t {
public:
	uintptr_t getFnAdrs(FN_name_t FnName, void* pVTable) {
		return (uintptr_t)util.GetVirtualTable(pVTable)[getFnIndex(FnName, pVTable)];
	}

	uint16_t getFnIndex(FN_name_t FnName, void* pVTable) {
		
		auto it = MAP_FnIndex.find(FnName);
		if (it != MAP_FnIndex.end()) { // function already cached
			return it->second; // return FN index
		}

		uint16_t index = 0;
		switch (FnName)
		{
		case FN_GET_TRACER_TYPE:
			index = searchPatter(pVTable, GET_TRACER_TYPE, SEARCH_THRESHOLD::THRESHOLD_250);
			break;
		case FN_GET_WPN_INFO:
			index = searchPatter(pVTable, GET_WPN_INFO, SEARCH_THRESHOLD::THRESHOLD_250);
			break;
		case FN_GET_PANEL_NAME:
			index = searchPatter(pVTable, GET_PANEL_NAME, SEARCH_THRESHOLD::THRESHOLD_50);
			break;
		case FN_PAINT_TRAVERSE:
			index = searchPatter(pVTable, PAINT_TRAVERSE, SEARCH_THRESHOLD::THRESHOLD_50);
			break;
		default:
			return 0;
		}

		if (!index) { // if failed to find signature
			#ifdef _DEBUG
			cons.Log(FG_RED, "FN INDEX MANAGER", "FN [%d] doesn't exist for : %p", FnName, pVTable);
			#endif // _DEBUG
			return 0;
		}
		#ifdef _DEBUG
		cons.Log(FG_GREEN, "FN INDEX MANAGER", "succesfully got index for [%d] @ [%d]", FnName, index);
		#endif
		
		MAP_FnIndex[FnName] = index;
		return index;
	}
private:
	std::unordered_map<FN_name_t, uint16_t> MAP_FnIndex;

	/* Putting a threash hold on this function was */
	int16_t searchPatter(void* pVTable, const char* signature, uint16_t seachingThreadHold = 25) {
	
		void** VTable = util.GetVirtualTable(pVTable);
		for (int i = 0; i < seachingThreadHold; i++) {
			if (util.FindPattern(signature, (uintptr_t)VTable[i])) {
				return i;
			}
		}
		return 0;
	}
};
inline FNindexManager_t g_FNindexManager;