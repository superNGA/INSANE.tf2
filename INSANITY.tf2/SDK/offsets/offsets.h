#pragma once
#include <unordered_map>
//#include <string>
#include <functional>
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


/* this holds offsets */
namespace offsets
{
	/* initialize netvars */
	bool initialize();

	/* global netvar map, once popullated holds the all offsets. */
	extern T_map* netvar_map;

	/* whether netvars initialized or not */
	extern bool netvar_initialized;

	/* retrives the netvar varibles and also checks whether the netvar map 
	is popullated or not */
	uintptr_t get_netvar(const char* name);
};