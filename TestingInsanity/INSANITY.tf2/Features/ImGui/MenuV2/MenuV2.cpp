#include "MenuV2.h"

#include <format>
#include <algorithm>

#include "../../../External Libraries/ImGui/imgui.h"
#include "../../../Hooks/DirectX Hook/DirectX_hook.h"
#include "../../FeatureHandler.h"
#include "../../../Resources/Fonts/FontManager.h"

// Renderer
#include "../../Graphics Engine V2/Graphics.h"
#include "../../Graphics Engine V2/Draw Objects/Box/Box.h"
#include "../../Graphics Engine V2/Draw Objects/Line/Line.h"
#include "../../Graphics Engine V2/Draw Objects/Circle/Circle.h"


constexpr float SIDEMENU_SCALE            =  0.2f; // Percentage of main body allocated to side menu.
constexpr float FRAME_PADDING_PXL         = 15.0f; // Padding between Main body & its contents.
constexpr float SECTION_PADDING_PXL       = 10.0f; // Padding between Section walls & its contents.
constexpr float INTER_FEATURE_PADDING_PXL =  5.0f; // Padding between each feature.
constexpr float FEATURE_PADDING_PXL       =  5.0f; // Padding between feautres walls & its contents.
constexpr float FEATURE_HEIGHT            = 30.0f; // Height of each feature.
constexpr float SECTION_NAME_HEIGHT       = 30.0f; // Height of each feature.


///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
MenuGUI_t::MenuGUI_t()
{
    m_clrPrimary.Init(); m_clrSecondary.Init(); m_clrTheme.Init();
}


///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
void MenuGUI_t::Draw()
{
    if (m_bVisible == false)
        return;

    if (_Initialize() == false)
        return;
    {
        ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0.0f, 0.0f, 0.0f, 0.0f));
    }
    
    _CalculateColors();
    
    // Drawing main body.
    float  flScaleMult = Features::Menu::Menu::Scale.GetData().m_flVal;
    ImVec2 vWindowSize(900.0f * flScaleMult, 650.0f * flScaleMult);
    ImVec2 vWindowPos  = _DrawMainBody(vWindowSize.x, vWindowSize.y);

    // Drawing side menu.
    _DrawTabBar(vWindowSize.x * SIDEMENU_SCALE, vWindowSize.y, vWindowPos.x, vWindowPos.y);


    {
        ImGui::PopStyleColor();
    }
}


///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
void MenuGUI_t::SetVisible(bool bVisible)
{
    m_bVisible = bVisible;

    if(m_pMainMenu != nullptr)
        m_pMainMenu->SetVisible(m_bVisible);

    if (m_pSideMenu != nullptr)
        m_pSideMenu->SetVisible(m_bVisible);
}


///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
bool MenuGUI_t::_Initialize()
{
    if (m_pMainMenu == nullptr)
    {
        m_pMainMenu = new BoxFilled2D_t();
        
        if (m_pMainMenu == nullptr)
            return false;

        WIN_LOG("Main body init done successfully");
    }

    if (m_pSideMenu == nullptr)
    {
        m_pSideMenu = new BoxFilled2D_t();
        
        if (m_pSideMenu == nullptr)
            return false;

        WIN_LOG("Side menu init done successfully");
    }

    return true;
}


///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
ImVec2 MenuGUI_t::_DrawMainBody(float flWidth, float flHeight)
{
    ImVec2           vWindowPos;
    ImGuiWindowFlags iWindowFlags = 
        ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoTitleBar | 
        ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoBringToFrontOnFocus;

    ImGui::SetNextWindowBgAlpha(0.0f);
    ImGui::SetNextWindowSize(ImVec2(flWidth, flHeight));
    if (ImGui::Begin("##MainMenuGUI", nullptr, iWindowFlags) == true)
    {
        vWindowPos = ImGui::GetWindowPos();

        // Pos & Size
        m_pMainMenu->SetVertex(vec(vWindowPos.x, vWindowPos.y, 0.0f), vec(vWindowPos.x + flWidth, vWindowPos.y + flHeight, 0.0f));

        // Blur
        float flBlurAmmount = Features::Menu::Menu::Blur.GetData().m_flVal;
        m_pMainMenu->SetBlur(flBlurAmmount <= 0.0f ? -1 : static_cast<int>(flBlurAmmount)); // -1 blur is no blur.

        // Colors ( independent for each vertex )
        m_pMainMenu->SetColor(Features::Menu::Menu::ColorBottomLeft.GetData().GetAsBytes(),  IBoxFilled_t::VertexType_BottomLeft);
        m_pMainMenu->SetColor(Features::Menu::Menu::ColorBottomRight.GetData().GetAsBytes(), IBoxFilled_t::VertexType_BottomRight);
        m_pMainMenu->SetColor(Features::Menu::Menu::ColorTopLeft.GetData().GetAsBytes(),     IBoxFilled_t::VertexType_TopLeft);
        m_pMainMenu->SetColor(Features::Menu::Menu::ColorTopRight.GetData().GetAsBytes(),    IBoxFilled_t::VertexType_TopRight);

        // RGB animation
        float flRGBSpeed = Features::Menu::Menu::RGBSpeed.GetData().m_flVal;
        bool  bRGBActive = Features::Menu::Menu::rgb.IsActive();
        m_pMainMenu->SetRGBAnimSpeed(bRGBActive == true ? flRGBSpeed : -1.0f);

        // Rounding
        m_pMainMenu->SetRounding(Features::Menu::Menu::Rounding.GetData().m_flVal);

        _DrawSections(m_pActiveTab, flWidth * (1.0f - SIDEMENU_SCALE), flHeight, vWindowPos.x + (flWidth * SIDEMENU_SCALE), vWindowPos.y - ImGui::GetScrollY());

        ImGui::End();
    }

    return vWindowPos;
}


