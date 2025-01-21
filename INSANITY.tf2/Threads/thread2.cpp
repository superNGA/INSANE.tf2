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
		netvar.local_player = (uintptr_t)local_player; //storing as uintptr_t
		entities::local::localplayer_class = (player_class)(*(int32_t*)(netvar.local_player + netvar.m_PlayerClass + netvar.m_iClass)); // player ingame class
		entities::local::team_num = *(int32_t*)(netvar.local_player + netvar.m_iTeamNum); // local players team
		entities::local::pos = local_player->GetAbsOrigin();
		entities::local::eye_pos = entities::local::pos + vec(0.0f, 0.0f, *(float*)((uintptr_t)local_player + netvar.m_vecViewOffset)); // storing local player's eyepos

		/* get ACTIVE WEAPON */
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
		
		const view_matrix& r_viewmatrix	= interface_tf2::engine->WorldToScreenMatrix();
		const view_matrix r_anglematrix	= interface_tf2::engine->WorldToViewMatrix();
		global_var_base* p_globalvar	= interface_tf2::engine_replay->GetClientGlobalVars();
		int16_t ent_count				= interface_tf2::entity_list->NumberOfEntities(false);
		int8_t localplayer_index		= interface_tf2::engine->GetLocalPlayer();
		float last_best_distance = 10000.69f, distance_from_crosshair; // this is just some obsurdly large value, not the FOV

		/* clearing inactive buffer */
		entities::target::active_buffer_index ? 
			entities::target::entity_scrnpos_buffer_0.clear():
			entities::target::entity_scrnpos_buffer_1.clear();

		/* entity list loop here */
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

			/* getting BONES */
			if (!ent->SetupBones(skeleton_cache, MAX_STUDIO_BONES, HITBOX_BONES, p_globalvar->curtime)) continue; // skipping loop if setup bones fail?

			/* AIMBOT DATA */
			if (config::aimbot::global)
			{
				qangle target_angles = entities::world_to_viewangles(entities::local::eye_pos, skeleton_cache[entities::target::target_bone].get_bone_coordinates());
				vec2 target_screen_pos;
				entities::world_to_screen(skeleton_cache[entities::target::target_bone].get_bone_coordinates(), target_screen_pos, &r_viewmatrix);
				distance_from_crosshair = entities::vec_dis_from_screen_center(target_screen_pos);						// <- this creates a simple circle for aimbot FOV
				//distance_from_crosshair = entities::distance_from_crosshair(viewangles_localplayer, target_angles);	// <- this creates a cone for aimbot FOV
				if (distance_from_crosshair < last_best_distance) // storing closed target angles
				{
					entities::target::best_angle = target_angles;
					last_best_distance = distance_from_crosshair;
				}
			}

			/* if ESP ENABLED */
			if (config::visuals::ESP)
			{
				int8_t ent_on_screen = 0;
				ent_on_screen += entities::world_to_screen(skeleton_cache[BONE_HEAD].get_bone_coordinates(), cached_entity_dimension.head, &r_viewmatrix);
				ent_on_screen += entities::world_to_screen(skeleton_cache[BONE_LEFT_SHOULDER].get_bone_coordinates(), cached_entity_dimension.left_shoulder, &r_viewmatrix);
				ent_on_screen += entities::world_to_screen(skeleton_cache[BONE_RIGHT_SHOULDER].get_bone_coordinates(), cached_entity_dimension.right_shoulder, &r_viewmatrix);
				ent_on_screen += entities::world_to_screen(skeleton_cache[BONE_LEFT_FOOT].get_bone_coordinates(), cached_entity_dimension.left_foot, &r_viewmatrix);
				ent_on_screen += entities::world_to_screen(skeleton_cache[BONE_RIGHT_FOOT].get_bone_coordinates(), cached_entity_dimension.right_foot, &r_viewmatrix);
				if (ent_on_screen)
				{
					entities::target::active_buffer_index ?
						entities::target::entity_scrnpos_buffer_0.push_back(cached_entity_dimension) :
						entities::target::entity_scrnpos_buffer_1.push_back(cached_entity_dimension);
				}
			}
		}
		/* FOV check */
		last_best_distance <= config::aimbot::FOV ?
			entities::target::found_valid_target = true :
			entities::target::found_valid_target = false;

		if (!entities::target::buffer_locked) entities::target::active_buffer_index = !entities::target::active_buffer_index; // buffer swap if not locked

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