#include "Movement.h"
#include "../config.h"

#include "../../SDK/class/Source Entity.h"
#include "../../SDK/class/CUserCmd.h"
#include "../../SDK/class/BaseWeapon.h"
#include "../../SDK/offsets/offsets.h"

extern local_netvars netvar;


#define SPACEBAR_STATE (1 << 0)


//=========================================================================
//                     PUBLIC METHODS
//=========================================================================
void Movement_t::Run(CUserCmd* pCmd, bool& result, BaseEntity* pLocalPlayer, baseWeapon* pActiveWeapon)
{
	_Bhop(pCmd, result, pLocalPlayer);
	_RocketJump(pCmd, result, pActiveWeapon);
	_ThirdPerson(pCmd, result, pLocalPlayer);
}


//=========================================================================
//                     PRIVATE METHODS
//=========================================================================
void Movement_t::_Bhop(CUserCmd* pCmd, bool& result, BaseEntity* pLocalPlayer)
{
	static uint8_t bits = 0;

	if (TempFeatureHelper::Bhop.IsActive() == false) 
		return;

	int32_t flag = *(int32_t*)((uintptr_t)pLocalPlayer + netvar.m_fFlags);

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
		pCmd->buttons |= IN_JUMP;
	}
	else if (!(bits & SPACEBAR_STATE))
	{
		bits |= SPACEBAR_STATE;
		pCmd->buttons |= IN_JUMP;
	}
	else
	{
		pCmd->buttons &= ~(IN_JUMP);
	}
}


void Movement_t::_RocketJump(CUserCmd* pCmd, bool& result, baseWeapon* pActiveWeapon)
{
	if (TempFeatureHelper::AutoRocketJump.IsActive() == false)
		return;

	static bool isRocketJumping = false;
	static int rocketJumpStage = 0;
	if (TempFeatureHelper::AutoRocketJump.IsActive() == true) { // Hotkey for rocket jump
		if (pActiveWeapon->getReloadMode() != reload_t::WPN_RELOAD_START) pCmd->buttons |= IN_ATTACK;
		if (!isRocketJumping) {
			isRocketJumping = true;
			rocketJumpStage = 0;
		}
	}
	if (isRocketJumping) {
		switch (rocketJumpStage) {
		case 0: // Adjust view angles
			pCmd->buttons |= IN_JUMP;      // Jump
			rocketJumpStage++;
			break;

		case 1: // Duck after jumping
			pCmd->buttons |= IN_DUCK;
			rocketJumpStage++;
			break;

		case 3: // Fire the rocket
			//cmd->viewangles.pitch = 40.0f; // Aim straight down
			pCmd->viewangles.yaw += 180.0f;  // Keep yaw unchanged
			pCmd->buttons |= IN_ATTACK;
			isRocketJumping = false; // Reset state after firing
			result = false;
			break;
		default:
			rocketJumpStage++;
			break;
		}
	}
}


void Movement_t::_ThirdPerson(CUserCmd* pCmd, bool& result, BaseEntity* pLocalPlayer)
{
	*reinterpret_cast<bool*>(reinterpret_cast<uintptr_t>(pLocalPlayer) + netvar.m_nForceTauntCam) = TempFeatureHelper::ThirdPerson.IsActive();
}
