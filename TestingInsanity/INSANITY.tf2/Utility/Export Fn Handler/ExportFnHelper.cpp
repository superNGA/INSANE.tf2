#include "ExportFnHelper.h"

IExportFnHelper_t::IExportFnHelper_t(const char* szDll, const char* szFunctionName)
{
    m_szDll = szDll;
    m_szFunctionName = szFunctionName;   

    allExportFns.AddExportFn(this);
}

bool IExportFnHelper_t::Initialize()
{
    auto pModule = GetModuleHandle(m_szDll);
    if (pModule == NULL)
        return false;

    m_pFunction = (uintptr_t)GetProcAddress(pModule, m_szFunctionName);
    return true;
}

bool AllExportFns_t::Initialize()
{
    for (auto* ExpFn : m_vecAllExportFns)
    {
        if (ExpFn->Initialize() == false)
        {
            FAIL_LOG("FAILED TO AQUIRE EXPORT FN : %s from %s", ExpFn->m_szFunctionName, ExpFn->m_szDll);
            return false;
        }

        WIN_LOG("captured fn [ %s ] from [ %s ]", ExpFn->m_szFunctionName, ExpFn->m_szDll);
    }

    WIN_LOG("Aquired all export fns");
    return true;
}