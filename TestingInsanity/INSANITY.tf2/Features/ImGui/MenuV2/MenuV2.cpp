#include "MenuV2.h"

#include "../../../External Libraries/ImGui/imgui.h"
#include "../../../Hooks/DirectX Hook/DirectX_hook.h"
#include "../../FeatureHandler.h"

// Renderer
#include "../../Graphics Engine V2/Graphics.h"
#include "../../Graphics Engine V2/Draw Objects/Box/Box.h"
#include "../../Graphics Engine V2/Draw Objects/Line/Line.h"
#include "../../Graphics Engine V2/Draw Objects/Circle/Circle.h"


constexpr float SIDEMENU_SCALE = 0.2f;


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
    
    constexpr float FRAME_PADDING_PXL         = 15.0f; // Padding between Main body & its contents.
    constexpr float SECTION_PADDING_PXL       = 10.0f; // Padding between Section walls & its contents.
    constexpr float INTER_FEATURE_PADDING_PXL =  5.0f; // Padding between each feature.
    constexpr float FEATURE_PADDING_PXL       =  5.0f; // Padding between feautres & its contents.
    constexpr float FEATURE_HEIGHT            = 30.0f; // Height of each feature.

    ImVec2 vLeftSectionCursor (x + FRAME_PADDING_PXL,                             y + FRAME_PADDING_PXL);
    ImVec2 vRightSectionCursor(x + (flWidth / 2.0f) + (FRAME_PADDING_PXL / 2.0f), y + FRAME_PADDING_PXL);

    bool bDrawingOnLeft = true;

    for (Section_t* pSection : pTab->m_vecSections)
    {
        // This is the screen pos for the cursor.
        ImVec2* pSectionScreenPos = bDrawingOnLeft == true ? &vLeftSectionCursor : &vRightSectionCursor;
        ImGui::SetCursorScreenPos(*pSectionScreenPos);
        ImVec2 vSectionLocalPos(pSectionScreenPos->x - vWindowPos.x, pSectionScreenPos->y - vWindowPos.y);


        float flFeatureWidth = (flWidth / 2.0f) - (1.5f * FRAME_PADDING_PXL) - (2 * SECTION_PADDING_PXL);
        ImGui::SetCursorPos(ImVec2(vSectionLocalPos.x + SECTION_PADDING_PXL + FEATURE_PADDING_PXL, ImGui::GetCursorPosY() + SECTION_PADDING_PXL));

        for (IFeature* pFeature : pSection->m_vecFeatures)
        {
            ImVec2 vCursorPos = ImGui::GetCursorPos();

            if(Features::Menu::Menu::Draw_Guides.IsActive() == true)
            {
                ImVec2 vCursorScreenPos = ImGui::GetCursorScreenPos(); 
                vCursorScreenPos.x -= FEATURE_PADDING_PXL; // we added the feature padding before this & we don't that here. ( so we remove it )
                ImGui::GetWindowDrawList()->AddRect(vCursorScreenPos, ImVec2(vCursorScreenPos.x + flFeatureWidth, vCursorScreenPos.y + FEATURE_HEIGHT), ImColor(1.0f, 0.0f, 0.0f, 1.0f));
            }

            // Drawing the got dayem features.
            switch (pFeature->m_iDataType)
            {
            case IFeature::DataType::DT_BOOLEAN:     _DrawBoolean    (pFeature, vCursorPos.x + flFeatureWidth - (2.0f * FEATURE_PADDING_PXL)); break;
            case IFeature::DataType::DT_COLORDATA:   _DrawColor      (pFeature, vCursorPos.x + flFeatureWidth - (2.0f * FEATURE_PADDING_PXL)); break;
            case IFeature::DataType::DT_INTSLIDER:   _DrawIntSlider  (pFeature, (flFeatureWidth / 2.0f) - FEATURE_PADDING_PXL, vCursorPos.x + ((flFeatureWidth / 2.0f) - FEATURE_PADDING_PXL)); break;
            case IFeature::DataType::DT_FLOATSLIDER: _DrawFloatSlider(pFeature, (flFeatureWidth / 2.0f) - FEATURE_PADDING_PXL, vCursorPos.x + ((flFeatureWidth / 2.0f) - FEATURE_PADDING_PXL)); break;
            case IFeature::DataType::DT_DROPDOWN:    _DrawDropDown   (pFeature, (flFeatureWidth / 2.0f) - FEATURE_PADDING_PXL, vCursorPos.x + ((flFeatureWidth / 2.0f) - FEATURE_PADDING_PXL)); break;
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

        // Adding section's size fo next section draws accordingly.
        pSectionScreenPos->y += vSectionSize.y + FRAME_PADDING_PXL;
        
        // Choosing the side with the most space.
        bDrawingOnLeft = (vLeftSectionCursor.y > vRightSectionCursor.y ? false : true);
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
void MenuGUI_t::_DrawBoolean(IFeature* pFeature, float flFeatureEnd) const
{
    Feature<bool>* pBoolFeature = reinterpret_cast<Feature<bool>*>(pFeature);
    
    float flCheckBoxWidth = ImGui::GetFrameHeight();

    // Feature's name
    ImGui::Text(pBoolFeature->m_szFeatureDisplayName.c_str()); ImGui::SameLine(flFeatureEnd - flCheckBoxWidth);

    // Checkbox
    ImGui::Checkbox(("##" + pBoolFeature->m_szTabName + pBoolFeature->m_szSectionName + pBoolFeature->m_szFeatureDisplayName).c_str(), &pBoolFeature->m_Data);

    // Tool tip
    if (ImGui::IsItemHovered() == true && pBoolFeature->m_szToolTip.empty() == false)
        ImGui::SetTooltip(pBoolFeature->m_szToolTip.c_str());
}


///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
void MenuGUI_t::_DrawIntSlider(IFeature* pFeature, float flWidgetWidth, float flWidgetStartX) const
{
    Feature<IntSlider_t>* pIntFeature = reinterpret_cast<Feature<IntSlider_t>*>(pFeature);

    // Feature's name
    ImGui::Text(pIntFeature->m_szFeatureDisplayName.c_str()); ImGui::SameLine(flWidgetStartX);

    // Checkbox
    ImGui::SetNextItemWidth(flWidgetWidth);
    ImGui::SliderInt(
        ("##" + pIntFeature->m_szTabName + pIntFeature->m_szSectionName + pIntFeature->m_szFeatureDisplayName).c_str(), 
        &pIntFeature->m_Data.m_iVal, pIntFeature->m_Data.m_iMin, pIntFeature->m_Data.m_iMax);

    // Tool tip
    if (ImGui::IsItemHovered() == true && pIntFeature->m_szToolTip.empty() == false)
        ImGui::SetTooltip(pIntFeature->m_szToolTip.c_str());
}


///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
void MenuGUI_t::_DrawFloatSlider(IFeature* pFeature, float flWidgetWidth, float flWidgetStartX) const
{
    Feature<FloatSlider_t>* pFloatFeature = reinterpret_cast<Feature<FloatSlider_t>*>(pFeature);

    // Feature's name
    ImGui::Text(pFloatFeature->m_szFeatureDisplayName.c_str()); ImGui::SameLine(flWidgetStartX);

    // Checkbox
    ImGui::SetNextItemWidth(flWidgetWidth);
    ImGui::SliderFloat(
        ("##" + pFloatFeature->m_szTabName + pFloatFeature->m_szSectionName + pFloatFeature->m_szFeatureDisplayName).c_str(), 
        &pFloatFeature->m_Data.m_flVal, pFloatFeature->m_Data.m_flMin, pFloatFeature->m_Data.m_flMax);

    // Tool tip
    if (ImGui::IsItemHovered() == true && pFloatFeature->m_szToolTip.empty() == false)
        ImGui::SetTooltip(pFloatFeature->m_szToolTip.c_str());
}


///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
void MenuGUI_t::_DrawDropDown(IFeature* pFeature, float flWidgetWidth, float flWidgetStartX) const
{
    Feature<DropDown_t>* pDropDownFeature = static_cast<Feature<DropDown_t>*>(pFeature);

    // Feature's name
    ImGui::Text(pDropDownFeature->m_szFeatureDisplayName.c_str()); ImGui::SameLine(flWidgetStartX);

    // Drop Down Feature
    ImGui::SetNextItemWidth(flWidgetWidth);
    ImGui::Combo(
        ("##" + pDropDownFeature->m_szTabName + pDropDownFeature->m_szSectionName + pDropDownFeature->m_szFeatureDisplayName).c_str(), 
        &pDropDownFeature->m_iActiveData, pDropDownFeature->m_data.m_pItems, pDropDownFeature->m_data.m_nItems);
    
    // Tool tip
    if (ImGui::IsItemHovered() == true && pDropDownFeature->m_szToolTip.empty() == false)
        ImGui::SetTooltip(pDropDownFeature->m_szToolTip.c_str());
}


///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
void MenuGUI_t::_DrawColor(IFeature* pFeature, float flFeatureWidth) const
{
    Feature<ColorData_t>* pColorFeature = reinterpret_cast<Feature<ColorData_t>*>(pFeature);

    float flColorPreviewSize = ImGui::GetFrameHeight();
    ImGui::Text(pColorFeature->m_szFeatureDisplayName.c_str()); ImGui::SameLine(flFeatureWidth - flColorPreviewSize);

    ImGui::ColorEdit3(
        ("##" + pColorFeature->m_szTabName + pColorFeature->m_szSectionName + pColorFeature->m_szFeatureDisplayName).c_str(), &pColorFeature->m_Data.r,
        ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoLabel);

    // Tool tip
    if (ImGui::IsItemHovered() == true && pColorFeature->m_szToolTip.empty() == false)
        ImGui::SetTooltip(pColorFeature->m_szToolTip.c_str());
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
