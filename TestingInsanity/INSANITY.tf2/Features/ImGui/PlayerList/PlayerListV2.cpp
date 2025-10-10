#include "PlayerListV2.h"

#include <chrono>
#include <vector>

#include "../MenuV2/MenuV2.h"
#include "../NotificationSystem/NotificationSystem.h"
#include "../../../External Libraries/ImGui/imgui.h"
#include "../../../Resources/Fonts/FontManager.h"
#include "../../../Utility/Containers/DoubleBuffer.h"

// SDK
#include "../../Entity Iterator/EntityIterator.h"
#include "../../Material Gen/MaterialGen.h"
#include "../../../SDK/class/BaseEntity.h"
#include "../../../SDK/class/IVEngineClient.h"


constexpr float CENTER_GAP_IN_PXL        = 40.0f; // Gap between enemy & teammate list
constexpr float WINDOW_PADDING_IN_PXL    =  5.0f; // Gap between window borders & its contents
constexpr float NAME_PADDING_IN_PXL      =  4.0f; // Gap between name box & its contents
constexpr float INTERNAME_PADDING_IN_PXL =  2.0f; // Gap between each name entry
constexpr float NAME_TAB_ROUNDING_IN_PXL =  4.0f; // Gap between each name entry


///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
void PlayerListV2_t::Draw()
{
    if(m_bVisible == false)
        return;

    if(I::iEngine->IsInGame() == false)
        return;

    m_playerListAnim.CalculateAnim();

    ImGui::PushFont(Resources::Fonts::JetBrainsMonoNerd_Small);

    // Set window size
    ImVec2 vWindowSize; Render::menuGUI.GetSize(vWindowSize.x, vWindowSize.y);
    vWindowSize.x = 250.0f; 
    ImGui::SetNextWindowSize(vWindowSize);
    
    // Set window pos
    ImVec2 vWindowPos; Render::menuGUI.GetPos(vWindowPos.x, vWindowPos.y);
    vWindowPos.x -= vWindowSize.x;
    vWindowPos.x -= MENU_PADDING_IN_PXL; // Padding
    ImGui::SetNextWindowPos(vWindowPos);

    
    // Styling window
    {
        ImGui::PushStyleColor(ImGuiCol_WindowBg, Render::menuGUI.GetPrimaryClr().GetAsImVec4());
        ImGui::PushStyleColor(ImGuiCol_Border,   ImVec4(0.0f, 0.0f, 0.0f, 0.0f));

        ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, Features::Menu::Menu::Rounding.GetData().m_flVal);
    }

    int iWindowFlags = ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoTitleBar;
    if(ImGui::Begin("##PlayerList", nullptr, iWindowFlags) == true)
    {
        RGBA_t clrText; Render::menuGUI.CalcTextClrForBg(clrText, Render::menuGUI.GetPrimaryClr());

        // Enemy list
        {
            std::vector<BaseEntity*>* vecAllEnemies = F::entityIterator.GetAllConnectedEnemiesList().GetReadBuffer();
            DOUBLEBUFFER_AUTO_RELEASE_READBUFFER(&F::entityIterator.GetAllConnectedEnemiesList(), vecAllEnemies); 

            ImVec2 vListPos(vWindowPos.x, vWindowPos.y + (vWindowSize.y / 2.0f) - (CENTER_GAP_IN_PXL / 2.0f));
            _DrawList(vListPos.x, vListPos.y, vWindowSize.x, vecAllEnemies, true);
        }

        // Team mate list
        {
            std::vector<BaseEntity*>* vecAllTeamMates = F::entityIterator.GetAllConnectedTeamMatesList().GetReadBuffer();
            DOUBLEBUFFER_AUTO_RELEASE_READBUFFER(&F::entityIterator.GetAllConnectedTeamMatesList(), vecAllTeamMates); 

            ImVec2 vListPos(vWindowPos.x, vWindowPos.y + (vWindowSize.y / 2.0f) + (CENTER_GAP_IN_PXL / 2.0f));
            _DrawList(vListPos.x, vListPos.y, vWindowSize.x, vecAllTeamMates, false);
        }

        ImGui::End();
    }


    ImGui::PopStyleColor(2); ImGui::PopStyleVar();

    ImGui::PopFont();
}


