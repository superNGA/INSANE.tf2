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
	static int boneIndex = 0;
	if (GetAsyncKeyState(VK_RETURN) & (1 << 0)) boneIndex++;
	else if (GetAsyncKeyState(VK_BACK) & (1 << 0)) boneIndex--;
	for (auto& ent : CHE_vecEntInfo)
	{
		ent.p_ent->SetupBones(ent.bones, MAX_STUDIO_BONES, HITBOX_BONES, entities::pGlobalVars->curtime);
		entities::world_to_screen(ent.bones[ent.infoBoneID->head].get_bone_coordinates(), ent.boneScreenPos.bonePos, &CHE_viewMatrix);
	}
	entities::entManager.update_vecEntities(CHE_vecEntInfo, true);
}