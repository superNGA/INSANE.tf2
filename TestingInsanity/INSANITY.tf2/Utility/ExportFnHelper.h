#pragma once
#include <Windows.h>
#include <vector>
#include "ConsoleLogging.h"

class ExportFnHelper_t
{
public:
    ExportFnHelper_t(const char* szDll, const char* szFunctionName);
    bool Initialize();

    template <typename ReturnType, typename... Args>
    inline ReturnType Call(Args... args)
    {
        return (reinterpret_cast<ReturnType(__stdcall*)(Args...)>(m_pFunction))(args...);
    }

    uintptr_t m_pFunction = NULL;
    const char* m_szDll = "NULL";
    const char* m_szFunctionName = "NULL";
};

#define GET_EXPORT_FN(name, dll) namespace ExportFn{inline ExportFnHelper_t name(dll, #name);}

class AllExportFns_t
{
public:
    bool Initialize();
    void AddExportFn(ExportFnHelper_t* pExportFn) { m_vecAllExportFns.push_back(pExportFn); }

private:
    std::vector<ExportFnHelper_t*> m_vecAllExportFns = {};
};
inline AllExportFns_t allExportFns;