///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
void PlayerListV2_t::SetVisible(bool bVisible)
{
    m_bVisible = bVisible;

    if (bVisible == false)
        m_playerListAnim.Reset();
}


///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
void PlayerListV2_t::_DrawList(float x, float y, float flWidth, const std::vector<BaseEntity*>* vecPlayers, bool bGrowUpwards)
{
    ImDrawList* pDrawList = ImGui::GetWindowDrawList();
    ImVec2 vCursorScreenPos(x + WINDOW_PADDING_IN_PXL, y);

    ImGuiIO& io = ImGui::GetIO();

    float flFrameHeight = ImGui::GetFrameHeight();

    RGBA_t clrSecondary = Render::menuGUI.GetSecondaryClr();
    RGBA_t clrPrimary   = Render::menuGUI.GetPrimaryClr();
    RGBA_t clrTheme     = Render::menuGUI.GetThemeClr();
    {
        ImGui::PushStyleColor(ImGuiCol_Button,        clrSecondary.GetAsImVec4());
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, clrPrimary.GetAsImVec4());
        ImGui::PushStyleColor(ImGuiCol_ButtonActive,  clrPrimary.GetAsImVec4());

        RGBA_t clrText; Render::menuGUI.CalcTextClrForBg(clrText, clrSecondary);
        ImGui::PushStyleColor(ImGuiCol_Text, clrText.GetAsImVec4());

        ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, NAME_TAB_ROUNDING_IN_PXL);
    }


    static player_info_t playerInfo;
    for(BaseEntity* pEnt : *vecPlayers)
    {
        if(pEnt == nullptr)
            continue;

        if(I::iEngine->GetPlayerInfo(pEnt->entindex(), &playerInfo) == false)
            continue;

        ImVec2 vTabSize(flWidth - (2.0f * WINDOW_PADDING_IN_PXL), ImGui::GetTextLineHeight() + (2.0f * NAME_TAB_ROUNDING_IN_PXL));
        ImVec2 vNameTabMin(vCursorScreenPos.x,              bGrowUpwards == true ? vCursorScreenPos.y - vTabSize.y : vCursorScreenPos.y);
        ImVec2 vNameTabMax(vCursorScreenPos.x + vTabSize.x, bGrowUpwards == true ? vCursorScreenPos.y : vCursorScreenPos.y + vTabSize.y);

        // Box for players name
        pDrawList->AddRectFilled(
            vNameTabMin, vNameTabMax,
            ImColor(Render::menuGUI.GetSecondaryClr().GetAsImVec4()),
            NAME_TAB_ROUNDING_IN_PXL
        );

        ImVec2 vButtonPos(vNameTabMin.x + vTabSize.x - NAME_PADDING_IN_PXL - flFrameHeight, vNameTabMin.y + (vTabSize.y - flFrameHeight) / 2.0f);
        ImGui::SetCursorScreenPos(vButtonPos);
        {
            bool bButtonHovered = 
                (io.MousePos.x >= vButtonPos.x && io.MousePos.x <= vButtonPos.x + flFrameHeight) &&
                (io.MousePos.y >= vButtonPos.y && io.MousePos.y <= vButtonPos.y + flFrameHeight);
            if(bButtonHovered == true)
            {
                ImGui::PushStyleColor(ImGuiCol_Text, clrTheme.GetAsImVec4());
            }

            std::string szButtonText = reinterpret_cast<const char*>(u8"\ueb94");
            szButtonText += "##"; szButtonText += playerInfo.name;
            if(ImGui::Button(szButtonText.c_str(), ImVec2(flFrameHeight, flFrameHeight)) == true)
            {
                ImGui::OpenPopup(("Popup" + std::string(playerInfo.name)).c_str());
            }
            _DrawEntSetting(pEnt, ("Popup" + std::string(playerInfo.name)).c_str(), playerInfo.name);


            if(bButtonHovered == true)
            {
                ImGui::PopStyleColor();
            }
        }

        bool bCustomMatSet = F::entityIterator.GetEntityMaterial(pEnt) != -1;
        if(bCustomMatSet == true)
        {
            float flFlashThickness = Features::Menu::SideMenu::AnimAccentSize.GetData().m_flVal;
            ImVec2 vAnimatedMax(vNameTabMax.x + flFlashThickness - ((vTabSize.x + flFlashThickness) * m_playerListAnim.GetAnimation()), vNameTabMax.y);
            ImVec2 vAnimatedMin(vAnimatedMax.x - flFlashThickness, vNameTabMin.y);
            pDrawList->AddRectFilled(
                vAnimatedMin, vAnimatedMax,
                ImColor(clrTheme.GetAsImVec4())
            );
        }

        // Player's name
        RGBA_t clrText; Render::menuGUI.CalcTextClrForBg(clrText, Render::menuGUI.GetSecondaryClr());
        pDrawList->AddText(
            ImVec2(vCursorScreenPos.x + NAME_PADDING_IN_PXL, bGrowUpwards == true ? vCursorScreenPos.y - NAME_TAB_ROUNDING_IN_PXL - ImGui::GetTextLineHeight() : vCursorScreenPos.y + NAME_PADDING_IN_PXL), 
            ImColor(bCustomMatSet == true ? clrTheme.GetAsImVec4() : clrText.GetAsImVec4()),
            playerInfo.name
        );

        // Adjusting position for next entry
        vCursorScreenPos.y = 
            bGrowUpwards == true ? 
                (vCursorScreenPos.y - vTabSize.y - INTERNAME_PADDING_IN_PXL) :
                (vCursorScreenPos.y + vTabSize.y + INTERNAME_PADDING_IN_PXL);
    }


    ImGui::PopStyleColor(4); ImGui::PopStyleVar();
}


