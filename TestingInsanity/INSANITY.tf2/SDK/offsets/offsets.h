#pragma once
#include <unordered_map>
#include "../../GlobalVars.h"

/* helper libraries */
#include "../../Libraries/Utility/Utility.h"
extern Utility util;

/* game classes */
#include "../class/I_BaseEntityDLL.h"

#ifdef _DEBUG
#include "../../Libraries/Console System/Console_System.h"
extern Console_System cons;
#endif

/* m_fFlag bit defines */
#define MF_ONGROUND (1<<0)

/* this holds offsets */
namespace offsets
{
	/* initialize netvars */
	bool initialize();

	/* whether netvars initialized or not */
	extern bool netvar_initialized;

	/* Fills in the only local netvar object */
	void fill_local_netvars(T_map& map, const char* table_name);
};

/* after initializing the netvar map, I am storing them in this struct.
this should make the netvar accesing very fast. */
struct local_netvars
{
	uintptr_t m_fFlags					= 0; // if first bit 1 -> on ground, 0 -> in air
	uintptr_t m_nForceTauntCam			= 0;
	uintptr_t m_iReloadMode				= 0;
	uintptr_t m_hActiveWeapon			= 0;
	uintptr_t m_PlayerClass				= 0;
	uintptr_t m_iClass					= 0;
	uintptr_t m_AttributeManager		= 0;
	uintptr_t m_Item					= 0;
	uintptr_t m_iItemDefinitionIndex	= 0;
	uintptr_t m_iHealth					= 0;
	uintptr_t m_lifeState				= 0;
	uintptr_t m_iTeamNum				= 0;
	uintptr_t m_vecViewOffset			= 0;
	uintptr_t m_bGlowEnabled			= 0;
	uintptr_t m_iMaxHealth				= 0;
	uintptr_t m_Shared					= 0;
	uintptr_t m_nPlayerCond				= 0;
	uintptr_t m_bReadyToBackstab		= 0;
	uintptr_t m_velocity				= 0;
	uintptr_t m_hItem					= 0;

	uintptr_t m_nSequence				= 0;
	uintptr_t m_flCycle					= 0;
	uintptr_t m_flPoseParameter			= 0;

	uintptr_t m_nTickBase = 0;
};

extern local_netvars netvar;