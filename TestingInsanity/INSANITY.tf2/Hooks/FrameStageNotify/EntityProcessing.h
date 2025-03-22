#pragma once
#include "../../GlobalVars.h"

#include "../../SDK/offsets/offsets.h"

extern local_netvars netvar;


//inline bool isTargetVisible(const vec& vecEyePos, const vec& vecTargetPos, entInfo_t* ent) {
//
//	I_client_entity* pLocalPlayer = entities::local::pLocalPlayer.load();
//	if (!pLocalPlayer) return false;
//
//	ray_t ray;
//	ray.Init(vecEyePos, vecTargetPos);
//	static trace_t gameTrace;
//	i_trace_filter filter(pLocalPlayer->GetCollideable()->GetEntityHandle());
//	//interface_tf2::pEngineTrace->TraceRay(ray, MASK_ALL, &filter, &gameTrace);
//
//	// draws lines from player eye pos to target bones. 
//	//interface_tf2::pDebugOverlay->ClearAllOverlays();
//	//interface_tf2::pDebugOverlay->AddLineOverlay(vecEyePos, vecTargetPos, 255, 255, 255, true, 0.2f);
//
//	bool didHit = (ent->p_ent == gameTrace.m_entity);
//	didHit ?
//		ent->setFlagBit(IS_VISIBLE) :
//		ent->clearFlagBit(IS_VISIBLE);
//	return didHit;
//}

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