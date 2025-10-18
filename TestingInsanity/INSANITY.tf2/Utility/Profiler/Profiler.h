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
#include "../../Features/ImGui/AnimationHandler.h"

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
    bool IsInfoPanelVisible();

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

    void _DrawInfoPanel();
    void _DrawGraph(ImVec2 vWindowSize, ImVec2 vWindowPos);
    void _DrawFuncList(const ProfilerScope_t* pScope, ImVec2 vWindowSize, ImVec2 vWindowPos);
    void _DrawScopeInfo(ImVec2 vWindowSize, ImVec2 vWindowPos);

    // Important graph variables.
    uint64_t m_iGraphRange      = 0llu;
    uint64_t m_iGraphBottomTime = 0llu;

    // NOTE : IMPORTANT : 
    //       This variable is only and only used the Draw() function of the profiler.
    // where we mutex lock the object who is holding pointer to this shit.
    // Race conditions can still occur, when some tries to store something into that object,
    // while we are reading :( . But this shouldn't cause a crash. :)
    //
    const FuncInfoStorage_t* m_pActiveGraphProfile = nullptr;
    const char* m_pActiveScopeName = nullptr;
    AnimationHandler_t       m_profilerAnim;
};
///////////////////////////////////////////////////////////////////////////

DECLARE_FEATURE_OBJECT(profiler, Profiler_t)

DEFINE_TAB(Performance, 12)
DEFINE_SECTION(Profiler, "Performance", 1)

static const char* g_szDummyScopes[] = {"Scope1", "Scope2"};
DEFINE_FEATURE(Profiler_ActiveScope,       "Scope",          DropDown_t, Profiler, Performance, 1, DropDown_t(g_szDummyScopes, 2), FeatureFlag_SupportKeyBind)
DEFINE_FEATURE(Profiler_EnableFlameGraph,  "Flame Graph",    bool, Profiler, Performance, 2, false, FeatureFlag_SupportKeyBind)
DEFINE_FEATURE(Profiler_UpdateRate,        "Profiling Rate ( sec )", FloatSlider_t, Profiler, Performance, 3, FloatSlider_t(1.0f, 0.0f, 60.0f), FeatureFlag_None)
DEFINE_FEATURE(Profiler_DrawFnNameOnFlame, "Flame Name",    bool, Profiler, Performance, 4, false, FeatureFlag_None)

DEFINE_SECTION(InfoPanel, "Performance", 2)
DEFINE_FEATURE(Profiler_InfoPanel,           "Info Panel",        bool,          InfoPanel, Performance, 5,  false, FeatureFlag_SupportKeyBind)
DEFINE_FEATURE(Profiler_InfoPanelMargin,     "Info Panel Margin", FloatSlider_t, InfoPanel, Performance, 6,  FloatSlider_t(100.0f, 0.0f, 500.0f), FeatureFlag_None)
DEFINE_FEATURE(Profiler_GraphRange,          "Graph Range",       IntSlider_t,   InfoPanel, Performance, 7,  IntSlider_t(64, 0, 1024), FeatureFlag_None)
DEFINE_FEATURE(Profiler_GraphRangeUpdateMax, "Range Update Max",  IntSlider_t,   InfoPanel, Performance, 8,  IntSlider_t(90, 0, 100), FeatureFlag_None)
DEFINE_FEATURE(Profiler_GraphRangeUpdateMin, "Range Update Min",  IntSlider_t,   InfoPanel, Performance, 9,  IntSlider_t(60, 0, 100), FeatureFlag_None)
DEFINE_FEATURE(Profiler_GraphRangeSmoothing, "Range Smoothing",   FloatSlider_t, InfoPanel, Performance, 10, FloatSlider_t(0.1f, 0.0f, 1.0f), FeatureFlag_None)
DEFINE_FEATURE(Profiler_GraphGrid,           "Grid",              bool,          InfoPanel, Performance, 11, true, FeatureFlag_None)
DEFINE_FEATURE(Profiler_GraphGridSize,       "Grid Size",         FloatSlider_t, InfoPanel, Performance, 12, FloatSlider_t(40.0f, 10.0f, 100.0f), FeatureFlag_None)
DEFINE_FEATURE(Profiler_GraphWidth,          "Graph Size",        FloatSlider_t, InfoPanel, Performance, 13, FloatSlider_t(0.7f, 0.2f, 0.7f), FeatureFlag_None)


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

