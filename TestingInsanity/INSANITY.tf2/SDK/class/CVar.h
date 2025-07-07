#pragma once
#include "IAppSystem.h"
#include "Convar.h"

#include "../../Utility/Interface Handler/Interface.h"

class ICvar: public IAppSystem
{
public:
	// Allocate a unique DLL identifier
	virtual void					AllocateDLLIdentifier() = 0;

	// Register, unregister commands
	virtual void					RegisterConCommand(ConCommandBase* pCommandBase) = 0;
	virtual void					UnregisterConCommand(ConCommandBase* pCommandBase) = 0;
	virtual void					UnregisterConCommands(int id) = 0;

	// If there is a +<varname> <value> on the command line, this returns the value.
	// Otherwise, it returns NULL.
	virtual const char*				GetCommandLineValue(const char* pVariableName) = 0;

	// Try to find the cvar pointer by name
	virtual ConCommandBase*			FindCommandBase(const char* name) = 0;
	virtual const ConCommandBase*	FindCommandBase(const char* name) const = 0;
	virtual ConVar*					FindVar(const char* var_name) = 0;
	virtual const ConVar*			FindVar(const char* var_name) const = 0;
	virtual ConCommandBase*			FindCommand(const char* name) = 0;
	virtual const ConCommandBase*	FindCommand(const char* name) const = 0;

	// Get first ConCommandBase to allow iteration
	virtual ConCommandBase*			GetCommands(void) = 0;
	virtual const ConCommandBase*	GetCommands(void) const = 0;
};

MAKE_INTERFACE_VERSION(iCvar, "VEngineCvar004", ICvar, VSTDLIB_DLL);