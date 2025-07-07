#pragma once
#include "ClientClass.h"
#include "../../Utility/Interface Handler/Interface.h"

class IBaseClientDLL
{
public:
	// Called once when the client DLL is loaded
	virtual int				Init(void* JackShit) = 0;

	virtual void			PostInit() = 0;

	// Called once when the client DLL is being unloaded
	virtual void			Shutdown(void) = 0;

	// Called once the client is initialized to setup client-side replay interface pointers
	virtual bool			ReplayInit(void* replayFactory) = 0;
	virtual bool			ReplayPostInit() = 0;

	// Called at the start of each level change
	virtual void			LevelInitPreEntity(char const* pMapName) = 0;
	// Called at the start of a new level, after the entities have been received and created
	virtual void			LevelInitPostEntity() = 0;
	// Called at the end of a level
	virtual void			LevelShutdown(void) = 0;

	// Request a pointer to the list of client datatable classes
	virtual ClientClass* GetAllClasses(void) = 0; //8th function starting from 0
};

MAKE_INTERFACE_VERSION(IBaseClient, "VClient017", IBaseClientDLL, "client.dll")