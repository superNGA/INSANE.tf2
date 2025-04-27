#pragma once
#include "ConCommandBase.h"

class ConVar : public ConCommandBase
{
public:
	float GetFloat() { return m_fValue; }
	int GetInt()	 { return m_nValue; }

public:
	void*						pVTable; // <-- This accounts for lack of virtual fns in my class.

	ConVar*						m_pParent;

	// Static data
	const char*					m_pszDefaultValue;

	// Value
	// Dynamically allocated
	char*						m_pszString;
	int							m_StringLength;

	// Values
	float						m_fValue;
	int							m_nValue;

	// Min/Max values
	bool						m_bHasMin;
	float						m_fMinVal;
	bool						m_bHasMax;
	float						m_fMaxVal;

	// Min/Max values for competitive.
	bool						m_bHasCompMin;
	float						m_fCompMinVal;
	bool						m_bHasCompMax;
	float						m_fCompMaxVal;

	bool						m_bCompetitiveRestrictions;


	// Call this function when ConVar changes
	//FnChangeCallback_t			m_fnChangeCallback;
};