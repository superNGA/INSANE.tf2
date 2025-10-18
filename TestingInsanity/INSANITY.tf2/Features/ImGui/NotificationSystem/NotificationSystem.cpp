#include "NotificationSystem.h"

#include <cstdarg>
#include <cstdio>
#include <string>

#include "../MenuV2/MenuV2.h"
#include "../../../Utility/ConsoleLogging.h"
#include "../../../Extra/math.h"
#include "../../../Resources/Fonts/FontManager.h"
#include "../../../Utility/Profiler/Profiler.h"

// SDK
#include "../../../SDK/class/IVEngineClient.h"


constexpr float ANCHOR_POS_TOLERANCE_IN_PXL      = 2.0f;
constexpr float ANIMATION_RESET_TOLERANCE_IN_PXL = 2.0f;
constexpr float NOTIFICATION_WINDOW_WIDTH        = 600.0f;


///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
NotificationSystem_t::NotificationSystem_t()
{
    m_qNotifications.clear();
}


///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
void NotificationSystem_t::Draw()
{
    PROFILER_RECORD_FUNCTION(EndScene);

    if (Features::Menu::Notification::RenderNotification.IsActive() == false)
        return;

    m_anchorAnim.CalculateAnim();

    _RemoveExpiredNtfsAndAnimate();
    _CalcOrigin();
    _CalcAnchorPos();
    
    _DrawImGui();
}


///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
void NotificationSystem_t::PushBack(const char* msg, ...)
{
    // Get the formatted string.
    va_list args; va_start(args, msg);
    std::string szFormattedString = _FormatString(msg, args);
    va_end(args);

    // push back the new notification
    m_qNotifications.push_back(Notification_t(szFormattedString));

    // Now adjust target anchor.
    _CalcTargetAnchorPos();
}


///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
void NotificationSystem_t::PushFront(const char* msg, ...)
{
    // Get the formatted string.
    va_list args; va_start(args, msg);
    std::string szFormattedString = _FormatString(msg, args);
    va_end(args);

    // push back the new notification
    m_qNotifications.push_front(Notification_t(szFormattedString));

    // Now adjust target anchor.
    _CalcTargetAnchorPos();
}


///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
void NotificationSystem_t::_CalcTargetAnchorPos()
{
    // This will one of the screen's corner.
    ImVec2 vOrigin = m_vOrigin;

    // Now find out new final position for anchor ( so we animate to that position )
    float flNtfCount  = static_cast<float>(m_qNotifications.size());
    float flNewHeight = (flNtfCount * m_flHeightInPxl) + ((flNtfCount + 1.0f) * m_flPaddingInPxl);
    
    // NOTE : at this point vOrigin.y is either 0 or equal to screen height.
    vOrigin.x  = vOrigin.x > 0.0f ? vOrigin.x - NOTIFICATION_WINDOW_WIDTH : NOTIFICATION_WINDOW_WIDTH; // width is fixed.
    vOrigin.y += flNewHeight * m_flGrowthDirection;

    // if difference between the original and new target pos is more than Tolerance ( 2.0f ) 
    // we reset the animation.
    if (fabsf(vOrigin.y - m_vTargetAnchorPos.y) > ANIMATION_RESET_TOLERANCE_IN_PXL)
    {
        m_anchorAnim.Reset();
        m_vLastAnchorAnimated = m_vAnchorAnimated;
        FAIL_LOG("Resetted notification animation");
    }

    m_vTargetAnchorPos = vOrigin;
}


///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
void NotificationSystem_t::_CalcAnchorPos()
{ 
    m_vAnchorAnimated.x = m_vTargetAnchorPos.x;
    m_vAnchorAnimated.y = m_vLastAnchorAnimated.y + (m_vTargetAnchorPos.y - m_vLastAnchorAnimated.y) * m_anchorAnim.GetAnimation();
}


