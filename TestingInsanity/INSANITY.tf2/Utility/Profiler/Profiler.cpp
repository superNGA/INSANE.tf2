#include "Profiler.h"
#include <thread>

#include "../../Libraries/Utility/Utility.h"
#include "../../Features/ImGui/MenuV2/MenuV2.h"
#include "../../Features/ImGui/NotificationSystem/NotificationSystem.h"
#include "../../Resources/Fonts/FontManager.h"
#include "../../Extra/math.h"

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
    PROFILER_RECORD_FUNCTION(EndScene);

    _UpdateScopeList();
    m_profilerAnim.CalculateAnim();

    // Flame graph...
    if (Features::Performance::Profiler::Profiler_EnableFlameGraph.IsActive() == true)
    {
        _DrawFlameGraph();
    }

    // Information panel...
    if (Features::Performance::InfoPanel::Profiler_InfoPanel.IsActive() == true)
    {
        _DrawInfoPanel();
    }
}


///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
bool Profiler_t::IsInfoPanelVisible()
{
    return Features::Performance::InfoPanel::Profiler_InfoPanel.IsActive();
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
    funcProfile.m_iCallIndex   = pProfilerScope->m_iCallCounter; pProfilerScope->m_iCallCounter += 1;
    funcProfile.m_iStackDepth  = pProfilerScope->m_qHelperStack.size();
    funcProfile.m_szFuncName   = szFuncName;
    funcProfile.m_startTime    = std::chrono::high_resolution_clock::now();

    pProfilerScope->m_qHelperStack.push_front(funcProfile);
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


    pProfilerScope->m_iCallCounter = 0;
    pProfilerScope->m_bActive      = true;
    pProfilerScope->m_szScopeName  = szScopeName;
    pProfilerScope->m_startTime    = std::chrono::high_resolution_clock::now();
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
    pProfilerScope->m_profileRecords.PushRecord(std::chrono::duration_cast<std::chrono::nanoseconds>(pProfilerScope->m_endTime - pProfilerScope->m_startTime).count());
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


///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
void Profiler_t::_DrawInfoPanel()
{
    int iScreenWidth = 0, iScreenHeight = 0; I::iEngine->GetScreenSize(iScreenWidth, iScreenHeight);
    ImVec2 vScreenSize(static_cast<float>(iScreenWidth), static_cast<float>(iScreenHeight));


    // Window's size & pos
    const float flMargin = Features::Performance::InfoPanel::Profiler_InfoPanelMargin.GetData().m_flVal;
    ImVec2 vWindowSize(vScreenSize.x - (2.0f * flMargin), vScreenSize.y - (2.0f * flMargin));
    ImVec2 vWindowPos(flMargin, flMargin);

    ImGui::SetNextWindowSize(vWindowSize); ImGui::SetNextWindowPos(vWindowPos);

    // Styling window
    {
        ImGui::PushStyleColor(ImGuiCol_WindowBg, Render::menuGUI.GetPrimaryClr().GetAsImVec4());
        ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
    }


    // NOTE : Aquire and lock the "ProfilerScope_t" object, so this remains thread safe & we don't encounter any race conditions.
    // The ProfilerScope_t object ( that holds all information required for rendering ) will be kept locked until we finish 
    // drawing, where it will be release automatically via the Macro "DOUBLEBUFFER_AUTO_RELEASE_READBUFFER".
    ProfilerScope_t* pScope = nullptr;
    int iActiveScopeIndex = Features::Performance::Profiler::Profiler_ActiveScope.GetData();
    if (iActiveScopeIndex < 0)
        return;

    // Get the ProfilerScope_t object, which holds the informatoin we need to draw.
    const char* szActiveScopeName = Features::Performance::Profiler::Profiler_ActiveScope.m_data.m_pItems[iActiveScopeIndex];
    uint32_t    iScopeID = _GetScopeID(szActiveScopeName);
    auto        it = m_umAllProfilerScopes.find(iScopeID);
    if (it == m_umAllProfilerScopes.end())
        return; // well, I wasn't expecting this at all.

    pScope = it->second->GetReadBuffer();
    DOUBLEBUFFER_AUTO_RELEASE_READBUFFER(it->second, pScope);


    ImGuiWindowFlags iWindowFlags = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoNav | ImGuiWindowFlags_NoNavInputs | ImGuiWindowFlags_NoBringToFrontOnFocus;
    if (ImGui::Begin("##Profiler_InfoPanel", nullptr, iWindowFlags) == true)
    {
        const     float flGraphWidthFraction     = Features::Performance::InfoPanel::Profiler_GraphWidth.GetData().m_flVal;
        constexpr float FUNC_LIST_OFFSET_IN_PXL  = 100.0f;
        constexpr float SCOPE_INFO_WIDTH         = 300.0f;
        ImVec2 vGraphWindowPos    (vWindowPos.x + (vWindowSize.x * (1.0f - flGraphWidthFraction)), vWindowPos.y);
        ImVec2 vGraphWindowSize   (vWindowSize.x * flGraphWidthFraction, vWindowSize.y);

        ImVec2 vFuncListWindowSize(vWindowSize.x * (1.0f - flGraphWidthFraction), vWindowSize.y - FUNC_LIST_OFFSET_IN_PXL);
        ImVec2 vFuncListWindowPos (vWindowPos.x, vWindowPos.y + FUNC_LIST_OFFSET_IN_PXL);

        ImVec2 vScopeInfoPos      (vGraphWindowPos.x + 10.0f, vGraphWindowPos.y + 10.0f);
        ImVec2 vScopeInfoSize     (SCOPE_INFO_WIDTH, -1.0f); // Height gets calculated in the function  _DrawScopeInfo();


        // Drawing all functions in the scope...
        _DrawFuncList(pScope, vFuncListWindowSize, vFuncListWindowPos);

        // Drawing Graph...
        _DrawGraph(vGraphWindowSize, vGraphWindowPos);

        // Drawing Scope's information...
        _DrawScopeInfo(vScopeInfoSize, vScopeInfoPos);

        ImGui::End();
    }

    ImGui::PopStyleVar(); ImGui::PopStyleColor();
}


///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
void Profiler_t::_DrawGraph(ImVec2 vWindowSize, ImVec2 vWindowPos)
{
    ImVec2 vParentWindowPos(ImGui::GetWindowPos()); ImVec2 vParentWindowSize(ImGui::GetWindowSize());

    // Styling
    RGBA_t clrText; Render::menuGUI.CalcTextClrForBg(clrText, Render::menuGUI.GetSecondaryClr());
    {
        ImGui::PushStyleColor(ImGuiCol_ChildBg, Render::menuGUI.GetSecondaryClr().GetAsImVec4());
    }

    ImGui::SetNextWindowPos(vWindowPos);
    ImGui::BeginChild("##Profiler_InfoPanel_Graph", vWindowSize);
    if (m_pActiveGraphProfile != nullptr)
    {
        ImDrawList* pDrawList = ImGui::GetWindowDrawList();


        // Drawing a grid made of dotted lines behing the graph. so it looks nice. :) ( at the cost of horrible loops )
        if(Features::Performance::InfoPanel::Profiler_GraphGrid.IsActive() == true)
        {
            const float flGridSize = Features::Performance::InfoPanel::Profiler_GraphGridSize.GetData().m_flVal;

            RGBA_t clrGrid; Render::menuGUI._FindElevatedClr(clrGrid, Render::menuGUI.GetSecondaryClr());
            ImVec2 vGridCursorPos(vWindowPos);
            while(vGridCursorPos.y < vWindowPos.y + vWindowSize.y)
            {
                vGridCursorPos.y += flGridSize;

                while (vGridCursorPos.x < vWindowPos.x + vWindowSize.x)
                {
                    pDrawList->AddLine(vGridCursorPos, ImVec2(vGridCursorPos.x + 2.0f, vGridCursorPos.y), ImColor(clrGrid.GetAsImVec4()));
                    vGridCursorPos.x += 4.0f;
                }

                vGridCursorPos.x = vWindowPos.x;
            }

            vGridCursorPos = vWindowPos;
            while (vGridCursorPos.x < vWindowPos.x + vWindowSize.x)
            {
                vGridCursorPos.x += flGridSize;

                while(vGridCursorPos.y < vWindowPos.y + vWindowSize.y)
                {
                    pDrawList->AddLine(vGridCursorPos, ImVec2(vGridCursorPos.x, vGridCursorPos.y + 2.0f), ImColor(clrGrid.GetAsImVec4()));
                    vGridCursorPos.y += 4.0f;
                }

                vGridCursorPos.y = vWindowPos.y;
            }
        }


        const std::deque<uint64_t>* pQTimeRecords = m_pActiveGraphProfile->GetTimeRecords();

        // Drawing data on the graph...
        constexpr float flMarginInPxl = 20.0f;

        ImVec2 vCursorPos(vWindowPos.x + vWindowSize.x, vWindowPos.y + vWindowSize.y);


        uint64_t iSmallestTime      = 0xFFffFFffFFffFFffLLU;
        uint64_t iLargestTime       = 0llu;
        //uint64_t iSmallestTime      = m_iGraphBottomTime;
        //uint64_t iLargestTime       = m_iGraphBottomTime + 1LLU;

        size_t   nRecords           = pQTimeRecords->size();
        size_t   nWishRecords       = static_cast<size_t>(Features::Performance::InfoPanel::Profiler_GraphRange.GetData().m_iVal);
        size_t   iRecordsToDisplay  = Maths::MIN<size_t>(nWishRecords, nRecords);
        float    flGapBtwPoints     = (vWindowSize.x) / static_cast<float>(iRecordsToDisplay + 1LLU);

        ImVec2 vLastPoint(-1.0f, -1.0f);
        for (size_t iRecordIndex = nRecords - 1LLU; iRecordIndex > nRecords - iRecordsToDisplay; iRecordIndex--)
        {
            uint64_t iNodeTime = (*pQTimeRecords)[iRecordIndex];

            if (iNodeTime < iSmallestTime)
            {
                iSmallestTime = iNodeTime;
            }
            if (iNodeTime > iLargestTime)
            {
                iLargestTime = iNodeTime;
            }

            iNodeTime -= m_iGraphBottomTime;

            ImVec2 vNodePos( vCursorPos.x,
                (vCursorPos.y /*- flMarginInPxl*/) - ((vWindowSize.y/* - flMarginInPxl*/) * static_cast<float>(static_cast<double>(iNodeTime) / static_cast<double>(m_iGraphRange))));

            // If not first record, the draw line connect this point to last one.
            if (iRecordIndex != nRecords - 1LLU)
            {
                pDrawList->AddLine(vLastPoint, vNodePos, ImColor(clrText.GetAsImVec4()), 2.0f);
            }

            constexpr float flNodeRadius = 4.0f;
            pDrawList->AddCircleFilled(vNodePos, flNodeRadius, ImColor(Render::menuGUI.GetThemeClr().GetAsImVec4()), 10);

            bool bNodeHovered = ImGui::IsMouseHoveringRect(
                ImVec2(vNodePos.x - (flNodeRadius * 2.0f), vNodePos.y - (flNodeRadius * 2.0f)),
                ImVec2(vNodePos.x + (flNodeRadius * 2.0f), vNodePos.y + (flNodeRadius * 2.0f)),
                false);
            if (bNodeHovered == true)
            {
                ImGui::PushStyleColor(ImGuiCol_PopupBg, Render::menuGUI.GetPrimaryClr().GetAsImVec4());
                ImGui::PushStyleColor(ImGuiCol_Border, Render::menuGUI.GetThemeClr().GetAsImVec4());
                ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, POPUP_ROUNDING);

                std::string szTimeString(""); _GetTimeString(static_cast<double>(iNodeTime), szTimeString);
                ImGui::SetTooltip("%s", szTimeString.c_str());

                ImGui::PopStyleColor(2); ImGui::PopStyleVar();
            }
            

            vLastPoint    = vNodePos;
            vCursorPos.x -= flGapBtwPoints;
            
            /*if (vCursorPos.x < vWindowPos.x + flMarginInPxl)
                break;*/
        }


        uint64_t iGraphRange = iLargestTime - iSmallestTime;
        const double flRangeUpperBound = static_cast<double>(Features::Performance::InfoPanel::Profiler_GraphRangeUpdateMax.GetData().m_iVal) / 100.0;
        const double flRangeLowerBound = static_cast<double>(Features::Performance::InfoPanel::Profiler_GraphRangeUpdateMin.GetData().m_iVal) / 100.0;
        bool bUpdateGraphRange =
            static_cast<double>(iGraphRange) > static_cast<double>(m_iGraphRange) * flRangeUpperBound ||
            static_cast<double>(iGraphRange) < static_cast<double>(m_iGraphRange) * flRangeLowerBound;
        if (bUpdateGraphRange == true)
        {
            // Since, we know that the range now is not withing the desired section of the graph's current range.
            // we will move the graph's range towards the new range. This way there won't be any sudden snap's n shit.
            // and veiwer won't loose focus immediatly upon range changes.
            double flRangeSmoothing = static_cast<double>(Features::Performance::InfoPanel::Profiler_GraphRangeSmoothing.GetData().m_flVal);
            m_iGraphRange = static_cast<uint64_t>(
                static_cast<double>(m_iGraphRange) * (1.0 - flRangeSmoothing) + static_cast<double>(iGraphRange) * flRangeSmoothing
            );

            m_iGraphBottomTime = iSmallestTime;
        }

    }
    ImGui::EndChild();

    ImGui::PopStyleColor();
}


