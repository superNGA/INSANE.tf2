#include "InfoWindow.h"

#include "../../../Utility/ConsoleLogging.h"
#include "../MenuV2/MenuV2.h"
#include "../../../External Libraries/ImGui/imgui.h"
#include "../../../Resources/Fonts/FontManager.h"
#include "../../../SDK/class/IVEngineClient.h"
#include "../../../SDK/class/Basic Structures.h"
#include "../../../Utility/Profiler/Profiler.h"


constexpr float flPaddingInPxl   = 2.0f;
constexpr float flDockRangeInPxl = 20.0f; // We will dock the window to the nearest window when it comes this close from its walls.



///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
InfoWindowV2_t::InfoWindowV2_t()
{
    m_umAllWindows.clear();
}


///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
void InfoWindowV2_t::Draw()
{
    PROFILER_RECORD_FUNCTION(EndScene);

    if (Features::Menu::FeatureInfo::FeatureInfo_Render.IsActive() == false)
        return;

    if (I::iEngine->IsInGame() == false)
        return;

    // Styling ( same for all windows. )
    {
        ImGui::PushStyleColor(ImGuiCol_WindowBg, Render::menuGUI.GetPrimaryClr().GetAsImVec4());
        ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
    }

    ImGui::PushFont(Resources::Fonts::JetBrainsMonoNerd_Small);
    constexpr ImGuiWindowFlags iDefaultWindowFlags = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoNav | ImGuiWindowFlags_NoNavInputs;
    for (auto& [szKey, windowInstance] : m_umAllWindows)
    {
        if (windowInstance.m_bVisible == false)
            continue;

        // Window's flags ( can move when menu is open, else static AF )
        ImGuiWindowFlags iWindowFlags = iDefaultWindowFlags;
        if (Render::menuGUI.IsVisible() == true)
        {
            iWindowFlags &= ~ImGuiWindowFlags_NoMove;
        }
        else
        {
            iWindowFlags |= ImGuiWindowFlags_NoMove;
        }

    
        std::string szWindowName("##InfoWindowV2_t" + std::string(szKey));
        if (ImGui::Begin(szWindowName.c_str(), nullptr, iWindowFlags) == true)
        {
            // Window Pos.
            windowInstance.m_vWindowPos = ImGui::GetWindowPos();
            const float flRowHeight     = ImGui::GetTextLineHeight();

            // Window size.
            windowInstance.m_vWindowSize.x = Features::Menu::FeatureInfo::FeatureInfo_WindowWidth.GetData().m_flVal;
            windowInstance.m_vWindowSize.y = (static_cast<float>(windowInstance.m_nRows + 1) * flRowHeight) + ((static_cast<float>(windowInstance.m_nRows + 2) * flPaddingInPxl));
            ImGui::SetWindowSize(windowInstance.m_vWindowSize);

            _DrawWindow(windowInstance, flRowHeight);

            // Check if we can dock this window.
            bool bIsWindowBeingDragged = 
                ImGui::IsMouseDragging(ImGuiMouseButton_Left) && ImGui::IsWindowFocused(ImGuiFocusedFlags_RootAndChildWindows);

            // Since these windows are not movable while menu is closed, we won't even bother checking it while menu is closed.
            if(Render::menuGUI.IsVisible() == true && Features::Menu::FeatureInfo::FeatureInfo_DockWindows.IsActive() == true)
            {
                if (bIsWindowBeingDragged == false && windowInstance.m_bDraggedLastFrame == true)
                {
                    _CheckWindowDocking(windowInstance);
                }
            }
            windowInstance.m_bDraggedLastFrame = bIsWindowBeingDragged;

            ImGui::End();
        }
    }
    ImGui::PopFont();

    ImGui::PopStyleColor(); ImGui::PopStyleVar();
}


///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
void InfoWindowV2_t::AddOrUpdate(std::string szKey, std::string&& szMessage, int iRow, int iAlignment)
{
    RGBA_t clrText; Render::menuGUI.CalcTextClrForBg(clrText, Render::menuGUI.GetPrimaryClr());

    // Not that we must pass in a refrence to prvalue, that is temporary value n stuff like that. 
    // Thats why I had to do this std::string(szMessage) shit.
    AddOrUpdate(szKey, std::string(szMessage), iRow, iAlignment, clrText);
}


