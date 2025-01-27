#pragma once
#include "../../GlobalVars.h"
#include "../../SDK/offsets/offsets.h"

extern local_netvars netvar;

inline void processEntities()
{
	/* updating matrices */
	entities::M_worldToScreen	= interface_tf2::engine->WorldToScreenMatrix();
	entities::M_worldToView		= interface_tf2::engine->WorldToViewMatrix();

	std::vector<entInfo_t> CHE_vecEntInfo = entities::entManager.get_vecEntities();
	if (CHE_vecEntInfo.empty()) 
		return;

	for (auto& ent : CHE_vecEntInfo)
	{
		printf("%.2f %.2f %.2f\n", ent.entVelocity.x, ent.entVelocity.y, ent.entVelocity.z);
	}
}