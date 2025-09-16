#pragma once

#include "../../Utility/Interface Handler/Interface.h"

class INetChannel;

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

	// Check's if the name is precached, but doesn't actually precache the name if not...
	virtual bool		IsModelPrecached(char const* s) const = 0;
	virtual bool		IsDecalPrecached(char const* s) const = 0;
	virtual bool		IsGenericPrecached(char const* s) const = 0;

	// Note that sounds are precached using the IEngineSound interface

	// Special purpose PVS checking
	// Get the cluster # for the specified position
	virtual int			GetClusterForOrigin(const vec& org) = 0;
	// Get the PVS bits for a specified cluster and copy the bits into outputpvs.  Returns the number of bytes needed to pack the PVS
	virtual int			GetPVSForCluster(int cluster, int outputpvslength, unsigned char* outputpvs) = 0;
	// Check whether the specified origin is inside the specified PVS
	virtual bool		CheckOriginInPVS(const vec& org, const unsigned char* checkpvs, int checkpvssize) = 0;
	// Check whether the specified worldspace bounding box is inside the specified PVS
	virtual bool		CheckBoxInPVS(const vec& mins, const vec& maxs, const unsigned char* checkpvs, int checkpvssize) = 0;

	// Returns the server assigned userid for this player.  Useful for logging frags, etc.  
	//  returns -1 if the edict couldn't be found in the list of players.
	virtual int			GetPlayerUserId(const void* e) = 0;
	virtual const char* GetPlayerNetworkIDString(const void* e) = 0;

	// Return the current number of used edict slots
	virtual int			GetEntityCount(void) = 0;
	// Given an edict, returns the entity index
	virtual int			IndexOfEdict(const void* pEdict) = 0;
	// Given and entity index, returns the corresponding edict pointer
	virtual void* PEntityOfEntIndex(int iEntIndex) = 0;

	// Get stats info interface for a client netchannel
	virtual INetChannel* GetPlayerNetInfo(int playerIndex) = 0;
};


MAKE_INTERFACE_VERSION(iVEngineServer, "VEngineServer023", IVEngineServer, ENGINE_DLL)