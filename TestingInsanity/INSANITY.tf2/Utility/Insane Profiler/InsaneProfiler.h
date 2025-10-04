//=========================================================================
//                      INSANE PROFILER
//=========================================================================
// by      : INSANE
// created : 30/06/2025
// 
// purpose : Record & displays time take by FNs in a "Reverse flame" like pattern
//-------------------------------------------------------------------------
#pragma once

// STD libs
#include <chrono>
#include <unordered_map>
#include <vector>
#include <stack>
#include <deque>
#include <Windows.h>
#include <string>

//#include "../../External Libraries/ImGui/imgui.h"

// UTILITY
#include "../ConsoleLogging.h"
#include "../../Features/FeatureHandler.h"


// This disable the entire profiler
#define ENABLE_INSANE_PROFILER true

#if (ENABLE_INSANE_PROFILER == true)

#define PROFILE_THREAD() \
{insaneProfiler.RegisterThread(GetCurrentThreadId(), __FUNCTION__);}\
ScopeTimer_t CONCAT(ScopeTimer_, __COUNTER__);


#define PROFILE_FUNCTION_FN_NAME()           FnTimer_t CONCAT(Timer_,__COUNTER__)(std::string(__FUNCTION__))
#define PROFILE_FUNCTION_CUSTOM_NAME(szName) FnTimer_t CONCAT(Timer_,__COUNTER__)(std::string(szName));

#define GET_THIRD_ARG(arg1, arg2, arg3, ...) arg3 // Simply returns the third argument

// Gets the suffix to choose between 2 macros.
// 1 argument -> _CUSTOM_NAME | 0 arguments -> _FN_NAME
#define GET_FN_NAME_SUFFIX(...) GET_THIRD_ARG(arg1, ##__VA_ARGS__, _CUSTOM_NAME,_FN_NAME)

// Merge 2 names tokens together
#define CHOOSE_PROFILER_FN(extension) CONCAT(PROFILE_FUNCTION, extension)

// Enter custom fn name or will use default one ( with namespace name n shit )
#define PROFILE_FUNCTION(...)   EXPAND(CHOOSE_PROFILER_FN(GET_FN_NAME_SUFFIX(##__VA_ARGS__))(__VA_ARGS__))


#else

#define PROFILE_THREAD()                     void(0)

#define PROFILE_FUNCTION_FN_NAME()           void(0)
#define PROFILE_FUNCTION_CUSTOM_NAME(szName) void(0)
#define GET_THIRD_ARG(arg1, arg2, arg3, ...) arg3
#define GET_FN_NAME_SUFFIX(...) GET_THIRD_ARG(arg1, ##__VA_ARGS__, _CUSTOM_NAME,_FN_NAME)
#define CHOOSE_PROFILER_FN(extension) CONCAT(PROFILE_FUNCTION, extension)

#define PROFILE_FUNCTION(...)   EXPAND(CHOOSE_PROFILER_FN(GET_FN_NAME_SUFFIX(##__VA_ARGS__))(__VA_ARGS__))

#endif

struct FnTimer_t;
struct ProfilerData_t;
struct ScopeData_t;
struct ImVec2;
struct ImDrawList;
struct ImColor;

namespace InsaneProfiler
{
    using ProfilerDataQueue = std::deque<ProfilerData_t>;
    using TimePoint         = std::chrono::high_resolution_clock::time_point;
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////
class InsaneProfiler_t
{
public:
    void RegisterThread(uint32_t iThreadID, std::string szName);

    void StartFunctionTimer(InsaneProfiler::TimePoint startTime);
    void StopFunctionTimer(InsaneProfiler::TimePoint endTime, std::string szName);

    void SetScopeStartTime(InsaneProfiler::TimePoint startTime, uint32_t iThreadID);
    void SetScopeEndTime(InsaneProfiler::TimePoint endTime, uint32_t iThreadID);

    void Uninitialize();

    // Rendering
    void Render();
    std::string GetTimeString(uint64_t iTimeInNs);
    void ConnectToTimeScale(ImVec2 vStart, ImVec2 vEnd, uint64_t iStartTimeInNs, uint64_t iEndTimeInNs, ImDrawList* pDrawList);
    std::string GetBlockName(ImVec2 vStart, ImVec2 vEnd, uint64_t iTimeConsumed, const std::string& szName, bool bIgnoreSizeConstraints);

private:
    std::unordered_map<uint32_t, ScopeData_t> m_mapRegisteredScopes = {};

