#include "InsaneProfiler.h"
#include <Windows.h>
#include <numeric>

#include "../../External Libraries/ImGui/imgui.h"
#include "../ConsoleLogging.h"
#include "../../Extra/math.h"
#include <stack>

/*
make a stack at time of registering scope
put time starts on top
when time end calls. 
*/


void InsaneProfiler_t::RegisterThread(uint32_t iThreadID, std::string szName)
{
    auto it = m_mapRegisteredScopes.find(iThreadID);

    // if already registered
    if (it != m_mapRegisteredScopes.end()) 
    {
        // Checking if we have any data stored from last time.
        ScopeData_t& parentScope = it->second;
        if (parentScope.m_qScopeTimerData.empty() == false)
        {
            // storing data from last tick
            parentScope.StoreRenderData();
        }

        // Clrearing the stack just in case
        parentScope.m_stkScopeData = std::deque<uint64_t>();

        return;
    }

    // storing a copy with key as thread ID.
    ScopeData_t newScope;
    newScope.m_szScopeName = szName;
    m_mapRegisteredScopes.insert({ iThreadID, newScope });
}



void InsaneProfiler_t::StartFunctionTimer(InsaneProfiler::TimePoint startTime)
{
    uint32_t iThreadID = GetCurrentThreadId();
    auto     it        = m_mapRegisteredScopes.find(iThreadID);

    // Checking if a scope is registered against this region
    if (it == m_mapRegisteredScopes.end())
    {
        FAIL_LOG("This region isn't scopped!");
        return;
    }

    ScopeData_t& parentScope = it->second;
    uint64_t     iStartTime  = std::chrono::duration_cast<std::chrono::nanoseconds>(startTime.time_since_epoch()).count() - parentScope.m_iStartTime;
    
    parentScope.m_stkScopeData.push_back(iStartTime);
}



void InsaneProfiler_t::StopFunctionTimer(InsaneProfiler::TimePoint endTime, std::string szName)
{
    uint32_t iScopeHash = GetCurrentThreadId();
    auto     it         = m_mapRegisteredScopes.find(iScopeHash);

    // NOTE : a check for scope isn't done here, since one has already been done when starting the timer.

    ScopeData_t& parentScope = it->second;
    uint64_t     iEndTime    = std::chrono::duration_cast<std::chrono::nanoseconds>(endTime.time_since_epoch()).count() - parentScope.m_iStartTime;
    
    // Adding timer data to the temp queue
    ProfilerData_t iTimerData(parentScope.m_stkScopeData[parentScope.m_stkScopeData.size() - 1], iEndTime, szName, parentScope.m_stkScopeData.size());
    parentScope.m_qScopeTimerData.push_front(iTimerData);

    // removing the element we just added from the stack
    parentScope.m_stkScopeData.pop_back();
}



void InsaneProfiler_t::SetScopeStartTime(InsaneProfiler::TimePoint startTime, uint32_t iThreadID)
{
    m_mapRegisteredScopes.find(iThreadID)->second.m_iStartTime = std::chrono::duration_cast<std::chrono::nanoseconds>(startTime.time_since_epoch()).count();
}


void InsaneProfiler_t::SetScopeEndTime(InsaneProfiler::TimePoint endTime, uint32_t iThreadID)
{
    auto&    scopeData   = m_mapRegisteredScopes.find(iThreadID)->second;
    scopeData.m_iEndTime = std::chrono::duration_cast<std::chrono::nanoseconds>(endTime.time_since_epoch()).count();
}



void InsaneProfiler_t::Uninitialize()
{
    return;
}


#if (ENABLE_INSANE_PROFILER == true)

