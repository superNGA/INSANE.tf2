#include "thread2.h"

void execute_thread2(HINSTANCE instance)
{
	/* wait till thread 1 gets all the INTERFACES and UI is initialized*/
	while (!thread_termination_status::thread1_primed)
	{
		std::this_thread::sleep_for(std::chrono::milliseconds(100));

		#ifdef _DEBUG
		cons.Log("Waiting for THREAD 1 to prime", FG_YELLOW);
		#endif
	}

	/* starting entity thread :) */
	#ifdef _DEBUG
	cons.Log(FG_GREEN, "initializing", "THREAD 2");
	#endif

	/* PURPOSE: retains old position from last iteration */
	std::unordered_map<int32_t, vec> map_entOldPos;

	while (!directX::UI::UI_has_been_shutdown)
	{
		std::this_thread::sleep_for(std::chrono::milliseconds(10)); // This thread must sleep for this much in each iteration.

		/* check if in game */
		if (!interface_tf2::engine->IsInGame()) {
			std::this_thread::sleep_for(std::chrono::seconds(1));
			continue;
		}

		/* LOCAL PLAYER */
		I_client_entity* local_player = interface_tf2::entity_list->GetClientEntity(interface_tf2::engine->GetLocalPlayer());
		qangle viewangles_localplayer = local_player->GetAbsAngles();
		if (!local_player) {
			std::this_thread::sleep_for(std::chrono::milliseconds(500));
			continue;
		}
		netvar.local_player					= (uintptr_t)local_player; //storing as uintptr_t
		entities::local::localplayer_class	= (player_class)(*(int32_t*)(netvar.local_player + netvar.m_PlayerClass + netvar.m_iClass)); // player ingame class
		entities::local::team_num			= *(int32_t*)(netvar.local_player + netvar.m_iTeamNum); // local players team
		entities::local::pos				= local_player->GetAbsOrigin();
		entities::local::eye_pos.store(entities::local::pos + vec(0.0f, 0.0f, *(float*)((uintptr_t)local_player + netvar.m_vecViewOffset))); // storing local player's eyepos
		entities::local::viewAngles.store(local_player->GetAbsAngles()); // storing view angles 

		/* get ACTIVE WEAPON */
		entities::local::active_weapon = interface_tf2::entity_list->GetClientEntity(local_player->get_active_weapon_handle());
		if (!entities::local::active_weapon) {
			std::this_thread::sleep_for(std::chrono::milliseconds(500));
			continue;
		}
		entities::local::ID_active_weapon		= *(int32_t*)((uintptr_t)entities::local::active_weapon + netvar.m_AttributeManager + netvar.m_Item + netvar.m_iItemDefinitionIndex); //gets weapon ID
		entities::local::b_hasProjectileWeapon	= getWeaponType(entities::local::ID_active_weapon); // are we holding a projectile weapon or a hitscan weapon

		/* cache storages for things to be used in entity list loop */
		static player_info_t			CHE_playerInfo;
		static matrix3x4_t				CHE_mBones[MAX_STUDIO_BONES];

		const view_matrix r_viewmatrix	= entities::M_worldToScreen.load();
		const view_matrix r_anglematrix	= entities::M_worldToView.load();
		entities::pGlobalVars			= interface_tf2::engine_replay->GetClientGlobalVars();
		int16_t ent_count				= interface_tf2::entity_list->NumberOfEntities(false);
		int8_t localplayer_index		= interface_tf2::engine->GetLocalPlayer();

		std::vector<entInfo_t> CHE_vecEntities; // PURPOSE : temporary cache vector for all valid which we shift into global storage after 
		CHE_vecEntities.clear();
		/* entity list loop here */
		for (int ent_num = 0; ent_num < ent_count; ent_num++)
		{
			//===================================== ENTITY FILTERING ====================================================
			if (ent_num == localplayer_index) { // skipping local player
				continue;
			}
			I_client_entity* ent = interface_tf2::entity_list->GetClientEntity(ent_num);
			if (!ent || ent->IsDormant()) { // DORMANT check
				continue;
			}
			interface_tf2::engine->GetPlayerInfo(ent_num, &CHE_playerInfo);
			if (CHE_playerInfo.name[0] == '\0') { // REAL PLAYER check
				continue;
			}
			if (*(int16_t*)((uintptr_t)ent + netvar.m_lifeState) != 0) { // DEAD check
				continue;
			}
			if (*(int16_t*)((uintptr_t)ent + netvar.m_iTeamNum) == entities::local::team_num) { // TEAM check
				continue;
			}

			//===================================== FILLING INFO ABOUT VAILD ENTITIES ====================================
			vec entOrigin = ent->GetAbsOrigin();
			entInfo_t CHE_entInfo;
			CHE_entInfo.p_ent			= ent;
			CHE_entInfo.entUserName		= &CHE_playerInfo.name[0];
			CHE_entInfo.entPos			= entOrigin;
			CHE_entInfo.activeWeapon	= *(int32_t*)((uintptr_t)(interface_tf2::entity_list->GetClientEntity(ent->get_active_weapon_handle())) + netvar.m_AttributeManager + netvar.m_Item + netvar.m_iItemDefinitionIndex);
			CHE_entInfo.charactorChoice = (player_class)(*(int32_t*)((uintptr_t)ent + netvar.m_PlayerClass + netvar.m_iClass));
			CHE_entInfo.entVelocity		= ent->getEntVelocity();

			/* store if entity is on ground or not, use full for projectile aimbot */
			*(int32_t*)((uintptr_t)ent + netvar.m_fFlags) & MF_ONGROUND ?
				CHE_entInfo.setFlagBit(ENT_ON_GROUND) : 
				CHE_entInfo.clearFlagBit(ENT_ON_GROUND);

			CHE_entInfo.infoBoneID = entities::boneManager.getBone((void*)ent, CHE_entInfo.charactorChoice); // return boneInfo pointer, and cache if not already
			
			/* processing target Bone ID accoring to entity state and active weapon */
			if (entities::local::b_hasProjectileWeapon) { 
				CHE_entInfo.getFlagBit(ENT_ON_GROUND) ? 
					CHE_entInfo.targetBoneID = LEFT_FOOT : // if doing projectile aimbot, and entity on ground, then shoot at foot
					CHE_entInfo.targetBoneID = CHEST; // else if in air, shoot at chest
			}
			else { // if using hit-scan weapon 
				CHE_entInfo.targetBoneID = HEAD; // shoot at head
			}

			CHE_vecEntities.push_back(CHE_entInfo);
		}
		entities::entManager.update_vecEntities(CHE_vecEntities); // updating global filtered entity list

		/* this flag will be used in features to prevent them from accessing invalid memory spaces */
		global::entities_popullated = true;
	}

	#ifdef _DEBUG
	cons.Log(FG_GREEN, "terminating", "THREAD 2");
	#endif

	thread_termination_status::thread2 = true;
	ExitThread(0);
	return;
}