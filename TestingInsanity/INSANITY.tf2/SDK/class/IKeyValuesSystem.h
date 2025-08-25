#pragma once
#include "../../Utility/Export Fn Handler/ExportFnHelper.h"


class KeyValues;


class IKeyValuesSystem
{
public:
	// registers the size of the KeyValues in the specified instance
	// so it can build a properly sized memory pool for the KeyValues objects
	// the sizes will usually never differ but this is for versioning safety
	virtual void RegisterSizeofKeyValues(int size) = 0;

	// allocates/frees a KeyValues object from the shared mempool
	virtual void* AllocKeyValuesMemory(int size) = 0;
	virtual void FreeKeyValuesMemory(void* pMem) = 0;

	// symbol table access (used for key names)
	virtual int GetSymbolForString(const char* name, bool bCreate = true) = 0;
	virtual const char* GetStringForSymbol(int symbol) = 0;

	// for debugging, adds KeyValues record into global list so we can track memory leaks
	virtual void AddKeyValuesToMemoryLeakList(void* pMem, int name) = 0;
	virtual void RemoveKeyValuesFromMemoryLeakList(void* pMem) = 0;

	// maintain a cache of KeyValues we load from disk. This saves us quite a lot of time on app startup. 
	virtual void AddFileKeyValuesToCache(const KeyValues* _kv, const char* resourceName, const char* pathID) = 0;
	virtual bool LoadFileKeyValuesFromCache(KeyValues* _outKv, const char* resourceName, const char* pathID, void* filesystem) const = 0;
	virtual void InvalidateCache() = 0;
	virtual void InvalidateCacheForFile(const char* resourceName, const char* pathID) = 0;
};

GET_EXPORT_FN_NO_ARGS(KeyValuesSystem, VSTDLIB_DLL, IKeyValuesSystem*)