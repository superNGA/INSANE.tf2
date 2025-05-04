#pragma once
#include "Basic Structures.h"
#include "../offsets/offsets.h"
#include "../../Libraries/Utility/Utility.h"


extern Utility util;
extern local_netvars netvar;

class I_client_entity;
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

class baseWeapon : public I_client_entity {
public:
	slot_t		getSlot();
	reload_t	getReloadMode();
	void		setCustomTracer(const char* tracerName);
	bool		canBackStab();
	CTFWeaponInfo* GetTFWeaponInfo();
	int			GetWeaponID();
	bool		CanCrit();
	
	float		GetCritBucket();
	void		SetCritBucket(float flCritBucket);
	int			GetTotalCritsOccured();
	void		SetTotalCritsOccured(int iCritsOccured);
	int			GetTotalCritChecks();
	void		SetTotalCritChecks(int iCritChecks);

	float GetObservedCritChance();
	float GetDamagePerShot();
	void SetWeaponSeed(int iSeed);

	float GetNextPrimaryAttackTime();
	float GetLastRapidFireCritCheckTime();

private:
	bool TracerHook = false;
};