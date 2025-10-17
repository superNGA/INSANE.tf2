#include "Profiler.h"
#include <thread>

#include "../../Libraries/Utility/Utility.h"
#include "../../Features/ImGui/MenuV2/MenuV2.h"
#include "../../Features/ImGui/NotificationSystem/NotificationSystem.h"
#include "../../Resources/Fonts/FontManager.h"

#include "../../SDK/class/IVEngineClient.h"


///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
Profiler_t::Profiler_t()
{
    m_umAllProfilerScopes.clear();
}


///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
void Profiler_t::Draw()
{
#if (ENABLE_PROFILER == false)
    return;
#endif

    _UpdateScopeList();

    // Flame graph...
    if (Features::Performance::Profiler::Profiler_EnableFlameGraph.IsActive() == true)
    {
        _DrawFlameGraph();
    }
}


///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
void Profiler_t::StartProfilingFunction(const char* szFuncName, const char* szScopeName)
{
    uint32_t iScopeID{ _GetScopeID(szScopeName) };


    // Get the scope for this function & make sure the scope is there.
    auto it = m_umAllProfilerScopes.find(iScopeID);
    assert(it != m_umAllProfilerScopes.end() && "Function scope is not registered!, check the spelling! (wanna be reverse engineer, can't spell shit correctly)");
    if (it == m_umAllProfilerScopes.end())
        return;


    ProfilerScope_t* pProfilerScope = it->second->GetWriteBuffer();
    DOUBLEBUFFER_AUTO_RELEASE_WRITEBUFFER_NOSWAP(it->second, pProfilerScope);

    // Scope inactive this iteration?
    if (pProfilerScope->m_bActive == false)
        return;


    // store timing information about this function & put it on the stack.
    ProfilerFunc_t funcProfile;
    funcProfile.m_iCallIndex   = pProfilerScope->m_vecProfiledFuncs.size();
    funcProfile.m_iStackDepth  = pProfilerScope->m_qHelperStack.size();
    funcProfile.m_szFuncName   = szFuncName;
    funcProfile.m_startTime    = std::chrono::high_resolution_clock::now();

    pProfilerScope->m_qHelperStack.push_front(funcProfile);
    LOG("started profiling function : %s", szFuncName);
}


///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
void Profiler_t::EndProfilingFunction(const char* szFuncName, const char* szScopeName, FuncInfoStorage_t* pFuncStorage)
{
    assert(pFuncStorage != nullptr && "function profiler storage can't be NULL");

    uint32_t iScopeID{ _GetScopeID(szScopeName) };

    
    // Get the scope for this function & make sure the scope is there.
    auto it = m_umAllProfilerScopes.find(iScopeID);
    assert(it != m_umAllProfilerScopes.end() && "Invalid scope name");
    if (it == m_umAllProfilerScopes.end())
        return;


    ProfilerScope_t* pProfilerScope = it->second->GetWriteBuffer();
    DOUBLEBUFFER_AUTO_RELEASE_WRITEBUFFER_NOSWAP(it->second, pProfilerScope);
    
    // Do we need to record this time?
    if (pProfilerScope->m_bActive == false)
        return;
    
    // If scope is active & we are trying to end a function profilling, these must be something in the heap.
    assert(pProfilerScope->m_qHelperStack.size() > 0 && "Trying to end profiling for function without starting it");
    
    ProfilerFunc_t& funcProfile = pProfilerScope->m_qHelperStack.front();
    funcProfile.m_pInfoStorage  = pFuncStorage;
    funcProfile.m_endTime       = std::chrono::high_resolution_clock::now();
    uint64_t iFnTimeInNs        = std::chrono::duration_cast<std::chrono::nanoseconds>(funcProfile.m_endTime - funcProfile.m_startTime).count();


    // Store this time in the history object.
    pFuncStorage->PushRecord(iFnTimeInNs);


    // Pop it from the heap & store in final destination.
    pProfilerScope->m_vecProfiledFuncs.push_back(funcProfile);
    pProfilerScope->m_qHelperStack.pop_front();

    LOG("%.6f microseconds for %s", static_cast<double>(iFnTimeInNs) / 1000.0, szFuncName);
}


