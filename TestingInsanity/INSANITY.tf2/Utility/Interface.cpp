#include "Interface.h"
#include "ConsoleLogging.h"

Interface_t::Interface_t(const char* szVersion, const char* szDll, void** pDestination)
{
    m_szIdentifier = szVersion;
    m_szDll = szDll;
    m_pDestination = pDestination;
    m_bToBeScanned = false;

    interfaceInitialize.AddInterface(this);
}

Interface_t::Interface_t(const char* signature, const char* szDll, void** ppDestination, uint32_t iOffset, const char* name)
{
    m_szInterfaceName = name;
    m_szDll = szDll;
    m_pDestination = ppDestination;
    m_iOffset = iOffset;
    m_szIdentifier = signature;
    m_bToBeScanned = true;

    interfaceInitialize.AddInterface(this);
}

bool Interface_t::Initialize()
{
    if(m_bToBeScanned == false)
    {
        int iReturnCode = 0;
        void* pOutput = util.GetInterface(m_szIdentifier, m_szDll, &iReturnCode);
        if (iReturnCode != 0)
            return false;

        m_pInterface = reinterpret_cast<uintptr_t>(pOutput);
        (*m_pDestination) = pOutput;
        return true;
    }   

    uintptr_t pInterface = util.FindPattern(m_szIdentifier, m_szDll);
    if (pInterface == NULL)
        return false;

    // this is custom made for extracting adresses form lea instruction ( load effective adrs ), if required we shall expand it much more.
    *m_pDestination = reinterpret_cast<void*>((pInterface + 7) + (*reinterpret_cast<int32_t*>(pInterface + m_iOffset)));
    return true;
}

bool InterfaceInitialize_t::Initialize()
{
    for (auto& I : m_vecInterfaces)
    {
        if (I->Initialize() == false)
        {
            FAIL_LOG("FAILED TO GET INTERFACE : %s FROM %s" , I->m_szInterfaceName, I->m_szDll);
            return false;
        }
        I->m_bToBeScanned ?
            WIN_LOG("found %s [ %s ] from %s @ [ %p ]", I->m_szInterfaceName, I->m_szIdentifier, I->m_szDll, *I->m_pDestination) :
            WIN_LOG("found %s from %s @ [ %p ]", I->m_szInterfaceName, I->m_szDll, *I->m_pDestination);
    }
    
    return true;
}