#pragma once

#include "Source Entity.h"
#include "../NetVars/NetVarHandler.h"

//----------------------- NETVARS -----------------------
// BasePlayer
NETVAR(m_lifeState, DT_BasePlayer)
NETVAR(m_iHealth,   DT_BasePlayer)
NETVAR(m_fFlags,   DT_BasePlayer)

// BaseEntity
NETVAR(m_iTeamNum,  DT_BaseEntity)

// Base Animating
NETVAR(m_flModelScale, DT_BaseAnimating)

// Local Player Exclusive
NETVAR(m_vecViewOffset, m_vecViewOffset[0], DT_LocalPlayerExclusive)
NETVAR(m_nTickBase, DT_LocalPlayerExclusive)

NETVAR(m_hActiveWeapon, DT_BaseCombatCharacter)
NETVAR(m_bGlowEnabled,  DT_BaseCombatCharacter)

NETVAR(m_PlayerClass, DT_TFPlayer)
NETVAR(m_Shared,	  DT_TFPlayer)
NETVAR(m_nForceTauntCam, DT_TFPlayer)
NETVAR(m_hItem, DT_TFPlayer)

NETVAR(m_nPlayerCond,	   DT_TFPlayerShared)
NETVAR(m_iCritMult,		   DT_TFPlayerShared)
NETVAR(m_bFeignDeathReady, DT_TFPlayerShared)

NETVAR(m_RoundScoreData, DT_TFPlayerSharedLocal)

NETVAR(m_iClass, DT_TFPlayerClassShared)

NETVAR(m_AttributeManager, DT_EconEntity)

NETVAR(m_Item, DT_AttributeContainer)
NETVAR(m_iItemDefinitionIndex, DT_ScriptCreatedItem)

class BaseEntity : public I_client_unknown, public I_client_renderable, public I_client_networkable, public I_client_thinkable
{
public:
	// Valves bullshit
	virtual void		Release(void) = 0;
	virtual const		vec& GetAbsOrigin(void) const = 0;
	virtual const		qangle& GetAbsAngles(void) const = 0;
	virtual CMouthInfo* GetMouth(void) = 0;
	virtual bool		GetSoundSpatialization(SpatializationInfo_t& info) = 0;

	// My Bullshit :)
	vec				GetEyePos() const;

	baseWeapon*		getActiveWeapon();

	NETVAR_GETTER(m_lifeState, DT_BasePlayer, lifeState_t)

	// What character is this player playing?
	player_class	getCharacterChoice();
	const char*		GetPlayerClassName();

	// returns the team num for this entity
	NETVAR_GETTER(m_iHealth, DT_BasePlayer, int32_t)
	NETVAR_GETTER(m_fFlags, DT_BasePlayer, int32_t)
	NETVAR_GETTER(m_iTeamNum, DT_BaseEntity, int32_t)
	
	// Local Player Exclusive
	NETVAR_GETTER(m_nTickBase, DT_LocalPlayerExclusive, uint32_t)

	NETVAR_SETTER(m_bGlowEnabled, DT_BaseCombatCharacter, bool)

	NETVAR_GETTER(m_nForceTauntCam, DT_TFPlayer, bool)
	NETVAR_SETTER(m_nForceTauntCam, DT_TFPlayer, bool)

	bool			IsEnemy();
	int32_t			GetWeaponIndex();

	inline bool	IsDisguised() { return InCond(TF_COND_DISGUISED); }
	inline bool	isCloaked()   { return InCond(TF_COND_STEALTHED); }

	// make visible using addToLeafSystem()
	void			changeThirdPersonVisibility(renderGroup_t renderGroup);

	uint32_t		GetPlayerCond();
	bool			InCond(FLAG_playerCond eCond);

	bool			isOnGround();

	float			GetCritMult();

	RoundStats_t*	GetPlayerRoundData();

	bool			IsCritBoosted();

	bool			IsFeignDeathReady();

	float			GetModelScale();

	// CALL_ATRIB_HOOK's
	void CALL_ATRIB_HOOK_INT(int& iAtributeOut, const char* szAtribute);
	void CALL_ATRIB_HOOK_FLOAT(float& flAtributeOut, const char* szAtribute);
};
