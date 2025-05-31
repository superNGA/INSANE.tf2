#pragma once

#include "Basic Structures.h"

class CMoveData
{
public:
	bool			m_bFirstRunOfFunctions : 1;
	bool			m_bGameCodeMovedPlayer : 1;

	// ( This was originally EntityHandle_t, but 4 bytes is 4 bytes )
	int				m_nPlayerHandle;	// edict index on server, client entity handle on client

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

	inline void		SetAbsOrigin(vec& vec) { m_vecAbsOrigin = vec; }
	inline const	vec& GetAbsOrigin() { return m_vecAbsOrigin; }

	vec			m_vecAbsOrigin;		// edict::origin
};