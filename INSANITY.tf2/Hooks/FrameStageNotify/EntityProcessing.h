#pragma once
#include "../../GlobalVars.h"
#include "../../SDK/offsets/offsets.h"

extern local_netvars netvar;


inline bool isTargetVisible(const vec& vecEyePos, const vec& vecTargetPos, entInfo_t* ent) {

	I_client_entity* pLocalPlayer = entities::local::pLocalPlayer.load();
	if (!pLocalPlayer) return false;

	ray_t ray;
	ray.Init(vecEyePos, vecTargetPos);
	static trace_t gameTrace;
	i_trace_filter filter(pLocalPlayer->GetCollideable()->GetEntityHandle());
	interface_tf2::pEngineTrace->TraceRay(ray, MASK_ALL, &filter, &gameTrace);

	// draws lines from player eye pos to target bones. 
	//interface_tf2::pDebugOverlay->ClearAllOverlays();
	//interface_tf2::pDebugOverlay->AddLineOverlay(vecEyePos, vecTargetPos, 255, 255, 255, true, 0.2f);

	bool didHit = (ent->p_ent == gameTrace.m_entity);
	didHit ?
		ent->setFlagBit(IS_VISIBLE) :
		ent->clearFlagBit(IS_VISIBLE);
	return didHit;
}


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
	int16_t SIZE_vecEnt			= CHE_vecEntInfo.size();
	int16_t SIZE_glowManager	= TF_objects::pGlowManager->count;
	if (SIZE_glowManager > SIZE_vecEnt) {
		loopSize = SIZE_glowManager;
	}
	else {
		loopSize = SIZE_vecEnt;
	}

	qangle localPlayerViewAngles = entities::local::viewAngles.load();
	vec localEyePos = entities::local::eye_pos.load();

	for (int i = 0; i < loopSize; i++) {

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

			// is this entity visible ?
			if (!isTargetVisible(localEyePos, intialTargetBonePos, &ent)) {
				continue;
			}

			// getting view angles & distance for this entity...
			ent.targetAngles = entities::worldToViewangles(entities::local::eye_pos.load(), targetBonePos); // getting angles for target bone
			float entDisFromCrosshair = entities::getFOV(localPlayerViewAngles , ent.targetAngles);
			if (entDisFromCrosshair > config::aimbot::FOV) { // not in FOV
				continue;
			}
			
			// finding best TARGET
			if (disFromCrosshair < 0.0f || entDisFromCrosshair < disFromCrosshair) {
				disFromCrosshair = entDisFromCrosshair;
				closestEntAngles = ent.targetAngles;
				p_bestEnt = &ent;
			}
		}

		// GLOW MANAGER
		if (i < SIZE_glowManager) { 

			glowDef& glow = TF_objects::pGlowManager->g_glowObject[i];			
			auto iterator = allEntMap->find(glow.getEntIndex());
			
			if (iterator == allEntMap->end()) {
				continue;
			}

			glowObject_t& glowObj = iterator->second;

			glow.alpha = 1.0f;
			switch (glowObj.classID)
			{
			case PLAYER:
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
				if (glowObj.isFrendly) {
					glow.color = vec(0.0f, 1.0f, 0.0f);
				}
				else {
					glow.color = vec(1.0f, 1.0f, 1.0f);
				}
				break;

			case TF_ITEM:
				//cons.Log(FG_GREEN, "GLOW MANAGER", "Glowing INTELLIGENCE CASE");
				if (glowObj.isFrendly) { // our intelligence case
					glow.color = vec(0.0f, 1.0f, 0.0f);
				}
				else { // enemy intelligence case
					glow.color = vec(1.0f, 1.0f, 1.0f);
				}
				break;

			// if this entity is unindentified
			default:
				break;
			}

		}
	}

	if(p_bestEnt) p_bestEnt->setFlagBit(SHOULD_LOCK_AIMBOT);
	entities::aimbotTargetAngles.store(closestEntAngles);
	p_bestEnt == nullptr ?
		entities::shouldDoAimbot.store(false) :
		entities::shouldDoAimbot.store(true);

	entities::allEntManager.sendBack(true);
	entities::entManager.setFlagBit(entities::C_targets::DOING_SECOND_HALF); // we have popullated the RENDER_vecEntities list
	entities::entManager.update_vecEntities(CHE_vecEntInfo, true); // updating RENDERABLE entity list
}

inline void processGlow() {

	entities::allEntManager_t::allEntMap* entMap = entities::allEntManager.getReadBuffer();
	
	// looping through the map and enabling glow for each entity
	for (const auto& [entIndex, glowObj] : *entMap) {
	
		// skipping null entries
		if (!glowObj.pEnt) {
			#ifdef _DEBUG
			cons.Log(FG_RED, "GLOW MANAGER", "null entry in glow map, entity index : %d", glowObj.entIndex);
			#endif
			continue;
		}

		// enabling glow
		glowObj.pEnt->setGlow(true);
	}

	entities::allEntManager.sendBack(true);
}