///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
void Profiler_t::StartProfilingScope(const char* szScopeName)
{
    uint32_t iScopeID{ _GetScopeID(szScopeName) };


    // Try to find it in the unordered-map & add if doesn't exist.
    auto it = m_umAllProfilerScopes.find(iScopeID);
    if (it == m_umAllProfilerScopes.end())
    {
        ProfilerScopeDB_t* pProfilerScope = new Containers::DoubleBuffer_t<ProfilerScope_t>;
        m_umAllProfilerScopes.emplace(iScopeID, pProfilerScope);
        it = m_umAllProfilerScopes.find(iScopeID);

        // Shout at everyone if this shit fails. Cause its their fault.
        if (it == m_umAllProfilerScopes.end())
        {
            Render::notificationSystem.PushBack("FAILED TO ADD [ %s ] SCOPE TO TABLE", szScopeName);
            FAIL_LOG("Failed to add scope to map.");
            return;
        }

        WIN_LOG("Registered profiler scope [ %s ]", szScopeName);
    }


    ProfilerScope_t* pProfilerScope = it->second->GetWriteBuffer();
    assert(pProfilerScope->m_bActive == false && "Scope started without ending!!!");
    // NOTE : We are not swapping for this scope yet, we will only swap when "EndProfilingScope" is called.
    DOUBLEBUFFER_AUTO_RELEASE_WRITEBUFFER_NOSWAP(it->second, pProfilerScope);

    // Is it time to profile again?
    uint64_t iTimeSinceLastProfile = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - pProfilerScope->m_startTime).count();
    if (static_cast<float>(iTimeSinceLastProfile) / 1000.0f < Features::Performance::Profiler::Profiler_UpdateRate.GetData().m_flVal)
    {
        pProfilerScope->m_bActive = false;
        return;
    }
    
    // Clear out stuff from the last run.
    pProfilerScope->m_qHelperStack.clear();
    pProfilerScope->m_vecProfiledFuncs.clear();


    pProfilerScope->m_bActive     = true;
    pProfilerScope->m_szScopeName = szScopeName;
    pProfilerScope->m_startTime   = std::chrono::high_resolution_clock::now();
}


///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
void Profiler_t::EndProfilingScope(const char* szScopeName)
{
    uint32_t iScopeID{ _GetScopeID(szScopeName) };

    // This scope must be started!
    auto it = m_umAllProfilerScopes.find(iScopeID);
    assert(it != m_umAllProfilerScopes.end() && "Trying to end scope. but this scope is not registered.");
    if (it == m_umAllProfilerScopes.end())
        return;


    // Notice that we are swapping the double-buffer here.
    ProfilerScope_t* pProfilerScope = it->second->GetWriteBuffer();
    
    // In case, this iteration this scope was inactive, we don't sawp buffer, doing so would invalidate the other 
    // buffer, which is currently being used to render.
    if (pProfilerScope->m_bActive == false)
    {
        DOUBLEBUFFER_AUTO_RELEASE_WRITEBUFFER_NOSWAP(it->second, pProfilerScope);
        return;
    }

    DOUBLEBUFFER_AUTO_RELEASE_WRITEBUFFER_SWAP(it->second, pProfilerScope);

    pProfilerScope->m_endTime = std::chrono::high_resolution_clock::now();
    pProfilerScope->m_bActive = false;

    WIN_LOG("{ %d scopes }", m_umAllProfilerScopes.size());
}


///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
uint32_t Profiler_t::_GetScopeID(const char* szScopeName)
{
    return FNV1A32(szScopeName);
}


///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
void Profiler_t::_UpdateScopeList()
{
    // Due to the way that I have desined my fucking feature handler mechanism, we need to 
    // do this each time we need a dynamic drop down menu. Thats cause I am dumb.
    static const char* s_szScopeList[32] = {};
    int iCopyIndex = 0;

    for (auto& [scopeID, scope] : m_umAllProfilerScopes)
    {
        ProfilerScope_t* pProfilerScope = scope->GetReadBuffer();
        DOUBLEBUFFER_AUTO_RELEASE_READBUFFER(scope, pProfilerScope);

        s_szScopeList[iCopyIndex] = pProfilerScope->m_szScopeName;
        iCopyIndex++;
    }

    Features::Performance::Profiler::Profiler_ActiveScope.SetItems(s_szScopeList, iCopyIndex);
}


