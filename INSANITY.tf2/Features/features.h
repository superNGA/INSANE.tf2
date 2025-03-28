#pragma once
#include "../GlobalVars.h"
#include "../SDK/Entity Manager/entityManager.h"
#include "../SDK/class/CUserCmd.h"
#include "../SDK/class/BaseWeapon.h"
#include "config.h"
#include "../Libraries/Utility/Utility.h"

extern local_netvars netvar;

#define SPACEBAR_STATE (1<<0)

namespace feature
{
	/*This bhop feature might seem badly constructed but this allows for double jumping when 
	playing as scout.*/
	inline void bhop(CUserCmd* cmd, uint8_t& bits)
	{
		if (!config.miscConfig.bhop) return;

		int32_t flag = *(int32_t*)((uintptr_t)entityManager.getLocalPlayer() + netvar.m_fFlags);

		if (!GetAsyncKeyState(VK_SPACE))
		{
			/* if in air and NOT holding space bar */
			if (!(flag & (1 << 0))) {
				bits &= ~SPACEBAR_STATE;
			}
			return;
		}

		if (flag & (1 << 0))
		{
			cmd->buttons |= IN_JUMP;
		}
		else if (!(bits & SPACEBAR_STATE))
		{
			bits |= SPACEBAR_STATE;
			cmd->buttons |= IN_JUMP;
		}
		else
		{
			cmd->buttons &= ~(IN_JUMP);
		}
	}


	/* I shall inprove upon this feature soon */
	inline void rocket_jump(CUserCmd* cmd, bool& result)
	{
		if (!config.miscConfig.rocket_jump) return;

		static bool isRocketJumping = false;
		static int rocketJumpStage = 0;
		if (GetAsyncKeyState(VK_XBUTTON2)) { // Hotkey for rocket jump
			if (entityManager.getActiveWeapon()->getReloadMode() != reload_t::WPN_RELOAD_START) cmd->buttons |= IN_ATTACK;
			if (!isRocketJumping) {
				isRocketJumping = true;
				rocketJumpStage = 0;
			}
		}
		if (isRocketJumping) {
			switch (rocketJumpStage) {
			case 0: // Adjust view angles
				cmd->buttons |= IN_JUMP;      // Jump
				rocketJumpStage++;
				break;

			case 1: // Duck after jumping
				cmd->buttons |= IN_DUCK;
				rocketJumpStage++;
				break;

			case 3: // Fire the rocket
				//cmd->viewangles.pitch = 40.0f; // Aim straight down
				cmd->viewangles.yaw += 180.0f;  // Keep yaw unchanged
				cmd->buttons |= IN_ATTACK;
				isRocketJumping = false; // Reset state after firing
				result = false;
				break;
			default:
				rocketJumpStage++;
				break;
			}
		}
	}


	/* this is a very basic third person mechanism */
	inline void third_person()
	{
		if (config.miscConfig.third_person == false)
		{
			return;
		}

		uintptr_t forceTauntCamState = (uintptr_t)entityManager.getLocalPlayer() + netvar.m_nForceTauntCam;
		bool thirdperson_state = *(bool*)(forceTauntCamState);
		
		if (thirdperson_state != input_util::key_detect(VK_XBUTTON1, true))
		{
			*(bool*)(forceTauntCamState) = !thirdperson_state;
		}
	}

	inline void aimbot(CUserCmd* cmd, bool& result)
	{
		if (!config.aimbotConfig.global) return;
		if (!entities::shouldDoAimbot.load()) return;
		if (entities::local::active_weapon.load()->getSlot() == slot_t::WPN_SLOT_MELLE) return;

		// AUTO SHOOT
		if (config.aimbotConfig.autoShoot) {

			// RELOAD MODE CHECK
			reload_t reloadMode = entities::local::active_weapon.load()->getReloadMode();
			if (reloadMode != reload_t::WPN_RELOAD_START &&
				reloadMode != reload_t::WPN_RELOAD_FINISH) {
				return;
			}

			// SNIPE AUTO SHOOT
			if (entities::local::localplayer_class == TF_SNIPER) {
				if (entities::local::pLocalPlayer.load()->getPlayerCond() & TF_COND_ZOOMED) {
					cmd->buttons |= IN_ATTACK;
				}
			}
			else { // OTHER CLASSES AUTO SHOOT
				cmd->buttons |= IN_ATTACK;
			}

			// setting ANGLES
			cmd->viewangles = entities::aimbotTargetAngles.load();
			result = false;
		}
		// NORMAL AIMBOT
		else if (GetAsyncKeyState(VK_LBUTTON)) {
			
			// setting ANGLES
			cmd->viewangles = entities::aimbotTargetAngles.load();
			result = false;
		}
	}


};