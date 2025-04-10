#pragma once
#include <vector>
#include "../Libraries/Utility/Utility.h"
extern Utility util;


class Interface_t
{
public:
    Interface_t(const char* szVersion, const char* szDll, void** pDestination);
    bool Initialize();

    uintptr_t m_pInterface  = NULL;
    const char* m_szVersion = "NULL";
    const char* m_szDll     = "NULL";
    void** m_pDestination   = nullptr;
};

#define MAKE_INTERFACE_VERSION(name, version, type, dll) namespace I{inline type* name = nullptr;} \
namespace INTERFACE_TEMP_SCOPE{inline Interface_t temp_##name(version, dll, reinterpret_cast<void**>(&I::name));}

class InterfaceInitialize_t
{
public:
    bool Initialize();
    void AddInterface(Interface_t* pInterface) { m_vecInterfaces.push_back(pInterface); }

private:
    std::vector<Interface_t*> m_vecInterfaces = {};
};
inline InterfaceInitialize_t interfaceInitialize;