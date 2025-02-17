#pragma once
#include "../GlobalVars.h"
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
		if (!config::miscellaneous::bhop) return;

		if (!global::entities_popullated) return;
		int32_t flag = *(int32_t*)(netvar.local_player + netvar.m_fFlags);

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
		if (!config::miscellaneous::rocket_jump) return;
		if (!global::entities_popullated) return;

		static bool isRocketJumping = false;
		static int rocketJumpStage = 0;
		if (GetAsyncKeyState(VK_XBUTTON2)) { // Hotkey for rocket jump
			if (entities::local::active_weapon.load()->getReloadMode() != reload_t::WPN_RELOAD_START) cmd->buttons |= IN_ATTACK;
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
		if (!config::miscellaneous::third_person) return;

		bool thirdperson_state = *(bool*)(netvar.local_player + netvar.m_nForceTauntCam);
		if (thirdperson_state != input_util::key_detect(VK_XBUTTON1, true)) *(bool*)(netvar.local_player + netvar.m_nForceTauntCam) = !thirdperson_state;
	}

	inline void aimbot(CUserCmd* cmd, bool& result)
	{
		if (!config::aimbot::global) return;
		if (!entities::shouldDoAimbot.load()) return;

		if (GetAsyncKeyState(VK_LBUTTON)) {

			// skip if MELLE
			if (entities::local::active_weapon.load()->getSlot() == slot_t::WPN_SLOT_MELLE) return;

			cmd->viewangles = entities::aimbotTargetAngles.load();
			result = false;
		}
	}
};