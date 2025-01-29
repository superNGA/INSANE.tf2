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

	view_matrix CHE_viewMatrix = entities::M_worldToScreen.load();
	matrix3x4_t CHE_boneMatrix[MAX_STUDIO_BONES];
	for (auto& ent : CHE_vecEntInfo) {

		ent.p_ent->SetupBones(CHE_boneMatrix, MAX_STUDIO_BONES, HITBOX_BONES, entities::pGlobalVars->curtime);
		ent.copyEntBones(CHE_boneMatrix);
		/* todo : do world to view angles here, for aimbot and more important stuff */
	}
	entities::entManager.update_vecEntities(CHE_vecEntInfo, true);
}