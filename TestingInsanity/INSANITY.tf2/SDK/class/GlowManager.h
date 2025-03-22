#pragma once
#include "Basic Structures.h"

struct glowDef
{
	int32_t	pEnt; // <-- Entity EHANDLE, just holds index first 12 bits
	vec		color;
	float	alpha;
	
	char PAD_MAXXING[12]; // padding maxxing by yours sincerely :)

	inline int16_t getEntIndex() {
		return pEnt & 0xFFF;
	}
};

class glowManager
{
public:
	glowDef* g_glowObject; // array of glowDef objects
	int count;
};