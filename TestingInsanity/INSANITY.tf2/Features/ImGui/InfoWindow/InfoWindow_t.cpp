#define _CRT_SECURE_NO_WARNINGS
#include "InfoWindow_t.h"
#include "../../../External Libraries/ImGui/imgui.h"
#include "../../../Hooks/EndScene/EndScene.h"

void InfoWindow_t::Draw()
{
    _DrawInfoWindow();
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

void InfoWindow_t::_DrawInfoWindow()
{
    ImGui::SetNextWindowPos(ImVec2(0.0f, 0.0f), ImGuiCond_Always);
    ImGui::Begin("Performance", nullptr,
        ImGuiWindowFlags_NoMove | ImGuiWindowFlags_AlwaysAutoResize |
        ImGuiWindowFlags_NoCollapse);
    ImGui::PushFont(directX::fonts::roboto);

    for (auto& data : m_mapAllDataInfoWindow)
    {
        ImGui::Text(data.second.c_str());
    }

    ImGui::PopFont();
    ImGui::End();
}