///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
void Profiler_t::_DrawFlameGraph()
{
    int iActiveScopeIndex = Features::Performance::Profiler::Profiler_ActiveScope.GetData();
    if (iActiveScopeIndex < 0)
        return;
    
    
    // Get the ProfilerScope_t object, which holds the informatoin we need to draw.
    const char* szActiveScopeName = Features::Performance::Profiler::Profiler_ActiveScope.m_data.m_pItems[iActiveScopeIndex];
    uint32_t    iScopeID          = _GetScopeID(szActiveScopeName);
    auto        it                = m_umAllProfilerScopes.find(iScopeID);
    if (it == m_umAllProfilerScopes.end())
        return; // well, I wasn't expecting this at all.
    

    // Extrat the object & don't forget to release.
    ProfilerScope_t* pScope = it->second->GetReadBuffer();
    DOUBLEBUFFER_AUTO_RELEASE_READBUFFER(it->second, pScope);
    if (pScope->m_vecProfiledFuncs.size() == 0LLU)
        return; // nothing to draw here.
    

    // Setting window's size & Pos
    int iHeight = 0, iWidth = 0; I::iEngine->GetScreenSize(iWidth, iHeight);
    // NOTE : Window size is set just before ending the flamegraph window ( ##Profiler_FlameGraph ), cause we need to find max
    //        call stack depth.
    ImGui::SetNextWindowPos(ImVec2(0.0f, 0.0f));

    // Styling window
    {
        ImGui::PushStyleColor(ImGuiCol_WindowBg, Render::menuGUI.GetPrimaryClr().GetAsImVec4());
        ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
        ImGui::PushFont(Resources::Fonts::JetBrainsMonoNerd_Small);
    }
    
    ImGuiWindowFlags iWindowFlags = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoNav | ImGuiWindowFlags_NoNavInputs;
    if (ImGui::Begin("##Profiler_FlameGraph", nullptr, iWindowFlags) == true)
    {
        ImVec2      vWindowSize    = ImGui::GetWindowSize();
        uint64_t    iScopeTimeInNs = std::chrono::duration_cast<std::chrono::nanoseconds>(pScope->m_endTime - pScope->m_startTime).count();
        ImDrawList* pDrawList      = ImGui::GetWindowDrawList();

        // Some constants for drawing flame.
        constexpr float FLAME_HEIGHT          = 30.0f; // Why 30? cause I feel like it.
        constexpr float FLAME_PADDING_IN_PXL  = 4.0f;
        constexpr float FLAME_ROUNDING_IN_PXL = POPUP_ROUNDING;

        int         iMaxStackDepth = 0;
        const float flCharWidth    = ImGui::GetFont()->GetCharAdvance(' ');

        for (const ProfilerFunc_t& funcProfile: pScope->m_vecProfiledFuncs)
        {
            // Time taken by Function.
            uint64_t iFuncTime = std::chrono::duration_cast<std::chrono::nanoseconds>(funcProfile.m_endTime - funcProfile.m_startTime).count();

            // Time between function start & scope start.
            uint64_t iTimeFromStart = std::chrono::duration_cast<std::chrono::nanoseconds>(funcProfile.m_startTime - pScope->m_startTime).count();

            float flFlameStartInPxl = vWindowSize.x * static_cast<float>(static_cast<double>(iTimeFromStart) / static_cast<double>(iScopeTimeInNs));
            float flFlameWidthInPxl = vWindowSize.x * static_cast<float>(static_cast<double>(iFuncTime)      / static_cast<double>(iScopeTimeInNs));


            // Flame min & max.
            ImVec2 vFlameMin(flFlameStartInPxl, FLAME_HEIGHT * static_cast<float>(funcProfile.m_iStackDepth));
            ImVec2 vFlameMax(vFlameMin.x + flFlameWidthInPxl, vFlameMin.y + FLAME_HEIGHT);
            
            // Adding padding, so adjacent boxes don't merge into each other.
            if(flFlameWidthInPxl > FLAME_PADDING_IN_PXL * 4.0f)
            { // If we remove padding worth more than flame width, a distorted ass flame will be rendered.
                vFlameMin.x += FLAME_PADDING_IN_PXL; vFlameMax.x -= FLAME_PADDING_IN_PXL;
            } 
            vFlameMin.y += FLAME_PADDING_IN_PXL; vFlameMax.y -= FLAME_PADDING_IN_PXL;

            // Calculating Flame's color.
            RGBA_t clrFlame = Render::menuGUI.GetSecondaryClr();
            clrFlame.LerpInPlace(Render::menuGUI.GetThemeClr(), flFlameWidthInPxl / vWindowSize.x, true, false);

            // Drawing the flame... ( its a fucking rectangle nigga )
            pDrawList->AddRectFilled(vFlameMin, vFlameMax, ImColor(clrFlame.GetAsImVec4()), FLAME_ROUNDING_IN_PXL);


            // When hovering, show the full information.
            if (ImGui::IsMouseHoveringRect(vFlameMin, vFlameMax) == true)
                _DrawFlamesInformation(funcProfile);


            // Drawing Fn Name on flame.
            if(Features::Performance::Profiler::Profiler_DrawFnNameOnFlame.IsActive() == true)
            {
                float flNameWidth = static_cast<float>(strlen(funcProfile.m_szFuncName)) * flCharWidth;
                if (flNameWidth + (2.0f * FLAME_PADDING_IN_PXL) < flFlameWidthInPxl)
                {
                    ImVec2 vNamePos(vFlameMin.x + (flFlameWidthInPxl - flNameWidth) / 2.0f, vFlameMin.y + ((vFlameMax.y - vFlameMin.y) - ImGui::GetTextLineHeight()) / 2.0f);
                    RGBA_t clrText; Render::menuGUI.CalcTextClrForBg(clrText, clrFlame);
                    pDrawList->AddText(vNamePos, ImColor(clrText.GetAsImVec4()), funcProfile.m_szFuncName);
                }
            }

            // Maintain max stack depth.
            if (funcProfile.m_iStackDepth > iMaxStackDepth)
                iMaxStackDepth = funcProfile.m_iStackDepth;
        }

        ImGui::SetWindowSize(ImVec2(static_cast<float>(iWidth), static_cast<float>(iMaxStackDepth + 1) * FLAME_HEIGHT));
        ImGui::End();
    }

    ImGui::PopFont(); ImGui::PopStyleVar(); ImGui::PopStyleColor();
}


