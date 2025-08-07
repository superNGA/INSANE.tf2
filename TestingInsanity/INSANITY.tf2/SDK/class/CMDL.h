#pragma once

#include "Basic Structures.h"

struct CMDL
{
	unsigned short	m_MDLHandle;
	RGBA_t			m_Color;
	int				m_nSkin;
	int				m_nBody;
	int				m_nSequence;
	int				m_nLOD;
	float			m_flPlaybackRate;
	float			m_flTime;
	float			m_pFlexControls[96 * 4];
	vec				m_vecViewTarget;
	bool			m_bWorldSpaceViewTarget;
	void*			m_pProxyData;
};