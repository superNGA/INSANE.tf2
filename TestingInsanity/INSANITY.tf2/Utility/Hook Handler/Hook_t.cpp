#include "Hook_t.h"
#include <assert.h>
#include "../ConsoleLogging.h"
#include "../../Libraries/Utility/Utility.h"
extern Utility util;

HookInfo_t::HookInfo_t(const char* szSignature, const char* szDll, void* pHookFn, void** ppOriginalFn, const char* szFnName)
{
    m_pHook      = pHookFn;
    m_ppOriginal = ppOriginalFn;
    m_szFnName   = szFnName;
    m_pTarget    = util.FindPattern(szSignature, szDll);
    m_szDll      = szDll;

    hook_t.AddHook(this);
}

HookInfo_t::HookInfo_t(uintptr_t pTarget, const char* szDll, void* pHookFn, void** ppOriginalFn, const char* szFnName)
{
    m_pHook      = pHookFn;
    m_ppOriginal = ppOriginalFn;
    m_szFnName   = szFnName;
    m_pTarget    = pTarget;
    m_szDll      = szDll;

    hook_t.AddHook(this);
}

HookInfo_t::HookInfo_t(const char* szInterfaceVersion, int iIndex, const char* szDll, void* pHookFn, void** ppOriginalFn, const char* szFnName)
{
    // Getting the interface
    int   iReturnCode = 0;
    void* pInterface  = reinterpret_cast<void*>(util.GetInterface(szInterfaceVersion, szDll, &iReturnCode));
    if (iReturnCode != 0 || pInterface == nullptr) // Invalid interface ?
    {
        m_pHook      = nullptr; 
        m_ppOriginal = nullptr;
        m_pTarget    = NULL;
        hook_t.AddHook(this); // Add this anyways, so "Hook_t::Initialize" Will notify us about failed hook.
        return;
    }

    m_pTarget    = reinterpret_cast<uintptr_t>(util.GetVirtualTable(pInterface)[iIndex]);

    m_szDll      = szDll;
    m_pHook      = pHookFn;
    m_ppOriginal = ppOriginalFn;
    m_szFnName   = szFnName;

    hook_t.AddHook(this);
}

bool Hook_t::Initialize()
{
    LOG("%d hooks found", m_vecAllHooks.size());

    for (HookInfo_t* hook : m_vecAllHooks)
    {
        if (hook->m_pHook == nullptr || hook->m_ppOriginal == nullptr || hook->m_pTarget == NULL)
        {
            FAIL_LOG("HOOKING DATA REALATED TO [ %s ] WAS INVALID", hook->m_szFnName);
            FAIL_LOG("TARGET ADRS     : %p", hook->m_pTarget);
            FAIL_LOG("ORIGNAL FN ADRS : %p", hook->m_ppOriginal);
            FAIL_LOG("HOOK FN ADRS    : %p", hook->m_pHook);
            return false;
        }
        
        if (MH_CreateHook((LPVOID)hook->m_pTarget, (LPVOID)hook->m_pHook, (LPVOID*)hook->m_ppOriginal) == MH_STATUS::MH_ERROR_FUNCTION_NOT_FOUND)
        {
            FAIL_LOG("FAILED TO HOOK : %s", hook->m_szFnName);
            return false;
        }
        WIN_LOG("hooked %s @ %p", hook->m_szFnName, hook->m_pTarget);
    }
    MH_EnableHook(MH_ALL_HOOKS);
    WIN_LOG("ALL HOOKS ENABLED!");

    return true;
}