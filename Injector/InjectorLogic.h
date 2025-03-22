//=========================================================================
//                      INJECTOR
//=========================================================================
// by      : INSANE
// created : 06/03/2025
// 
// purpose : injects dll into process, with multiple configuration avialable
//-------------------------------------------------------------------------
#pragma once
#include <Windows.h>

#ifdef NDEBUG
#define LOG(message) void(0)
#else
#define LOG(message) printf("%s\n", message)
#endif

#define MAX_PATH_LENGTH 260


//----------------------- Function Templates -----------------------
typedef BOOL(__stdcall* T_DllMain)(void*, DWORD, void*); // dllMain fn template
typedef uintptr_t(__stdcall* T_getProcAdrs)(HINSTANCE, const char*); // getProcAddress
typedef HINSTANCE(__stdcall* T_loadLIbraryA)(const char*); // loadLibraryA


enum injectionConfig_t
{
    LOAD_LIBRARY = 0,
    MANUAL_MAP
};


struct ManualMappingData_t
{
    T_loadLIbraryA  LoadLibraryA;
    T_getProcAdrs   GetProcAdrs;
    HINSTANCE       hModule;
};


class injector_t
{
public:
    bool        Intitialize(const char* targetProcName, const char* dllPath);
    bool        inject(injectionConfig_t method);

    // Target process imformation
    int         targetProcessID     = 0;
    char        _dllPath[MAX_PATH_LENGTH];

    // this is where we will be loading the image
    void*       pDllVirtualImage    = nullptr; 

private:
    int         _getProcessID(const char* procName);

    bool        _injectLoadLibrary();
    bool        _injectManualMap();

    const char* _targetProcessName  = "NONE";
    bool        _initialized        = false;
};  
extern injector_t injector;