#pragma once
#include "Basic Structures.h"


///////////////////////////////////////////////////////////////////////////
enum ParticleAttachment_t
{
	PATTACH_ABSORIGIN = 0,			// Create at absorigin, but don't follow
	PATTACH_ABSORIGIN_FOLLOW,		// Create at absorigin, and update to follow the entity
	PATTACH_CUSTOMORIGIN,			// Create at a custom origin, but don't follow
	PATTACH_POINT,					// Create on attachment point, but don't follow
	PATTACH_POINT_FOLLOW,			// Create on attachment point, and update to follow the entity

	PATTACH_WORLDORIGIN,			// Used for control points that don't attach to an entity

	PATTACH_ROOTBONE_FOLLOW,		// Create at the root bone of the entity, and update to follow

	MAX_PATTACH_TYPES,
};
///////////////////////////////////////////////////////////////////////////


///////////////////////////////////////////////////////////////////////////
struct te_tf_particle_effects_colors_t
{
	vec m_vecColor1;
	vec m_vecColor2;
};
///////////////////////////////////////////////////////////////////////////


///////////////////////////////////////////////////////////////////////////
struct te_tf_particle_effects_control_point_t
{
	ParticleAttachment_t m_eParticleAttachment;
	vec					 m_vecOffset;
};
///////////////////////////////////////////////////////////////////////////


///////////////////////////////////////////////////////////////////////////
class CEffectData
{
public:
	vec     m_vOrigin;
	vec     m_vStart;
	vec     m_vNormal;
	qangle  m_vAngles;
	int		m_fFlags;
	int		m_nEntIndex;
	float	m_flScale;
	float	m_flMagnitude;
	float	m_flRadius;
	int		m_nAttachmentIndex;
	short	m_nSurfaceProp;

	// Some TF2 specific things
	int		m_nMaterial;
	int		m_nDamageType;
	int		m_nHitBox;

	unsigned char	m_nColor;

	// Color customizability
	bool							m_bCustomColors;
	te_tf_particle_effects_colors_t	m_CustomColors;

	bool									m_bControlPoint1;
	te_tf_particle_effects_control_point_t	m_ControlPoint1;

	int m_iEffectName;	// Entry in the EffectDispatch network string table. The is automatically handled by DispatchEffect().
};