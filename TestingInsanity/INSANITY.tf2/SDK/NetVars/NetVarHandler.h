#pragma once

#include <vector>
#include <unordered_map>
#include <string>

class ClientClass;
class RecvTable;

class NetVar_t
{
public:
    NetVar_t(std::string szTableName, std::string szNetVarName, int32_t iOffset, int64_t* pDestination);

    uint64_t m_iHash        = 0;
    int32_t  m_iOffset      = 0;
    int64_t* m_pDestination = nullptr;

    std::string m_szTableName  = "";
    std::string m_szNetVarName = "";
};

class NetVarHandler_t
{
public:
    bool Initialize();
    void RegisterNetVar(NetVar_t* pNetVar);
    void AddNetVar(NetVar_t* pNetVar) { m_vecNetvars.push_back(pNetVar); }

private:
    void _RecurseClientClass(ClientClass* pClientClass);
    void _RecurseTable(RecvTable* pTable, uint64_t iParentOffset);

    uint32_t m_nFoundNetVars = 0;

    bool m_bFailedRegisteration = false;

    // Note : we are using a vector of netvar pointers here, so we can handle 
    // Duplicates and initialize them all.
    std::unordered_map<uint64_t, std::vector<NetVar_t*>> m_mapRegisteredNetvars = {};
    std::vector<NetVar_t*> m_vecNetvars = {};
};
inline NetVarHandler_t netVarHandler;


// MACROS
#define NETVAR_SAME_NAME(szNetVarName, szTableName) \
namespace Netvars{ namespace szTableName { inline int64_t szNetVarName = 0; } }\
namespace TempNetvar{inline NetVar_t NetVar_##szNetVarName(#szTableName, #szNetVarName, 0, &Netvars::szTableName::szNetVarName); }

#define NETVAR_DIFF_NAME(szName, szNetVarName, szTableName)\
namespace Netvars { namespace szTableName { inline int64_t szName = 0; } }\
namespace TempNetvar { inline NetVar_t NetVar_##szName(#szTableName, #szNetVarName, 0, &Netvars::szTableName::szName); }

#define EXPAND(x) x
#define GET_4TH_ARG(arg1, arg2, arg3, arg4, ...) arg4
#define CHOOSE_NETVAR_MACRO(...) GET_4TH_ARG(__VA_ARGS__, NETVAR_SAME_NAME, NETVAR_DIFF_NAME)

// Name, NetVarName, TableName
#define NETVAR(...) EXPAND(GET_4TH_ARG(__VA_ARGS__, NETVAR_DIFF_NAME, NETVAR_SAME_NAME)(__VA_ARGS__))

#define NETVAR_OFFSET(szName, szNetVarName, szTableName, iOffset)\
namespace Netvars { namespace szTableName { inline int64_t szName = 0; } }\
namespace TempNetvar { inline NetVar_t NetVar_##szName(#szTableName, #szNetVarName, iOffset, &Netvars::szTableName::szName); }

#define NETVAR_GETTER(name, table, type)\
inline type name(){return *reinterpret_cast<type*>(reinterpret_cast<uintptr_t>(this) + Netvars::table::name);};
#define NETVAR_SETTER(name, table, type)\
inline void name(type data){ *reinterpret_cast<type*>(reinterpret_cast<uintptr_t>(this) + Netvars::table::name) = data;};