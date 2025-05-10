// PURPOSE : gives a template to fill up while entity list filtering in thread 2 for each entity
//			 which can be further used in other places like FrameStageNotify

#pragma once
#include <string>
#include "class/Basic Structures.h"
#include "class/Source Entity.h"

#define IS_MAXBONES 7

/* HOW TO ADD A NEW BONE :
	INCREASE IS_MAXBONES size accoring to boneInfo_t size.
	add bone name to enum INDEX_boneInfo
	add bone name to boneInfo_t struct
	add bone copy mechanism to copyEntBones in entInfo_t
	add bone caching in boneManager in globalvars.h->entities::boneManager_t */

enum INDEX_boneInfo {
	HEAD=0,
	LEFT_SHOULDER,
	RIGHT_SHOULDER,
	LEFT_FOOT,
	RIGHT_FOOT,
	CHEST,
	PELVIS
};

struct boneInfo_t {
	int16_t head = 0;
	int16_t leftShoulder = 0;
	int16_t rightShoulder = 0;
	int16_t leftFoot = 0;

	int16_t rightFoot = 0;
	int16_t chest = 0;
	int16_t pelvis = 0;
};

enum BIT_entInfo {
	ENT_ON_GROUND,	// is this entity ON ground or NOT ?
	ON_SCREEN,		// is this entity ON screen or NOT ?
	
	SHOULD_LOCK_AIMBOT, // is this entity locked in for aimbot
	IS_PLAYER,		// is this entity a actual player or something like a dispenser or some shit
	IS_BUILDING,	// is Dispensor, Teleporter or Sentery gun?
	IS_VISIBLE,		// is this entity visible ?
	FRENDLY,		// is friendly ?
	IS_DISGUISED,	// is disguised ?
	IS_CLOAKED		// is invisible ?
};

enum IDclass_t {
	NOT_DEFINED,
	PLAYER,
	AMMO_PACK,
	DISPENSER,
	SENTRY_GUN,
	TELEPORTER,
	TF_ITEM,
	CAPTURE_POINT,
	WEAPON,
	PAYLOAD,
	CBASEANIMATING, // <-- this seems to only hold medKits, rotating AmmoPacks and whatEver weapon you are holding

	ROCKET,
	DEMO_PROJECTILES,
	ID_DROPPED_WEAPON,

	// CTFResourceManager, has max health arry for all entities :)
	ENT_RESOURCE_MANAGER
};

struct boneScreenPos_t
{
	vec2 bonePos;
};

struct entInfo_t
{
	std::string entUserName = "NotPlayer";
	BaseEntity* p_ent;
	baseWeapon* pActiveWeapon;
	int16_t entIndex = 0;

	/* is this entity a player or dispenser of what? */
	IDclass_t classID = NOT_DEFINED;

	/* which class is this player playing */
	player_class charactorChoice;

	/* what weapon is this player holding 
	weapon ID for active weapon */
	int16_t activeWeapon;

	// what is the maximum health for this entity ? 
	int16_t maxHealth = 0;

	// Current health for this entity
	int16_t health = 0;

	vec entPos;
	matrix3x4_t bones[IS_MAXBONES]; 
	vec entVelocity;

	boneInfo_t* infoBoneID = nullptr;
	INDEX_boneInfo targetBoneID = HEAD;
	qangle targetAngles;

	/* array for each bones screen position, popullated inside end scene's first 
	visuals rendering function */
	vec2 boneScreenPos[IS_MAXBONES];

	/* copies essential imformation about important bones from setUpBones to 
	out entities "bones[IS_MAXBONES]". */
	void copyEntBones(matrix3x4_t* maxStudioBones) {
		bones[HEAD]				= maxStudioBones[infoBoneID->head];
		bones[LEFT_SHOULDER]	= maxStudioBones[infoBoneID->leftShoulder];
		bones[RIGHT_SHOULDER]	= maxStudioBones[infoBoneID->rightShoulder];
		bones[LEFT_FOOT]		= maxStudioBones[infoBoneID->leftFoot];
		bones[RIGHT_FOOT]		= maxStudioBones[infoBoneID->rightFoot];
		bones[CHEST]			= maxStudioBones[infoBoneID->chest];
		bones[PELVIS]			= maxStudioBones[infoBoneID->pelvis];
	}

	/* This is a 16-bit bit field. each bit represent some information about the entity.
	refer BIT_entInfo for to see which bit represents what :) */
	int16_t flags = 0;

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

struct glowObject_t
{
	BaseEntity* pEnt	= nullptr;
	int16_t entIndex		= 0;
	bool isFrendly			= false;
	IDclass_t classID		= NOT_DEFINED;
};