///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
void MenuGUI_t::_DrawTabBar(float flWidth, float flHeight, float x, float y)
{
    ImGuiWindowFlags iWindowFlags = ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse;

    const float flSideMenuPadding = flWidth * 0.025f;
    {
        ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(flSideMenuPadding, flSideMenuPadding));
    }

    ImGui::SetNextWindowBgAlpha(0.0f);
    ImGui::SetNextWindowSize(ImVec2(flWidth, flHeight));
    ImGui::SetNextWindowPos(ImVec2(x, y));
    if (ImGui::Begin("##SideMenuGUI", nullptr, iWindowFlags) == true)
    {
        // Pos & Size
        m_pSideMenu->SetVertex(vec(x, y, 0.0f), vec(x + flWidth, y + flHeight, 0.0f));
        ImGui::Text("INSANE.tf2");

        // Blur
        float flBlurAmmount = Features::Menu::SideMenu::Blur.GetData().m_flVal;
        m_pSideMenu->SetBlur(flBlurAmmount <= 0.0f ? -1 : static_cast<int>(flBlurAmmount)); // -1 blur is no blur.

        // Colors ( independent for each vertex )
        m_pSideMenu->SetColor(Features::Menu::SideMenu::ColorBottomLeft.GetData().GetAsBytes(),  IBoxFilled_t::VertexType_BottomLeft);
        m_pSideMenu->SetColor(Features::Menu::SideMenu::ColorBottomRight.GetData().GetAsBytes(), IBoxFilled_t::VertexType_BottomRight);
        m_pSideMenu->SetColor(Features::Menu::SideMenu::ColorTopLeft.GetData().GetAsBytes(),     IBoxFilled_t::VertexType_TopLeft);
        m_pSideMenu->SetColor(Features::Menu::SideMenu::ColorTopRight.GetData().GetAsBytes(),    IBoxFilled_t::VertexType_TopRight);

        // RGB animation
        float flRGBSpeed = Features::Menu::SideMenu::RGBSpeed.GetData().m_flVal;
        bool  bRGBActive = Features::Menu::SideMenu::rgb.IsActive();
        m_pSideMenu->SetRGBAnimSpeed(bRGBActive == true ? flRGBSpeed : -1.0f);

        // Rounding
        m_pSideMenu->SetRounding(Features::Menu::SideMenu::Rounding.GetData().m_flVal);

        
        // Drawing tabs.
        const ImVec2 vButtonSize(flWidth - (flSideMenuPadding * 4.0f), flHeight / 20.0f);
        _StyleSideMenuBottons();
        {
            const std::vector<Tab_t*> vecTabs = Config::featureHandler.GetFeatureMap();
            for (Tab_t* pTab : vecTabs)
            {
                bool bShouldPop = false;
                if (pTab == m_pActiveTab)
                {
                    ImGui::PushStyleColor(ImGuiCol_Button, m_clrSideMenuButtons.GetAsImVec4());
                    bShouldPop = true;
                }


                if (ImGui::Button(std::string("    " + pTab->m_szTabDisplayName).c_str(), vButtonSize) == true)
                {
                    m_pActiveTab = pTab;
                }


                if (bShouldPop == true)
                {
                    ImGui::PopStyleColor();
                }
            }
        }
        _PopAllStyles();

        ImGui::End();
    }

    ImGui::PopStyleVar();
}


