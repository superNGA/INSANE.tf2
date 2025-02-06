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

	// getting vecEntities
	std::vector<entInfo_t> CHE_vecEntInfo = entities::entManager.get_vecEntities();
	if (CHE_vecEntInfo.empty()) 
		return;

	// getting map for all entities
	entities::allEntManager_t::allEntMap* allEntMap = entities::allEntManager.getReadBuffer();

	view_matrix CHE_viewMatrix = entities::M_worldToScreen.load();
	matrix3x4_t CHE_boneMatrix[MAX_STUDIO_BONES];
	
	// loading values related to local player 
	vec eyePos = entities::local::eye_pos.load();
	
	entInfo_t* p_bestEnt = nullptr; // pointer to best aimbot target
	float disFromCrosshair = -1.0f; // if negative after processing, then no best entity
	qangle closestEntAngles; // <- best aimbot angles

	// getting loop size
	int16_t loopSize = 0;
	int16_t SIZE_vecEnt = CHE_vecEntInfo.size();
	int16_t SIZE_mapEnt = allEntMap->size();
	if (SIZE_vecEnt > SIZE_mapEnt) {
		loopSize = SIZE_vecEnt;
	}
	else {
		loopSize = SIZE_mapEnt;
	}

	printf("GLOW MANAGER SIZE : %d, map size : %d, vector size : %d, loop size : %d\n", TF_objects::pGlowManager->count ,SIZE_mapEnt, SIZE_vecEnt, loopSize);
	for (int i = 0; i < TF_objects::pGlowManager->count; i++) {

		// AIMBOT LOGIC
		if (i < SIZE_vecEnt) {

			entInfo_t& ent = CHE_vecEntInfo[i]; 
			if (ent.getFlagBit(FRENDLY)) continue; // skipping entites from our team

			ent.p_ent->SetupBones(CHE_boneMatrix, MAX_STUDIO_BONES, HITBOX_BONES, entities::pGlobalVars->curtime);
			ent.copyEntBones(CHE_boneMatrix);

			/* initial position of target bone, if gonna do projectile aimbot then process it, else use it as it is. */
			vec intialTargetBonePos = ent.bones[ent.targetBoneID].get_bone_coordinates();
			vec targetBonePos;
			entities::local::b_hasProjectileWeapon ?
				targetBonePos = entities::projAimbotCalculations(eyePos, intialTargetBonePos, ent.entVelocity, ent.getFlagBit(ENT_ON_GROUND)) :
				targetBonePos = intialTargetBonePos;

			ent.targetAngles = entities::worldToViewangles(entities::local::eye_pos.load(), targetBonePos); // getting angles for target bone
			float entDisFromCrosshair = entities::disFromCrosshair(entities::local::viewAngles.load(), ent.targetAngles); // target angles distance from crosshair

			/* calculating closest entity from crosshair */
			if (disFromCrosshair < 0.0f) {
				disFromCrosshair = entDisFromCrosshair;
				closestEntAngles = ent.targetAngles;
				p_bestEnt = &ent;
			}
			else if (entDisFromCrosshair < disFromCrosshair) {
				disFromCrosshair = entDisFromCrosshair;
				closestEntAngles = ent.targetAngles;
				p_bestEnt = &ent;
			}
		}

		// GLOW MANAGER
		if (i < TF_objects::pGlowManager->count) { 

			glowDef& glow = TF_objects::pGlowManager->g_glowObject[i];
			glowObject_t& glowObj = (*allEntMap)[glow.getEntIndex()];

			printf("ENTITY : %d\n", glow.getEntIndex());

			glow.alpha = 1.0f;
			switch (glowObj.classID)
			{
			case PLAYER:
				//cons.Log(FG_GREEN, "GLOW MANAGER", "Glowing PLAYER");
				if (glowObj.isFrendly) {
					glow.color = vec(0.0f, 1.0f, 1.0f);
				}
				else {
					glow.color = vec(1.0f, 1.0f, 1.0f);
				}
				break;

			case DISPENSER:
			case SENTRY_GUN:
			case TELEPORTER:

				cons.Log(FG_GREEN, "GLOW MANAGER", "Glowing BUILDING");
				if (glowObj.isFrendly) {
					glow.color = vec(0.0f, 1.0f, 0.0f);
				}
				else {
					glow.color = vec(1.0f, 1.0f, 1.0f);
				}
				break;

			case TF_ITEM:
				cons.Log(FG_GREEN, "GLOW MANAGER", "Glowing INTELLIGENCE CASE");
				if (glowObj.isFrendly) { // our intelligence case
					glow.color = vec(0.0f, 1.0f, 0.0f);
				}
				else { // enemy intelligence case
					glow.color = vec(1.0f, 1.0f, 1.0f);
				}
				break;

			case AMMO_PACK:
				cons.Log(FG_GREEN, "GLOW MANAGER", "Glowing AMMO PACK");
				glow.color = vec(1.0f, 1.0f, 0.0f);
				break;

			// if this entity is unindentified
			default:
				cons.Log(FG_GREEN, "GLOW MANAGER", "BULLSHIT");
				glow.color = vec(1.0f, 1.0f, 1.0f);
				break;
			}
		}
	}

	if(p_bestEnt) p_bestEnt->setFlagBit(SHOULD_LOCK_AIMBOT);
	entities::aimbotTargetAngles.store(closestEntAngles);

	entities::allEntManager.sendBack(true);
	entities::entManager.setFlagBit(entities::C_targets::DOING_SECOND_HALF); // we have popullated the RENDER_vecEntities list
	entities::entManager.update_vecEntities(CHE_vecEntInfo, true); // updating RENDERABLE entity list
}

inline void processGlow() {

	entities::allEntManager_t::allEntMap* entMap = entities::allEntManager.getReadBuffer();
	for (const auto& [entIndex, glowObj] : *entMap) {
	
		if (!glowObj.pEnt) {
			continue;
		}

		*(bool*)((uintptr_t)glowObj.pEnt + netvar.m_bGlowEnabled) = true;
	}

	entities::allEntManager.sendBack(true);
}