void InsaneProfiler_t::Render()
{
    if (Features::Insane_Profiler::Settings::Enable.IsActive() == false)
        return;

    bool  bOpen = true;
    auto& io    = ImGui::GetIO();

    uint32_t        iMaxheight = 1;
    constexpr float flDefaultHeight  = 20.0f;
    ImGui::SetNextWindowSize({ io.DisplaySize.x, 0.0f });
    ImGui::SetNextWindowPos({ 0.0f, 0.0f });
    
    // Setting window's BG clr
    const auto& bgClr = Features::Insane_Profiler::Settings::BG_Clr.GetData();
    ImGui::PushStyleColor(ImGuiCol_WindowBg, {bgClr.r, bgClr.g, bgClr.b, bgClr.a});
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);

    // open window
    ImGui::Begin("Insane Profiler", &bOpen, 
        ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_AlwaysAutoResize |
        ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoDecoration);

    ImVec2 vOrigin   = ImGui::GetCursorPos();
    auto*  pDrawList = ImGui::GetWindowDrawList();

    for (auto& [iThreadID, parentScope] : m_mapRegisteredScopes)
    {
        // Getting data to render
        const auto* qRenderData = parentScope.GetRenderData();
        if (qRenderData->m_qData.empty() == true)
            continue;

        // Drawing the scale ( on top )
        {
            constexpr float flScaleStepSize   = 0.25f;
            constexpr float flScaleMarkLength = 6.0f;
            ImVec2 flTimeStringSize;
            int nScaleMarks = static_cast<int>(1.0f / flScaleStepSize);
            for (int i = 0; i <= nScaleMarks; i++)
            {
                float flPos = i * flScaleStepSize * io.DisplaySize.x;
                pDrawList->AddLine({ flPos, vOrigin.y }, { flPos, vOrigin.y + flScaleMarkLength }, ImColor(255, 255, 255, 255), 5.0f);
                
                // Drawing time's text
                std::string szTime = GetTimeString(i * flScaleStepSize * qRenderData->m_iScopeExecTimeInNs);
                flTimeStringSize = ImGui::CalcTextSize(szTime.c_str());
                pDrawList->AddText({ flPos - (flTimeStringSize.x / 2.0f), vOrigin.y + 5.0f }, ImColor(255, 255, 255, 255), szTime.c_str());
            }

            // so we don't draw over the scale.
            vOrigin.y += flScaleMarkLength + flTimeStringSize.y + 2.0f; // extra 2 pixels just for padding.
        }

        /*
        We need to draw dashed lines from some boxes to the scale to represent the start & end times
        also write the time comsumed at the end of the names. 
        do this for all height 0 boxes and for hovered boxes.
        */

        // Drawing each timer record on top of that scope box we just made
        for (const auto& data : qRenderData->m_qData)
        {
            float flStart = Maths::RemapValClamped(data.m_iStartTime, 0, qRenderData->m_iScopeExecTimeInNs, 0, io.DisplaySize.x);
            float flEnd   = Maths::RemapValClamped(data.m_iEndTime,   0, qRenderData->m_iScopeExecTimeInNs, 0, io.DisplaySize.x);
            
            if (flEnd >= io.DisplaySize.x)
                FAIL_LOG("Scope avg. %llu | start [ %llu ] | end [ %llu ]", qRenderData->m_iScopeExecTimeInNs, data.m_iEndTime, data.m_iStartTime);

            // Box's measurments
            float flTop    = vOrigin.y + (flDefaultHeight * (data.m_iHeight - 1));
            float flBottom = vOrigin.y + (flDefaultHeight * data.m_iHeight);
            ImVec2 vStart{ flStart, flTop };
            ImVec2 vEnd{   flEnd,   flBottom };

            // Drawing a rectangle represent the time taken
            pDrawList->AddRectFilled(vStart, vEnd, _GetFlameColor(data.m_iHeight, (flEnd - flStart) / io.DisplaySize.x), 1.0f);

            // Connecting to scale
            if (data.m_iHeight == 1)
            {
                ConnectToTimeScale(vStart, { vEnd.x, vStart.y }, data.m_iStartTime, data.m_iEndTime, pDrawList);
            }

            // Drawing FN name
            std::string szStatString = GetBlockName(vStart, vEnd, data.m_iEndTime - data.m_iStartTime, data.m_szName, false);
            float       flStatStringWidth  = ImGui::CalcTextSize(szStatString.c_str()).x;
            if (szStatString != "")
            {
                pDrawList->AddText(
                    { flStart + ((flEnd - flStart) - flStatStringWidth) / 2.0f, flTop }, // Centering the text in the box
                    ImColor(255, 255, 255, 255), szStatString.c_str());
            }

            if (ImGui::IsMouseHoveringRect(vStart, vEnd, true) == true)
            {
                ImGui::SetTooltip(GetBlockName(vStart, vEnd, data.m_iEndTime - data.m_iStartTime, data.m_szName, true).c_str());

                // if being hovered over & not on level 1 then draw start & end times
                if (data.m_iHeight > 1)
                {
                    ConnectToTimeScale(vStart, { vEnd.x, vStart.y }, data.m_iStartTime, data.m_iEndTime, pDrawList);
                }
            }

            // Updating Max height
            if (data.m_iHeight > iMaxheight)
                iMaxheight = data.m_iHeight;
        }

        // Updating cursor pos so auto-resize could work
        ImGui::SetCursorPosY(vOrigin.y + (iMaxheight * flDefaultHeight));
    }

    ImGui::End();
    ImGui::PopStyleVar();
    ImGui::PopStyleColor();
}

