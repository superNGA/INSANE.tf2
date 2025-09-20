#include "MenuV2.h"

#include "../../../External Libraries/ImGui/imgui.h"
#include "../../../Hooks/DirectX Hook/DirectX_hook.h"

// Renderer
#include "../../Graphics Engine V2/Graphics.h"
#include "../../Graphics Engine V2/Draw Objects/Box/Box.h"
#include "../../Graphics Engine V2/Draw Objects/Line/Line.h"
#include "../../Graphics Engine V2/Draw Objects/Circle/Circle.h"


///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
void MenuGUI_t::Draw()
{
    if (directX::UI::UI_visble == false)
    {
        if (m_pMainMenu != nullptr)
        {
            m_pMainMenu->SetVisible(false);
        }
        return;
    }

    ImGuiIO& io = ImGui::GetIO();

    // Setting menu's size.
    float flScaleMult = Features::Menu::Menu::Scale.GetData().m_flVal;
    ImVec2 vWindowSize(io.DisplaySize.x * 0.45f * flScaleMult, io.DisplaySize.y * 0.6f * flScaleMult);
    ImGui::SetNextWindowSize(vWindowSize);

    // Setting window's color
    {
        ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0.0f, 0.0f, 0.0f, 0.0f));
        ImGui::SetNextWindowBgAlpha(0.0f);
    }

    ImVec2 vWindowPos(0.0f, 0.0f);

    ImGuiWindowFlags iWindowFlags =
        ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoTitleBar |
        ImGuiWindowFlags_NoCollapse;
    if (ImGui::Begin("##MainMenuGUI", nullptr, iWindowFlags) == true)
    {
        vWindowPos = ImGui::GetWindowPos();
        ImGui::Text("INSANE.TF2 BY !!GODLIKE_INSANE!!");
        ImGui::End();
    }

    // Popping ImGui styles
    {
        ImGui::PopStyleColor();
    }

    // Drawing background using our renderer. ( for more flexible drawing )
    {
        if (m_pMainMenu == nullptr)
        {
            m_pMainMenu = new BoxFilled2D_t();
        }

        if (m_pMainMenu == nullptr)
            return;

        m_pMainMenu->SetVisible(true);
        m_pMainMenu->SetVertex(vec(vWindowPos.x, vWindowPos.y, 0.0f), vec(vWindowPos.x + vWindowSize.x, vWindowPos.y + vWindowSize.y, 0.0f));

        float flBlurAmmount = Features::Menu::Menu::Blur.GetData().m_flVal;
        if (flBlurAmmount <= 0.0f)
        {
            m_pMainMenu->SetBlur(-1);
        }
        else
        {
            m_pMainMenu->SetBlur(static_cast<int>(flBlurAmmount));
        }

        m_pMainMenu->SetColor(Features::Menu::Menu::ColorBottomLeft.GetData().GetAsBytes(),  IBoxFilled_t::VertexType_BottomLeft);
        m_pMainMenu->SetColor(Features::Menu::Menu::ColorBottomRight.GetData().GetAsBytes(), IBoxFilled_t::VertexType_BottomRight);
        m_pMainMenu->SetColor(Features::Menu::Menu::ColorTopLeft.GetData().GetAsBytes(),     IBoxFilled_t::VertexType_TopLeft);
        m_pMainMenu->SetColor(Features::Menu::Menu::ColorTopRight.GetData().GetAsBytes(),    IBoxFilled_t::VertexType_TopRight);

        float flRGBSpeed = Features::Menu::Menu::RGBSpeed.GetData().m_flVal;
        if(Features::Menu::Menu::rgb.IsActive() == false)
        {
            m_pMainMenu->SetRGBAnimSpeed(-1);
        }
        else
        {
            m_pMainMenu->SetRGBAnimSpeed(flRGBSpeed);
        }

        m_pMainMenu->SetRounding(Features::Menu::Menu::Rounding.GetData().m_flVal);
    }
}