///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
void NotificationSystem_t::_CalcOrigin()
{
    PROFILER_RECORD_FUNCTION(EndScene);

    // Set to top-left by default.
    m_vOrigin.x = 0.0f; m_vOrigin.y = 0.0f;
    m_flGrowthDirection = 1.0f;

    // Set notification origin according to users prefrence
    int iHeight = 0, iWidth = 0; I::iEngine->GetScreenSize(iWidth, iHeight);

    int iUserNtfOriginChoice = Features::Menu::Notification::NotificationOrigin.GetData();
    if (iUserNtfOriginChoice == NotificationOrigin_TopRight)
    {
        m_vOrigin.x = static_cast<float>(iWidth); m_vOrigin.y = 0.0f;
    }
    else if (iUserNtfOriginChoice == NotificationOrigin_BottomLeft)
    {
        m_vOrigin.x = 0.0f; m_vOrigin.y = static_cast<float>(iHeight);
        m_flGrowthDirection = -1.0f;
    }
    else if (iUserNtfOriginChoice == NotificationOrigin_BottomRight)
    {
        m_vOrigin.x = static_cast<float>(iWidth); m_vOrigin.y = static_cast<float>(iHeight);
        m_flGrowthDirection = -1.0f;
    }
}


///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
void NotificationSystem_t::_RemoveExpiredNtfsAndAnimate()
{
    PROFILER_RECORD_FUNCTION(EndScene);

    for (auto it = m_qNotifications.begin(); it != m_qNotifications.end(); )
    {
        auto   now          = std::chrono::high_resolution_clock::now();
        double flNtfLife    = std::chrono::duration_cast<std::chrono::duration<double>>(now - (*it).GetStartTime()).count();
        float  flNtfMaxLife = Features::Menu::Notification::NotificationLifeTime.GetData().m_flVal;

        if (flNtfLife > flNtfMaxLife)
        {
            it = m_qNotifications.erase(it);
            _CalcTargetAnchorPos();
        }
        else
        {
            if ((*it).m_bExiting == false && flNtfLife > flNtfMaxLife - (*it).m_animation.GetCompletionTime())
            {
                (*it).m_bExiting = true;
                (*it).m_animation.Reset();
            }

            (*it).CalculateAnim(); // Animate this shit
            it++;
        }
    }
}


///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
std::string NotificationSystem_t::_FormatString(const char* msg, va_list args)
{
    va_list argsCopy;
    va_copy(argsCopy, args);

    int iSize = vsnprintf(nullptr, 0, msg, argsCopy);
    va_end(argsCopy);

    if (iSize <= 0)
        return "Failed";

    std::string szMessageOut(iSize, '\0');
    vsnprintf(szMessageOut.data(), iSize + 1, msg, args);
    return szMessageOut;
}


