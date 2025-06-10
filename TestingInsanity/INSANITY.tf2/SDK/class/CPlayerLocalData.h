#pragma once

#include "Basic Structures.h"

static constexpr int MAX_AREA_STATE_BYTES		 = 32;
static constexpr int MAX_AREA_PORTAL_STATE_BYTES = 24;

class CPlayerLocalData
{
public:
	void*					pVTable;
	unsigned char			m_chAreaBits[MAX_AREA_STATE_BYTES];				// Area visibility flags.
	unsigned char			m_chAreaPortalBits[MAX_AREA_PORTAL_STATE_BYTES];// Area portal visibility flags.

	int						m_iHideHUD;			// bitfields containing sections of the HUD to hide

	float					m_flFOVRate;		// rate at which the FOV changes


	bool					m_bDucked;
	bool					m_bDucking;
	bool					m_bInDuckJump;
	float					m_flDucktime;
	float					m_flDuckJumpTime;
	float					m_flJumpTime;
	int						m_nStepside;
	float					m_flFallVelocity;
	int						m_nOldButtons;
	float					m_flOldForwardMove;

	vec						m_vecClientBaseVelocity;
	qangle	                m_vecPunchAngle;		// auto-decaying view angle adjustment
	qangle	                m_iv_vecPunchAngle;

	qangle				    m_vecPunchAngleVel;		// velocity of auto-decaying view angle adjustment
	qangle				    m_iv_vecPunchAngleVel;
	bool					m_bDrawViewmodel;
	bool					m_bWearingSuit;
	bool					m_bPoisoned;
	float					m_flStepSize;
	bool					m_bAllowAutoMovement;

	// Don't need these for now.
	// 
	//sky3dparams_t			m_skybox3d;
	//fogplayerparams_t		m_PlayerFog;
	//audioparams_t			m_audio;
	//bool					m_bSlowMovement;
};