///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
void Profiler_t::_DrawFuncList(const ProfilerScope_t* pScope, ImVec2 vWindowSize, ImVec2 vWindowPos)
{
    // Styling buttons
    {
        RGBA_t clrButton = Render::menuGUI.GetSecondaryClr();
        RGBA_t clrElevated; Render::menuGUI._FindElevatedClr(clrElevated, Render::menuGUI.GetPrimaryClr());
        ImGui::PushStyleColor(ImGuiCol_Button,        clrButton.GetAsImVec4());
        ImGui::PushStyleColor(ImGuiCol_ButtonActive,  clrElevated.GetAsImVec4());
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, clrElevated.GetAsImVec4());

        ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, WIDGET_ROUNDING);
    }

 
    ImGui::SetNextWindowPos(vWindowPos);
    ImGui::BeginChild("##Profiler_InfoPanel_FuncList", vWindowSize, 0, ImGuiWindowFlags_NoScrollbar);
    {
        float flScroll = ImGui::GetScrollY();

        // some constants.
        constexpr float PADDING                       = 4.0f;
        constexpr float PROFILER_WIDGET_HEIGHT_IN_PXL = 30.0f;
        constexpr float INDENTATION_IN_PXL            = PROFILER_WIDGET_HEIGHT_IN_PXL;

        ImDrawList* pDrawList  = ImGui::GetWindowDrawList();
        ImVec4      vClrWidget = Render::menuGUI.GetSecondaryClr().GetAsImVec4();
        RGBA_t      clrText;     Render::menuGUI.CalcTextClrForBg(clrText, Render::menuGUI.GetSecondaryClr());
        
        ImVec2 vChildWindowSize(ImGui::GetWindowSize());
        ImVec2 vChildWindowPos (ImGui::GetWindowPos());
        ImVec2 vCursorPos(vChildWindowPos);
        vCursorPos.x += PADDING; vCursorPos.y += PADDING + (flScroll * -1.0f);

        
        // Drawing a box on top, just for scope.
        {
            ImVec2 vScopeBoxMin(vCursorPos);
            ImVec2 vScopeBoxMax(vChildWindowPos.x + vChildWindowSize.x - PADDING, vScopeBoxMin.y + PROFILER_WIDGET_HEIGHT_IN_PXL);


            // Drawing a button to take inputs.
            std::string szButtonName("##Profiler_infoPanel_button" + std::string(pScope->m_szScopeName));
            ImGui::SetCursorScreenPos(vScopeBoxMin);
            if (ImGui::Button(szButtonName.c_str(), ImVec2(vScopeBoxMax.x - vScopeBoxMin.x, vScopeBoxMax.y - vScopeBoxMin.y)) == true)
            {
                // Now this will be rendered on the graph.
                m_pActiveGraphProfile = &pScope->m_profileRecords;
                m_pActiveScopeName    = pScope->m_szScopeName;
                Render::notificationSystem.PushBack("Plotting [ %s ]'s profile on graph.", pScope->m_szScopeName);

                // Animations to make it look pretty.
                m_profilerAnim.Reset();
            }

            // Button's name.
            ImVec2 vTextPos(vScopeBoxMin.x + PADDING, vScopeBoxMin.y + (PROFILER_WIDGET_HEIGHT_IN_PXL - ImGui::GetTextLineHeight()) / 2.0f);
            pDrawList->AddText(vTextPos, ImColor(clrText.GetAsImVec4()), pScope->m_szScopeName);


            // Drawing outline if selected...
            if (m_pActiveScopeName == pScope->m_szScopeName)
            {
                pDrawList->AddRect(vScopeBoxMin, vScopeBoxMax, ImColor(Render::menuGUI.GetThemeClr().GetAsImVec4()), WIDGET_ROUNDING);
            }
        }
        vCursorPos.y += PADDING + PROFILER_WIDGET_HEIGHT_IN_PXL;


        for (const ProfilerFunc_t& funcProfile : pScope->m_vecProfiledFuncs)
        {
            ImGui::PushID(funcProfile.m_iCallIndex);

            // Drawing Box for this function's data...
            ImVec2 vWidgetMin(vCursorPos.x + (funcProfile.m_iStackDepth * INDENTATION_IN_PXL), vCursorPos.y + (funcProfile.m_iCallIndex * (PROFILER_WIDGET_HEIGHT_IN_PXL + PADDING)));
            ImVec2 vWidgetMax(vChildWindowPos.x + vChildWindowSize.x - PADDING, vWidgetMin.y + PROFILER_WIDGET_HEIGHT_IN_PXL);


            // Drawing a button to take inputs.
            ImGui::SetCursorScreenPos(vWidgetMin);
            if(ImGui::Button("##SetAsGraphProfile", ImVec2(vWidgetMax.x - vWidgetMin.x, vWidgetMax.y - vWidgetMin.y)) == true)
            {
                // Now this will be rendered on the graph.
                m_pActiveGraphProfile = funcProfile.m_pInfoStorage;
                m_pActiveScopeName    = funcProfile.m_szFuncName;
                Render::notificationSystem.PushBack("Plotting [ %s ]'s profile on graph.", funcProfile.m_szFuncName);

                // Animations to make it look pretty.
                m_profilerAnim.Reset();
            }
            

            // Drawing function's name...
            ImVec2 vTextPos(vWidgetMin.x + PADDING, vWidgetMin.y + (PROFILER_WIDGET_HEIGHT_IN_PXL - ImGui::GetTextLineHeight()) / 2.0f);
            pDrawList->AddText(vTextPos, ImColor(clrText.GetAsImVec4()), funcProfile.m_szFuncName);


            // Drawing outline if selected...
            if (m_pActiveGraphProfile == funcProfile.m_pInfoStorage)
            {
                pDrawList->AddRect(vWidgetMin, vWidgetMax, ImColor(Render::menuGUI.GetThemeClr().GetAsImVec4()), WIDGET_ROUNDING);
            }

            ImGui::PopID();
        }

    }
    ImGui::EndChild();

    ImGui::PopStyleVar(); ImGui::PopStyleColor(3);
}


