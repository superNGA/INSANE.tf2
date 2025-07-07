#pragma once
#include <vector>
#include "../../Libraries/Utility/Utility.h"
#include "../../External Libraries/MinHook/MinHook.h"
#include "../ModuleNames.h"

class HookInfo_t
{
public:
    // Simple Signature Hook
    HookInfo_t(const char* szSignature, const char* szDll, void* pHookFn, void** ppOriginalFn, const char* szFnName);
    
    // Hook via adrs.
    HookInfo_t(uintptr_t pTarget, const char* szDll, void* pHookFn, void** ppOriginalFn, const char* szFnName);

    // VTable hooks
    HookInfo_t(const char * szInterfaceVersion, int iIndex, const char* szDll, void* pHookFn, void** ppOriginalFn, const char* szFnName);

    uintptr_t   m_pTarget    = NULL;
    void**      m_ppOriginal = nullptr;
    void*       m_pHook      = nullptr;
    const char* m_szFnName   = "NULL";
    const char* m_szDll      = "NULL";
};


class Hook_t
{
public:
    void AddHook(HookInfo_t* hookInfo) { m_vecAllHooks.push_back(hookInfo); }
    bool Initialize();
    int GetNumberOfHooks() { return m_vecAllHooks.size(); }

private:
    std::vector<HookInfo_t*> m_vecAllHooks;

};
inline Hook_t hook_t;



//======================= WORKS WITH SIGNATURES =======================
#define MAKE_HOOK(name, signature, callConvention, dll, returnType, ...) \
namespace Hook {\
    namespace name{\
        typedef returnType(callConvention* T_##name)(__VA_ARGS__);\
        inline T_##name O_##name = nullptr;\
        returnType callConvention H_##name(__VA_ARGS__); \
    }\
}\
namespace HOOK_TEMP{inline HookInfo_t temp_##name(signature, dll, reinterpret_cast<void*>(Hook::name::H_##name), reinterpret_cast<void**>(&Hook::name::O_##name), #name); }\
inline returnType callConvention Hook::name::H_##name(__VA_ARGS__)



//======================= WORKS WILL ACTUAL ADRS =======================
#define MAKE_HOOK_ADRS(name, pTargetFn, callConvention, dll, returnType, ...) \
namespace Hook {\
    namespace name{\
        typedef returnType(callConvention* T_##name)(__VA_ARGS__);\
        inline T_##name O_##name = nullptr;\
        returnType callConvention H_##name(__VA_ARGS__); \
    }\
}\
namespace HOOK_TEMP{inline HookInfo_t temp_##name(pTargetFn, dll, reinterpret_cast<void*>(Hook::name::H_##name), reinterpret_cast<void**>(&Hook::name::O_##name), #name); }\
inline returnType callConvention Hook::name::H_##name(__VA_ARGS__)



//======================= WORKS WITH VTABLES =======================
#define MAKE_HOOK_VTABLE(name, szInterfaceVersion, iIndex, callConvention, dll, returnType, ...) \
namespace Hook {\
    namespace name{\
        typedef returnType(callConvention* T_##name)(__VA_ARGS__);\
        inline T_##name O_##name = nullptr;\
        returnType callConvention H_##name(__VA_ARGS__); \
    }\
}\
namespace HOOK_TEMP{inline HookInfo_t temp_##name(szInterfaceVersion, iIndex, dll, reinterpret_cast<void*>(Hook::name::H_##name), reinterpret_cast<void**>(&Hook::name::O_##name), #name); }\
inline returnType callConvention Hook::name::H_##name(__VA_ARGS__)