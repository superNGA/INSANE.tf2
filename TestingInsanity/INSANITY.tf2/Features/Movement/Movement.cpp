#include "Movement.h"
#include "../config.h"

#include "../../Extra/math.h"

// SDK
#include "../../SDK/class/Source Entity.h"
#include "../../SDK/class/CUserCmd.h"
#include "../../SDK/class/BaseWeapon.h"
#include "../../SDK/class/IInputSystem.h"

#define SPACEBAR_STATE (1 << 0)

Movement_t::Movement_t()
{
	m_bInitializedKeyCodes = false;
	m_iJumpKeyCode		   = 0;

	Reset();
}

//=========================================================================
//                     PUBLIC METHODS
//=========================================================================
void Movement_t::Run(CUserCmd* pCmd, bool& result, BaseEntity* pLocalPlayer, baseWeapon* pActiveWeapon)
{
	_InitializeKeyCodes();

	_Bhop(pCmd, result, pLocalPlayer);
	_RocketJump(pCmd, result, pActiveWeapon, pLocalPlayer);
	_ThirdPerson(pCmd, result, pLocalPlayer);
}


void Movement_t::Reset()
{
	m_bLastBhopState = false;
}


//=========================================================================
//                     PRIVATE METHODS
//=========================================================================
void Movement_t::_Bhop(CUserCmd* pCmd, bool& result, BaseEntity* pLocalPlayer)
{
	// Feature so simple, yet I am not capable enough to make the same for CS2 :(.
	bool bActive		  = Features::Movement::Movement::Bhop.IsActive();
	bool bOnGround		  = pLocalPlayer->m_fFlags() & (1 << 0);
	bool bKeysOverlapping = (m_iJumpKeyCode == Features::Movement::Movement::Bhop.m_iKey);

	if (bOnGround == true && bActive == true)
		pCmd->buttons |= IN_JUMP;

	// If keybind for jump & Bhop are same, then disable jumping Mid-Air ( double jumping ) until
	// user releases the jump key.
	if (bOnGround == false && bKeysOverlapping == true && m_bLastBhopState == bActive)
	{
		pCmd->buttons &= ~IN_JUMP;
	}

	// Store this bhop state
	m_bLastBhopState = bActive;
}


void Movement_t::_RocketJump(CUserCmd* pCmd, bool& result, baseWeapon* pActiveWeapon, BaseEntity* pLocalPlayer)
{
	static int iRocketJumpingStage = 0;
	if (Features::Movement::Movement::AutoRocketJump.IsActive() == false)
	{
		iRocketJumpingStage = 0;
		return;
	}

	// Can Rocket jump?
	if (pLocalPlayer->m_iClass() != TF_SOLDIER || pActiveWeapon->getSlot() != WPN_SLOT_PRIMARY)
		return;

	// If Got no ammo, then return
	if (pActiveWeapon->m_iClip1() <= 0)
		return;

	switch (iRocketJumpingStage)
	{
	case 0 :
		pCmd->buttons |= IN_JUMP;
		pCmd->buttons |= IN_DUCK;
		++iRocketJumpingStage;
		// Canceling reload
		if (pActiveWeapon->m_iReloadMode() != reload_t::WPN_RELOAD_START)
			pCmd->buttons |= IN_ATTACK;
		break;

	case 1 :
		pCmd->viewangles.yaw += 180.0f;
		Maths::ClampQAngle(pCmd->viewangles);
		pCmd->buttons |= IN_ATTACK;
		++iRocketJumpingStage;
		result = false;
		break;

	default:
		break;
	}
}


void Movement_t::_ThirdPerson(CUserCmd* pCmd, bool& result, BaseEntity* pLocalPlayer)
{
	pLocalPlayer->m_nForceTauntCam(Features::Movement::Movement::ThirdPerson.IsActive());
}

void Movement_t::_InitializeKeyCodes()
{
	if (m_bInitializedKeyCodes == true)
		return;

	m_iJumpKeyCode = I::iInputSystem->VirtualKeyForInput("+jump");

	m_bInitializedKeyCodes = true;
}
