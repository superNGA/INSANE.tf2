#include "Interface.h"
#include "ConsoleLogging.h"


Interface_t::Interface_t(const char* szVersion, const char* szDll, void** pDestination)
{
    m_szVersion = szVersion;
    m_szDll = szDll;
    m_pDestination = pDestination;
    interfaceInitialize.AddInterface(this);
}

bool Interface_t::Initialize()
{
    int iReturnCode = 0;
    void* pOutput = util.GetInterface(m_szVersion, m_szDll, &iReturnCode);
    if (iReturnCode != 0)
        return false;

    m_pInterface = reinterpret_cast<uintptr_t>(pOutput);
    (*m_pDestination) = pOutput;
    return true;
}

bool InterfaceInitialize_t::Initialize()
{
    for (auto& I : m_vecInterfaces)
    {
        if (I->Initialize() == false)
        {
            FAIL_LOG("FAILED TO GET INTERFACE : %s FROM %s" , I->m_szVersion, I->m_szDll);
            return false;
        }
        WIN_LOG("found %s from %s @ [ %p ]", I->m_szVersion, I->m_szDll, *I->m_pDestination);
    }
    
    return true;
}