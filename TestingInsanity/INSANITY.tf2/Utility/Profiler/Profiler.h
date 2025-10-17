//=========================================================================
//                      PROFILER
//=========================================================================
// by      : INSANE
// created : 15/10/2025
// 
// purpose : Profiles functions & renders information
//-------------------------------------------------------------------------
#pragma once

#include <chrono>
#include <string>
#include <vector>
#include <deque>
#include <unordered_map>
#include "../Containers/DoubleBuffer.h"
#include "../../Features/FeatureHandler.h"

#include "ProfilerHelper.h"

class Profiler_t;
class ProfilerHelper_t;


////////////////////////////// MACROS ////////////////////////////////////
#define ENABLE_PROFILER true

#if (ENABLE_PROFILER == true)

    #define PROFILER_START_SCOPE()                  F::profiler.StartProfilingScope(__FUNCTION__);
    #define PROFILER_START_SCOPE_NAMED(szScopeName) F::profiler.StartProfilingScope(szScopeName);

    #define PROFILER_END_SCOPE()                    F::profiler.EndProfilingScope(__FUNCTION__);
    #define PROFILER_END_SCOPE_NAMED(szScopeName)   F::profiler.EndProfilingScope(szScopeName);

    #define PROFILER_RECORD_FUNCTION_NAMED(szFunctionName, szScopeName)\
        static FuncInfoStorage_t CONCAT(szScopeName);\
        ProfilerHelper_t CONCAT(szScopeName, __COUNTER__)(szFunctionName, #szScopeName, &szScopeName);

    #define PROFILER_RECORD_FUNCTION(szScopeName) PROFILER_RECORD_FUNCTION_NAMED(__FUNCTION__, szScopeName)

#else

    #define PROFILER_START_SCOPE()                                      void(0);
    #define PROFILER_START_SCOPE_NAMED(szScopeName)                     void(0);

    #define PROFILER_END_SCOPE()                                        void(0);
    #define PROFILER_END_SCOPE_NAMED(szScopeName)                       void(0);

    #define PROFILER_RECORD_FUNCTION_NAMED(szFunctionName, szScopeName) void(0);
    #define PROFILER_RECORD_FUNCTION(szScopeName)                       void(0);

#endif
///////////////////////////////////////////////////////////////////////////


///////////////////////////////////////////////////////////////////////////
class Profiler_t
{
public:
    Profiler_t();

    void Draw();

    void StartProfilingFunction(const char* szFuncName, const char* szScopeName);
    void EndProfilingFunction  (const char* szFuncName, const char* szScopeName, FuncInfoStorage_t* pFuncStorage);

    void StartProfilingScope   (const char* szScopeName);
    void EndProfilingScope     (const char* szScopeName);

private:
    using ProfilerScopeDB_t = Containers::DoubleBuffer_t<ProfilerScope_t>;

    std::unordered_map<uint32_t, ProfilerScopeDB_t*> m_umAllProfilerScopes;

    uint32_t _GetScopeID(const char* szScopeName);

    // Drawing Functions...
    void _UpdateScopeList();
    void _DrawFlameGraph();
    void _DrawFlamesInformation(const ProfilerFunc_t& funcProfile);
    void _GetTimeString(double flTimeInNs, std::string& szTimeOut);
};
///////////////////////////////////////////////////////////////////////////

DECLARE_FEATURE_OBJECT(profiler, Profiler_t)

DEFINE_TAB(Performance, 12)
DEFINE_SECTION(Profiler, "Performance", 1)

static const char* g_szDummyScopes[] = {"Scope1", "Scope2"};
DEFINE_FEATURE(Profiler_ActiveScope,      "Scope",          DropDown_t, Profiler, Performance, 1, DropDown_t(g_szDummyScopes, 2), FeatureFlag_SupportKeyBind)
DEFINE_FEATURE(Profiler_EnableFlameGraph, "Flame Graph",    bool, Profiler, Performance, 2, false, FeatureFlag_SupportKeyBind)
DEFINE_FEATURE(Profiler_UpdateRate,       "Profiling Rate ( sec )", FloatSlider_t, Profiler, Performance, 3, FloatSlider_t(1.0f, 0.0f, 60.0f), FeatureFlag_None)
DEFINE_FEATURE(Profiler_DrawFnNameOnFlame, "Flame Name",    bool, Profiler, Performance, 4, false, FeatureFlag_None)


///////////////////////////////////////////////////////////////////////////
class ProfilerHelper_t
{
public:
    ProfilerHelper_t(const char* szFunctionName, const char* szScopeName, FuncInfoStorage_t* pStorage)
    {
        m_szFunctionName = szFunctionName;
        m_szScopeName    = szScopeName;
        m_pStorage       = pStorage;

        F::profiler.StartProfilingFunction(m_szFunctionName, m_szScopeName);
    }
    ~ProfilerHelper_t()
    {
        F::profiler.EndProfilingFunction(m_szFunctionName, m_szScopeName, m_pStorage);
    }

private:
    const char*        m_szFunctionName;
    const char*        m_szScopeName;
    FuncInfoStorage_t* m_pStorage;
};
///////////////////////////////////////////////////////////////////////////

