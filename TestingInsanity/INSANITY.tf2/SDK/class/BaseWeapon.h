#pragma once
#include "Basic Structures.h"
#include "../offsets/offsets.h"
#include "../../Libraries/Utility/Utility.h"


extern Utility util;
extern local_netvars netvar;

class BaseEntity;
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
	int getSlot();
	reload_t	getReloadMode();
	bool		canBackStab();
	CTFWeaponInfo* GetTFWeaponInfo();
	
	// Weapon IDs
	int			GetWeaponTypeID(); // <- ETFWeaponInfo ID ( i.e. same for all bats for scout )
	int			GetWeaponDefinitionID(); // <- Item specific ID ( i.e. different for each bat type for scout )

	bool		CanCrit(); // <- Just a wrapper for CalcIsAttackHelper()
	
	// Getters for Crit Bucket's stats
	float		GetCritBucket();
	int			GetTotalCritsOccured();
	int			GetTotalCritChecks();
	
	// Setter for Crit Bucket's stats
	void		SetCritBucket(float flCritBucket);
	void		SetTotalCritsOccured(int iCritsOccured);
	void		SetTotalCritChecks(int iCritChecks);

	float GetObservedCritChance();
	float GetDamagePerShot();
	void SetWeaponSeed(int iSeed);

	float GetNextPrimaryAttackTime();
	float GetLastRapidFireCritCheckTime();

private:
	bool TracerHook = false;
};