#pragma once
#include "Basic Structures.h"
#include <iostream>
#include "Source Entity.h"

#include "../../Utility/Interface.h"

struct ray_t
{
	//default contructor
	ray_t() : m_vStart(), m_vDelta(), m_vStartOffset(0.0f, 0.0f, 0.0f), m_vExtends(0.0f, 0.0f, 0.0f), m_bIsRay(false), m_bIsSwept(false) {}

	vecAligned m_vStart;
	vecAligned m_vDelta;
	vecAligned m_vStartOffset;
	vecAligned m_vExtends;

	bool m_bIsRay;
	bool m_bIsSwept;

	void Init(vec const& start, vec const& end)
	{
		//magnitude	= end - start;
		m_vDelta.x = end.x - start.x;
		m_vDelta.y = end.y - start.y;
		m_vDelta.z = end.z - start.z;
		
		//m_vStart		= start;
		m_vStart.x = start.x;
		m_vStart.y = start.y;
		m_vStart.z = start.z;
	}

	void InitV2(const vec& vStart, const vec& vEnd)
	{
		m_vStart  = vStart;
		m_vDelta  = vEnd - vStart;
		
		// if Length sqaured != 0
		m_bIsSwept = (m_vDelta.length() * m_vDelta.length() != 0);
		m_bIsRay   = true;
	}

	void Init(const vec& vStart, const vec& vEnd, const vec& vHullMin, const vec& vHullMax)
	{
		m_vDelta       = vEnd - vStart;

		// if Length sqaured != 0
		m_bIsSwept     = ((m_vDelta.length() * m_vDelta.length()) != 0);
				
		m_vExtends	   = vHullMax - vHullMin;
		m_vExtends	   *= 0.5f;
		
		// Is this "Hull" a "ray" ? ( I did it cause valve did it, and ray traces are sensitive AF and I don't want more segfaults )
		m_bIsRay       = ((m_vExtends.length() * m_vExtends.length()) < 1e-6);

		// Setting up Start Offset
		m_vStartOffset = vHullMin + vHullMax;
		m_vStartOffset *= 0.5f;
		m_vStart	   = m_vStartOffset + vStart;
		m_vStartOffset *= -1.0f;

		printf("<------------------>\n");
		LOG_VEC3(vStart);
		LOG_VEC3(vEnd);
		LOG_VEC3(m_vDelta);
		LOG_VEC3(m_vStart);
		LOG_VEC3(m_vStartOffset);
		LOG_VEC3(m_vExtends);
	}
};

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


class i_trace_filter {
private:
	I_handle_entity* skip;

public:
	i_trace_filter(I_handle_entity* pEntSkip, int iCollisionGroup = COLLISION_GROUP_NONE) : 
		skip(pEntSkip), m_iCollisionGroup(iCollisionGroup) {}

	virtual bool ShouldHitEntity(I_handle_entity* pEntToCheck, int iCollisionGroup) 
	{
		printf("colliding with %p | skip entity %p | collisionGroup : %d\n", pEntToCheck, skip, iCollisionGroup);
		return (pEntToCheck != skip); // returning true if ray didn't hit local player
	}

	virtual TraceType_t	GetTraceType() 
	{
		return TraceType_t::TRACE_EVERYTHING;
	}

	void*			 m_pPassEnt;
	int				 m_iCollisionGroup;
};

struct cplane_tt {
	vec normal;
	float  m_dist;
	BYTE   m_type;
	BYTE   m_sign_bits;
	BYTE   m_pad[2];
};
struct trace_t {
	vec					m_start;
	vec					m_end;
	cplane_tt			m_plane;
	float				m_fraction; // 1.0f = didn't hit shit
	int					m_contents;
	unsigned short		m_disp_flags;
	bool				m_allsolid;
	bool				m_start_solid = false;
	float				m_fraction_left_solid;
	csurface_t			m_surface;
	int					m_hitgroup;
	short				m_physics_bone;
	BaseEntity*	m_entity;
	int					m_hitbox;

	bool did_hit() const { return m_fraction < 1.f || m_allsolid || m_start_solid; }
};

typedef trace_t CGameTrace;

class IEngineTrace
{
public:
	virtual int		GetPointContents(const vec& vecAbsPosition, void** ppEntity = nullptr) = 0;
	virtual int		GetPointContents_Collideable(ICollideable* pCollide, const vec& vecAbsPosition) = 0;
	virtual void	ClipRayToEntity(const ray_t& ray, unsigned int fMask, void* pEnt, CGameTrace* pTrace) = 0;
	virtual void	ClipRayToCollideable(const ray_t& ray, unsigned int fMask, ICollideable* pCollide, CGameTrace* pTrace) = 0;
	virtual void	TraceRay(const ray_t& ray, unsigned int fMask, i_trace_filter* pTraceFilter, CGameTrace* pTrace) = 0;
	virtual void	SetupLeafAndEntityListRay(const ray_t& ray, DWORD& traceData) = 0;
	virtual void    SetupLeafAndEntityListBox(const vec& vecBoxMin, const vec& vecBoxMax, DWORD& traceData) = 0;
	virtual void	TraceRayAgainstLeafAndEntityList(const ray_t& ray, DWORD& traceData, unsigned int fMask, i_trace_filter* pTraceFilter, CGameTrace* pTrace) = 0;
	virtual void	SweepCollideable(ICollideable* pCollide, const vec& vecAbsStart, const vec& vecAbsEnd,
		const qangle& vecAngles, unsigned int fMask, i_trace_filter* pTraceFilter, CGameTrace* pTrace) = 0;
	virtual void	EnumerateEntities(const ray_t& ray, bool triggers, void* pEnumerator) = 0;
	virtual void	EnumerateEntities(const vec& vecAbsMins, const vec& vecAbsMaxs, void* pEnumerator) = 0;
	virtual ICollideable* GetCollideable(void** pEntity) = 0;
	virtual int		GetStatByIndex(int index, bool bClear) = 0;

	inline void UTIL_TraceHull(const vec& vecAbsStart, const vec& vecAbsEnd, const vec& hullMin,
		const vec& hullMax, unsigned int mask, i_trace_filter* pFilter, trace_t* ptr)
	{
		ray_t ray;
		ray.Init(vecAbsStart, vecAbsEnd, hullMin, hullMax);

		TraceRay(ray, mask, pFilter, ptr);
	}
};

MAKE_INTERFACE_VERSION(EngineTrace, "EngineTraceClient003", IEngineTrace, ENGINE_DLL)