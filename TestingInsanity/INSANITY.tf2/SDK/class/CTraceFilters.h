#pragma once

#include "BaseEntity.h"
#include "../../Utility/ClassIDHandler/ClassIDHandler.h"


enum TraceType_t
{
	TRACE_EVERYTHING = 0,
	TRACE_WORLD_ONLY,				// NOTE: This does *not* test static props!!!
	TRACE_ENTITIES_ONLY,			// NOTE: This version will *not* test static props
	TRACE_EVERYTHING_FILTER_PROPS,	// NOTE: This version will pass the IHandleEntity for props through the filter, unlike all other filters
};

enum Collision_Group_t
{
	COLLISION_GROUP_NONE = 0,
	COLLISION_GROUP_DEBRIS,			// Collides with nothing but world and static stuff
	COLLISION_GROUP_DEBRIS_TRIGGER, // Same as debris, but hits triggers
	COLLISION_GROUP_INTERACTIVE_DEBRIS,	// Collides with everything except other interactive debris or debris
	COLLISION_GROUP_INTERACTIVE,	// Collides with everything except interactive debris or debris
	COLLISION_GROUP_PLAYER,
	COLLISION_GROUP_BREAKABLE_GLASS,
	COLLISION_GROUP_VEHICLE,
	COLLISION_GROUP_PLAYER_MOVEMENT,  // For HL2, same as Collision_Group_Player, for
	// TF2, this filters out other players and CBaseObjects
	COLLISION_GROUP_NPC,			// Generic NPC group
	COLLISION_GROUP_IN_VEHICLE,		// for any entity inside a vehicle
	COLLISION_GROUP_WEAPON,			// for any weapons that need collision detection
	COLLISION_GROUP_VEHICLE_CLIP,	// vehicle clip brush to restrict vehicle movement
	COLLISION_GROUP_PROJECTILE,		// Projectiles!
	COLLISION_GROUP_DOOR_BLOCKER,	// Blocks entities not permitted to get near moving doors
	COLLISION_GROUP_PASSABLE_DOOR,	// Doors that the player shouldn't collide with
	COLLISION_GROUP_DISSOLVING,		// Things that are dissolving are in this group
	COLLISION_GROUP_PUSHAWAY,		// Nonsolid on client and server, pushaway in player code

	COLLISION_GROUP_NPC_ACTOR,		// Used so NPCs in scripts ignore the player.
	COLLISION_GROUP_NPC_SCRIPTED,	// USed for NPCs in scripts that should not collide with each other

	LAST_SHARED_COLLISION_GROUP
};



class ITraceFilter 
{
public:
	ITraceFilter(BaseEntity* pIgnoreEnt, int iCollisionGroup = COLLISION_GROUP_NONE) :
		m_pIgnoreEnt(pIgnoreEnt) {}

	virtual bool ShouldHitEntity(BaseEntity* pHitEnt, int iCollisionGroup)
	{
		return pHitEnt != m_pIgnoreEnt;
	}

	virtual TraceType_t	GetTraceType()
	{
		return TraceType_t::TRACE_EVERYTHING;
	}

	BaseEntity* m_pIgnoreEnt = nullptr;
};



class ITraceFilter_IgnoreSpawnVisualizer : public ITraceFilter
{
public:
	ITraceFilter_IgnoreSpawnVisualizer(BaseEntity* pIgnoreEnt) : ITraceFilter(pIgnoreEnt) {}

	bool ShouldHitEntity(BaseEntity* pHitEnt, int iCollisionGroup) override
	{
		if (pHitEnt->GetClientClass()->m_ClassID == ClassID::CFuncRespawnRoomVisualizer)
			return false;

		return ITraceFilter::ShouldHitEntity(pHitEnt, iCollisionGroup);
	}
};
