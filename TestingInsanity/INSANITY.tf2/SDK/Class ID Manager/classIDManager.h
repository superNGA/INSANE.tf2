//=========================================================================
//                      CLASS ID MANAGER
//=========================================================================
// by      : INSANE
// created : 25/02/2025
// 
// purpose : cache the class IDs for entities, for fast lookup & also cause 
// they are prone to change with updates.

#pragma once
#include <unordered_map>
#include "../entInfo_t.h"

enum GameObjectID_t : uint32_t
{
	UNDEFINED_OBJECT		= 0,
	CTF_PLAYER				= ( 1 << 1),
	CTF_AMMOPACK			= ( 1 << 2),
	CTF_PLAYER_RESOURCE		= ( 1 << 3),
	CTF_DROPPED_WEAPON		= ( 1 << 4),

	// Projectiles
	CTF_PROJECTILE_ROCKET	= ( 1 << 5),
	CTF_PROJECTILE_PIPEBOMB	= ( 1 << 6),

	// Buildings
	C_OBJECT_TELEPORTER		= ( 1 << 7),
	C_OBJECT_SENTRY_GUN		= ( 1 << 8),
	C_OBJECT_DISPENSER		= ( 1 << 9),
	
	C_CAPTURE_FLAG			= ( 1 << 10),
	C_FUNC_TRACK_TRAIN		= ( 1 << 11),
	C_BASE_COMBAT_WEAPON	= ( 1 << 12),
	C_BASE_ANIMATING		= ( 1 << 13)
};

class BaseEntity;

class IDManager_t {

public:
	GameObjectID_t GetObjectID(BaseEntity* pEnt);

	IDclass_t getID(BaseEntity* ent);

private:
	std::unordered_map<std::string, GameObjectID_t> m_mapNameToObjectID = 
	{
		{"CTFPlayer" ,					 GameObjectID_t::CTF_PLAYER},
		{"CObjectSentrygun",			 GameObjectID_t::C_OBJECT_SENTRY_GUN},
		{"CObjectDispenser",			 GameObjectID_t::C_OBJECT_DISPENSER},
		{"CObjectTeleporter",			 GameObjectID_t::C_OBJECT_TELEPORTER},
		{"CTFAmmoPack",					 GameObjectID_t::CTF_AMMOPACK},
		{"CFuncTrackTrain",				 GameObjectID_t::C_FUNC_TRACK_TRAIN},
		{"CCaptureFlag",				 GameObjectID_t::C_CAPTURE_FLAG},
		{"CTFPlayerResource",			 GameObjectID_t::CTF_PLAYER_RESOURCE},
		{"CBaseCombatWeapon",			 GameObjectID_t::C_BASE_COMBAT_WEAPON},
		{"CBaseAnimating",				 GameObjectID_t::C_BASE_ANIMATING},
		{"CTFProjectile_Rocket",		 GameObjectID_t::CTF_PROJECTILE_ROCKET},
		{"CTFGrenadePipebombProjectile", GameObjectID_t::CTF_PROJECTILE_PIPEBOMB},
		{"CTFDroppedWeapon",			 GameObjectID_t::CTF_DROPPED_WEAPON},
	};

	std::unordered_map<int, GameObjectID_t> m_mapClassIDToObjectID = {};

	std::unordered_map<std::string, IDclass_t> CHE_mapID;
};
inline IDManager_t IDManager;