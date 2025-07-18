#include "CVarHandler.h"

#include "../../SDK/class/CVar.h"
#include "../ConsoleLogging.h"


void CVarHandler_t::InitializeAllCVars()
{
    if (m_bInitialzed == true)
        return;

    int iFailedCVars = 0;

    // NOTE : CVars are only one of the 2 types. Int & floats. maybe :)
    for (const CVarHelper_t* pCVar : m_vecRegisteredCVars)
    {
        // Finding CVar...
        ConVar* pConVar = I::iCvar->FindVar(pCVar->m_szCVarName);
        if (pConVar == nullptr)
        {
            FAIL_LOG("Failed to find CVar [ %s ]", pCVar->m_szCVarName);
            ++iFailedCVars;
            continue;
        }

        // Storing CVar...
        if (pCVar->m_bIsInt == true)
        {
            *reinterpret_cast<int*>(pCVar->m_pDestination) = pConVar->GetInt();
            LOG("Found CVar [ %s ] -> [ %d ]", pCVar->m_szCVarName, *reinterpret_cast<int*>(pCVar->m_pDestination));
        }
        else
        {
            *reinterpret_cast<float*>(pCVar->m_pDestination) = pConVar->GetFloat();
            LOG("Found CVar [ %s ] -> [ %.2f ]", pCVar->m_szCVarName, *reinterpret_cast<float*>(pCVar->m_pDestination));
        }

    }

    if (iFailedCVars == 0)
    {
        m_bInitialzed = true;
    }
}


void CVarHandler_t::RegisterCVar(CVarHelper_t* pCVar)
{
    m_vecRegisteredCVars.push_back(pCVar);
}