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
		entities::local::pLocalPlayer.store(local_player);
		qangle viewangles_localplayer = local_player->GetAbsAngles();
		if (!local_player || local_player->getLifeState() != LIFE_ALIVE) { 
			entities::entManager.clearFlagBit(entities::C_targets::DOING_FIRST_HALF); // clearing 'DOING FIRST HALF OF THE ENTITY LIST' bit
			std::this_thread::sleep_for(std::chrono::milliseconds(500));
			continue;
		}
		netvar.local_player					= (uintptr_t)local_player; //storing as uintptr_t
		entities::local::localplayer_class.store(local_player->getCharacterChoice());
		entities::local::team_num			= *(int32_t*)(netvar.local_player + netvar.m_iTeamNum); // local players team
		entities::local::pos				= local_player->GetAbsOrigin();
		//entities::local::eye_pos.store(entities::local::pos + vec(0.0f, 0.0f, *(float*)((uintptr_t)local_player + netvar.m_vecViewOffset))); // storing local player's eyepos
		entities::local::eye_pos.store(local_player->getLocalEyePos());
		entities::local::viewAngles.store(local_player->GetAbsAngles()); // storing view angles 
		local_player->changeThirdPersonVisibility(renderGroup_t::RENDER_GROUP_VIEW_MODEL_OPAQUE);

		/* get ACTIVE WEAPON */
		baseWeapon* pActiveWeapon = local_player->getActiveWeapon();
		pActiveWeapon->setCustomTracer("merasmus_zap"); // ADDING TRACERS
		pActiveWeapon->changeThirdPersonVisibility(renderGroup_t::RENDER_GROUP_OPAQUE_ENTITY); // MAKING VISIBLE ALWAYS RENDER IN THIRD PERSON
		entities::local::active_weapon.store(pActiveWeapon);
		if (!pActiveWeapon) {
			entities::entManager.clearFlagBit(entities::C_targets::DOING_FIRST_HALF); // clearing 'DOING FIRST HALF OF THE ENTITY LIST' bit
			std::this_thread::sleep_for(std::chrono::milliseconds(500));
			continue;
		}
		entities::local::ID_active_weapon = pActiveWeapon->getWeaponIndex(); // weapon ID
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

			switch (CHE_entInfo.classID)
			{
			case ENT_RESOURCE_MANAGER:
				entities::ARR_maxHealth = (int32_t*)((uintptr_t)ent + netvar.m_iMaxHealth);
				break;

			case PLAYER:

				// skip Dead players
				if (ent->getLifeState() != LIFE_ALIVE) break;

				interface_tf2::engine->GetPlayerInfo(ent_num, &CHE_playerInfo);
				CHE_entInfo.entUserName = std::string(CHE_playerInfo.name);

				CHE_entInfo.p_ent			= ent; // entity pointer vecEntities
				CHE_glowObj.pEnt			= ent; // entity pointer allEntMap
				CHE_entInfo.pActiveWeapon	= ent->getActiveWeapon(); // active weapon pointer
				CHE_entInfo.activeWeapon	= CHE_entInfo.pActiveWeapon->getWeaponIndex();
				CHE_entInfo.charactorChoice = ent->getCharacterChoice(); // which character is this entity playing?
				CHE_entInfo.entVelocity		= ent->getEntVelocity();
				CHE_entInfo.health			= ent->getEntHealth();
				
				// checking if disguised or not
				ent->isDisguised() ?
					CHE_entInfo.setFlagBit(IS_DISGUISED) :
					CHE_entInfo.clearFlagBit(IS_DISGUISED);

				// if cloaked or not
				ent->isCloaked() ?
					CHE_entInfo.setFlagBit(IS_CLOAKED) :
					CHE_entInfo.clearFlagBit(IS_CLOAKED);

				// getting entitis max health
				if (entities::ARR_maxHealth) {
					CHE_entInfo.maxHealth = entities::ARR_maxHealth[ent_num];
				}

				CHE_entInfo.setFlagBit(IS_PLAYER); // setting IS_PLAYER bit dahh.. :)

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
				//printf("BUILDING pushing %p | index : %d\n", CHE_glowObj.pEnt, CHE_glowObj.entIndex);
				(*allEntMap)[ent_num]	= CHE_glowObj;

				break;

			case AMMO_PACK:
				CHE_glowObj.pEnt = ent; // entity pointer allEntMap
				CHE_glowObj.entIndex = ent_num;
				(*allEntMap)[ent_num] = CHE_glowObj;

				break;
			case TF_ITEM:
			case PAYLOAD:
				//printf("payload or item\n");
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