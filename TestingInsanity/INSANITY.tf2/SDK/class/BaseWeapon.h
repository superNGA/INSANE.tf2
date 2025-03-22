#pragma once
#include "Basic Structures.h"

#include "../../Libraries/Utility/Utility.h"
extern Utility util;

#include "../offsets/offsets.h"
extern local_netvars netvar;

class I_client_entity;

enum slot_t {
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

private:
	bool TracerHook = false;
};