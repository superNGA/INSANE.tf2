#pragma once
#include <algorithm>
#include "../GlobalVars.h"
#include "../SDK/Entity Manager/entityManager.h"
#include "../SDK/class/CUserCmd.h"
#include "../SDK/class/BaseWeapon.h"
#include "../SDK/TF object manager/TFOjectManager.h"
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

	inline void airMove(CUserCmd* cmd, bool& result, BaseEntity* pLocalPlayer)
	{
		static float lastYaw = cmd->viewangles.yaw;
		if (config.miscConfig.airMove == false || pLocalPlayer->isOnGround())
		{
			lastYaw = cmd->viewangles.yaw;
			return;
		}
		
		/*
		* ASSUMPTIONS : 
		* -> view angles are the angles that makes the local player strafe and not the engine angles
		* -> This feature can and will be done in CreateMove and not in the processMovement.
		* -> the feature implementation one would find in open sources repos are not the actual implementation
		*	 and just the rebuild versions of the game's movement calculations.
		*/

		// calculate wish directions
		vec wishDir(cmd->forwardmove, cmd->sidemove, 0.0f);
		
		if (wishDir.isEmpty() == true)
			return;

		/* STEPS : 
		* 1. get the wish direction vector
		* 2. get the angle between the wish direction and the ENGINE Angles
		* 3. get the angles between the wish Directions and the velocity <- DO WE EVEN NEED THIS STEP? idk
		* 4. rotate the viewAngles until by view angles direction is equal to the wish direction
		*/

		/* NOTES : 
		* Engins angles are absolute, so 0 will always point in same direction,
		* also they are oriented the same way as the view angles.
		*/

		qangle engineAngles;
		tfObject.engine->GetViewAngles(engineAngles);
		float yaw = engineAngles.yaw;

		// Getting angle between Wish Direction and the view Angles, which at this point in the tick, 
		// should be equal to the engine Angles.
		float deltaAngleInDeg = 0.0f;
		if (wishDir.x < 0.0f)
		{
			if (wishDir.y > 0.0f)
			{
				deltaAngleInDeg = (atan(wishDir.y / -wishDir.x) + (M_PI / 2.0f)) * RAD2DEG;
			}
			else if (wishDir.y < 0.0f)
			{
				deltaAngleInDeg = (atan(wishDir.y / -wishDir.x) - (M_PI / 2.0f)) * RAD2DEG;
			}
		}
		else
		{
			deltaAngleInDeg = atan(wishDir.y / wishDir.x) * RAD2DEG;
		}
		deltaAngleInDeg *= -1.0f;

		// THE MF TURN SCALE. CHANGE AS PER REQUIREMENT!
		const static float TURN_SCALE = 5.0f;

		// if this is true, we need to turn the view angles more
		if (abs(lastYaw - engineAngles.yaw) < abs(deltaAngleInDeg))
		{
			//printf("lastYaw : %.2f | engine angles : %.2f | delta : %.2f | deltaAngle : %.2f\n", lastYaw, engineAngles.yaw, lastYaw - engineAngles.yaw, deltaAngleInDeg);
			if (deltaAngleInDeg < 0.0f)
			{
				lastYaw -= TURN_SCALE;
				cmd->sidemove = 450.0f;
			}
			else
			{
				lastYaw += TURN_SCALE;
				cmd->sidemove = -450.0f;
			}

			if (lastYaw > 180.0f)
			{
				lastYaw -= 360.0f;
			}
			else if (lastYaw < -180.0f)
			{
				lastYaw += 360.0f;
			}
			cmd->viewangles.yaw = lastYaw;
		}

		#ifdef _DEBUG
		printf("delta angles : %.2f, angle left : %.2f, Mainteined view angles : %.2f, target Angles : %.2f\n", deltaAngleInDeg, lastYaw - engineAngles.yaw, lastYaw, engineAngles.yaw + deltaAngleInDeg);
		#endif

		result	= false;
	}

	inline void autoBackStab(CUserCmd* cmd, BaseEntity* pLocalPlayer, baseWeapon* pActiveWeapon)
	{
		if (pLocalPlayer->getCharacterChoice() != TF_SPY || config.miscConfig.autoBackStab == false)
			return;

		if (pActiveWeapon->canBackStab() == false)
			return;

		cmd->buttons |= IN_ATTACK;
	}
};