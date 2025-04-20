#pragma once
#include <vector>
#include "../Libraries/Utility/Utility.h"
extern Utility util;

#define CLIENT_DLL				"client.dll"
#define ENGINE_DLL				"engine.dll"
#define VGUI2_DLL				"vgui2.dll"
#define MATERIALSYSTEM_DLL		"materialsystem.dll"
#define STUDIORENDER_DLL		"studiorender.dll"
#define VSTDLIB_DLL				"vstdlib.dll"

class Interface_t
{
public:
    Interface_t(const char* szVersion, const char* szDll, void** pDestination);
    Interface_t(const char* signature, const char* szDll, void** ppDestination, uint32_t offset, const char* name);
    bool Initialize();

    uintptr_t m_pInterface    = NULL;
    const char* m_szIdentifier   = "NULL"; // version or signature
    const char* m_szDll       = "NULL";
    void** m_pDestination     = nullptr;
    bool m_bToBeScanned       = false;
    const char* m_szInterfaceName = "NULL";
    uint32_t m_iOffset = 0;
};

#define MAKE_INTERFACE_VERSION(name, version, type, dll) namespace I{inline type* name = nullptr;} \
namespace INTERFACE_TEMP_SCOPE{inline Interface_t temp_##name(version, dll, reinterpret_cast<void**>(&I::name));}

#define MAKE_INTERFACE_SIGNATURE(name, signature, type, dll, offset) namespace I{inline type* name = nullptr;}\
namespace INTERFACE_TEMP_SCOPE{inline Interface_t temp_##name(signature, dll, reinterpret_cast<void**>(&I::name), offset, #name);}

class InterfaceInitialize_t
{
public:
    bool Initialize();
    void AddInterface(Interface_t* pInterface) { m_vecInterfaces.push_back(pInterface); }

private:
    std::vector<Interface_t*> m_vecInterfaces = {};
};
inline InterfaceInitialize_t interfaceInitialize;