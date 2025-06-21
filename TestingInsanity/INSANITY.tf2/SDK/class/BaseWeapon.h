#pragma once

// SDK 
#include "Basic Structures.h"
#include "BaseEntity.h"

// UTILITY
#include "../NetVars/NetVarHandler.h"
#include "../../Libraries/Utility/Utility.h"

NETVAR_OFFSET(m_flSmackTime, m_bReadyToBackstab, DT_TFWeaponKnife, -0x14)
NETVAR(m_bReadyToBackstab, DT_TFWeaponKnife)
NETVAR(m_iReloadMode,	   DT_TFWeaponBase)

NETVAR(m_iClip1,				DT_LocalWeaponData)
NETVAR(m_iClip2,				DT_LocalWeaponData)
NETVAR(m_bFlipViewModel,		DT_LocalWeaponData)
NETVAR(m_flObservedCritChance,	DT_LocalTFWeaponData)
NETVAR(m_flLastCritCheckTime,	DT_LocalTFWeaponData)
NETVAR(m_flNextPrimaryAttack,	DT_LocalActiveWeaponData)

// This is charge time for both stickies and sniper's arrows.
NETVAR(m_flChargeBeginTime,		DT_PipebombLauncherLocalData)

NETVAR_OFFSET(m_nCritSeedRequests, m_nViewModelIndex, DT_LocalWeaponData, -0x4)
NETVAR_OFFSET(m_nCritChecks,	   m_nViewModelIndex, DT_LocalWeaponData, -0x8)
NETVAR_OFFSET(m_flCritTokenBucket, m_nViewModelIndex, DT_LocalWeaponData, -0xC)


class CTFWeaponInfo;

enum slot_t {
	WPN_SLOT_INVALID = -1,
	WPN_SLOT_PRIMARY=0,
	WPN_SLOT_SECONDARY,
	WPN_SLOT_MELLE
};

enum reload_t
{
	WPN_RELOAD_START = 0,
	WPN_RELOADING,
	WPN_RELOADING_CONTINUE,
	WPN_RELOAD_FINISH
};

class baseWeapon : public BaseEntity {
public:
	int			getSlot();

	// Reload mode
	NETVAR_GETTER(m_iReloadMode, DT_TFWeaponBase, reload_t)
	NETVAR_SETTER(m_iReloadMode, DT_TFWeaponBase, reload_t)

	NETVAR_GETTER(m_iClip1, DT_LocalWeaponData, int)
	NETVAR_GETTER(m_iClip2, DT_LocalWeaponData, int)

	NETVAR_GETTER(m_bFlipViewModel, DT_LocalWeaponData, bool)
	
	// Crit Bucket Getters
	NETVAR_GETTER(m_nCritSeedRequests, DT_LocalWeaponData, int)
	NETVAR_GETTER(m_nCritChecks,	   DT_LocalWeaponData, int)
	NETVAR_GETTER(m_flCritTokenBucket, DT_LocalWeaponData, float)
	
	// Crit Bucket Setter
	NETVAR_SETTER(m_nCritSeedRequests, DT_LocalWeaponData, int)
	NETVAR_SETTER(m_nCritChecks,	   DT_LocalWeaponData, int)
	NETVAR_SETTER(m_flCritTokenBucket, DT_LocalWeaponData, float)

	NETVAR_GETTER(m_flObservedCritChance, DT_LocalTFWeaponData, float)
	NETVAR_GETTER(m_flLastCritCheckTime, DT_LocalTFWeaponData, float)

	NETVAR_GETTER(m_flNextPrimaryAttack, DT_LocalActiveWeaponData, float)
	NETVAR_GETTER(m_flSmackTime,		 DT_TFWeaponKnife, float)

	NETVAR_GETTER(m_flChargeBeginTime, DT_PipebombLauncherLocalData, float)

	bool		IsProjectile();

	CTFWeaponInfo* GetTFWeaponInfo();
	
	// Weapon IDs
	int			GetWeaponTypeID(); // <- ETFWeaponInfo ID ( i.e. same for all bats for scout )
	int			GetWeaponDefinitionID(); // <- Item specific ID ( i.e. different for each bat type for scout )

	float		GetDamagePerShot();

private:
	bool TracerHook = false;
};