#pragma once
#include "Basic Structures.h"
#include <iostream>
#include "Source Entity.h"

#include "../../Utility/Interface.h"

struct ray_t
{
	//default contructor
	ray_t() : m_start(), magnitude(), start_offset(0.0f, 0.0f, 0.0f), m_extend(0.0f, 0.0f, 0.0f), is_ray(true), is_swept(true) {}

	vecAligned m_start;
	vecAligned magnitude;
	vecAligned start_offset;
	vecAligned m_extend;

	bool is_ray;
	bool is_swept;

	void Init(vec const& start, vec const& end)
	{
		//magnitude	= end - start;
		magnitude.x = end.x - start.x;
		magnitude.y = end.y - start.y;
		magnitude.z = end.z - start.z;
		
		//m_start		= start;
		m_start.x = start.x;
		m_start.y = start.y;
		m_start.z = start.z;
	}
};

enum TraceType_t
{
	TRACE_EVERYTHING = 0,
	TRACE_WORLD_ONLY,				// NOTE: This does *not* test static props!!!
	TRACE_ENTITIES_ONLY,			// NOTE: This version will *not* test static props
	TRACE_EVERYTHING_FILTER_PROPS,	// NOTE: This version will pass the IHandleEntity for props through the filter, unlike all other filters
};


class i_trace_filter {
private:
	I_handle_entity* skip;

public:
	i_trace_filter(I_handle_entity* pEntSkip) : skip(pEntSkip) {}

	virtual bool ShouldHitEntity(I_handle_entity* pEntToCheck, int contentsMask) {
		return (pEntToCheck != skip); // returning true if ray didn't hit local player
	}

	virtual TraceType_t	GetTraceType() {
		return TraceType_t::TRACE_EVERYTHING;
	}
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
	float				m_fraction;
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
};

MAKE_INTERFACE_VERSION(EngineTrace, "EngineTraceClient003", IEngineTrace, ENGINE_DLL)