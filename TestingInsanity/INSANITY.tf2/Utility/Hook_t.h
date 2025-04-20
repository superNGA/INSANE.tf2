#pragma once
#include <vector>
#include "../Libraries/Utility/Utility.h"
#include "../External Libraries/MinHook/MinHook.h"

#define CLIENT_DLL				"client.dll"
#define ENGINE_DLL				"engine.dll"
#define VGUI2_DLL				"vgui2.dll"
#define MATERIALSYSTEM_DLL		"materialsystem.dll"
#define STUDIORENDER_DLL		"studiorender.dll"
#define VSTDLIB_DLL				"vstdlib.dll"

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
    int GetNumberOfHooks() { return m_vecAllHooks.size(); }

private:
    std::vector<HookInfo_t*> m_vecAllHooks;

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
namespace HOOK_TEMP{inline HookInfo_t temp_##name(signature, dll, reinterpret_cast<void*>(Hook::name::H_##name), reinterpret_cast<void**>(&Hook::name::O_##name), #name); }\
inline returnType callConvention Hook::name::H_##name(__VA_ARGS__)