///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
void MenuGUI_t::_DrawSections(Tab_t* pTab, float flWidth, float flHeight, float x, float y)
{
    if (pTab == nullptr)
        return;

    ImVec2 vWindowPos = ImGui::GetWindowPos();
    ImGui::SetCursorScreenPos(ImVec2(x, y));

    {
        ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0.0f, 0.0f));
    }


    ImVec2 vLeftSectionCursor (x + FRAME_PADDING_PXL,                             y + FRAME_PADDING_PXL);
    ImVec2 vRightSectionCursor(x + (flWidth / 2.0f) + (FRAME_PADDING_PXL / 2.0f), y + FRAME_PADDING_PXL);
    bool bDrawingOnLeft = true;

    // List of all background boxes for our renderer
    static std::vector<BoxFilled2D_t*> vecSectionBoxes = {};

    int iSectionIndex = 0; int nSections = pTab->m_vecSections.size();
    for (iSectionIndex = 0; iSectionIndex < nSections; iSectionIndex++)
    {
        Section_t* pSection = pTab->m_vecSections[iSectionIndex];

        // This is the screen pos for the cursor.
        ImVec2* pSectionScreenPos = bDrawingOnLeft == true ? &vLeftSectionCursor : &vRightSectionCursor;
        ImGui::SetCursorScreenPos(*pSectionScreenPos);
        ImVec2 vSectionLocalPos(pSectionScreenPos->x - vWindowPos.x, pSectionScreenPos->y - vWindowPos.y);
        
        // Drawing Section name.
        ImVec2 vSectionNamePos(pSectionScreenPos->x + FEATURE_PADDING_PXL, pSectionScreenPos->y + (SECTION_NAME_HEIGHT - ImGui::GetTextLineHeight()) / 2.0f);
        ImDrawList* pDrawList = ImGui::GetWindowDrawList();
        pDrawList->AddText(vSectionNamePos, ImColor(m_clrSecondary.GetAsImVec4()), pSection->m_szSectionDisplayName.c_str());
        pSectionScreenPos->y += SECTION_NAME_HEIGHT; // Compensate for the section name, so section box doesn't draw over the name.

        // Adjusting cursor pos before drawing feautres.
        float flFeatureWidth = (flWidth / 2.0f) - (1.5f * FRAME_PADDING_PXL) - (2 * SECTION_PADDING_PXL);
        ImGui::SetCursorPos(ImVec2(vSectionLocalPos.x + SECTION_PADDING_PXL + FEATURE_PADDING_PXL, pSectionScreenPos->y - vWindowPos.y + SECTION_PADDING_PXL)); 

        for (IFeature* pFeature : pSection->m_vecFeatures)
        {
            ImVec2 vCursorPos        = ImGui::GetCursorPos();
            ImVec2 vFeatureMinPadded = vCursorPos;
            ImVec2 vFeatureMaxPadded(vCursorPos.x + flFeatureWidth - (2.0f * FEATURE_PADDING_PXL), vCursorPos.y + FEATURE_HEIGHT);

            if(Features::Menu::Menu::Draw_Guides.IsActive() == true)
            {
                ImVec2 vCursorScreenPos = ImGui::GetCursorScreenPos(); 
                vCursorScreenPos.x -= FEATURE_PADDING_PXL; // we added the feature padding before this & we don't that here. ( so we remove it )
                ImGui::GetWindowDrawList()->AddRect(vCursorScreenPos, ImVec2(vCursorScreenPos.x + flFeatureWidth, vCursorScreenPos.y + FEATURE_HEIGHT), ImColor(1.0f, 0.0f, 0.0f, 1.0f));
            }

            // Drawing the got dayem features.
            switch (pFeature->m_iDataType)
            {
            case IFeature::DataType::DT_BOOLEAN:     _DrawBoolean    (pFeature, vFeatureMinPadded, vFeatureMaxPadded); break;
            case IFeature::DataType::DT_COLORDATA:   _DrawColor      (pFeature, vFeatureMinPadded, vFeatureMaxPadded); break;
            case IFeature::DataType::DT_INTSLIDER:   _DrawIntSlider  (pFeature, vFeatureMinPadded, vFeatureMaxPadded); break;
            case IFeature::DataType::DT_FLOATSLIDER: _DrawFloatSlider(pFeature, vFeatureMinPadded, vFeatureMaxPadded); break;
            case IFeature::DataType::DT_DROPDOWN:    _DrawDropDown   (pFeature, vFeatureMinPadded, vFeatureMaxPadded); break;
            default: break;
            }

            // Adjust cursor for the next feature.
            ImGui::SetCursorPosX(vSectionLocalPos.x + SECTION_PADDING_PXL + FEATURE_PADDING_PXL      );
            ImGui::SetCursorPosY(vCursorPos.y       + FEATURE_HEIGHT      + INTER_FEATURE_PADDING_PXL);
        }


        ImVec2 vSectionSize = _CalculateSectionSize(
            pSection->m_vecFeatures.size(),                 // No. of features.
            INTER_FEATURE_PADDING_PXL, SECTION_PADDING_PXL, // Padding
            flFeatureWidth, FEATURE_HEIGHT                  // Width & Height for each feature.
        );

        if (Features::Menu::Menu::Draw_Guides.IsActive() == true)
        {
            ImGui::GetWindowDrawList()->AddRect(
                *pSectionScreenPos,
                ImVec2(pSectionScreenPos->x + vSectionSize.x, pSectionScreenPos->y + vSectionSize.y),
                ImColor(1.0f, 1.0f, 1.0f, 1.0f)
            );
        }

        // Creating section's UI boxes.
        if (iSectionIndex >= vecSectionBoxes.size())
        {
            vecSectionBoxes.push_back(new BoxFilled2D_t());
        }

        // Setting Section's UI boxes size & pos
        BoxFilled2D_t* pBox = vecSectionBoxes[iSectionIndex];
        // Clamping vertex for section's UI boxes.
        vec vSectionBoxMin(pSectionScreenPos->x,                  pSectionScreenPos->y, 0.0f);
        vec vSectionBoxMax(pSectionScreenPos->x + vSectionSize.x, pSectionScreenPos->y + vSectionSize.y, 0.0f);
        {
            vSectionBoxMin.x = std::clamp<float>(vSectionBoxMin.x, x,            x + flWidth);
            vSectionBoxMin.y = std::clamp<float>(vSectionBoxMin.y, vWindowPos.y, vWindowPos.y + flHeight);

            vSectionBoxMax.x = std::clamp<float>(vSectionBoxMax.x, x,            x + flWidth);
            vSectionBoxMax.y = std::clamp<float>(vSectionBoxMax.y, vWindowPos.y, vWindowPos.y + flHeight);
        }

        // Setting Visual settings for section boxes.
        pBox->SetVertex(vSectionBoxMin, vSectionBoxMax);
        pBox->SetColor(Features::Menu::SectionBoxes::ColorTopLeft.GetData().GetAsBytes(),     IBoxFilled_t::VertexType_t::VertexType_TopLeft);
        pBox->SetColor(Features::Menu::SectionBoxes::ColorTopRight.GetData().GetAsBytes(),    IBoxFilled_t::VertexType_t::VertexType_TopRight);
        pBox->SetColor(Features::Menu::SectionBoxes::ColorBottomLeft.GetData().GetAsBytes(),  IBoxFilled_t::VertexType_t::VertexType_BottomLeft);
        pBox->SetColor(Features::Menu::SectionBoxes::ColorBottomRight.GetData().GetAsBytes(), IBoxFilled_t::VertexType_t::VertexType_BottomRight);
        pBox->SetRGBAnimSpeed(Features::Menu::SectionBoxes::rgb.IsActive() == false ? -1.0 : Features::Menu::SectionBoxes::RGBSpeed.GetData().m_flVal);    
        pBox->SetBlur(Features::Menu::SectionBoxes::Blur.GetData().m_flVal);
        pBox->SetRounding(Features::Menu::SectionBoxes::Rounding.GetData().m_flVal);
        pBox->SetVisible(true);

        // Adding section's size fo next section draws accordingly.
        pSectionScreenPos->y += vSectionSize.y + FRAME_PADDING_PXL;
        
        // Choosing the side with the most space.
        bDrawingOnLeft = (vLeftSectionCursor.y > vRightSectionCursor.y ? false : true);
    }

    // Disabling unused section UI boxes.
    size_t nBoxes = vecSectionBoxes.size();
    for (int iBoxIndex = iSectionIndex; iBoxIndex < nBoxes; iBoxIndex++)
    {
        vecSectionBoxes[iBoxIndex]->SetVisible(false);
    }


    {
        ImGui::PopStyleVar();
    }
}


