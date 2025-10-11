#pragma once

#include "../../Utility/Interface Handler/Interface.h"
#include "../../Utility/Signature Handler/signatures.h"

#define MAX_TABLES 32

#define MODEL_PRECACHE_TABLENAME	"modelprecache"
#define GENERIC_PRECACHE_TABLENAME	"genericprecache"
#define SOUND_PRECACHE_TABLENAME	"soundprecache"
#define DECAL_PRECACHE_TABLENAME	"decalprecache"

constexpr int INVALID_TABLE_INDEX = UINT16_MAX;
constexpr int INVALID_STRING_INDEX = UINT16_MAX;

class INetworkStringTable
{
public:

	virtual					~INetworkStringTable(void) {};

	// Table Info
	virtual const char*		GetTableName(void) const = 0;
	virtual int				GetTableId(void) const = 0;
	virtual int				GetNumStrings(void) const = 0;
	virtual int				GetMaxStrings(void) const = 0;
	virtual int				GetEntryBits(void) const = 0;

	// Networking
	virtual void			SetTick(int tick) = 0;
	virtual bool			ChangedSinceTick(int tick) const = 0;

	// Accessors (length -1 means don't change user data if string already exits)
	virtual int				AddString(bool bIsServer, const char* value, int length = -1, const void* userdata = 0) = 0;

	virtual const char* GetString(int stringNumber) = 0;
	virtual void			SetStringUserData(int stringNumber, int length, const void* userdata) = 0;
	virtual const void* GetStringUserData(int stringNumber, int* length) = 0;
	virtual int				FindStringIndex(char const* string) = 0; // returns INVALID_STRING_INDEX if not found

	// Callbacks
	virtual void			SetStringChangedCallback(void* object, /*pfnStringChanged*/int changeFunc) = 0;
};


class INetworkStringTableContainer
{
public:

	virtual					~INetworkStringTableContainer(void) {};

	// table creation/destruction
	virtual INetworkStringTable* CreateStringTable(const char* tableName, int maxentries, int userdatafixedsize = 0, int userdatanetworkbits = 0) = 0;
	virtual void				RemoveAllTables(void) = 0;

	// table infos
	virtual INetworkStringTable* FindTable(const char* tableName) const = 0;
	virtual INetworkStringTable* GetTable(int stringTable) const = 0;
	virtual int					GetNumTables(void) const = 0;

	virtual INetworkStringTable* CreateStringTableEx(const char* tableName, int maxentries, int userdatafixedsize = 0, int userdatanetworkbits = 0, bool bIsFilenames = false) = 0;
	virtual void				SetAllowClientSideAddString(INetworkStringTable* table, bool bAllowClientSideAddString) = 0;

    //inline INetworkStringTable* CreateStringTableUnristricted(const char* tableName, int maxentries, int userdatafixedsize = 0, int userdatanetworkbits = 0, char bIsFilenames = false);

    bool		m_bAllowCreation;	// creat guard Guard
    int			m_nTickCount;		// current tick
    bool		m_bLocked;			// currently locked?
    bool		m_bEnableRollback;	// enables rollback feature
};

MAKE_INTERFACE_VERSION(iNetworkStringTableContainer, "VEngineClientStringTable001", INetworkStringTableContainer, ENGINE_DLL)


MAKE_SIG(sub_1C7730, "40 53 48 83 EC ? 48 8B D9 E8 ? ? ? ? 4C 8B C0", ENGINE_DLL, __int64, __int64)
MAKE_SIG(sub_1E53E0, "48 89 5C 24 ? 48 89 6C 24 ? 48 89 74 24 ? 48 89 7C 24 ? 41 56 48 83 EC ? 80 61", ENGINE_DLL, char*,
    __int64, int, const char*, int, int, int, char)
    MAKE_SIG(sub_1C7A40, "48 89 5C 24 ? 57 48 83 EC ? 48 8B DA 48 8B F9 E8 ? ? ? ? 4C 8B C8", ENGINE_DLL, char*, __int64, __int64)