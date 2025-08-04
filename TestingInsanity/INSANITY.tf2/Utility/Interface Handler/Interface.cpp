#include "Interface.h"
#include "../ConsoleLogging.h"

Interface_t::Interface_t(const char* szVersion, const char* szDll, void** pDestination)
{
    m_szIdentifier = szVersion;
    m_szDll = szDll;
    m_pDestination = pDestination;
    m_bToBeScanned = false;

    interfaceInitialize.AddInterface(this);
}

Interface_t::Interface_t(const char* signature, const char* szDll, void** ppDestination, uint32_t iOffset, uint32_t iCurInstructionSize, const char* name)
{
    m_szInterfaceName = name;
    m_szDll = szDll;
    m_pDestination = ppDestination;
    m_iOffset = iOffset;
    m_szIdentifier = signature;
    m_bToBeScanned = true;
    m_iCurInstrutionSize = iCurInstructionSize;

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

    LOG("Target instruction found @ [ %p ]", (void*)pInterface);

    // This is done to prevent unflowing int when adding an negative offset to adrs ( unsigned 8 byte int )
    int64_t pTargetSafe = static_cast<int64_t>(pInterface);

    // Add offset to the base adrs, i.e. fn adrs.
    int64_t iAdrsOffset = static_cast<int64_t>(*reinterpret_cast<int32_t*>(pTargetSafe + static_cast<int64_t>(m_iOffset)));
    int64_t iAdrsBase   = pTargetSafe + static_cast<int64_t>(m_iCurInstrutionSize);
    *m_pDestination     = reinterpret_cast<void*>(iAdrsBase + iAdrsOffset);
    return true;

    // this is custom made for extracting adresses form lea instruction ( load effective adrs ), if required we shall expand it much more.
    *m_pDestination = reinterpret_cast<void*>((pInterface + m_iCurInstrutionSize) + (*reinterpret_cast<int32_t*>(pInterface + m_iOffset)));
    return true;
}

bool InterfaceInitialize_t::Initialize()
{
    for (auto& I : m_vecInterfaces)
    {
        if (I->Initialize() == false)
        {
            FAIL_LOG("FAILED TO GET INTERFACE : %s FROM %s" , I->m_szIdentifier, I->m_szDll);
            return false;
        }
        I->m_bToBeScanned ?
            WIN_LOG("found %s [ %s ] from %s @ [ %p ]", I->m_szInterfaceName, I->m_szIdentifier, I->m_szDll, *I->m_pDestination) :
            WIN_LOG("found %s from %s @ [ %p ]", I->m_szIdentifier, I->m_szDll, *I->m_pDestination);
    }
    
    return true;
}