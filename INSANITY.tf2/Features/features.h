#pragma once
#include "../SDK/class/CUserCmd.h"
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
			if (*(int32_t*)((uintptr_t)entities::local::active_weapon + netvar.m_iReloadMode) != 0) cmd->buttons |= IN_ATTACK;
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
		/*if (!config::aimbot::global) return;

		if (GetAsyncKeyState(VK_LBUTTON) && entities::target::found_valid_target)
		{
			cmd->viewangles = entities::target::best_angle;
			result = false;
		}*/
	}
};