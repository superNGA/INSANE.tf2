#pragma once
#include "../../GlobalVars.h"
#include "../../SDK/offsets/offsets.h"

extern local_netvars netvar;

inline void processEntities()
{
	/* updating matrices */
	entities::M_worldToScreen	= interface_tf2::engine->WorldToScreenMatrix();
	entities::M_worldToView		= interface_tf2::engine->WorldToViewMatrix();

	if (!entities::entManager.getFlagBit(entities::C_targets::DOING_FIRST_HALF)) { // if first vecEntites ( first half is not popullated )
		entities::entManager.clearFlagBit(entities::C_targets::DOING_SECOND_HALF); // we are not popullating second half
		return;
	}

	std::vector<entInfo_t> CHE_vecEntInfo = entities::entManager.get_vecEntities();
	if (CHE_vecEntInfo.empty()) 
		return;

	view_matrix CHE_viewMatrix = entities::M_worldToScreen.load();
	matrix3x4_t CHE_boneMatrix[MAX_STUDIO_BONES];
	
	// loading values related to local player 
	vec eyePos = entities::local::eye_pos.load();
	
	entInfo_t* p_bestEnt = nullptr; // pointer to best aimbot target
	float disFromCrosshair = -1.0f; // if negative after processing, then no best entity
	qangle closestEntAngles; // <- best aimbot angles
	for (auto& ent : CHE_vecEntInfo) {

		ent.p_ent->SetupBones(CHE_boneMatrix, MAX_STUDIO_BONES, HITBOX_BONES, entities::pGlobalVars->curtime);
		ent.copyEntBones(CHE_boneMatrix);

		/* initial position of target bone, if gonna do projectile aimbot then process it, else use it as it is. */
		vec intialTargetBonePos = ent.bones[ent.targetBoneID].get_bone_coordinates(); 
		vec targetBonePos;
		entities::local::b_hasProjectileWeapon ?
			targetBonePos = entities::projAimbotCalculations(eyePos, intialTargetBonePos, ent.entVelocity, ent.getFlagBit(ENT_ON_GROUND)):
			targetBonePos = intialTargetBonePos;

		ent.targetAngles = entities::worldToViewangles(entities::local::eye_pos.load(), targetBonePos); // getting angles for target bone
		float entDisFromCrosshair = entities::disFromCrosshair(entities::local::viewAngles.load(), ent.targetAngles); // target angles distance from crosshair

		/* calculating closest entity from crosshair */
		if (disFromCrosshair < 0.0f) {
			disFromCrosshair	= entDisFromCrosshair;
			closestEntAngles	= ent.targetAngles;
			p_bestEnt			= &ent;
		}
		else if (entDisFromCrosshair < disFromCrosshair) {
			disFromCrosshair	= entDisFromCrosshair;
			closestEntAngles	= ent.targetAngles;
			p_bestEnt			= &ent;
		}
	}

	if(p_bestEnt) p_bestEnt->setFlagBit(SHOULD_LOCK_AIMBOT);
	entities::aimbotTargetAngles.store(closestEntAngles);

	entities::entManager.setFlagBit(entities::C_targets::DOING_SECOND_HALF); // we have popullated the RENDER_vecEntities list
	entities::entManager.update_vecEntities(CHE_vecEntInfo, true); // updating RENDERABLE entity list
}

inline void processGlow() {

	auto vecEnt = entities::entManager.get_vecEntities();
	
	for (auto& ent : vecEnt) {
		*(bool*)((uintptr_t)ent.p_ent + netvar.m_bGlowEnabled) = true;
	}
}