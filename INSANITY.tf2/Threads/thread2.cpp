#include "thread2.h"

void execute_thread2(HINSTANCE instance)
{
	/* wait till thread 1 gets all the INTERFACES and UI is initialized*/
	while (!thread_termination_status::thread1_primed)
	{
		std::this_thread::sleep_for(std::chrono::milliseconds(100));

		#ifdef _DEBUG
		cons.Log("Waiting for thread1 to prime", FG_YELLOW);
		#endif
	}

	/* starting entity thread :) */
	#ifdef _DEBUG
	cons.Log("Starting Entity thread", FG_GREEN);
	#endif

	while (!directX::UI::UI_has_been_shutdown)
	{
		std::this_thread::sleep_for(std::chrono::milliseconds(5)); // This thread must sleep for this much in each iteration.

		/* check if in game */
		if (!interface_tf2::engine->IsInGame()) {
			std::this_thread::sleep_for(std::chrono::seconds(1));
			continue;
		}

		/* local player */
		I_client_entity* local_player = interface_tf2::entity_list->GetClientEntity(interface_tf2::engine->GetLocalPlayer());
		if (!local_player) {
			std::this_thread::sleep_for(std::chrono::milliseconds(500));
			continue;
		}
		netvar.local_player = (uintptr_t)local_player;
		entities::local::localplayer_class = (player_class)(*(int32_t*)(netvar.local_player + netvar.m_PlayerClass + netvar.m_iClass)); // player ingame class
		entities::local::team_num = *(int32_t*)(netvar.local_player + netvar.m_iTeamNum); // local players team

		/* get active weapon */
		entities::local::active_weapon = interface_tf2::entity_list->GetClientEntity(local_player->get_active_weapon_handle());
		if (!entities::local::active_weapon) {
			std::this_thread::sleep_for(std::chrono::milliseconds(500));
			continue;
		}
		entities::local::ID_active_weapon = *(int32_t*)((uintptr_t)entities::local::active_weapon + netvar.m_AttributeManager + netvar.m_Item + netvar.m_iItemDefinitionIndex); //gets weapon ID

		/* cache storages for things to be used in entity list loop */
		static player_info_t playerinfo_cache;
		static matrix3x4_t skeleton_cache[MAX_STUDIO_BONES];
		static entities::entity_dimensions cached_entity_dimension;

		/* entity list loop here */
		const view_matrix r_viewmatrix	= interface_tf2::engine->WorldToScreenMatrix();
		global_var_base* p_globalvar	= interface_tf2::engine_replay->GetClientGlobalVars();
		int16_t ent_count				= interface_tf2::entity_list->NumberOfEntities(false);
		int8_t localplayer_index		= interface_tf2::engine->GetLocalPlayer();
		entities::target::all_entity_dimensions.clear();
		for (int ent_num = 0; ent_num < ent_count; ent_num++)
		{
			/* skipping local player */
			if (ent_num == localplayer_index) continue;

			/* dormancy check */
			I_client_entity* ent = interface_tf2::entity_list->GetClientEntity(ent_num);
			if (!ent || ent->IsDormant()) continue;

			/* is player check */
			interface_tf2::engine->GetPlayerInfo(ent_num, &playerinfo_cache);
			if (playerinfo_cache.name[0] == '\0') continue;

			/* dead & team check */
			if (*(int16_t*)((uintptr_t)ent + netvar.m_lifeState) != 0) continue;
			if (*(int16_t*)((uintptr_t)ent + netvar.m_iTeamNum) == entities::local::team_num) continue;

			/* getting entity bones and storing it if entity is on the screen */
			/*ent->SetupBones(skeleton_cache, MAX_STUDIO_BONES, HITBOX_BONES, p_globalvar->curtime);
			if (entities::world_to_screen(skeleton_cache[BONE_HEAD].get_bone_coordinates(), cached_entity_dimension.head, &r_viewmatrix) ||
				entities::world_to_screen(skeleton_cache[BONE_RIGHT_FOOT].get_bone_coordinates(), cached_entity_dimension.right_foot, &r_viewmatrix) ||
				entities::world_to_screen(skeleton_cache[BONE_LEFT_FOOT].get_bone_coordinates(), cached_entity_dimension.left_foot, &r_viewmatrix) ||
				entities::world_to_screen(skeleton_cache[BONE_LEFT_SHOULDER].get_bone_coordinates(), cached_entity_dimension.left_shoulder, &r_viewmatrix) ||
				entities::world_to_screen(skeleton_cache[BONE_RIGHT_SHOULDER].get_bone_coordinates(), cached_entity_dimension.right_shoulder, &r_viewmatrix))
			{
				entities::target::all_entity_dimensions.push_back(cached_entity_dimension);
			}*/

			/*if (entities::target::all_entity_dimensions.empty()) continue;*/
			/*printf("%.2f %.2f\n", entities::target::all_entity_dimensions[0].head.x, entities::target::all_entity_dimensions[0].head.y);*/
		}

		/* this flag will be used in features to prevent them from accessing invalid memory spaces */
		global::entities_popullated = true;
	}

	#ifdef _DEBUG
	cons.Log("Exited entity thread", FG_GREEN);
	#endif

	thread_termination_status::thread2 = true;
	ExitThread(0);
	return;
}