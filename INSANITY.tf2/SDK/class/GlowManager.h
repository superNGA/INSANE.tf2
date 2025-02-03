#pragma once
#include "Basic Structures.h"

struct glowDef
{
	int32_t	pEnt; // <-- Entity EHANDLE, just holds index first 12 bits
	vec		color;
	float	alpha;
	bool	renderWhenOccluded; 
	bool	renderWhenUnoccluded; 
};

class glowManager
{
public:
	glowDef* g_glowObject; // array of glowDef objects
	int count;
};