///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
void Profiler_t::_DrawFlamesInformation(const ProfilerFunc_t& funcProfile)
{
    ImGui::PushStyleColor(ImGuiCol_PopupBg, Render::menuGUI.GetPrimaryClr().GetAsImVec4());
    ImGui::PushStyleColor(ImGuiCol_Border, Render::menuGUI.GetThemeClr().GetAsImVec4());
    ImGui::PushStyleVar(ImGuiStyleVar_PopupBorderSize, POPUP_BORDER_THICKNESS);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, POPUP_ROUNDING);

    RGBA_t clrText; Render::menuGUI.CalcTextClrForBg(clrText, Render::menuGUI.GetPrimaryClr());

    {
        ImGui::BeginTooltip();

        ImVec2      vWindowPos = ImGui::GetWindowPos();
        ImVec2      vCursorPos = vWindowPos;
        ImDrawList* pDrawList  = ImGui::GetWindowDrawList();

        ImGui::PushFont(Resources::Fonts::JetBrainsMonoNerd_Mid);
        ImGui::TextColored(Render::menuGUI.GetThemeClr().GetAsImVec4(), funcProfile.m_szFuncName);
        ImGui::PopFont();

        std::string szTimeString(""); 
        _GetTimeString(funcProfile.m_pInfoStorage->GetAvgOf(32), szTimeString);
        ImGui::TextColored(clrText.GetAsImVec4(), "32   Tick Avg : %s", szTimeString.c_str());
        _GetTimeString(funcProfile.m_pInfoStorage->GetAvgOf(64), szTimeString);
        ImGui::TextColored(clrText.GetAsImVec4(), "64   Tick Avg : %s", szTimeString.c_str());
        _GetTimeString(funcProfile.m_pInfoStorage->GetAvgOf(128), szTimeString);
        ImGui::TextColored(clrText.GetAsImVec4(), "128  Tick Avg : %s", szTimeString.c_str());
        _GetTimeString(funcProfile.m_pInfoStorage->GetAvgOf(256), szTimeString);
        ImGui::TextColored(clrText.GetAsImVec4(), "256  Tick Avg : %s", szTimeString.c_str());
        _GetTimeString(funcProfile.m_pInfoStorage->GetAvgOf(512), szTimeString);
        ImGui::TextColored(clrText.GetAsImVec4(), "512  Tick Avg : %s", szTimeString.c_str());
        _GetTimeString(funcProfile.m_pInfoStorage->GetAvgOf(1024), szTimeString);
        ImGui::TextColored(clrText.GetAsImVec4(), "1024 Tick Avg : %s", szTimeString.c_str());

        ImGui::EndTooltip();
    }

    ImGui::PopStyleVar(2); ImGui::PopStyleColor(2);
}


///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
void Profiler_t::_GetTimeString(double flTimeInNs, std::string& szTimeOut)
{
    szTimeOut.clear();

    // Invalid times?
    if(flTimeInNs < 0.0)
    {
        szTimeOut = "N/A";
        return;
    }


    if (flTimeInNs > 1000000.0)
    {
        szTimeOut = std::format("{:.3f}", flTimeInNs / 1000000.0);
        szTimeOut += " ms";
    }
    else if (flTimeInNs > 1000.0)
    {
        szTimeOut = std::format("{:.3f}", flTimeInNs / 1000.0);
        szTimeOut += " us";
    }
    else
    {
        szTimeOut = std::format("{:.3f}", flTimeInNs);
        szTimeOut += " ns";
    }
}


/* 

TODO : 
    -> Make the flame graph.
    -> Handle nothing to draw case.
    -> Add disabling options / Macro.
    
    -> Store scope's timings.
    -> Draw Scope's graph.
    -> Draw static panel, with all functions informatoin averaged out n stuff.

*/