    ImColor _GetFlameColor(uint32_t iHeight, float flIntensity);
    static constexpr uint8_t m_clrMin[4] = { 0,   255, 89, 255};
    static constexpr uint8_t m_clrMax[4] = { 255, 0,   0, 255};
    static constexpr float m_flMinAlpha  = 10.0f;
    static constexpr float m_flMaxAlpha  = 200.0f;
    static constexpr float m_flMaxHeight = 4.0f;

    uint8_t m_lastBaseClr[4] = { 0,0,0,0 };
};
inline InsaneProfiler_t insaneProfiler;
/////////////////////////////////////////////////////////////////////////////////////////////////////////



struct FnTimer_t
{
    FnTimer_t(std::string szName)
    {
        m_szName = szName;
        insaneProfiler.StartFunctionTimer(std::chrono::high_resolution_clock::now());
    }
    ~FnTimer_t()
    {
        insaneProfiler.StopFunctionTimer(std::chrono::high_resolution_clock::now(), m_szName);
    }
    std::string m_szName;
};
struct ScopeTimer_t
{
    ScopeTimer_t()
    {
        insaneProfiler.SetScopeStartTime(std::chrono::high_resolution_clock::now(), GetCurrentThreadId());
    }
    ~ScopeTimer_t()
    {
        insaneProfiler.SetScopeEndTime(std::chrono::high_resolution_clock::now(), GetCurrentThreadId());
    }
};



struct ProfilerData_t
{
    ProfilerData_t() : m_iStartTime(0), m_iEndTime(0) {};

    ProfilerData_t(uint64_t iStartTime, uint64_t iEndTime, std::string szName, uint32_t iHeight) :
        m_iStartTime(iStartTime), m_iEndTime(iEndTime), m_szName(szName),
        m_iHeight(iHeight) {}

    uint64_t m_iStartTime = 0;
    uint64_t m_iEndTime   = 0;
    uint32_t m_iHeight    = 0; // 4 bytes wasted cause I need to store nanoseconds :(

    std::string m_szName  = "";
};

namespace InsaneProfiler
{
    struct AvgProfilerData_t
    {
        std::string m_szName              = "";
        uint64_t    m_iAvgExecTimeInNs    = 0;
        uint32_t    m_iCount              = 0;
        float       m_flPercentageOfTotal = 0.0f;
    };

    struct ProfilerRenderData_t
    {
        ProfilerDataQueue m_qData;
        uint64_t          m_iScopeExecTimeInNs  = 0;

        std::deque<AvgProfilerData_t> m_qAvgData;
    };
}

struct ScopeData_t
{
    ScopeData_t()
    {
        m_lastBufferSawpTime = std::chrono::high_resolution_clock::now();
    }


    // Holds averaged-out data used for rendering.
    InsaneProfiler::ProfilerRenderData_t  m_pRenderDataBuffer[2];
    InsaneProfiler::TimePoint             m_lastBufferSawpTime;
    uint16_t                              m_iWriteBufferIndex = 0;
    InsaneProfiler::ProfilerRenderData_t* GetRenderData();
    void StoreRenderData();
    bool m_bSwapInProgress = false;

    // Timer data...
    InsaneProfiler::ProfilerDataQueue m_qScopeTimerData = {};
    std::deque<uint64_t>              m_stkScopeData    = {};

    // Scope's start time ( time since epoch )
    uint64_t m_iStartTime = 0;
    uint64_t m_iEndTime   = 0;

    std::string m_szScopeName;
};

////////////////////////  UI  //////////////////////////////////////////
//#if (ENABLE_INSANE_PROFILER == true)

DEFINE_TAB(Insane_Profiler, 12);
DEFINE_SECTION(Settings, "Insane_Profiler", 1);

DEFINE_FEATURE(
    Enable, bool, Settings, Insane_Profiler,
    1, true, FeatureFlag_SupportKeyBind,
    "Render profiler window"
)

DEFINE_FEATURE(
    UpdateRate_InSec, FloatSlider_t, Settings, Insane_Profiler,
    2, FloatSlider_t(1.0f, 0.0f, 10.0f), FeatureFlag_None,
    "Will update data after this much time ( in seconds )."
)

DEFINE_FEATURE(
    BG_Clr, ColorData_t, Settings, Insane_Profiler,
    3, ColorData_t(0.0f, 0.0f, 0.0f, 0.0f), FeatureFlags::FeatureFlag_SupportKeyBind,
    "Background color for your profiler window"
)

//#endif
////////////////////////////////////////////////////////////////////////
