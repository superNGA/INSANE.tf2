#pragma once

#include "Source Entity.h"
#include "../NetVars/NetVarHandler.h"

class CUserCmd;

//----------------------- NETVARS -----------------------
// BasePlayer
NETVAR(m_lifeState,				 DT_BasePlayer)
NETVAR(m_iHealth,				 DT_BasePlayer)
NETVAR(m_fFlags,				 DT_BasePlayer)
NETVAR(m_flMaxspeed,			 DT_BasePlayer)
NETVAR(m_iFOV,					 DT_BasePlayer)
								 
NETVAR(m_nHitboxSet,			 DT_BaseAnimating)
NETVAR(m_nSequence,				DT_BaseAnimating)
								 
// BaseEntity					 
NETVAR(m_iTeamNum,				 DT_BaseEntity)
NETVAR(m_vecOrigin,				 DT_BaseEntity)
NETVAR(m_hOwnerEntity,			 DT_BaseEntity)
NETVAR(m_hEffectEntity,			 DT_BaseEntity)
NETVAR(m_angRotation,			 DT_BaseEntity)
NETVAR(m_flSimulationTime,		 DT_BaseEntity)
NETVAR(m_fEffects,				 DT_BaseEntity)
								 
// Base Animating				 
NETVAR(m_flModelScale,			 DT_BaseAnimating)

// Local Player Exclusive
NETVAR(m_vecViewOffset,			 m_vecViewOffset[0], DT_LocalPlayerExclusive)
NETVAR(m_angEyeAngles, m_angEyeAngles[0], DT_TFLocalPlayerExclusive)
NETVAR(m_nTickBase,				 DT_LocalPlayerExclusive)
NETVAR(m_Local,					 DT_LocalPlayerExclusive)
NETVAR(m_hGroundEntity,			 DT_LocalPlayerExclusive)

NETVAR(m_flNextAttack,			 DT_BCCLocalPlayerExclusive)

NETVAR(m_iObjectType,			 DT_BaseObject)
NETVAR(m_bBuilding,				 DT_BaseObject)

NETVAR(m_bDucked,				 DT_Local)
NETVAR(m_bDucking,				 DT_Local)
NETVAR(m_bInDuckJump,			 DT_Local)
NETVAR(m_flDucktime,			 DT_Local)
NETVAR(m_flDuckJumpTime,		 DT_Local)
NETVAR(m_flJumpTime,			 DT_Local)

NETVAR(m_hActiveWeapon,          DT_BaseCombatCharacter)
NETVAR(m_bGlowEnabled,           DT_BaseCombatCharacter)
						         
NETVAR(m_PlayerClass,            DT_TFPlayer)
NETVAR(m_Shared,	             DT_TFPlayer)
NETVAR(m_nForceTauntCam,         DT_TFPlayer)
NETVAR(m_hItem,			         DT_TFPlayer)

NETVAR(m_nPlayerCond,	         DT_TFPlayerShared)
NETVAR(m_iCritMult,		         DT_TFPlayerShared)
NETVAR(m_bFeignDeathReady,       DT_TFPlayerShared)
NETVAR(m_iAirDash,		         DT_TFPlayerShared)

NETVAR(m_bTouched,				 DT_TFProjectile_Pipebomb)
NETVAR(m_iType,					 DT_TFProjectile_Pipebomb)
NETVAR(m_bIsLive,			     DT_BaseGrenade)

NETVAR(m_RoundScoreData,	     DT_TFPlayerSharedLocal)
							     
NETVAR(m_iClass,			     DT_TFPlayerClassShared)
							     
NETVAR(m_AttributeManager,	     DT_EconEntity)
							     
NETVAR(m_Item,				     DT_AttributeContainer)
NETVAR(m_iItemDefinitionIndex,   DT_ScriptCreatedItem)

NETVAR(m_flAnimTime,			 DT_AnimTimeMustBeFirst)
NETVAR(m_flCycle, DT_ServerAnimationData)

