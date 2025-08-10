#pragma once

#include "Basic Structures.h"
#include "../../Utility/Signature Handler/signatures.h"

struct CMDL;
struct matrix3x4_t;

MAKE_SIG(CMDL_SetMDL,   "48 89 5C 24 ? 57 48 83 EC ? 0F B7 DA 48 8B F9 66 39 11", CLIENT_DLL, void, CMDL*, unsigned short)
MAKE_SIG(CMDL_DrawEasy, "48 89 6C 24 ? 56 48 83 EC ? 48 83 3D",				      CLIENT_DLL, void, CMDL*, matrix3x4_t*)

struct CMDL
{
	CMDL()
	{
		m_MDLHandle = 0xFFFF;
		m_Color = RGBA_t(255, 255, 255, 255);
		m_nSkin = 0;
		m_nBody = 0;
		m_nSequence = 0;
		m_nLOD = 0;
		m_flPlaybackRate = 30.0f;
		m_flTime = 0.0f;
		m_vecViewTarget.Init();
		m_bWorldSpaceViewTarget = false;
		memset(m_pFlexControls, 0, sizeof(m_pFlexControls));
		m_pProxyData = NULL;
	}

	inline void Draw(matrix3x4_t* pRootToWorld)  { Sig::CMDL_DrawEasy(this, pRootToWorld); }
	inline void SetMDL(unsigned short MDLHandle) { Sig::CMDL_SetMDL(this, MDLHandle); }
	inline unsigned short GetMDLHandle() const   { return m_MDLHandle; }

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