//=========================================================================
//                         FUNCTION INDEX MANAGER
//=========================================================================
// by      : INSANE
// created : 25/02/2025
// 
// purpose : this is supposed to signature scan the VTable and get the function indexs
// at runtime, so I don't have to hardcode them.

#pragma once
#include "unordered_map"

#include "../../Libraries/Utility/Utility.h"
#include "../../Libraries/Console System/Console_System.h"
extern Utility util;
extern Console_System cons;

// FN SIGNATURES
#define GET_TRACER_TYPE		"48 89 5C 24 ? 48 89 6C 24 ? 48 89 74 24 ? 57 48 83 EC ? 48 8B D9 E8 ? ? ? ? 4C 8D 0D"
#define GET_PANEL_NAME		"48 8B 02 48 8B CA 48 FF A0 ? ? ? ? CC CC CC 48 8B 02 48 8B CA 48 FF A0 ? ? ? ? CC CC CC 48 8B 02 48 8B CA 48 FF A0 ? ? ? ? CC CC CC 48 83 EC"
#define PAINT_TRAVERSE		"48 89 5C 24 ? 57 48 83 EC ? 48 8B 01 41 0F B6 D9"
#define FRAME_STAGE_NOTIFY	"48 83 EC ? 89 15"
#define GET_WPN_INFO		"66 3B 0D"
#define FIND_MATERIAL		"48 83 EC ? 48 8B 44 24 ? 4C 8B 11"

//#define IS_ATTACK_CRIT		"48 89 74 24 ? 57 48 83 EC ? 48 8B F9 E8 ? ? ? ? 48 8B C8 C7 44 24 ? ? ? ? ? 4C 8D 0D ? ? ? ? 33 D2 4C 8D 05 ? ? ? ? E8 ? ? ? ? 48 8B F0 48 85 C0 0F 84 ? ? ? ? 48 8B 10"
//#define IS_ATTACK_CRIT		"48 89 5C 24 ? 55 56 57 48 81 EC ? ? ? ? 0F 29 74 24"
#define IS_ATTACK_CRIT		"40 57 48 83 EC ? 48 8B 05 ? ? ? ? 48 8B F9 83 78 ? ? 75"

// giving index for function names
enum FN_name_t {
	FN_GET_TRACER_TYPE=0,
	FN_GET_WPN_INFO,
	FN_GET_PANEL_NAME,
	FN_PAINT_TRAVERSE,
	FN_FRAME_STAGE_NOTIFY,
	FN_FIND_MATERIAL,
	FN_IS_ATTACK_CRIT
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


class FNindexManager_t 
{
public:
	uintptr_t getFnAdrs(FN_name_t FnName, void* pObject);
	uint16_t getFnIndex(FN_name_t FnName, void* pObject);

private:
	int16_t searchPatter(void* pObject, const char* signature, uint16_t seachingThreadHold = 25);
	std::unordered_map<FN_name_t, uint16_t> MAP_FnIndex;
};
inline FNindexManager_t g_FNindexManager;