// Offset Netvars
NETVAR_OFFSET(m_vecAbsVelocity,  m_Collision,		DT_BaseEntity,		-120)
NETVAR_OFFSET(m_vecVelocity,	 m_Collision,		DT_BaseEntity,		-120 -80 -12 -4)
NETVAR_OFFSET(m_pCurrentCommand, m_flMaxspeed,		DT_BasePlayer,		-0x60)
NETVAR_OFFSET(m_RefEHandle,		 m_PredictableID,	DT_PredictableId,	0x18)
NETVAR_OFFSET(m_hRender,		 m_nRenderMode,		DT_BaseEntity,		0x2)
NETVAR_OFFSET(m_vecAbsOrigin,    m_flElasticity,	DT_BaseEntity,		0x18)
NETVAR_OFFSET(m_angAbsAngles,    m_flElasticity,	DT_BaseEntity,		0x24)

class BaseEntity : public I_client_unknown, public I_client_renderable, public I_client_networkable, public I_client_thinkable
{
public:
	virtual void		Release(void) = 0;
	virtual vec&		GetAbsOrigin(void) const = 0;
	virtual qangle&		GetAbsAngles(void) const = 0;
	virtual CMouthInfo* GetMouth(void) = 0;
	virtual bool		GetSoundSpatialization(SpatializationInfo_t& info) = 0;

	vec				GetEyePos() const;

	baseWeapon*		getActiveWeapon();

	NETVAR_GETTER(m_lifeState, DT_BasePlayer, lifeState_t)

	// What character is this player playing?
	NETVAR_GETTER(m_iClass, DT_TFPlayerClassShared, player_class)
	const char*		GetPlayerClassName();

	NETVAR_GETTER(m_iHealth, DT_BasePlayer, int32_t)
	NETVAR_GETTER(m_fFlags, DT_BasePlayer, int32_t)
	NETVAR_SETTER(m_fFlags, DT_BasePlayer, int32_t)
	NETVAR_GETTER(m_iFOV, DT_BasePlayer,   int32_t)
	
	NETVAR_GETTER(m_flAnimTime, DT_AnimTimeMustBeFirst, float)
	NETVAR_SETTER(m_flAnimTime, DT_AnimTimeMustBeFirst, float)
	NETVAR_GETTER(m_flCycle, DT_ServerAnimationData, float)
	NETVAR_SETTER(m_flCycle, DT_ServerAnimationData, float)
	
	NETVAR_GETTER(m_iType,    DT_TFProjectile_Pipebomb, int)
	NETVAR_GETTER(m_bTouched, DT_TFProjectile_Pipebomb, bool)
	NETVAR_GETTER(m_bIsLive,  DT_BaseGrenade, bool)
	
	NETVAR_GETTER(m_iObjectType, DT_BaseObject, int)
	NETVAR_GETTER(m_bBuilding, DT_BaseObject,	bool)
	
	
	NETVAR_GETTER(m_vecOrigin, DT_BaseEntity, vec)
	NETVAR_SETTER(m_vecOrigin, DT_BaseEntity, vec)
	NETVAR_GETTER(m_iTeamNum, DT_BaseEntity, int32_t)

	NETVAR_GETTER(m_hRender, DT_BaseEntity, unsigned short)
	
	NETVAR_GETTER(m_vecAbsOrigin, DT_BaseEntity, vec)
	NETVAR_SETTER(m_vecAbsOrigin, DT_BaseEntity, vec)
	
	NETVAR_GETTER(m_angAbsAngles, DT_BaseEntity, qangle)
	NETVAR_SETTER(m_angAbsAngles, DT_BaseEntity, qangle)

	// Simulation Time
	NETVAR_GETTER(m_flSimulationTime, DT_BaseEntity, float)
	NETVAR_SETTER(m_flSimulationTime, DT_BaseEntity, float)

	NETVAR_GETTER(m_flNextAttack, DT_BCCLocalPlayerExclusive, float)
	
	NETVAR_GETTER(m_nTickBase, DT_LocalPlayerExclusive, uint32_t)
	NETVAR_SETTER(m_nTickBase, DT_LocalPlayerExclusive, uint32_t)