///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
ImVec2 MenuGUI_t::_CalculateSectionSize(int nFeatures, float flInterFeaturePadding, float flSectionPadding, float flFeatureWidth, float flFeatureHeight) const
{
    return ImVec2(
        (flSectionPadding * 2.0f) + flFeatureWidth,
        (static_cast<float>(nFeatures) * flFeatureHeight) + (static_cast<float>(nFeatures - 1) * flInterFeaturePadding) + (2.0f * flSectionPadding)
    );
}


///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
void MenuGUI_t::_DrawBoolean(IFeature* pFeature, ImVec2 vMinWithPadding, ImVec2 vMaxWithPadding) const
{
    Feature<bool>* pBoolFeature = reinterpret_cast<Feature<bool>*>(pFeature);
    
    // Styling
    ImGui::PushStyleColor(ImGuiCol_FrameBg,        pBoolFeature->m_Data == true ? m_clrTheme.GetAsImVec4() : m_clrSecondary.GetAsImVec4());
    ImGui::PushStyleColor(ImGuiCol_FrameBgHovered, m_clrTheme.GetAsImVec4());
    ImGui::PushStyleColor(ImGuiCol_FrameBgActive,  m_clrTheme.GetAsImVec4());
    ImGui::PushStyleColor(ImGuiCol_CheckMark,      ImVec4(1.0f, 1.0f, 1.0f, 1.0f));
    ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 3.0f);
    
    float  flCheckBoxSide  = ImGui::GetFrameHeight();
    float  flFeatureHeight = fabsf(vMinWithPadding.y - vMaxWithPadding.y);
    float  flFeatureWidth  = fabsf(vMaxWithPadding.x - vMinWithPadding.x);
    ImVec2 vWindowPos      = ImGui::GetWindowPos();

    // Feature's name
    ImGui::SetCursorScreenPos(ImVec2(vWindowPos.x + vMinWithPadding.x, vWindowPos.y + vMinWithPadding.y + (flFeatureHeight - ImGui::GetTextLineHeight()) / 2.0f));
    ImGui::Text(pBoolFeature->m_szFeatureDisplayName.c_str()); ImGui::SameLine();

    // Checkbox
    ImGui::SetCursorScreenPos(ImVec2(vWindowPos.x + vMaxWithPadding.x - flCheckBoxSide, vWindowPos.y + vMinWithPadding.y + (flFeatureHeight - flCheckBoxSide) / 2.0f));
    ImGui::Checkbox(("##" + pBoolFeature->m_szTabName + pBoolFeature->m_szSectionName + pBoolFeature->m_szFeatureDisplayName).c_str(), &pBoolFeature->m_Data);

    // Tool tip
    if (ImGui::IsItemHovered() == true && pBoolFeature->m_szToolTip.empty() == false)
        ImGui::SetTooltip(pBoolFeature->m_szToolTip.c_str());

    // Pop all style vars.
    ImGui::PopStyleColor(4);
    ImGui::PopStyleVar(1);
}


