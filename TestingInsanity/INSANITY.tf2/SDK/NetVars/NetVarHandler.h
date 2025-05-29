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

// MACROS
#define NETVAR_SAME_NAME(szNetVarName, szTableName) \
namespace Netvars{ namespace szTableName { inline int64_t szNetVarName = 0; } }\
namespace TempNetvar{inline NetVar_t NetVar_##szNetVarName(#szTableName, #szNetVarName, 0, &Netvars::szTableName::szNetVarName); }

#define NETVAR_DIFF_NAME(szName, szNetVarName, szTableName)\
namespace Netvars { namespace szTableName { inline int64_t szName = 0; } }\
namespace TempNetvar { inline NetVar_t NetVar_##szNetVarName(#szTableName, #szNetVarName, 0, &Netvars::szTableName::szNetVarName); }

#define EXPAND(x) x
#define GET_4TH_ARG(arg1, arg2, arg3, arg4, ...) arg4
#define CHOOSE_NETVAR_MACRO(...) GET_4TH_ARG(__VA_ARGS__, NETVAR_SAME_NAME, NETVAR_DIFF_NAME)
#define NETVAR(...) EXPAND(GET_4TH_ARG(__VA_ARGS__, NETVAR_DIFF_NAME, NETVAR_SAME_NAME)(__VA_ARGS__))

#define NETVAR_OFFSET(szName, szNetVarName, szTableName, iOffset)\
namespace Netvars { namespace szTableName { inline int64_t szName = 0; } }\
namespace TempNetvar { inline NetVar_t NetVar_##szNetVarName(#szTableName, #szNetVarName, iOffset, &Netvars::szTableName::szName); }


class NetVarHandler_t
{
public:
    bool Initialize();
    void RegisterNetVar(NetVar_t* pNetVar);

private:
    void _RecurseClientClass(ClientClass* pClientClass);
    void _RecurseTable(RecvTable* pTable);

    bool m_bFailedRegisteration = false;

    // Note : we are using a vector of netvar pointers here, so we can handle 
    // Duplicates and initialize them all.
    std::unordered_map<uint64_t, std::vector<NetVar_t*>> m_mapRegisteredNetvars = {};
};
inline NetVarHandler_t netVarHandler;