///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
void NotificationSystem_t::_DrawImGui()
{
    PROFILER_RECORD_FUNCTION(EndScene);

    ImGui::PushFont(Resources::Fonts::JetBrainsMonoNerd_Small);

    // Setup coordinates & size.
    ImVec2 vMin(Maths::MIN<float>(m_vOrigin.x, m_vAnchorAnimated.x), Maths::MIN<float>(m_vOrigin.y, m_vAnchorAnimated.y));
    ImVec2 vMax(Maths::MAX<float>(m_vOrigin.x, m_vAnchorAnimated.x), Maths::MAX<float>(m_vOrigin.y, m_vAnchorAnimated.y));

    ImVec2 vWindowSize(vMax.x - vMin.x, vMax.y - vMin.y);

    ImGui::SetNextWindowPos(vMin);
    ImGui::SetNextWindowSize(vWindowSize);

    // Styling
    {
        ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.0f, 0.0f, 0.0f, 0.0f));
        ImGui::PushStyleColor(ImGuiCol_Border,   ImVec4(0.0f, 0.0f, 0.0f, 0.0f));
    }

    // Start drawing whatever's in the notification queue.
    ImGuiWindowFlags iWindowFlags = ImGuiWindowFlags_NoNav | ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoInputs;
    if (ImGui::Begin("##NotificationPanel", nullptr, iWindowFlags) == true)
    {
        ImVec2 vNotificationSize(0.0f, m_flHeightInPxl); // x depends on notification's message.
        ImVec2 vCursorPos(m_vOrigin); vCursorPos.y += (m_flGrowthDirection * m_flPaddingInPxl);

        float flAwayFromCenterDirection = m_vOrigin.x > 0.0f ? 1.0f : -1.0f; // Which side is not the screen center.
        float flCharWidth               = ImGui::GetFont()->GetCharAdvance(' ');

        ImDrawList* pDrawList = ImGui::GetWindowDrawList();
        for (const Notification_t& ntf : m_qNotifications)
        {
            vNotificationSize.x = (ntf.m_szMessage.size() * flCharWidth) + (2.0f * m_flPaddingInPxl);

            // If exitting, play animation in reverse.
            float flAnimation = ntf.m_bExiting == true ? 1.0f - ntf.m_animation.GetAnimation() : ntf.m_animation.GetAnimation();

            ImVec2 vMaxAnimated(
                vCursorPos.x + (vNotificationSize.x * flAwayFromCenterDirection) - (vNotificationSize.x * flAwayFromCenterDirection * flAnimation), 
                vCursorPos.y + (m_flGrowthDirection < 0.0f ? 0.0f : vNotificationSize.y));
            ImVec2 vMinAnimated(
                vMaxAnimated.x + (vNotificationSize.x * flAwayFromCenterDirection * -1.0f), 
                vCursorPos.y + (m_flGrowthDirection < 0.0f ? -1.0f * vNotificationSize.y : 0.0f));

            constexpr float THEME_ACCENT_FINAL_SIZE = 10.0f; // The final width of the theme colored stip at the end of the notification.

            // Adjust flashmin such that when coming in its ahead reaches before the animation and when exiting, it stick to the notification nicely.
            ImVec2 vFlashMin(vMinAnimated.x, vMinAnimated.y);
            vFlashMin.x += ntf.m_bExiting == true ? (THEME_ACCENT_FINAL_SIZE * flAwayFromCenterDirection * -1.0f) : (200.0f * flAwayFromCenterDirection * -1.0f);

            // Clamping the "Flash" or theme accent so that it doens't go out of bound ( 10 pixels in this case ).
            float flFlashMaxX = vCursorPos.x + ((vNotificationSize.x + THEME_ACCENT_FINAL_SIZE) * flAwayFromCenterDirection * -1.0f);

            // When notification is on the right side of the screen, flAwayFromCenterDirection > 0 and whenever flashmin goes more off than
            // 10 pixels from notification size its clamped. But when notification is on the left side of the screen, 
            // flAwayFromCenterDirection < 0 ( negative ) so we going towards the center decreases the values. And whenever 
            // the values go more than 10 pixel offset, its clamped.
            //
            if (vFlashMin.x * flAwayFromCenterDirection < flFlashMaxX * flAwayFromCenterDirection)
                vFlashMin.x = flFlashMaxX;

            // Flash effect ( theme colored shit thats on the far end of the notification )
            pDrawList->AddRectFilled(vFlashMin,    vMaxAnimated, ImColor(Render::menuGUI.GetThemeClr().GetAsImVec4()));

            // Box for text.
            pDrawList->AddRectFilled(vMinAnimated, vMaxAnimated, ImColor(Render::menuGUI.GetPrimaryClr().GetAsImVec4()));

            // Drawing text.
            RGBA_t clrText; Render::menuGUI.CalcTextClrForBg(clrText, Render::menuGUI.GetPrimaryClr());
            ImVec2 vTextPos(
                (flAwayFromCenterDirection > 0.0f ? vMinAnimated.x : vMaxAnimated.x) + m_flPaddingInPxl,
                vMinAnimated.y + (vNotificationSize.y - ImGui::GetTextLineHeight()) / 2.0f
            );
            pDrawList->AddText(vTextPos, ImColor(clrText.GetAsImVec4()), ntf.m_szMessage.c_str());

            // Adjust for next item.
            vCursorPos.y += (m_flHeightInPxl + m_flPaddingInPxl) * m_flGrowthDirection;
        }

        ImGui::End();
    }

    ImGui::PopStyleColor(2);
    ImGui::PopFont();
}