///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
void InfoWindowV2_t::_DrawWindow(InfoWindowInstance_t& window, float flRowHeight)
{
    PROFILER_RECORD_FUNCTION(EndScene);

    ImDrawList* pDrawList = ImGui::GetWindowDrawList();
    RGBA_t clrText; Render::menuGUI.CalcTextClrForBg(clrText, Render::menuGUI.GetPrimaryClr());


    // Draw slider widgets.
    for (const InfoWindowSliderWidget_t& widget : window.m_vecSliderWidgets)
    {
        ImVec2 vWidgetPos; 
        vWidgetPos.x = window.m_vWindowPos.x + flPaddingInPxl;
        vWidgetPos.y = window.m_vWindowPos.y + (static_cast<float>(widget.m_iRow + 1) * flPaddingInPxl) + (static_cast<float>(widget.m_iRow) * flRowHeight);

        ImVec2 vWidgetSize(window.m_vWindowSize.x - (2.0f * flPaddingInPxl), ImGui::GetTextLineHeight());

        // Putting widget in the center of the row.
        vWidgetPos.y = vWidgetPos.y + (flRowHeight - vWidgetSize.y) / 2.0f;


        pDrawList->AddRectFilled(
            vWidgetPos, ImVec2(vWidgetPos.x + vWidgetSize.x, vWidgetPos.y + vWidgetSize.y),
            ImColor(Render::menuGUI.GetSecondaryClr().GetAsImVec4()), 1000.0f
        );

        float flFillLevel = (widget.m_flVal - widget.m_flMin) / (widget.m_flMax - widget.m_flMin);
        pDrawList->AddRectFilled(
            vWidgetPos, ImVec2(vWidgetPos.x + (vWidgetSize.x * flFillLevel), vWidgetPos.y + vWidgetSize.y),
            ImColor(Render::menuGUI.GetThemeClr().GetAsImVec4()), 1000.0f
        );
    }


    // Draw text widgets.
    float flCharWidth = ImGui::GetFont()->GetCharAdvance(' ');
    for (const InfoWindowWidget_t& widget : window.m_vecWidgets)
    {
        // Calculating widget's X coordinate according to its alignment.
        float flWidgetPosX = 0.0f;
        if (widget.m_iAlignment == InfoWindowWidget_t::Alignment_Left)
        {
            flWidgetPosX = window.m_vWindowPos.x + flPaddingInPxl;
        }
        else if (widget.m_iAlignment == InfoWindowWidget_t::Alignment_Middle)
        {
            flWidgetPosX = window.m_vWindowPos.x + (window.m_vWindowSize.x - (widget.m_szMessage.size() * flCharWidth)) / 2.0f;
        }
        else if (widget.m_iAlignment == InfoWindowWidget_t::Alignment_Right)
        {
            flWidgetPosX = window.m_vWindowPos.x + window.m_vWindowSize.x - flPaddingInPxl - (widget.m_szMessage.size() * flCharWidth);
        }

        ImVec2 vWidgetPos(
            flWidgetPosX,
            window.m_vWindowPos.y + (static_cast<float>(widget.m_iRow + 1) * flPaddingInPxl) + (static_cast<float>(widget.m_iRow) * flRowHeight)
        );

        pDrawList->AddText(vWidgetPos, ImColor(widget.m_clrText.GetAsImVec4()), widget.m_szMessage.c_str());
    }
}


