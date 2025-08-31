#include "ClassIDHandler.h"

// SDK
#include "../../SDK/class/I_BaseEntityDLL.h"
#include "../../SDK/class/ClientClass.h"

// Utility
#include "../ConsoleLogging.h"

ClassID_t::ClassID_t(int* pDestination, const std::string& szClassName)
{
    m_pDestination = pDestination;
    m_szClassName  = szClassName;

    F::classIDHandler.RegisterClassID(this);
}


void ClassIDHandler_t::RegisterClassID(ClassID_t* pClassID)
{
    auto it = m_mapClassNameToID.find(pClassID->m_szClassName);
    if (it != m_mapClassNameToID.end())
    {
        FAIL_LOG("Duplicate found for [ %s ]!", pClassID->m_szClassName.c_str());
        return;
    }

    m_mapClassNameToID.insert({ pClassID->m_szClassName, pClassID });
}


bool ClassIDHandler_t::Initialize()
{
    if (m_bInitialized == true)
        return true;

    ClientClass* pClientClass = I::IBaseClient->GetAllClasses();

    while (pClientClass != nullptr)
    {
        // Finding & storing class ID.
        auto it = m_mapClassNameToID.find(std::string(pClientClass->m_pNetworkName));
        if (it != m_mapClassNameToID.end())
        {
            *(it->second->m_pDestination) = pClientClass->m_ClassID;
            if(pClientClass->m_ClassID > 0)
            {
                m_mapClassNameToID.erase(it);
                LOG("initialized [ %s ] with class ID [ %d ]", pClientClass->m_pNetworkName, pClientClass->m_ClassID);
            }
            else
            {
                FAIL_LOG("Bullshit class ID [ %d ] for class [ %s ]", pClientClass->m_ClassID, pClientClass->m_pNetworkName);
            }
        }

        pClientClass = pClientClass->m_pNext;
    }

    
    // we found all the IDs!
    if (m_mapClassNameToID.empty() == true)
    {
        m_bInitialized = true;
        WIN_LOG("CACHED CLASS IDs");
        return true;
    }

    FAIL_LOG("Failed to find all classes, [ %d ] remaining", m_mapClassNameToID.size());
    return false;
}