///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
void MenuGUI_t::_DrawIntSlider(IFeature* pFeature, ImVec2 vMinWithPadding, ImVec2 vMaxWithPadding) const
{
    Feature<IntSlider_t>* pIntFeature = reinterpret_cast<Feature<IntSlider_t>*>(pFeature);

    float flFeatureWidth  = fabsf(vMaxWithPadding.x - vMinWithPadding.x);
    float flFeatureHeight = fabsf(vMinWithPadding.y - vMaxWithPadding.y);

    // Styling
    ImGui::PushStyleColor(ImGuiCol_FrameBg,          ImVec4(0.0f, 0.0f, 0.0f, 0.0f));
    ImGui::PushStyleColor(ImGuiCol_FrameBgActive,    ImVec4(0.0f, 0.0f, 0.0f, 0.0f));
    ImGui::PushStyleColor(ImGuiCol_FrameBgHovered,   ImVec4(0.0f, 0.0f, 0.0f, 0.0f));
    ImGui::PushStyleColor(ImGuiCol_SliderGrab,       ImVec4(0.0f, 0.0f, 0.0f, 0.0f));
    ImGui::PushStyleColor(ImGuiCol_SliderGrabActive, ImVec4(0.0f, 0.0f, 0.0f, 0.0f));
    ImGui::PushStyleColor(ImGuiCol_Text,             ImVec4(0.0f, 0.0f, 0.0f, 0.0f));

    // Feature's name
    constexpr float GAP_BEFORE_TRACK = 0.4f;
    ImGui::Text(pIntFeature->m_szFeatureDisplayName.c_str()); ImGui::SameLine(vMinWithPadding.x + (flFeatureWidth * GAP_BEFORE_TRACK));

    // Slider.
    const float flTrackWidth = flFeatureWidth * 0.4f/*/ 2.0f*/;
    ImGui::SetNextItemWidth(flTrackWidth);
    ImGui::SliderInt(
        ("##" + pIntFeature->m_szTabName + pIntFeature->m_szSectionName + pIntFeature->m_szFeatureDisplayName).c_str(),
        &pIntFeature->m_Data.m_iVal, pIntFeature->m_Data.m_iMin, pIntFeature->m_Data.m_iMax);

    // Tool tip
    if (ImGui::IsItemHovered() == true && pIntFeature->m_szToolTip.empty() == false)
        ImGui::SetTooltip(pIntFeature->m_szToolTip.c_str());

    // Pop all style vars.
    ImGui::PopStyleColor(6);

    ImDrawList* pDrawList  = ImGui::GetWindowDrawList();
    ImVec2      vWindowPos = ImGui::GetWindowPos();

    // Drawing label for slider.
    pDrawList->AddText(
        ImVec2(vWindowPos.x + vMinWithPadding.x, vWindowPos.y + vMinWithPadding.y + (flFeatureHeight - ImGui::GetTextLineHeight()) / 2.0f),
        ImColor(255, 255, 255, 255), pIntFeature->m_szFeatureDisplayName.c_str()
    );


    // Drawing the slider manually
    constexpr float TRACK_THICKNESS_PXL = 4.0f;
    pDrawList->AddRectFilled( // Drawing track.
        ImVec2(vWindowPos.x + vMinWithPadding.x + (flFeatureWidth * GAP_BEFORE_TRACK), vWindowPos.y + vMinWithPadding.y + (flFeatureHeight / 2.0f) - TRACK_THICKNESS_PXL),
        ImVec2(vWindowPos.x + vMinWithPadding.x + (flFeatureWidth * GAP_BEFORE_TRACK) + flTrackWidth, vWindowPos.y + vMinWithPadding.y + (flFeatureHeight / 2.0f) + TRACK_THICKNESS_PXL),
        ImColor(255, 255, 255, 255), 10.0f);


    // Drawing knob.
    constexpr float KNOB_SIZE_PXL = 10.0f;
    float flKnobPos = static_cast<float>(pIntFeature->GetData().m_iVal - pIntFeature->GetData().m_iMin) / static_cast<float>(pIntFeature->GetData().m_iMax - pIntFeature->GetData().m_iMin);
    ImVec2 vKnobOriginScreen(
        vWindowPos.x + vMinWithPadding.x + (flFeatureWidth * GAP_BEFORE_TRACK) + (flTrackWidth * flKnobPos),
        vWindowPos.y + ((vMinWithPadding.y + vMaxWithPadding.y) / 2.0f)
    );
    pDrawList->AddRectFilled(
        ImVec2(vKnobOriginScreen.x - KNOB_SIZE_PXL, vKnobOriginScreen.y - KNOB_SIZE_PXL),
        ImVec2(vKnobOriginScreen.x + KNOB_SIZE_PXL, vKnobOriginScreen.y + KNOB_SIZE_PXL),
        ImColor(m_clrTheme.GetAsImVec4()), 20.0f
    );

    ImVec2 vFloatInputScreenPos(
        vWindowPos.x + vMinWithPadding.x + (flFeatureWidth * GAP_BEFORE_TRACK) + flTrackWidth + FEATURE_PADDING_PXL,
        vWindowPos.y + vMinWithPadding.y + ((flFeatureHeight - ImGui::GetFrameHeight()) / 2.0f)
    );

    // Styling & drawing float input.
    ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize,  2.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding,    5.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_ButtonTextAlign,  ImVec2(0.5f, 0.5f));
    ImGui::PushStyleColor(ImGuiCol_Border,        m_clrTheme.GetAsImVec4());
    ImGui::PushStyleColor(ImGuiCol_Text,          ImVec4(0.0f, 0.0f, 0.0f, 0.0f));
    ImGui::PushStyleColor(ImGuiCol_FrameBg,       m_clrSecondary.GetAsImVec4());
    ImGui::PushStyleColor(ImGuiCol_FrameBgActive, m_clrTheme.GetAsImVec4());

    ImGui::SetCursorScreenPos(vFloatInputScreenPos);
    float flIntInputWidth = (vWindowPos.x + vMaxWithPadding.x) - vFloatInputScreenPos.x;
    ImGui::SetNextItemWidth(flIntInputWidth);

    ImGui::InputInt(
        ("##IntInput" + pIntFeature->m_szTabName + pIntFeature->m_szSectionName + pIntFeature->m_szFeatureDisplayName).c_str(),
        &pIntFeature->m_Data.m_iVal, 0, 0);

    ImGui::PopStyleVar(3);
    ImGui::PopStyleColor(4);

    std::string szSliderValue = std::format("{}", pIntFeature->m_Data.m_iVal);
    size_t      nDigits       = szSliderValue.size();
    float       flCharWidth   = Resources::Fonts::JetBrains_SemiBold_NL_Small->GetCharAdvance(' '); // Assuming using a mono font.
    pDrawList->AddText(
        ImVec2(vFloatInputScreenPos.x + ((flIntInputWidth - (static_cast<float>(nDigits) * flCharWidth)) / 2.0f), vFloatInputScreenPos.y + ((ImGui::GetFrameHeight() - ImGui::GetTextLineHeight()) / 2.0f)),
        ImColor(255, 255, 255, 255), szSliderValue.c_str()
    );
}


