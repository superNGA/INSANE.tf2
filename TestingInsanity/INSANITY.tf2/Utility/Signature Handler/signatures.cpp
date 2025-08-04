#include "signatures.h"
#include "../../Libraries/Utility/Utility.h"
#include "../ConsoleLogging.h"

extern Utility util;

ISignature_t::ISignature_t(const char* signature, const char* szDllName, const char* szSigName, int iOffsetStart, int iOffsetEnd, bool bLEA)
{
    m_szSignature   = signature;
    m_szDllName     = szDllName;
    m_szSigName     = szSigName;

    m_bLEA          = bLEA;
    m_iOffsetStart  = iOffsetStart;
    m_iOffsetEnd    = iOffsetEnd;
    
    allSignatures.AddSig(this);
}


bool ISignature_t::Initialize()
{
    uintptr_t pTargetAdrs = util.FindPattern(m_szSignature, m_szDllName);
    if (pTargetAdrs == NULL)
        return false;   
    
    // Normal adrs
    if (m_bLEA == false)
    {
        m_ullAdrs = pTargetAdrs;
        return true;
    }

    // LEA adrs
    if (m_iOffsetEnd - m_iOffsetStart != 4)
    {
        FAIL_LOG("BAD SIGNATURE [ %s ]",m_szSigName);
        return false;
    }

    // This is done to prevent unflowing int when adding an negative offset to adrs ( unsigned 8 byte int )
    int64_t pTargetSafe = static_cast<int64_t>(pTargetAdrs);

    // Add offset to the base adrs, i.e. fn adrs.
    int64_t iAdrsOffset = static_cast<int64_t>(*reinterpret_cast<int32_t*>(pTargetSafe + static_cast<int64_t>(m_iOffsetStart)));
    int64_t iAdrsBase   = pTargetSafe + static_cast<int64_t>(m_iOffsetEnd);
    m_ullAdrs = iAdrsBase + iAdrsOffset;

    // Sanity checks ( just in case )
    if (m_ullAdrs == NULL)
        return false;

    return true;
}


bool AllSignatures_t::Initialize()
{
    for (ISignature_t* sig : m_vecSignatures)
    {
        if (sig->Initialize() == false)
        {
            FAIL_LOG("FAILED TO INITIALIZE SIGNATURE : %s [ %s ]", sig->m_szSigName, sig->m_szSignature);
            return false;
        }
        WIN_LOG("found %s [ %s ] @ [ %p ]", sig->m_szSigName, sig->m_szSignature, sig->m_ullAdrs);
    }

    return true;
}