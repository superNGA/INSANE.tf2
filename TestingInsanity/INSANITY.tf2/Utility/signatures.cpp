#include "signatures.h"
#include "../Libraries/Utility/Utility.h"
#include "ConsoleLogging.h"

extern Utility util;

Signature_t::Signature_t(const char* signature, const char* szDllName, const char* szSigName)
{
    m_szSignature = signature;
    m_szDllName = szDllName;
    m_szSigName = szSigName;
    
    allSignatures.AddSig(this);
}


bool Signature_t::Initialize()
{
    m_ullAdrs = util.FindPattern(m_szSignature, m_szDllName);
    if (m_ullAdrs == NULL)
        return false;   
    
    return true;
}


bool AllSignatures_t::Initialize()
{
    for (Signature_t* sig : m_vecSignatures)
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