///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
void MenuGUI_t::_DrawFloatSlider(IFeature* pFeature, ImVec2 vMinWithPadding, ImVec2 vMaxWithPadding) const
{
    Feature<FloatSlider_t>* pFloatFeature = reinterpret_cast<Feature<FloatSlider_t>*>(pFeature);

    float flFeatureWidth  = fabsf(vMaxWithPadding.x - vMinWithPadding.x);
    float flFeatureHeight = fabsf(vMinWithPadding.y - vMaxWithPadding.y);

    // Styling
    ImGui::PushStyleColor(ImGuiCol_FrameBg,          ImVec4(0.0f, 0.0f, 0.0f, 0.0f));
    ImGui::PushStyleColor(ImGuiCol_FrameBgActive,    ImVec4(0.0f, 0.0f, 0.0f, 0.0f));
    ImGui::PushStyleColor(ImGuiCol_FrameBgHovered,   ImVec4(0.0f, 0.0f, 0.0f, 0.0f));
    ImGui::PushStyleColor(ImGuiCol_SliderGrab,       ImVec4(0.0f, 0.0f, 0.0f, 0.0f));
    ImGui::PushStyleColor(ImGuiCol_SliderGrabActive, ImVec4(0.0f, 0.0f, 0.0f, 0.0f));
    ImGui::PushStyleColor(ImGuiCol_Text,             ImVec4(0.0f, 0.0f, 0.0f, 0.0f));
    
    // Feature's name
    constexpr float GAP_BEFORE_TRACK = 0.4f;
    ImGui::Text(pFloatFeature->m_szFeatureDisplayName.c_str()); ImGui::SameLine(vMinWithPadding.x + (flFeatureWidth * GAP_BEFORE_TRACK));

    // Slider.
    const float flTrackWidth = flFeatureWidth * 0.4f/*/ 2.0f*/;
    ImGui::SetNextItemWidth(flTrackWidth);
    ImGui::SliderFloat(
        ("##" + pFloatFeature->m_szTabName + pFloatFeature->m_szSectionName + pFloatFeature->m_szFeatureDisplayName).c_str(),
        &pFloatFeature->m_Data.m_flVal, pFloatFeature->m_Data.m_flMin, pFloatFeature->m_Data.m_flMax);

    // Tool tip
    if (ImGui::IsItemHovered() == true && pFloatFeature->m_szToolTip.empty() == false)
        ImGui::SetTooltip(pFloatFeature->m_szToolTip.c_str());

    // Pop all style vars.
    ImGui::PopStyleColor(6);

    ImDrawList* pDrawList  = ImGui::GetWindowDrawList();
    ImVec2      vWindowPos = ImGui::GetWindowPos();
    
    // Drawing label for slider.
    pDrawList->AddText(
        ImVec2(vWindowPos.x + vMinWithPadding.x, vWindowPos.y + vMinWithPadding.y + (flFeatureHeight - ImGui::GetTextLineHeight()) / 2.0f),
        ImColor(255, 255, 255, 255), pFloatFeature->m_szFeatureDisplayName.c_str()
    );


    // Drawing the slider manually
    constexpr float TRACK_THICKNESS_PXL = 4.0f;
    pDrawList->AddRectFilled( // Drawing track.
        ImVec2(vWindowPos.x + vMinWithPadding.x + (flFeatureWidth * GAP_BEFORE_TRACK),                vWindowPos.y + vMinWithPadding.y + (flFeatureHeight / 2.0f) - TRACK_THICKNESS_PXL),
        ImVec2(vWindowPos.x + vMinWithPadding.x + (flFeatureWidth * GAP_BEFORE_TRACK) + flTrackWidth, vWindowPos.y + vMinWithPadding.y + (flFeatureHeight / 2.0f) + TRACK_THICKNESS_PXL),
        ImColor(255, 255, 255, 255), 10.0f);


    // Drawing knob.
    constexpr float KNOB_SIZE_PXL = 10.0f;
    float flKnobPos = (pFloatFeature->GetData().m_flVal - pFloatFeature->GetData().m_flMin) / (pFloatFeature->GetData().m_flMax - pFloatFeature->GetData().m_flMin);
    ImVec2 vKnobOriginScreen(
        vWindowPos.x + vMinWithPadding.x + (flFeatureWidth * GAP_BEFORE_TRACK) + (flTrackWidth * flKnobPos),
        vWindowPos.y + ((vMinWithPadding.y + vMaxWithPadding.y) / 2.0f)
    );
    pDrawList->AddRectFilled(
        ImVec2(vKnobOriginScreen.x - KNOB_SIZE_PXL, vKnobOriginScreen.y - KNOB_SIZE_PXL),
        ImVec2(vKnobOriginScreen.x + KNOB_SIZE_PXL, vKnobOriginScreen.y + KNOB_SIZE_PXL),
        ImColor(m_clrTheme.GetAsImVec4()), 20.0f
    );

    ImVec2 vFloatInputScreenPos(
        vWindowPos.x + vMinWithPadding.x + (flFeatureWidth * GAP_BEFORE_TRACK) + flTrackWidth + FEATURE_PADDING_PXL,
        vWindowPos.y + vMinWithPadding.y + ((flFeatureHeight - ImGui::GetFrameHeight()) / 2.0f)
    );

    // Styling & drawing float input.
    ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 2.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding,   5.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_ButtonTextAlign, ImVec2(0.5f, 0.5f));
    ImGui::PushStyleColor(ImGuiCol_Border,  m_clrTheme.GetAsImVec4());
    ImGui::PushStyleColor(ImGuiCol_Text,    ImVec4(0.0f, 0.0f, 0.0f, 0.0f));
    ImGui::PushStyleColor(ImGuiCol_FrameBg, m_clrSecondary.GetAsImVec4());
    ImGui::PushStyleColor(ImGuiCol_FrameBgActive, m_clrTheme.GetAsImVec4());

    ImGui::SetCursorScreenPos(vFloatInputScreenPos);
    float flFloatInputWidth = (vWindowPos.x + vMaxWithPadding.x) - vFloatInputScreenPos.x;
    ImGui::SetNextItemWidth(flFloatInputWidth);

    ImGui::InputFloat(
        ("##FloatInput" + pFloatFeature->m_szTabName + pFloatFeature->m_szSectionName + pFloatFeature->m_szFeatureDisplayName).c_str(),
        &pFloatFeature->m_Data.m_flVal, 0.0f, 0.0f, "%.1f");

    ImGui::PopStyleVar(3);
    ImGui::PopStyleColor(4);

    std::string szSliderValue = std::format("{:.1f}", pFloatFeature->m_Data.m_flVal);
    size_t      nDigits       = szSliderValue.size();
    float       flCharWidth   = Resources::Fonts::JetBrains_SemiBold_NL_Small->GetCharAdvance(' '); // Assuming using a mono font.
    pDrawList->AddText(
        ImVec2(vFloatInputScreenPos.x + ((flFloatInputWidth - (static_cast<float>(nDigits) * flCharWidth)) / 2.0f), vFloatInputScreenPos.y + ((ImGui::GetFrameHeight() - ImGui::GetTextLineHeight()) / 2.0f)),
        ImColor(255, 255, 255, 255), szSliderValue.c_str()
    );
}


