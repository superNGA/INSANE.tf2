//=========================================================================
//                      PROCESS MOVEMENT HOOK
//=========================================================================
// by      : INSANE
// created : 17/03/2025
// 
// purpose : This hook is primarly used to store the pMove poitner
//    as it looks like this can help us create the airMove feature.
//-------------------------------------------------------------------------
#pragma once
#include "../../SDK/class/Basic Structures.h"

struct CMoveData
{
	bool			m_bFirstRunOfFunctions : 1;
	bool			m_bGameCodeMovedPlayer : 1;

	int32_t			m_nPlayerHandle;	// this shit was orignally a EntityHandle_t but I don't give a flying shit @ a speed of 250 km/hr!

	int				m_nImpulseCommand;	// Impulse command issued.
	qangle			m_vecViewAngles;	// Command view angles (local space)
	qangle			m_vecAbsViewAngles;	// Command view angles (world space)
	int				m_nButtons;			// Attack buttons.
	int				m_nOldButtons;		// From host_client->oldbuttons;
	float			m_flForwardMove;
	float			m_flOldForwardMove;
	float			m_flSideMove;
	float			m_flUpMove;

	float			m_flMaxSpeed;
	float			m_flClientMaxSpeed;

	// Variables from the player edict (sv_player) or entvars on the client.
	// These are copied in here before calling and copied out after calling.
	vec				m_vecVelocity;		// edict::velocity		// Current movement direction.
	qangle			m_vecAngles;		// edict::angles
	qangle			m_vecOldAngles;

	// Output only
	float			m_outStepHeight;	// how much you climbed this move
	vec				m_outWishVel;		// This is where you tried 
	vec				m_outJumpVel;		// This is your jump velocity

	// Movement constraints	(radius 0 means no constraint)
	vec				m_vecConstraintCenter;
	float			m_flConstraintRadius;
	float			m_flConstraintWidth;
	float			m_flConstraintSpeedFactor;
	vec				m_vecAbsOrigin;		// edict::origin
};

namespace hook
{
    namespace processMovement
    {
        typedef void(__fastcall* T_processMovement)(void*, void*, void*);
        extern T_processMovement O_processMovement;
        void __fastcall H_processMovement(void* pVTable, void* pPlayer, CMoveData* pMove);
    }
};