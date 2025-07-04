#pragma once
#include "IAppSystem.h"
#include "IphysicsEnviorment.h"
#include "../../Utility/Interface.h"

class IPhysics : public IAppSystem
{
public:
	virtual	IPhysicsEnvironment * CreateEnvironment(void) = 0;
	virtual void DestroyEnvironment(IPhysicsEnvironment*) = 0;
	virtual IPhysicsEnvironment* GetActiveEnvironmentByIndex(int index) = 0;

	// Creates a fast hash of pairs of objects
	// Useful for maintaining a table of object relationships like pairs that do not collide.
	virtual void* CreateObjectPairHash() = 0;
	virtual void  DestroyObjectPairHash(void* pHash) = 0;

	// holds a cache of these by id.  So you can get by id to search for the previously created set
	// UNDONE: Sets are currently limited to 32 elements.  More elements will return NULL in create.
	// NOTE: id is not allowed to be zero.
	virtual void* FindOrCreateCollisionSet(unsigned int id, int maxElementCount) = 0;
	virtual void* FindCollisionSet(unsigned int id) = 0;
	virtual void  DestroyAllCollisionSets() = 0;
};

MAKE_INTERFACE_VERSION(iPhysics, "VPhysics031", IPhysics, VPHYSICS_DLL)