#else

void InsaneProfiler_t::Render() { return; }

#endif


std::string InsaneProfiler_t::GetTimeString(uint64_t iTimeInNs)
{
    if (iTimeInNs / int(1e3) == 0)
    {
        return std::format("{}ns", iTimeInNs);
    }
    else if (iTimeInNs / int(1e6) == 0)
    {
        return std::format("{}us", iTimeInNs / int(1e3));
    }
    else if (iTimeInNs / int(1e9) == 0)
    {
        return std::format("{}ms", iTimeInNs / int(1e6));
    }
    else
    {
        return std::format("{}s", iTimeInNs / int(1e9));
    }

}


void InsaneProfiler_t::ConnectToTimeScale(ImVec2 vStart, ImVec2 vEnd, uint64_t iStartTimeInNs, uint64_t iEndTimeInNs, ImDrawList* pDrawList)
{   
    std::string szTimeString    = GetTimeString(iStartTimeInNs);
    ImVec2      vTimeStringSize = ImGui::CalcTextSize(szTimeString.c_str());

    // Don't draw if box not big enough
    if (vEnd.x - vStart.x < vTimeStringSize.x * 2.0f)
        return;

    // START TIME
    // Drawing line connecting to scale & time
    pDrawList->AddLine(vStart, { vStart.x, vTimeStringSize.y }, ImColor(255, 255, 255, 255), 2.0f);
    pDrawList->AddText({ vStart.x, /*vPos.y*/0.0f }, ImColor(255, 255, 255, 255), szTimeString.c_str());
    
    // END TIME
    // Drawing line connecting to scale & time
    szTimeString    = GetTimeString(iEndTimeInNs);
    vTimeStringSize = ImGui::CalcTextSize(szTimeString.c_str());

    pDrawList->AddLine(vEnd, { vEnd.x, vTimeStringSize.y }, ImColor(255, 255, 255, 255), 2.0f);
    pDrawList->AddText({ vEnd.x - vTimeStringSize.x, /*vPos.y*/0.0f }, ImColor(255, 255, 255, 255), szTimeString.c_str());
}


