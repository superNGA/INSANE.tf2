#include "PlayerList.h"
#include "../../Entity Iterator/EntityIterator.h"

// SDK
#include "../../../SDK/class/BaseEntity.h"
#include "../../../SDK/class/IVEngineClient.h"

#include "../../Material Gen/MaterialGen.h"
#include "../../../Utility/ConsoleLogging.h"
#include "../../../Hooks/DirectX Hook/DirectX_hook.h"
#include "../../../Resources/Fonts/FontManager.h"
#include "../Menu/Menu.h"

// External libraries
#include "../../../External Libraries/ImGui/imgui.h"

///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
void PlayerList_t::Draw()
{
    if (directX::UI::UI_visble == false)
        return;

    if (I::iEngine->IsInGame() == false)
        return;

    {
        ImGui::PushFont(Resources::Fonts::JetBrains_SemiBold_NL_Mid);
    }

    std::vector<BaseEntity*>* vecEnemies = F::entityIterator.GetAllConnectedEnemiesList().GetReadBuffer();
    if (vecEnemies == nullptr)
        return;

    std::vector<BaseEntity*>* vecMates   = F::entityIterator.GetAllConnectedTeamMatesList().GetReadBuffer();
    if (vecMates == nullptr)
    {
        // Release what we already acquired. to prevent bullshit happening.
        F::entityIterator.GetAllConnectedEnemiesList().ReturnReadBuffer(vecEnemies);
        return;
    }

    float x = 0.0f, y = 0.0f; Render::uiMenu.GetWindowPos(x, y);
    float flMenuWidth = 0.0f, flMenuHeight = 0.0f; Render::uiMenu.GetWindowSize(flMenuHeight, flMenuWidth);

    constexpr float flWidth = 250.0f;
   
    ImVec2 vWindowSize(flWidth, (vecEnemies->size() + vecMates->size() + 2) * ImGui::GetTextLineHeight());
    ImGui::SetNextWindowSize(vWindowSize);
    ImVec2 vWindowPos(x - flWidth, y + (flMenuHeight / 2.0f) - ((vecEnemies->size() + 1) * ImGui::GetTextLineHeight()));
    ImGui::SetNextWindowPos(vWindowPos);

    if (ImGui::Begin("##PlayerList", NULL, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoScrollbar) == true)
    {
        ImDrawList* pDrawList = ImGui::GetWindowDrawList();
        ImVec2 vTextPos(vWindowPos.x + 20.0f, vWindowPos.y);
        ImVec2 vButtonSize(40.0f, ImGui::GetTextLineHeight() - 4.0f);

        // Drawing Enemy names.
        static player_info_t playerinfo;
        static BaseEntity* pActiveEntity = nullptr;
        for (BaseEntity* pEnt : *vecEnemies)
        {
            if (pEnt == nullptr)
                continue;

            if (I::iEngine->GetPlayerInfo(pEnt->entindex(), &playerinfo) == false)
                continue;

            ImGui::SetCursorScreenPos(vTextPos);
            ImGui::SetCursorPosX(vWindowSize.x - vButtonSize.x - 10.0f);

            pDrawList->AddText(vTextPos, ImColor(255, 0, 0, 255), playerinfo.name);
            vTextPos.y += ImGui::GetTextLineHeight();

            if (ImGui::Button(std::string("...##" + std::string(playerinfo.name)).c_str(), vButtonSize) == true)
            {
                pActiveEntity = pEnt;
                ImGui::OpenPopup("##EntSettings");
            }
        }

        // Gap
        vTextPos.y += ImGui::GetTextLineHeight() * 2.0f;

        // Drawing friend names.
        for (BaseEntity* pEnt : *vecMates)
        {
            if (pEnt == nullptr)
                continue;

            if (I::iEngine->GetPlayerInfo(pEnt->entindex(), &playerinfo) == false)
                continue;

            ImGui::SetCursorScreenPos(vTextPos);
            ImGui::SetCursorPosX(vWindowSize.x - vButtonSize.x - 10.0f);

            pDrawList->AddText(vTextPos, ImColor(0, 255, 0, 255), playerinfo.name);
            vTextPos.y += ImGui::GetTextLineHeight();

            if (ImGui::Button(std::string("...##" + std::string(playerinfo.name)).c_str(), vButtonSize) == true)
            {
                pActiveEntity = pEnt;
                ImGui::OpenPopup("##EntSettings");
            }
        }

        if (ImGui::BeginPopup("##EntSettings") == true)
        {
            if (ImGui::Button("set material") == true)
            {
                ImGui::OpenPopup("ChooseMaterial");
            }


            if (ImGui::BeginPopup("ChooseMaterial") == true)
            {
                const auto& vecMaterials = F::materialGen.GetMaterialList();
                int nMaterials = vecMaterials.size();
                for (int iMatIndex = -1; iMatIndex < nMaterials; iMatIndex++)
                {
                    const char* szMatName = iMatIndex < 0 ? "None" : vecMaterials[iMatIndex].m_szMatBundleName.c_str();
                    if (ImGui::Button(szMatName) == true)
                    {
                        LOG("Trying to set mateiral for entity %p as %d", pActiveEntity, iMatIndex);
                        F::entityIterator.SetEntityMaterial(pActiveEntity, iMatIndex);
                        pActiveEntity = nullptr;
                    }
                }

                ImGui::EndPopup();
            }

            ImGui::EndPopup();
        }

        ImGui::End();
    }

    F::entityIterator.GetAllConnectedEnemiesList().ReturnReadBuffer(vecEnemies);
    F::entityIterator.GetAllConnectedTeamMatesList().ReturnReadBuffer(vecMates);

    {
        ImGui::PopFont();
    }
}