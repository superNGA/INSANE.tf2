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

	while (!directX::UI::UI_has_been_shutdown)
	{
		std::this_thread::sleep_for(std::chrono::milliseconds(10)); // This thread must sleep for this much in each iteration.

		/* check if in game */
		if (!interface_tf2::engine->IsInGame()) {
			entities::entManager.clearFlagBit(entities::C_targets::DOING_FIRST_HALF); // clearing 'DOING FIRST HALF OF THE ENTITY LIST' bit
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

		// PURPOSE : temporary cache vector for all valid which we shift into global storage after 
		std::vector<entInfo_t> CHE_vecEntities; 
		CHE_vecEntities.clear();

		// PURPOSE : this map stores some imformation about all entities if they are even slightly useful
		entities::allEntManager_t::allEntMap* allEntMap = entities::allEntManager.getWriteBuffer();

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

			// Temp entInfo & glowObject_t object
			entInfo_t		CHE_entInfo;
			glowObject_t	CHE_glowObj;
			
			CHE_entInfo.classID = entities::IDManager.getID(ent);
			CHE_glowObj.classID = CHE_entInfo.classID;
			
			//===================================== FILLING INFO ABOUT VAILD ENTITIES ====================================
			
			// PROTOTYPING
			//switch (CHE_entInfo.classID)
			//{
			//	// heighest priority entities
			//case PLAYER:
			//	interface_tf2::engine->GetPlayerInfo(ent_num, &CHE_playerInfo);
			//	cons.Log(FG_GREEN, "THREAD 2", "Processing player : %s", CHE_playerInfo.name);
			//	CHE_entInfo.p_ent = ent;
			//	CHE_entInfo.setFlagBit(IS_PLAYER);
			//	CHE_entInfo.entUserName		= std::string(CHE_playerInfo.name);

			//	// DEAD ?
			//	if (*(int16_t*)((uintptr_t)ent + netvar.m_lifeState) != 0) { // DEAD check for Player & Building
			//		break;
			//	}

			//	// getting ent + 0XD10
			//	CHE_entInfo.activeWeapon	= *(int32_t*)((uintptr_t)(interface_tf2::entity_list->GetClientEntity(ent->get_active_weapon_handle())) + netvar.m_AttributeManager + netvar.m_Item + netvar.m_iItemDefinitionIndex);
			//	CHE_entInfo.charactorChoice = (player_class)(*(int32_t*)((uintptr_t)ent + netvar.m_PlayerClass + netvar.m_iClass));
			//	CHE_entInfo.entVelocity		= ent->getEntVelocity();

			//	*(int32_t*)((uintptr_t)ent + netvar.m_fFlags)& MF_ONGROUND ?
			//		CHE_entInfo.setFlagBit(ENT_ON_GROUND) :
			//		CHE_entInfo.clearFlagBit(ENT_ON_GROUND);

			//	if (entities::local::b_hasProjectileWeapon) {
			//		CHE_entInfo.getFlagBit(ENT_ON_GROUND) ?
			//			CHE_entInfo.targetBoneID = LEFT_FOOT :	// if doing projectile aimbot, and entity on ground, then shoot at foot
			//			CHE_entInfo.targetBoneID = CHEST;		// else if in air, shoot at chest
			//	}
			//	else { // if using hit-scan weapon 
			//		CHE_entInfo.targetBoneID = HEAD;			// shoot at head
			//	}
			//	
			//	CHE_entInfo.infoBoneID = entities::boneManager.getBone(&CHE_entInfo, CHE_entInfo.charactorChoice); // return boneInfo pointer, and cache it if not already

			//	// 2nd heighest priority entities 
			//case SENTRY_GUN:
			//case TELEPORTER:
			//case DISPENSER:
			//	if (!CHE_entInfo.getFlagBit(IS_PLAYER)) printf("Processing a building\n");
			//	CHE_entInfo.p_ent = ent;
			//	CHE_entInfo.setFlagBit(IS_BUILDING);

			//	// storing Entity index
			//	CHE_entInfo.entIndex = ent_num;

			//	// DEAD ?
			//	if (*(int16_t*)((uintptr_t)ent + netvar.m_lifeState) != 0) { // DEAD check for Player & Building
			//		break;
			//	}

			//	// FRENDLY ENTITY ?
			//	*(int16_t*)((uintptr_t)ent + netvar.m_iTeamNum) == entities::local::team_num ?
			//		CHE_entInfo.setFlagBit(FRENDLY) :
			//		CHE_entInfo.clearFlagBit(FRENDLY);

			//	// Pushing PLAYERS & Buildings
			//	CHE_vecEntities.push_back(CHE_entInfo);

			//case PAYLOAD:
			//case TF_ITEM:
			//	if (!CHE_entInfo.getFlagBit(IS_BUILDING)) printf("Processing a peace of shit\n");
			//	CHE_entInfo.entPos	= ent->GetAbsOrigin();;
			//	CHE_entInfo.p_ent	= ent;
			//	
			//	// managing glow Object
			//	CHE_glowObj.pEnt			= ent;
			//	CHE_glowObj.classID			= CHE_entInfo.classID;
			//	CHE_glowObj.entIndex		= ent_num;
			//	CHE_glowObj.isFrendly		= CHE_entInfo.getFlagBit(FRENDLY) ? true : false;

			//	//MAP_allEntities[ent_num]	= CHE_glowObj;
			//	
			//	break;
			//
			//default:
			//	break;
			//}

			switch (CHE_entInfo.classID)
			{
			case PLAYER:
				// skip Dead players
				if (ent->getLifeState() != LIFE_ALIVE) break;

				interface_tf2::engine->GetPlayerInfo(ent_num, &CHE_playerInfo);
				CHE_entInfo.entUserName = std::string(CHE_playerInfo.name);

				CHE_entInfo.p_ent			= ent; // entity pointer vecEntities
				CHE_glowObj.pEnt			= ent; // entity pointer allEntMap
				CHE_entInfo.pActiveWeapon	= interface_tf2::entity_list->GetClientEntity(ent->get_active_weapon_handle()); // active weapon pointer
				CHE_entInfo.activeWeapon	= *(int32_t*)((uintptr_t)CHE_entInfo.pActiveWeapon + netvar.m_AttributeManager + netvar.m_Item + netvar.m_iItemDefinitionIndex); // active weapon's weapon ID
				CHE_entInfo.charactorChoice = ent->getCharacterChoice(); // which character is this entity playing?
				CHE_entInfo.setFlagBit(IS_PLAYER); // setting IS_PLAYER bit dahh.. :)

				CHE_entInfo.entVelocity		= ent->getEntVelocity();

				// is on ground or not? useful for projectile aimbot and shit
				*(int32_t*)((uintptr_t)ent + netvar.m_fFlags) & MF_ONGROUND ?
					CHE_entInfo.setFlagBit(ENT_ON_GROUND) :
					CHE_entInfo.clearFlagBit(ENT_ON_GROUND);

				// getting bone info pointer for this entity if cached else cache then return
				CHE_entInfo.infoBoneID = entities::boneManager.getBone(&CHE_entInfo, CHE_entInfo.charactorChoice);

				// Choosing correct bone (to be aimed at by the aimbot) for this entity, depending on our weapon and their m_fFlags
				if (entities::local::b_hasProjectileWeapon) {
					CHE_entInfo.getFlagBit(ENT_ON_GROUND) ?
						CHE_entInfo.targetBoneID	= LEFT_FOOT :	// if doing projectile aimbot, and entity on ground, then shoot at foot
						CHE_entInfo.targetBoneID	= CHEST;		// else if in air, shoot at chest
				}
				else { // if using hit-scan weapon 
					CHE_entInfo.targetBoneID		= HEAD;			// shoot at head
				}

				// is this entity friend or enemy
				if (ent->getTeamNum() == entities::local::team_num) {
					CHE_entInfo.setFlagBit(FRENDLY);
					CHE_glowObj.isFrendly = true;
				}
				else {
					CHE_entInfo.clearFlagBit(FRENDLY);
					CHE_glowObj.isFrendly = false;
				}

				// storing entity index for use in places like glowManager and shit...
				CHE_entInfo.entIndex = ent_num;
				CHE_glowObj.entIndex = ent_num;

				// finally pushing it in the vector & MAP
				CHE_vecEntities.push_back(CHE_entInfo);
				(*allEntMap)[ent_num] = CHE_glowObj;

				break;

			case DISPENSER:
			case SENTRY_GUN:
			case TELEPORTER:

				CHE_glowObj.pEnt = ent; // entity pointer allEntMap

				// is this entity friend or enemy
				if (ent->getTeamNum() == entities::local::team_num) {
					CHE_entInfo.setFlagBit(FRENDLY);
					CHE_glowObj.isFrendly = true;
				}
				else {
					CHE_entInfo.clearFlagBit(FRENDLY);
					CHE_glowObj.isFrendly = false;
				}
				CHE_glowObj.entIndex	= ent_num; 
				(*allEntMap)[ent_num]	= CHE_glowObj;

				break;

			case AMMO_PACK:

				break;
			case TF_ITEM:
			case PAYLOAD:

				break;
			default:
				break;
			}
		}

		entities::entManager.update_vecEntities(CHE_vecEntities); // updating global filtered entity list
		entities::entManager.setFlagBit(entities::C_targets::DOING_FIRST_HALF);
		entities::allEntManager.sendBack(false);

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