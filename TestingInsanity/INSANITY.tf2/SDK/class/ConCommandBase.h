#pragma once


class ConCommandBase
{
public:
	virtual					~ConCommandBase(void);

	virtual	bool			IsCommand(void) const;

	// Check flag
	virtual bool			IsFlagSet(int flag) const;
	// Set flag
	virtual void			AddFlags(int flags);

	// Return name of cvar
	virtual const char*		GetName(void) const;

	// Return help text for cvar
	virtual const char*		GetHelpText(void) const;

	virtual bool			IsRegistered(void) const;

	// Returns the DLL identifier
	virtual void			GetDLLIdentifier() const;

	virtual void			CreateBase(const char* pName, const char* pHelpString = 0,
		int flags = 0);

	// Used internally by OneTimeInit to initialize/shutdown
	virtual void			Init();

	ConCommandBase*			m_pNext;

	// Has the cvar been added to the global list?
	bool					m_bRegistered;

	// Static data
	const char*				m_pszName;
	const char*				m_pszHelpString;

	// ConVar flags
	int						m_nFlags;

	static ConCommandBase*	s_pConCommandBases;

	// ConVars in this executable use this 'global' to access values.
	static void*			s_pAccessor;
};