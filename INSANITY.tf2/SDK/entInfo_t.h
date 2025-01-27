// PURPOSE : gives a template to fill up while entity list filtering in thread 2 for each entity
//			 which can be further used in other places like FrameStageNotify

#pragma once
#include <string>
#include "class/Basic Structures.h"

enum BIT_entInfo
{
	VALID_VELOCITY = 0,
	ENT_ON_GROUND
};

struct entInfo_t
{
	char* entUserName;
	I_client_entity* p_ent;

	/* which class is this player playing */
	int8_t charactorChoice;

	/* what weapon is this player holding */
	int16_t activeWeapon;

	vec entPos;
	matrix3x4_t *bones; // TODO : reduce size if possible
	vec entVelocity;

	/* This is a 8-bit bit field. each bit represent some information about the entity.
	refer BIT_entInfo for to see which bit represents what :) */
	int8_t flags = 0;

	inline void setFlagBit(const BIT_entInfo bitIndex) {
		flags |= (1 << bitIndex);
	}
	
	inline void clearFlagBit(const BIT_entInfo bitIndex) {
		flags &= ~(1 << bitIndex);
	}

	inline void toggleFlagBit(const BIT_entInfo bitIndex) {
		flags ^= (1 << bitIndex);
	}

	inline bool getFlagBit(const BIT_entInfo bitIndex) {
		return flags & (1 << bitIndex);
	}
};