///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
void Profiler_t::_DrawScopeInfo(ImVec2 vWindowSize, ImVec2 vWindowPos)
{
    if (m_pActiveGraphProfile == nullptr)
        return;

    ImGui::PushStyleColor(ImGuiCol_ChildBg, Render::menuGUI.GetPrimaryClr().GetAsImVec4());
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(10.0f, 0.0f));
    ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, POPUP_ROUNDING);

    constexpr float flPaddingFromWalls = 10.0f;
    vWindowSize.y = (9.0f * ImGui::GetTextLineHeight()) + (2.0f * flPaddingFromWalls); // We will be drawing 9 lines of text, and these fucing "resizeY n X " flags don't seem to work.
    ImGui::SetNextWindowPos(vWindowPos);
    ImGui::BeginChild("##Profiler_InfoPanel_ScopeInfo", vWindowSize, ImGuiChildFlags_AutoResizeY | ImGuiChildFlags_ResizeY);
    {
        RGBA_t clrText; Render::menuGUI.CalcTextClrForBg(clrText, Render::menuGUI.GetPrimaryClr());

        const     float flLineHeight       = ImGui::GetTextLineHeight();
        ImVec2 vCursorPos(vWindowPos.x + flPaddingFromWalls, vWindowPos.y + flPaddingFromWalls);

        // Graph's min time...
        std::string szGraphTimeString(""); 
        ImGui::SetCursorScreenPos(vCursorPos);
        _GetTimeString(static_cast<double>(m_iGraphBottomTime), szGraphTimeString);
        ImGui::TextColored(clrText.GetAsImVec4(), "Graph Min Time : %s", szGraphTimeString.c_str());
        vCursorPos.y += flLineHeight;

        // Graph's range...
        ImGui::SetCursorScreenPos(vCursorPos);
        _GetTimeString(static_cast<double>(m_iGraphRange), szGraphTimeString);
        ImGui::TextColored(clrText.GetAsImVec4(), "Graph Range    : %s", szGraphTimeString.c_str());
        vCursorPos.y += flLineHeight;

        // Average time for 16, 32, 64, 128, 256, 512, 1024 ticks...
        for (int i = 16; i <= 1024; i *= 2)
        {
            ImGui::SetCursorScreenPos(vCursorPos);
            _GetTimeString(m_pActiveGraphProfile->GetAvgOf(i), szGraphTimeString);
            ImGui::TextColored(clrText.GetAsImVec4(), "%d ticks avg.  %s", i, szGraphTimeString.c_str());
            vCursorPos.y += flLineHeight;
        }
    }
    ImGui::EndChild();

    ImGui::PopStyleVar(2); ImGui::PopStyleColor();
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