///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
void MenuGUI_t::_DrawDropDown(IFeature* pFeature, ImVec2 vMinWithPadding, ImVec2 vMaxWithPadding) const
{
    // Styling
    ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 1.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding,   5.0f);
    ImGui::PushStyleColor(ImGuiCol_Border,         m_clrTheme.GetAsImVec4());
    ImGui::PushStyleColor(ImGuiCol_FrameBg,        m_clrSecondary.GetAsImVec4());
    ImGui::PushStyleColor(ImGuiCol_FrameBgHovered, m_clrSecondary.GetAsImVec4());
    ImGui::PushStyleColor(ImGuiCol_FrameBgActive,  m_clrSecondary.GetAsImVec4());
    ImGui::PushStyleColor(ImGuiCol_Button,         m_clrSecondary.GetAsImVec4());
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered,  m_clrSecondary.GetAsImVec4());
    ImGui::PushStyleColor(ImGuiCol_ButtonActive,   m_clrSecondary.GetAsImVec4());
                                                   
    Feature<DropDown_t>* pDropDownFeature = static_cast<Feature<DropDown_t>*>(pFeature);

    float  flWidgetHeight     = ImGui::GetFrameHeight();
    float  flFeatureHeight    = fabsf(vMinWithPadding.y - vMaxWithPadding.y);
    float  flFeatureWidth     = fabsf(vMaxWithPadding.x - vMinWithPadding.x);
    ImVec2 vWindowPos         = ImGui::GetWindowPos();

    // Feature's name
    ImGui::SetCursorScreenPos(ImVec2(vWindowPos.x + vMinWithPadding.x, vWindowPos.y + vMinWithPadding.y + (flFeatureHeight - ImGui::GetTextLineHeight()) / 2.0f));
    ImGui::Text(pDropDownFeature->m_szFeatureDisplayName.c_str()); ImGui::SameLine();

    // Drop Down Feature
    ImGui::SetNextItemWidth(flFeatureWidth / 2.0f);
    ImGui::SetCursorScreenPos(ImVec2(vWindowPos.x + vMaxWithPadding.x - flFeatureWidth / 2.0f, vWindowPos.y + vMinWithPadding.y + (flFeatureHeight - flWidgetHeight) / 2.0f));
    ImGui::Combo(
        ("##" + pDropDownFeature->m_szTabName + pDropDownFeature->m_szSectionName + pDropDownFeature->m_szFeatureDisplayName).c_str(),
        &pDropDownFeature->m_iActiveData, pDropDownFeature->m_data.m_pItems, pDropDownFeature->m_data.m_nItems);

    // Tool tip
    if (ImGui::IsItemHovered() == true && pDropDownFeature->m_szToolTip.empty() == false)
        ImGui::SetTooltip(pDropDownFeature->m_szToolTip.c_str());

    // Pop all style vars.
    ImGui::PopStyleColor(7);
    ImGui::PopStyleVar(2);
}


///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
void MenuGUI_t::_DrawColor(IFeature* pFeature, ImVec2 vMinWithPadding, ImVec2 vMaxWithPadding) const
{
    Feature<ColorData_t>* pColorFeature = reinterpret_cast<Feature<ColorData_t>*>(pFeature);

    float  flColorPreviewSide = ImGui::GetFrameHeight();
    float  flFeatureHeight    = fabsf(vMinWithPadding.y - vMaxWithPadding.y);
    float  flFeatureWidth     = fabsf(vMaxWithPadding.x - vMinWithPadding.x);
    ImVec2 vWindowPos         = ImGui::GetWindowPos();
    
    ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding,   3.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 2.0f);
    ImGui::PushStyleColor(ImGuiCol_Border, m_clrTheme.GetAsImVec4());

    // Label
    ImGui::SetCursorScreenPos(ImVec2(vWindowPos.x + vMinWithPadding.x, vWindowPos.y + vMinWithPadding.y + (flFeatureHeight - ImGui::GetTextLineHeight()) / 2.0f));
    ImGui::Text(pColorFeature->m_szFeatureDisplayName.c_str()); ImGui::SameLine();

    // Color preview widget.
    ImGui::SetCursorScreenPos(ImVec2(vWindowPos.x + vMaxWithPadding.x - flColorPreviewSide, vWindowPos.y + vMinWithPadding.y + (flFeatureHeight - flColorPreviewSide) / 2.0f));
    ImGui::ColorEdit4(
        ("##" + pColorFeature->m_szTabName + pColorFeature->m_szSectionName + pColorFeature->m_szFeatureDisplayName).c_str(), &pColorFeature->m_Data.r,
        ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoLabel);

    // Tool tip
    if (ImGui::IsItemHovered() == true && pColorFeature->m_szToolTip.empty() == false)
        ImGui::SetTooltip(pColorFeature->m_szToolTip.c_str());

    ImGui::PopStyleColor();
    ImGui::PopStyleVar(2);
}


