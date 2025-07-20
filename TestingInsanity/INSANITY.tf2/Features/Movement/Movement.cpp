#include "Movement.h"
//#include "../config.h"

// SDK
#include "../../SDK/class/Source Entity.h"
#include "../../SDK/class/CUserCmd.h"
#include "../../SDK/class/BaseWeapon.h"
#include "../../SDK/class/IInputSystem.h"
#include "../../SDK/TF object manager/TFOjectManager.h"

// UTILITY
#include "../../Utility/Insane Profiler/InsaneProfiler.h"
#include "../../Utility/CVar Handler/CVarHandler.h"
#include "../../Extra/math.h"

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
	PROFILE_FUNCTION();

	_InitializeKeyCodes();

	_Bhop(pCmd, result, pLocalPlayer);
	_RocketJump(pCmd, result, pActiveWeapon, pLocalPlayer);
	_ThirdPerson(pCmd, result, pLocalPlayer);
	_AutoStrafer(pLocalPlayer, pCmd);
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


void Movement_t::_AutoStrafer(BaseEntity* pLocalPlayer, CUserCmd* pCmd)
{
	if (Features::Movement::Movement::DirectionStrafe.IsActive() == false)
		return;

	// is in air ?
	if (pLocalPlayer->m_fFlags() & FL_ONGROUND)
		return;

	// Does user wanna strafe ?
	if (pCmd->forwardmove == 0.0f && pCmd->sidemove == 0.0f)
		return;

	vec vVelocity = pLocalPlayer->m_vecVelocity();
	vVelocity.z   = 0.0f;

	// Getting the angles where user wants to strafe to.
	qangle qFinalAngles;
	{
		vec vMoves(pCmd->forwardmove, pCmd->sidemove, 0.0f);
		qangle qMoves;
		Maths::VectorAnglesFromSDK(vMoves, qMoves);

		qMoves.yaw *= -1.0f;
		Maths::WrapYaw(qMoves);

		qFinalAngles.yaw = pCmd->viewangles.yaw + qMoves.yaw;
		Maths::WrapYaw(qFinalAngles);
	}

	// Current Velocity angles.
	qangle qVelocity;  Maths::VectorAnglesFromSDK(vVelocity, qVelocity); Maths::WrapYaw(qVelocity);
	
	// Angle from Current Velocity to "Where we wanna go" ( signed )
	float flDelta = Maths::DeltaAngle(qVelocity.yaw, qFinalAngles.yaw);
	if (fabs(flDelta) <= 5.0f)
		return;

	// Constructing Wish Direction ( 90 degrees more than velocity direction, in the side where we wanna go. )
	float flWishDirYaw = qVelocity.yaw;
	flWishDirYaw      += (flDelta >= 0.0f ? 90.0f : -90.0f) + (flDelta * Features::Movement::Movement::AutoStrafe_Agression.GetData().m_flVal);
	flWishDirYaw	   = Maths::DeltaAngle(pCmd->viewangles.yaw, flWishDirYaw);

	// Converting Wish direction to appropriate Forward & Side move.
	constexpr float flMaxMoveMagnitude = MAX_MOVE_USERCMD * 1.41421356237f;
	pCmd->forwardmove = std::clamp<float>(cosf(DEG2RAD(flWishDirYaw)) * flMaxMoveMagnitude, -450.0f, 450.0f);
	pCmd->sidemove    = std::clamp<float>(sinf(DEG2RAD(flWishDirYaw)) * flMaxMoveMagnitude, -450.0f, 450.0f) * -1.0f; // SIDE MOVE IS FUCKING INVERTED. WHO THE FUCK MADE THIS FUCKING SOURCE ENGINE! WHY DOES IT USE SO FUCKED UP ANGLES! WHY IS THE FUCKING PITCH INVERTED! WHY IS EVERYTHING OPPOSITE OF WHAT IT SHOULD BE!!!!!!!!
}



void Movement_t::_InitializeKeyCodes()
{
	if (m_bInitializedKeyCodes == true)
		return;

	m_iJumpKeyCode = I::iInputSystem->VirtualKeyForInput("+jump");

	m_bInitializedKeyCodes = true;
}