///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
void InfoWindowV2_t::_CheckWindowDocking(InfoWindowInstance_t& window)
{
    // Center of this window.
    ImVec2 vWindowCenter(
        window.m_vWindowPos.x + (window.m_vWindowSize.x / 2.0f),
        window.m_vWindowPos.y + (window.m_vWindowSize.y / 2.0f));

    InfoWindowInstance_t* pParentWindow     = nullptr;
    ImVec2                vIdealParentWindowCenter(-1.0f, -1.0f); // To be set in this loop below.
    float                 flBestDistanceSqr = INFINITY;
    
    // NOTE : Iterating all windows and finding one which is most suitable for docking.
    // that window will be called parent.
    for (auto& [szKey, windowInstance] : m_umAllWindows)
    {
        // skip itself.
        if (&windowInstance == &window)
            continue;

        if (windowInstance.m_bVisible == false)
            continue;

        // center of this parent window.
        ImVec2 vParentWindowCenter(
            windowInstance.m_vWindowPos.x + (windowInstance.m_vWindowSize.x / 2.0f),
            windowInstance.m_vWindowPos.y + (windowInstance.m_vWindowSize.y / 2.0f));

        ImVec2 vDelta(vWindowCenter.x - vParentWindowCenter.x, vWindowCenter.y - vParentWindowCenter.y);
        float  flDistanceSqr   = (vDelta.x * vDelta.x) + (vDelta.y * vDelta.y);

        // This will create a rectangular range surrounding the parent window, and if the this window ( i.e. child window ) 
        // is in that range, then it parent window is valid for clamping.
        bool   bInDockingRange =
            fabsf(vDelta.x) <= ((windowInstance.m_vWindowSize.x / 2.0f) + (window.m_vWindowSize.x / 2.0f)) + flDockRangeInPxl &&
            fabsf(vDelta.y) <= ((windowInstance.m_vWindowSize.y / 2.0f) + (window.m_vWindowSize.y / 2.0f)) + flDockRangeInPxl;

        // Get the parent window with the shortest distance from the child window.
        if (bInDockingRange == true && flDistanceSqr < flBestDistanceSqr)
        {
            pParentWindow            = &windowInstance;
            vIdealParentWindowCenter = vParentWindowCenter;
            flBestDistanceSqr        = flDistanceSqr;
        }
    }

    // if parent window is nullptr, then there is no window which is close enough to be docked too.
    if (pParentWindow == nullptr)
        return;


    // Now dock it.
    ImVec2 vParentWindowCenter(vIdealParentWindowCenter); // I didn't like the name. thats all.
    ImVec2 vFinalPos(window.m_vWindowPos);
    
    // if our window is above or below parent window, then stick our window above or below parent window.
    // NOTE : As you can see, we are biasing vertical docking, cause the windows will be long ass rectangles in 9 / 10 cases, so a fair
    //        angle check like (45 to 135 degres) will feel like shit.
    float flThetaAbs      = fabsf(RAD2DEG(atan2f(vWindowCenter.y - vParentWindowCenter.y, vWindowCenter.x - vParentWindowCenter.x)));
    bool  bDoVerticalDock = flThetaAbs >= 20.0f && flThetaAbs <= 160.0f;
    
    // NOTE : window.m_vWindowPos is top left corner and not the center of the window.s
    if (bDoVerticalDock == true)
    {
        vFinalPos.x = pParentWindow->m_vWindowPos.x;

        vFinalPos.y = window.m_vWindowPos.y < pParentWindow->m_vWindowPos.y ?
            pParentWindow->m_vWindowPos.y - window.m_vWindowSize.y :
            pParentWindow->m_vWindowPos.y + pParentWindow->m_vWindowSize.y;
    }
    else
    {
        vFinalPos.y = pParentWindow->m_vWindowPos.y;
        
        vFinalPos.x = window.m_vWindowPos.x < pParentWindow->m_vWindowPos.x ?
            pParentWindow->m_vWindowPos.x - window.m_vWindowSize.x :
            pParentWindow->m_vWindowPos.x + pParentWindow->m_vWindowSize.x;
    }


    // set position and we are good.
    window.m_vWindowPos = vFinalPos;
    ImGui::SetWindowPos(window.m_vWindowPos);
}


