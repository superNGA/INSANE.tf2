#pragma once
#include <vector>
#include "../Libraries/Utility/Utility.h"
#include "../External Libraries/MinHook/MinHook.h"

class HookInfo_t
{
public:
    HookInfo_t(const char* szSignature, const char* szDll, void* pHookFn, void** ppOriginalFn, const char* szFnName);

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

private:
    std::vector<HookInfo_t*> m_vecAllHooks = {};

};
inline Hook_t hook_t;

/* 
* I had 3 options for this macro. Either I can make it take a pointer to where the adrs will be store after processing
* or I can make it wait for signature scanning to complete
* or I can pass in the signature to it is independent of all other bullshit.
*/

#define MAKE_HOOK(name, signature, callConvention, dll, returnType, ...) \
namespace Hook {\
    namespace name{\
        typedef returnType(callConvention* T_##name)(__VA_ARGS__);\
        inline T_##name O_##name = nullptr;\
        returnType callConvention H_##name(__VA_ARGS__); \
    }\
}\
namespace HOOK_TEMP{HookInfo_t temp_##name(signature, dll, reinterpret_cast<void*>(Hook::name::H_##name), reinterpret_cast<void**>(&Hook::name::O_##name), #name); }\
returnType callConvention Hook::name::H_##name(__VA_ARGS__)