std::string InsaneProfiler_t::GetBlockName(ImVec2 vStart, ImVec2 vEnd, uint64_t iTimeConsumed, const std::string& szName, bool bIgnoreSizeConstraints = false)
{
    if (bIgnoreSizeConstraints == true)
    {
        return std::string(szName + " (" + GetTimeString(iTimeConsumed) + ")");
    }
    
    std::string szTimeString = std::string("(" + GetTimeString(iTimeConsumed) + ")"); // std::string most probably has operator for + but const char might not have that. so we used converted the open bracket to std::string.
    ImVec2 vTimeStringSize = ImGui::CalcTextSize(szTimeString.c_str());

    float flWidth = vEnd.x - vStart.x;

    if (flWidth < vTimeStringSize.x)
        return "";

    ImVec2 vNameStringSize = ImGui::CalcTextSize(szName.c_str());
    
    // if can only can only fit time, then return time.
    if (flWidth < vNameStringSize.x)
        return szTimeString;

    // if can only fit either time or name, then draw name
    if (flWidth < vNameStringSize.x + vTimeStringSize.x)
        return szName;

    // else print both time and name
    return szName + szTimeString;
}


ImColor InsaneProfiler_t::_GetFlameColor(uint32_t iHeight, float flIntensity)
{
    float flPercentage = std::clamp(flIntensity, 0.0f, 1.0f);

    // Making the base color for the flame between the min color & the max color
    if (iHeight == 1)
    {
        m_lastBaseClr[0] = m_clrMin[0] + static_cast<uint8_t>((static_cast<float>(m_clrMax[0] - m_clrMin[0]) * flPercentage));
        m_lastBaseClr[1] = m_clrMin[1] + static_cast<uint8_t>((static_cast<float>(m_clrMax[1] - m_clrMin[1]) * flPercentage));
        m_lastBaseClr[2] = m_clrMin[2] + static_cast<uint8_t>((static_cast<float>(m_clrMax[2] - m_clrMin[2]) * flPercentage));
    }

    // Adjusting the darkness level according to the height.
    uint8_t clrFlame[4] = {
        static_cast<uint8_t>(Maths::RemapValClamped(iHeight, 0.0f, m_flMaxHeight, static_cast<float>(m_lastBaseClr[0]), 50.0f)),
        static_cast<uint8_t>(Maths::RemapValClamped(iHeight, 0.0f, m_flMaxHeight, static_cast<float>(m_lastBaseClr[1]), 50.0f)),
        static_cast<uint8_t>(Maths::RemapValClamped(iHeight, 0.0f, m_flMaxHeight, static_cast<float>(m_lastBaseClr[2]), 50.0f)),
        static_cast<uint8_t>(Maths::RemapValClamped(iHeight, 0.0f, m_flMaxHeight, m_flMinAlpha, m_flMaxAlpha))
    };

    return ImColor(clrFlame[0], clrFlame[1], clrFlame[2], clrFlame[3]);
}


void ScopeData_t::StoreRenderData()
{
    // Making sure all TimeRecords are of the same size.
    if (m_qScopeTimerData.size() <= 0)
        return;

    // Storing scope's total execution time for this data
    m_pRenderDataBuffer[m_iWriteBufferIndex].m_iScopeExecTimeInNs = m_iEndTime - m_iStartTime;

    // Storing this data in write buffer
    m_bSwapInProgress = true;
    m_pRenderDataBuffer[m_iWriteBufferIndex].m_qData = std::move(m_qScopeTimerData);
    m_qScopeTimerData.clear();
    m_bSwapInProgress = false;
}


InsaneProfiler::ProfilerRenderData_t* ScopeData_t::GetRenderData()
{
    if (m_bSwapInProgress == false)
    {
        auto  now                   = std::chrono::high_resolution_clock::now();
        float flTimeSinceLastUpdate = static_cast<float>(std::chrono::duration_cast<std::chrono::milliseconds>(now - m_lastBufferSawpTime).count()) / 1000.0f;
        
        // SWAPPING BUFFER if enough time has gone by.
        if (flTimeSinceLastUpdate > Features::Insane_Profiler::Settings::UpdateRate_InSec.GetData().m_flVal)
        {
            m_iWriteBufferIndex  = (m_iWriteBufferIndex == 0 ? 1 : 0);
            m_lastBufferSawpTime = now;
        }
    }

    return m_iWriteBufferIndex == 0 ? &m_pRenderDataBuffer[1] : &m_pRenderDataBuffer[0];
}