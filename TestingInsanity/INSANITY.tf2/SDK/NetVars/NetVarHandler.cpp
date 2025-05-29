#include "NetVarHandler.h"

// SDK
#include "../class/ClientClass.h"
#include "../class/I_BaseEntityDLL.h"

// UTILITY
#include "../../Libraries/Utility/Utility.h"
#include "../../Utility/ConsoleLogging.h"

inline uint64_t HashPropName(const std::string& szTableName, const std::string& szNetVarName)
{
    uint64_t iHashFinal = 0;
    uint32_t iHash1 = FNV1A32(szTableName.c_str());
    uint32_t iHash2 = FNV1A32(szNetVarName.c_str());

    // iHash1 is Upper 4 byte of final hash & iHash2 is lower 4 bytes.
    iHashFinal |= iHash1;
    iHashFinal = iHashFinal << 32;
    iHashFinal |= iHash2;

    return iHashFinal;
}


NetVar_t::NetVar_t(std::string szTableName, std::string szNetVarName, int32_t iOffset, int64_t* pDestination)
{
    m_iHash        = HashPropName(szTableName, szNetVarName);
    m_iOffset      = iOffset;
    m_szTableName  = szTableName;
    m_szNetVarName = szNetVarName;
    m_pDestination = pDestination;

    netVarHandler.RegisterNetVar(this);
}


void NetVarHandler_t::RegisterNetVar(NetVar_t* pNetVar)
{
    // If we already failed registeration, then don't procced
    if (m_bFailedRegisteration == true)
        return;

    // Did Hash got created properly or not?
    if (pNetVar->m_iHash == 0)
    {
        FAIL_LOG("Hash is [ %llu ] for NetVar [ %s->%s ]", pNetVar->m_iHash, pNetVar->m_szTableName, pNetVar->m_szNetVarName);
        m_bFailedRegisteration = true;
        return;
    }

    // If this is the first element of this hash, then initialize its vector
    auto it = m_mapRegisteredNetvars.find(pNetVar->m_iHash);
    if (it == m_mapRegisteredNetvars.end())
    {
        m_mapRegisteredNetvars.insert({ pNetVar->m_iHash, std::vector<NetVar_t*>() });
    }

    // Check if it got added or not?
    auto it2 = m_mapRegisteredNetvars.find(pNetVar->m_iHash);
    if (it2 == m_mapRegisteredNetvars.end())
    {
        FAIL_LOG("Failed to add element [ %s->%s ]", pNetVar->m_szTableName, pNetVar->m_szNetVarName);
        m_bFailedRegisteration = true;
        return;
    }

    // Finally, register it!
    it2->second.push_back(pNetVar);
    WIN_LOG("Registered NetVar [ %s->%s ]", pNetVar->m_szTableName, pNetVar->m_szNetVarName);
}


bool NetVarHandler_t::Initialize()
{
    // If some error occured during registerstion, then don't initialize. ( else segfaults )
    if (m_bFailedRegisteration == true)
        return false;

    ClientClass* pClientClass = I::IBaseClient->GetAllClasses();
    _RecurseClientClass(pClientClass);

    return true;
}


void NetVarHandler_t::_RecurseClientClass(ClientClass* pClientClass)
{
    if (pClientClass == nullptr)
        return;

    RecvTable* pTable = pClientClass->m_pRecvTable;
    if (pTable == nullptr)
        return;

    _RecurseTable(pTable);

    _RecurseClientClass(pClientClass->m_pNext);
}

void NetVarHandler_t::_RecurseTable(RecvTable* pTable)
{
    // Iterate props for this table
    uint32_t nProps = pTable->m_nProps;
    for (int iProp = 0; iProp < nProps; iProp++)
    {
        RecvProp* pProp = &pTable->m_pProps[iProp];
        if (pProp == nullptr)
            continue;

        // is this prop a child table pointer?
        if (pProp->child_table != nullptr)
        {
            _RecurseTable(pProp->child_table);
            continue;
        }

        // Finding this hash in the map
        uint64_t iHash = HashPropName(pTable->m_pNetTableName, pProp->m_pVarName);
        auto it = m_mapRegisteredNetvars.find(iHash);

        // if not found, then we don't want it
        if (it == m_mapRegisteredNetvars.end())
            continue;

        // Storing this netvar
        for (auto* pNetVar : it->second)
        {
            *pNetVar->m_pDestination = static_cast<int64_t>(pProp->m_Offset + pNetVar->m_iOffset);
        }
    }
}