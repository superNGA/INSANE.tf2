#pragma once
#include <Windows.h>
#include <vector>
#include "../ConsoleLogging.h"

class IExportFnHelper_t
{
public:
    IExportFnHelper_t(const char* szDll, const char* szFunctionName);
    bool Initialize();

    uintptr_t m_pFunction = NULL;
    const char* m_szDll = "NULL";
    const char* m_szFunctionName = "NULL";
};

template <typename Output, typename... Args>
class ExportFnHelper : public IExportFnHelper_t
{
public:
    ExportFnHelper(const char* szDll, const char* szFunctionName) :
        IExportFnHelper_t(szDll, szFunctionName) {};
    inline Output operator()(Args... args)
    {
        return (reinterpret_cast<Output(__stdcall*)(Args...)>(m_pFunction))(args...);
    }
};

#define GET_EXPORT_FN(name, dll, returnType, ...) namespace ExportFn{inline ExportFnHelper<returnType, __VA_ARGS__> name(dll, #name);}
#define GET_EXPORT_FN_NO_ARGS(name, dll, returnType) namespace ExportFn{inline ExportFnHelper<returnType> name(dll, #name);}

class AllExportFns_t
{
public:
    bool Initialize();
    void AddExportFn(IExportFnHelper_t* pExportFn) { m_vecAllExportFns.push_back(pExportFn); }

private:
    std::vector<IExportFnHelper_t*> m_vecAllExportFns = {};
};
inline AllExportFns_t allExportFns;