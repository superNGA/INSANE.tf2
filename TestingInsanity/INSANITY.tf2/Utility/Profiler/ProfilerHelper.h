//=========================================================================
//                      PROFILER HELPER
//=========================================================================
// by      : INSANE
// created : 15/10/2025
// 
// purpose : Structs to help profiler work efficiently and properly.
//-------------------------------------------------------------------------
#pragma once
#include <vector>
#include <deque>
#include <chrono>
#include <string>


struct ProfilerFunc_t;
struct ProfilerScope_t;
class  FuncInfoStorage_t;


static const char* szDefaultProfilerHelperName = "NO NAME!";


///////////////////////////////////////////////////////////////////////////
class FuncInfoStorage_t
{
public:
    // NOTE : Time recorded here is in Nanoseconds. don't be a fool and store something else.

    FuncInfoStorage_t();

    void   PushRecord(uint64_t iTimeInNs);
    double GetAvgOf(int nSamples);

private:
    bool _Init();
    bool m_bInitialized = false;

    void _NormalizePrefixSum();

    std::deque<uint64_t>* m_pQTimeRecords = nullptr;
    std::deque<uint64_t>* m_pQPrefixSum   = nullptr;
};
///////////////////////////////////////////////////////////////////////////


///////////////////////////////////////////////////////////////////////////
struct ProfilerFunc_t
{
    ProfilerFunc_t()
    {
        m_pInfoStorage = nullptr;
        m_iStackDepth  = -1; 
        m_iCallIndex   = -1;
        m_szFuncName   = szDefaultProfilerHelperName;
    }

    FuncInfoStorage_t* m_pInfoStorage;
    std::chrono::high_resolution_clock::time_point m_startTime;
    std::chrono::high_resolution_clock::time_point m_endTime;
    int32_t            m_iStackDepth;
    int32_t            m_iCallIndex;
    const char*        m_szFuncName;
};
///////////////////////////////////////////////////////////////////////////


///////////////////////////////////////////////////////////////////////////
class ProfilerScope_t
{
public:
    ProfilerScope_t()
    {
        m_vecProfiledFuncs.clear();
        m_qHelperStack.clear();
        m_szScopeName = szDefaultProfilerHelperName;
        m_bActive     = false;

        m_startTime   = std::chrono::high_resolution_clock::now();
        m_endTime     = std::chrono::high_resolution_clock::now();
    }

    std::deque <ProfilerFunc_t>                    m_qHelperStack;
    std::vector<ProfilerFunc_t>                    m_vecProfiledFuncs;
    std::chrono::high_resolution_clock::time_point m_startTime;
    std::chrono::high_resolution_clock::time_point m_endTime;
    const char*                                    m_szScopeName;
    bool                                           m_bActive;
};
///////////////////////////////////////////////////////////////////////////