	// Ground Entity
	NETVAR_GETTER(m_hGroundEntity, DT_LocalPlayerExclusive, uint32_t)
	NETVAR_SETTER(m_hGroundEntity, DT_LocalPlayerExclusive, uint32_t)

	NETVAR_SETTER(m_bGlowEnabled, DT_BaseCombatCharacter, bool)

	NETVAR_GETTER(m_nForceTauntCam, DT_TFPlayer, bool)
	NETVAR_SETTER(m_nForceTauntCam, DT_TFPlayer, bool)

	NETVAR_GETTER(m_vecAbsVelocity, DT_BaseEntity, vec)
	NETVAR_GETTER(m_vecVelocity, DT_BaseEntity, vec)
	NETVAR_SETTER(m_vecVelocity, DT_BaseEntity, vec)
	NETVAR_GETTER(m_angRotation, DT_BaseEntity, qangle)
	NETVAR_GETTER(m_fEffects,	 DT_BaseEntity, int)

	NETVAR_GETTER(m_vecViewOffset, DT_LocalPlayerExclusive, vec)
	NETVAR_SETTER(m_vecViewOffset, DT_LocalPlayerExclusive, vec)
	
	NETVAR_GETTER(m_angEyeAngles, DT_TFLocalPlayerExclusive, qangle)

	NETVAR_GETTER(m_RefEHandle, DT_PredictableId, int)
	NETVAR_GETTER(m_hOwnerEntity, DT_BaseEntity, int)
	NETVAR_GETTER(m_hEffectEntity, DT_BaseEntity, int)

	// CMD
	NETVAR_GETTER(m_pCurrentCommand, DT_BasePlayer, CUserCmd*)
	NETVAR_SETTER(m_pCurrentCommand, DT_BasePlayer, CUserCmd*)

	// CPlayerLocalData
	NETVAR_GETTER(m_flJumpTime,     DT_Local, float)
	NETVAR_GETTER(m_flDuckJumpTime, DT_Local, float)
	NETVAR_GETTER(m_flDucktime,		DT_Local, float)
	NETVAR_GETTER(m_bDucked,		DT_Local, bool)
	NETVAR_GETTER(m_bDucking,		DT_Local, bool)
	NETVAR_GETTER(m_bInDuckJump,	DT_Local, bool)
	
	NETVAR_SETTER(m_flJumpTime,     DT_Local, float)
	NETVAR_SETTER(m_flDuckJumpTime, DT_Local, float)
	NETVAR_SETTER(m_flDucktime,		DT_Local, float)
	NETVAR_SETTER(m_bDucked,		DT_Local, bool)
	NETVAR_SETTER(m_bDucking,		DT_Local, bool)
	NETVAR_SETTER(m_bInDuckJump,	DT_Local, bool)

	NETVAR_GETTER(m_nHitboxSet, DT_BaseAnimating, int)
	NETVAR_GETTER(m_nSequence, DT_BaseAnimating, int)
	NETVAR_SETTER(m_nSequence, DT_BaseAnimating, int)

	bool			IsEnemy();
	int32_t			GetWeaponIndex();

	inline bool	IsDisguised() { return InCond(TF_COND_DISGUISED); }
	inline bool	isCloaked()   { return InCond(TF_COND_STEALTHED); }

	// make visible using addToLeafSystem()
	void			changeThirdPersonVisibility(renderGroup_t renderGroup);

	uint32_t		GetPlayerCond();
	bool			InCond(FLAG_playerCond eCond);

	bool			isOnGround();
	int32_t			GetAirDash(); // Number of Air Dashes done.

	float			GetCritMult();

	RoundStats_t*	GetPlayerRoundData();

	bool			IsCritBoosted();

	bool			IsFeignDeathReady();

	NETVAR_GETTER(m_flModelScale, DT_BaseAnimating, float)
	NETVAR_SETTER(m_flModelScale, DT_BaseAnimating, float)

	// CALL_ATRIB_HOOK's
	void CALL_ATRIB_HOOK_INT(int& iAtributeOut, const char* szAtribute) const;
	void CALL_ATRIB_HOOK_FLOAT(float& flAtributeOut, const char* szAtribute) const;
};