///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
void MenuGUI_t::_CalculateColors()
{
    // Primary color
    {
        ColorData_t clrBottomLeft  = Features::Menu::Menu::ColorBottomLeft.GetData();
        ColorData_t clrBottomRight = Features::Menu::Menu::ColorBottomRight.GetData();
        ColorData_t clrTopLeft     = Features::Menu::Menu::ColorTopLeft.GetData();
        ColorData_t clrTopRight    = Features::Menu::Menu::ColorTopRight.GetData();

        // Averaging out the color for each corner.
        m_clrPrimary.r = static_cast<unsigned char>(((clrBottomLeft.r + clrBottomRight.r + clrTopLeft.r + clrTopRight.r) / 4.0f) * 255.0f);
        m_clrPrimary.g = static_cast<unsigned char>(((clrBottomLeft.g + clrBottomRight.g + clrTopLeft.g + clrTopRight.g) / 4.0f) * 255.0f);
        m_clrPrimary.b = static_cast<unsigned char>(((clrBottomLeft.b + clrBottomRight.b + clrTopLeft.b + clrTopRight.b) / 4.0f) * 255.0f);
        m_clrPrimary.a = static_cast<unsigned char>(((clrBottomLeft.a + clrBottomRight.a + clrTopLeft.a + clrTopRight.a) / 4.0f) * 255.0f);
    }

    // Secondary color
    {
        ColorData_t clrBottomLeft  = Features::Menu::SideMenu::ColorBottomLeft.GetData();
        ColorData_t clrBottomRight = Features::Menu::SideMenu::ColorBottomRight.GetData();
        ColorData_t clrTopLeft     = Features::Menu::SideMenu::ColorTopLeft.GetData();
        ColorData_t clrTopRight    = Features::Menu::SideMenu::ColorTopRight.GetData();

        // Averaging out the color for each corner.
        m_clrSecondary.r = static_cast<unsigned char>(((clrBottomLeft.r + clrBottomRight.r + clrTopLeft.r + clrTopRight.r) / 4.0f) * 255.0f);
        m_clrSecondary.g = static_cast<unsigned char>(((clrBottomLeft.g + clrBottomRight.g + clrTopLeft.g + clrTopRight.g) / 4.0f) * 255.0f);
        m_clrSecondary.b = static_cast<unsigned char>(((clrBottomLeft.b + clrBottomRight.b + clrTopLeft.b + clrTopRight.b) / 4.0f) * 255.0f);
        m_clrSecondary.a = static_cast<unsigned char>(((clrBottomLeft.a + clrBottomRight.a + clrTopLeft.a + clrTopRight.a) / 4.0f) * 255.0f);
    }

    // Theme
    m_clrTheme = Features::Menu::Theme::Theme.GetData().GetAsBytes();

    // Side menu buttom color
    {
        HSVA_t hsv = m_clrSecondary.ToHSVA();
        hsv.h = fmodf(hsv.h + 180.0f, 360.0f);
        //hsv.s = fmodf(hsv.s * 1.2f,   1.0f);
        //hsv.v = fmodf(hsv.v * 1.2f,   1.0f);
        // is there enough room that we can make the color distinct by making it brighter.
        if (hsv.s <= 0.85f && hsv.v <= 0.85f)
        {
            hsv.s += 0.15f; hsv.v += 0.15f;
        }
        else // Else change the gotdayem color. AAAAAAAHHHHHHH!!!!!!!!!!!!!!!!!!
        {
            hsv.s = 1.0f; hsv.v = 1.0f;
            hsv.h += 10.0f;
            if (hsv.h >= 360.0f)
            {
                hsv.h = fmodf(hsv.h, 360.0f);
            }
        }
        m_clrSideMenuButtons = hsv.ToRGBA();
    }
}


///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
void MenuGUI_t::_StyleSideMenuBottons()
{
    // TODO : Set active buttons color to primary.
    ImGui::PushStyleColor(ImGuiCol_Button,             ImVec4(0.0f, 0.0f, 0.0f, 0.0f));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered,      m_clrSideMenuButtons.GetAsImVec4());
    ImGui::PushStyleColor(ImGuiCol_ButtonActive,       m_clrSideMenuButtons.GetAsImVec4());
    m_nPushedStyleColors += 3;

    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding,    ImVec2(0.0f, 0.0f));
    ImGui::PushStyleVar(ImGuiStyleVar_ButtonTextAlign, ImVec2(0.0f, 0.5f));
    ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding,   5.0f);
    m_nPushedStyleVars += 3;
}


///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
void MenuGUI_t::_PopAllStyles()
{
    assert(m_nPushedStyleColors >= 0 && "Pushed styles count is negative");
    assert(m_nPushedStyleVars   >= 0 && "Pushed styles vars is negative");

    ImGui::PopStyleColor(m_nPushedStyleColors); m_nPushedStyleColors = 0;
    ImGui::PopStyleVar(m_nPushedStyleVars);     m_nPushedStyleVars   = 0;
}
