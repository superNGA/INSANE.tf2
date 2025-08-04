#pragma once

#include "../../Utility/Interface Handler/Interface.h"

class IVEngineServer
{
public:
	// Tell engine to change level ( "changelevel s1\n" or "changelevel2 s1 s2\n" )
	virtual void		ChangeLevel(const char* s1, const char* s2) = 0;

	// Ask engine whether the specified map is a valid map file (exists and has valid version number).
	virtual int			IsMapValid(const char* filename) = 0;

	// Is this a dedicated server?
	virtual bool		IsDedicatedServer(void) = 0;

	// Is in Hammer editing mode?
	virtual int			IsInEditMode(void) = 0;

	// Add to the server/client lookup/precache table, the specified string is given a unique index
	// NOTE: The indices for PrecacheModel are 1 based
	//  a 0 returned from those methods indicates the model or sound was not correctly precached
	// However, generic and decal are 0 based
	// If preload is specified, the file is loaded into the server/client's cache memory before level startup, otherwise
	//  it'll only load when actually used (which can cause a disk i/o hitch if it occurs during play of a level).
	virtual int			PrecacheModel(const char* s, bool preload = false) = 0;
	virtual int			PrecacheSentenceFile(const char* s, bool preload = false) = 0;
	virtual int			PrecacheDecal(const char* name, bool preload = false) = 0;
	virtual int			PrecacheGeneric(const char* s, bool preload = false) = 0;

	virtual bool		IsModelPrecached(char const* s) const = 0;
	virtual bool		IsDecalPrecached(char const* s) const = 0;
	virtual bool		IsGenericPrecached(char const* s) const = 0;
};


MAKE_INTERFACE_VERSION(iVEngineServer, "VEngineServer023", IVEngineServer, ENGINE_DLL)