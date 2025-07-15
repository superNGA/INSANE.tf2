#pragma once

#include "../../Utility/Interface Handler/Interface.h"
#include "Basic Structures.h"

#define MAX_DLIGHTS 32

struct dlight_t
{
	int		flags;
	vec		origin;
	float	radius;
	RGBA_t	color;		// Light color with exponent
	float	die;		// stop lighting after this time
	float	decay;		// drop this each second
	float	minlight;	// don't add when contributing less
	int		key;
	int		style;		// lightstyle

	vec		m_Direction;		// center of the light cone
	float	m_InnerAngle;
	float	m_OuterAngle;
};

class IVEfx
{
public:
	// Retrieve decal texture index from decal by name
	virtual	int				Draw_DecalIndexFromName(char* name) = 0;

	// Apply decal
	virtual	void			DecalShoot(int textureIndex, int entity,
		const void* model, const vec& model_origin, const qangle& model_angles,
		const vec& position, const vec* saxis, int flags) = 0;

	// Apply colored decal
	virtual	void			DecalColorShoot(int textureIndex, int entity,
		const void* model, const vec& model_origin, const qangle& model_angles,
		const vec& position, const vec* saxis, int flags, const int& rgbaColor) = 0;

	virtual void			PlayerDecalShoot(void* material, void* userdata, int entity, const void* model,
		const vec& model_origin, const qangle& model_angles,
		const vec& position, const vec* saxis, int flags, const int& rgbaColor) = 0;

	// Allocate a dynamic world light ( key is the entity to whom it is associated )
	virtual	dlight_t* CL_AllocDlight(int key) = 0;

	// Allocate a dynamic entity light ( key is the entity to whom it is associated )
	virtual	dlight_t* CL_AllocElight(int key) = 0;

	// Get a list of the currently-active dynamic lights.
	virtual int CL_GetActiveDLights(dlight_t* pList[MAX_DLIGHTS]) = 0;

	// Retrieve decal texture name from decal by index
	virtual	const char* Draw_DecalNameFromIndex(int nIndex) = 0;

	// Given an elight key, find it. Does not search ordinary dlights. May return NULL.
	virtual dlight_t* GetElightByKey(int key) = 0;
};


MAKE_INTERFACE_VERSION(iVEfx, "VEngineEffects001", IVEfx, ENGINE_DLL);