///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
void InfoWindowV2_t::AddOrUpdate(std::string szKey, std::string&& szMessage, int iRow, int iAlignment, RGBA_t clrText)
{
    auto it = m_umAllWindows.find(szKey);

    // If not added, then add.
    if (it == m_umAllWindows.end())
    {
        m_umAllWindows.insert({ szKey, InfoWindowInstance_t() });
        it = m_umAllWindows.find(szKey);

        if (it == m_umAllWindows.end())
        {
            FAIL_LOG("Failed to add InfoWindowInstance for key [ %s ]", szKey);
            return;
        }
    }

    InfoWindowInstance_t* pWindowInstance = &it->second;
    pWindowInstance->m_bVisible           = true;

    // NOTE : I am not expecting some crazy ammount of widgets inside one window, so 
    //        I am storing them in an array, and will search linearly. ( should be just fine. )
    InfoWindowWidget_t* pTargetWidget = nullptr;
    for (InfoWindowWidget_t& widget : pWindowInstance->m_vecWidgets)
    {
        if (widget.m_iRow == iRow && widget.m_iAlignment == iAlignment)
            pTargetWidget = &widget;
    }

    // Clamping alignment before storing just to be safe, from any unhinged monkey brained ass caller. ( probably me )
    InfoWindowWidget_t::Alignment_t iAlignmentClamped =
        static_cast<InfoWindowWidget_t::Alignment_t>(std::clamp<int>(iAlignment, InfoWindowWidget_t::Alignment_Left, InfoWindowWidget_t::Alignment_Right));

    // Keep record of the "Height required for this window"
    if (iRow > pWindowInstance->m_nRows)
    {
        pWindowInstance->m_nRows = iRow;
    }

    // Finally add the entry.
    if (pTargetWidget == nullptr)
    {
        pWindowInstance->m_vecWidgets.push_back(InfoWindowWidget_t(szMessage, iRow, iAlignmentClamped, clrText));
    }
    else
    {
        pTargetWidget->m_szMessage  = std::move(szMessage);
        pTargetWidget->m_iRow       = iRow;
        pTargetWidget->m_iAlignment = iAlignmentClamped;
        pTargetWidget->m_clrText    = clrText;
    }
}


///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
void InfoWindowV2_t::AddOrUpdate(std::string szKey, float flVal, float flMin, float flMax, int iRow)
{
    auto it = m_umAllWindows.find(szKey);

    // If not added, then add.
    if (it == m_umAllWindows.end())
    {
        m_umAllWindows.insert({ szKey, InfoWindowInstance_t() });
        it = m_umAllWindows.find(szKey);

        if (it == m_umAllWindows.end())
        {
            FAIL_LOG("Failed to add InfoWindowInstance for key [ %s ]", szKey);
            return;
        }
    }

    InfoWindowInstance_t* pWindowInstance = &it->second;
    pWindowInstance->m_bVisible           = true;

    // NOTE : I am not expecting some crazy ammoung of widgets inside one window, so 
    //        I am storing them in an array, and will search for required entry linearly. ( shouldn't effect performance )
    InfoWindowSliderWidget_t* pTargetWidget = nullptr;
    for (InfoWindowSliderWidget_t& widget : pWindowInstance->m_vecSliderWidgets)
    {
        if (widget.m_iRow == iRow)
            pTargetWidget = &widget;
    }


    // Keep record of the "Height required for this window"
    if (iRow > pWindowInstance->m_nRows)
    {
        pWindowInstance->m_nRows = iRow;
    }

    // Finally add the entry.
    if (pTargetWidget == nullptr)
    {
        pWindowInstance->m_vecSliderWidgets.push_back(InfoWindowSliderWidget_t(iRow, flVal, flMin, flMax));
    }
    else
    {
        pTargetWidget->m_iRow  = iRow;
        pTargetWidget->m_flVal = flVal;
        pTargetWidget->m_flMin = flMin;
        pTargetWidget->m_flMax = flMax;
    }
}


///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
void InfoWindowV2_t::AddOrUpdate(std::string szKey, int iVal, int iMin, int iMax, int iRow)
{
    AddOrUpdate(szKey, static_cast<float>(iVal), static_cast<float>(iMin), static_cast<float>(iMax), iRow);
}


///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
void InfoWindowV2_t::Hide(std::string szKey)
{
    auto it = m_umAllWindows.find(szKey);
    if (it == m_umAllWindows.end())
        return;

    it->second.m_bVisible = false;
}
