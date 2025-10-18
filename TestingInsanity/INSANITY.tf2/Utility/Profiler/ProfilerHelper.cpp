#include "ProfilerHelper.h"
#include "../ConsoleLogging.h"
#include "../../Features/ImGui/NotificationSystem/NotificationSystem.h"

constexpr size_t PROFILER_MAX_TIME_RECORDS = 1024;



///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
FuncInfoStorage_t::FuncInfoStorage_t()
{
    m_bInitialized = false;
    m_pQPrefixSum  = nullptr; m_pQTimeRecords = nullptr;
}


///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
void FuncInfoStorage_t::PushRecord(uint64_t iTimeInNs)
{
    if (m_bInitialized == false)
        m_bInitialized = _Init();

    // Add this shit to record.
    m_pQTimeRecords->push_back(iTimeInNs);

    // assert size.
    if (m_pQTimeRecords->size() > PROFILER_MAX_TIME_RECORDS)
        m_pQTimeRecords->pop_front();

    // maintain prefix sum.
    uint64_t iPrefix    = (m_pQPrefixSum->size() > 0llu ? m_pQPrefixSum->back() : 0llu);
    uint64_t iPrefixSum = iPrefix + iTimeInNs;
    m_pQPrefixSum->push_back(iPrefixSum);
    
    // if this occurs then the largest / final value in our prefix sum has some how managed to exceed the 
    // range of an uint64_t ( that can hold 584 years worth of time stored in nanoseconds ), so we subtract all by smallest value
    // to normalize. Something's gotta be wrong.
    if (iPrefix > iPrefixSum)
    {
        _NormalizePrefixSum();

        FAIL_LOG("Prfix Sum overflowed a fucking uint64_t. something must be wrong here.!");
        Render::notificationSystem.PushBack("Prefix Sum overflowed a fucking uint64_t. something must be wrong here.!");
    }

    // assert size for prefix sum.
    if (m_pQPrefixSum->size() > PROFILER_MAX_TIME_RECORDS)
        m_pQPrefixSum->pop_front();
}


///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
double FuncInfoStorage_t::GetAvgOf(int nSamples) const
{
    if (m_bInitialized == false)
        return -1.0;

    if (m_pQPrefixSum->size() <= 0LLU)
        return -1.0;

    if (m_pQPrefixSum->size() < nSamples)
        return -1.0;

    uint64_t nSamplesSafe = std::clamp<uint64_t>(static_cast<uint64_t>(nSamples), 1LLU, m_pQPrefixSum->size());

    uint64_t iTimeMax = m_pQPrefixSum->back();
    uint64_t iTimeMin = (*m_pQPrefixSum)[m_pQPrefixSum->size() - 1LLU - (nSamplesSafe - 1LLU)];

    return static_cast<double>(iTimeMax - iTimeMin) / static_cast<double>(nSamplesSafe);
}


///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
uint64_t FuncInfoStorage_t::Latest() const
{
    return m_pQPrefixSum->back();
}


///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
const std::deque<uint64_t>* FuncInfoStorage_t::GetTimeRecords() const
{
    return m_pQTimeRecords;
}


///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
bool FuncInfoStorage_t::_Init()
{
    m_pQPrefixSum   = new std::deque<uint64_t>;
    m_pQTimeRecords = new std::deque<uint64_t>;

    if (m_pQPrefixSum == nullptr || m_pQTimeRecords == nullptr)
        return false;

    //m_pQPrefixSum->resize(PROFILER_MAX_TIME_RECORDS);
    //m_pQTimeRecords->resize(PROFILER_MAX_TIME_RECORDS);

    return true;
}


///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
void FuncInfoStorage_t::_NormalizePrefixSum()
{
    uint64_t iOldestTime = m_pQPrefixSum->front();

    for (uint64_t time : *m_pQPrefixSum)
    {
        time -= iOldestTime;
    }
}
