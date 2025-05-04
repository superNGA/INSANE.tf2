#define _CRT_SECURE_NO_WARNINGS
#include "InfoWindow_t.h"
#include "../../../External Libraries/ImGui/imgui.h"
#include "../../../Hooks/EndScene/EndScene.h"


const ImVec4 CLR_RED(1.0f, 0.0f, 0.0f, 1.0f);
const ImVec4 CLR_BLUE(0.0f, 0.0f, 1.0f, 1.0f);
const ImVec4 CLR_YELLOW(1.0f, 1.0f, 0.0f, 1.0f);
const ImVec4 CLR_GREEN(0.0f, 1.0f, 0.0f, 1.0f);
const ImVec4 CLR_WHITE(1.0f, 1.0f, 1.0f, 1.0f);


void InfoWindow_t::Draw()
{
    _DrawInfoWindow();
    _DrawCenterWindow();
    //_DrawInfoBar();
}

void InfoWindow_t::AddToInfoWindow(std::string caller, std::string msg)
{
    uint32_t iCallerHash = FNV1A32(caller.c_str());

    auto it = m_mapAllDataInfoWindow.find(iCallerHash);
    if (it == m_mapAllDataInfoWindow.end())
        m_mapAllDataInfoWindow.insert({ iCallerHash, msg });

    m_mapAllDataInfoWindow[iCallerHash] = msg;
}


void InfoWindow_t::AddToCenterConsole(std::string caller, std::string msg, TextMsgClr_t clr)
{
    uint32_t iCallerHash = FNV1A32(caller.c_str());

    auto it = m_mapAllDataCenterWindow.find(iCallerHash);
    if (it == m_mapAllDataCenterWindow.end())
        m_mapAllDataCenterWindow.insert(
            { 
                iCallerHash, TextMsg_t(clr, msg)
            });

    m_mapAllDataCenterWindow[iCallerHash] = TextMsg_t(clr, msg);
}


void InfoWindow_t::_DrawInfoWindow()
{
    ImGui::SetNextWindowPos(ImVec2(0.0f, 0.0f), ImGuiCond_Always);
    ImGui::Begin("Performance", nullptr,
        ImGuiWindowFlags_NoMove | ImGuiWindowFlags_AlwaysAutoResize |
        ImGuiWindowFlags_NoCollapse);
    ImGui::PushFont(directX::fonts::roboto); // <- gonna change it to something like JetBrains mono semi-bold.

    for (auto& data : m_mapAllDataInfoWindow)
    {
        ImGui::Text(data.second.c_str());
    }

    ImGui::PopFont();
    ImGui::End();
}


void InfoWindow_t::_DrawCenterWindow()
{
    auto vGameWindowSize = ImGui::GetMainViewport()->Size;
    ImGui::SetNextWindowPos(ImVec2(vGameWindowSize.x/2.0f, vGameWindowSize.y/2.0f), ImGuiCond_Always);
    ImGui::SetNextWindowBgAlpha(0.1f);
    ImGui::Begin("CenterConsole", nullptr,
        ImGuiWindowFlags_NoMove | ImGuiWindowFlags_AlwaysAutoResize |
        ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar);
    ImGui::PushFont(directX::fonts::roboto); // <- gonna change it to something like JetBrains mono semi-bold.

    for (auto& data : m_mapAllDataCenterWindow)
    {
        ImGui::TextColored(_GetTextClr(data.second.m_clr), data.second.m_sTextMsg.c_str());
    }

    ImGui::PopFont();
    ImGui::End();
}

const ImVec4& InfoWindow_t::_GetTextClr(TextMsgClr_t clr)
{
    switch (clr)
    {
    case RED:
        return CLR_RED;
    case BLUE:
        return CLR_BLUE;
    case YELLOW:
        return CLR_YELLOW;
    case GREEN:
        return CLR_GREEN;
    default:
        return CLR_WHITE;
    }

    return CLR_WHITE;
}