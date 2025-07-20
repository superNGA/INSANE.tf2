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

	// Users on ground.
	if (pLocalPlayer->m_fFlags() & FL_ONGROUND)
		return;

	// User doesn't wanna move.
	if (pCmd->forwardmove == 0.0f && pCmd->sidemove == 0.0f)
		return;

	vec vVelocity = pLocalPlayer->m_vecVelocity();
	vVelocity.z   = 0.0f;

	float flSpeed = vVelocity.Length();
	if (flSpeed <= 0.0f)
		return;

	// Getting the angles where user wants to go. [ WORKING ]
	qangle qFinalAngles;
	{
		vec vMoves(pCmd->forwardmove, pCmd->sidemove, 0.0f);
		qangle qMoves;
		Maths::VectorAnglesFromSDK(vMoves, qMoves);

		qMoves.yaw *= -1.0f;
		Maths::WrapYaw(qMoves);
		//printf("DESIRED YAW : %.2f | ", qMoves.yaw);

		qFinalAngles.yaw = pCmd->viewangles.yaw + qMoves.yaw;
		Maths::WrapYaw(qFinalAngles);
	}
	I::IDebugOverlay->AddAngleOverlay(qFinalAngles, pLocalPlayer->GetAbsOrigin(), 100.0f, 0, 255, 0, 255, 1.0f);


	qangle qVelocity;
	Maths::VectorAnglesFromSDK(vVelocity, qVelocity);
	Maths::WrapYaw(qVelocity);

	// Strafing done ?
	if (fabs(Maths::DeltaAngle(qVelocity.yaw, qFinalAngles.yaw)) < 5.0f)
		return;

	// Rotating
	float flDeltaYaw	  = Maths::DeltaAngle(qVelocity.yaw, qFinalAngles.yaw);
	float flAngleStepSize = Features::Movement::Movement::AutoStrafe_Agression.GetData().m_flVal;
	float flAngleOffset   = flDeltaYaw >= 0.0f ? flAngleStepSize : -flAngleStepSize;
	qVelocity.yaw		 += flAngleOffset;
	I::IDebugOverlay->AddAngleOverlay(qVelocity, pLocalPlayer->GetAbsOrigin(), 100.0f, 255, 0, 0, 255, 1.0f);
	//printf("[ %s ] gonna add-> %.2f | angle between velocity & where we wanna go : %.2f\n", flAngleOffset >= 0.0f ? "RIGHT" : "LEFT", flAngleOffset, flDeltaYaw);
	
	// Getting Forward & side move values for desired strafe.
	{
		float flWishDirDelta = Maths::DeltaAngle(pCmd->viewangles.yaw, qVelocity.yaw);
		//printf("Angle between view & velocity + strafe : %.2f  |  ", flWishDirDelta);

		constexpr float flMaxMoveMagnitude = MAX_MOVE_USERCMD * 1.41421356237f;
		pCmd->sidemove	  = flMaxMoveMagnitude * cosf(DEG2RAD(flWishDirDelta)) * (flDeltaYaw >= 0.0f ? -1.0f : 1.0f);
		pCmd->forwardmove = flMaxMoveMagnitude * sinf(DEG2RAD(flWishDirDelta)) * (flDeltaYaw >= 0.0f ? -1.0f : 1.0f);

		//printf("SideMove : cos(%.2f) | FwdMove : sin(%.2f) | ", flWishDirDelta, flWishDirDelta);
	}

	pCmd->forwardmove = std::clamp<float>(pCmd->forwardmove, -450.0f, 450.0f);
	pCmd->sidemove	  = std::clamp<float>(pCmd->sidemove,	 -450.0f, 450.0f);

	//printf("F : %.2f , S : %.2f\n", pCmd->forwardmove, pCmd->sidemove);
}



void Movement_t::_InitializeKeyCodes()
{
	if (m_bInitializedKeyCodes == true)
		return;

	m_iJumpKeyCode = I::iInputSystem->VirtualKeyForInput("+jump");

	m_bInitializedKeyCodes = true;
}
