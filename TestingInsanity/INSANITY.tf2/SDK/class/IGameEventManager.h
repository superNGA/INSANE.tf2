#pragma once
#include <cstdint>
#include "../../Utility/Interface.h"

#include "../../Features/CritHack/CritHack.h"

class IGameEvent
{
public:
	virtual ~IGameEvent() {};
	virtual const char* GetName() const = 0;	// get event name

	virtual bool  IsReliable() const = 0; // if event handled reliable
	virtual bool  IsLocal() const = 0; // if event is never networked
	virtual bool  IsEmpty(const char* keyName = NULL) = 0; // check if data field exists

	// Data access
	virtual bool  GetBool(const char* keyName = NULL, bool defaultValue = false) = 0;
	virtual int   GetInt(const char* keyName = NULL, int defaultValue = 0) = 0;
	virtual float GetFloat(const char* keyName = NULL, float defaultValue = 0.0f) = 0;
	virtual const char* GetString(const char* keyName = NULL, const char* defaultValue = "") = 0;

	virtual void SetBool(const char* keyName, bool value) = 0;
	virtual void SetInt(const char* keyName, int value) = 0;
	virtual void SetFloat(const char* keyName, float value) = 0;
	virtual void SetString(const char* keyName, const char* value) = 0;
};

class IGameEventListener2
{
public:
	virtual	~IGameEventListener2(void) {};

	bool Initialize();

	virtual void FireGameEvent(IGameEvent* event);
};
extern IGameEventListener2 iEventListener;


class IBaseInterface
{
public:
	virtual	~IBaseInterface() {}
};

class IGameEventManager2 : public IBaseInterface
{
public:
	virtual	~IGameEventManager2(void) {};

	// load game event descriptions from a file eg "resource\gameevents.res"
	virtual int LoadEventsFromFile(const char* filename) = 0;

	// removes all and anything
	virtual void  Reset() = 0;

	// adds a listener for a particular event
	virtual bool AddListener(IGameEventListener2* listener, const char* name, bool bServerSide) = 0;

	// returns true if this listener is listens to given event
	virtual bool FindListener(IGameEventListener2* listener, const char* name) = 0;

	// removes a listener 
	virtual void RemoveListener(IGameEventListener2* listener) = 0;

	// create an event by name, but doesn't fire it. returns NULL is event is not
	// known or no listener is registered for it. bForce forces the creation even if no listener is active
	virtual IGameEvent* CreateEvent(const char* name, bool bForce = false) = 0;

	// fires a server event created earlier, if bDontBroadcast is set, event is not send to clients
	virtual bool FireEvent(IGameEvent* event, bool bDontBroadcast = false) = 0;

	// fires an event for the local client only, should be used only by client code
	virtual bool FireEventClientSide(IGameEvent* event) = 0;

	// create a new copy of this event, must be free later
	virtual IGameEvent* DuplicateEvent(IGameEvent* event) = 0;

	// if an event was created but not fired for some reason, it has to bee freed, same UnserializeEvent
	virtual void FreeEvent(IGameEvent* event) = 0;
};


MAKE_INTERFACE_VERSION(iGameEventManager, "GAMEEVENTSMANAGER002", IGameEventManager2, ENGINE_DLL);