///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
void PlayerListV2_t::_DrawEntSetting(BaseEntity* pEnt, std::string szPopupId, const char* szPlayerName)
{
    {
        ImGui::PushStyleVar(ImGuiStyleVar_FramePadding,  ImVec2(0.0f, 0.0f));
        ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing,   ImVec2(0.0f, 0.0f));
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
        ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 0.0f);
    }

    ImVec2 vButtonSize(100.0f, 30.0f);
    if(ImGui::BeginPopup(szPopupId.c_str()) == true)
    {
        if(ImGui::Button("Material", vButtonSize) == true)
        {
            ImGui::OpenPopup("MaterialList");
        }
        
        // Material list
        static ImVec2 vMateiralButtonSize(150.0f, 20.0f);
        if(ImGui::BeginPopup("MaterialList") == true)
        {
            const std::vector<MaterialBundle_t>& vecMaterials = F::materialGen.GetMaterialList();
            int nMaterials = vecMaterials.size();
            for (int iMatIndex = -1; iMatIndex < nMaterials; iMatIndex++)
            {
                const char* szMatName = iMatIndex < 0 ? "None" : vecMaterials[iMatIndex].m_szMatBundleName.c_str();
                if (ImGui::Button(szMatName, vMateiralButtonSize) == true)
                {
                    LOG("Trying to set mateiral for entity %p as %d", pEnt, iMatIndex);
                    F::entityIterator.SetEntityMaterial(pEnt, iMatIndex);
                    m_playerListAnim.Reset();

                    if (iMatIndex < 0)
                    {
                        Render::notificationSystem.PushBack("Resetted %s 's material", szPlayerName);
                    }
                    else
                    {
                        Render::notificationSystem.PushBack("Set %s 's material to %s", szPlayerName, vecMaterials[iMatIndex].m_szMatBundleName.c_str());
                    }
                }
            }
            ImGui::EndPopup();
        }

        ImGui::EndPopup();
    }


    ImGui::PopStyleVar(4);
}
