#pragma once

#include <chrono>
#include <deque>
#include <string>

#include "../../FeatureHandler.h"
#include "../../../External Libraries/ImGui/imgui.h"
#include "../AnimationHandler.h"


///////////////////////////////////////////////////////////////////////////
class Notification_t
{
public:
    Notification_t(std::string szMessage = "Message not set!!!"):
        m_startTime(std::chrono::high_resolution_clock::now()),
        m_szMessage(std::move(szMessage)), // std::move() so no copy happens
        m_animation()
    {}

    inline std::chrono::high_resolution_clock::time_point GetStartTime() const { return m_startTime;          }
    inline void                                           CalculateAnim()      { m_animation.CalculateAnim(); }

    std::string        m_szMessage;
    AnimationHandler_t m_animation;
    bool               m_bExiting = false;

private:
    std::chrono::high_resolution_clock::time_point m_startTime;
};
///////////////////////////////////////////////////////////////////////////


///////////////////////////////////////////////////////////////////////////
class NotificationSystem_t
{
public:
    NotificationSystem_t();

    void Draw();

    // new notification
    void PushBack (const char* msg, ...);
    void PushFront(const char* msg, ...);

private:
    // m_vTargetAnchorPos : This is the Max coordinate of the notification window, it grows and shrinks whenever we added a new notification
    //                      or an existing notification expires. Both x and y coordinates are maintainted each frame, but x's magnitude is fixed. ( width )
    ImVec2                      m_vTargetAnchorPos;
    ImVec2                      m_vAnchorAnimated;
    ImVec2                      m_vLastAnchorAnimated;

    // m_vOrigin : screen corner's coordinates where the user wishes to render the notification.
    ImVec2                      m_vOrigin;

    float                       m_flGrowthDirection = 1.0f;
    std::deque<Notification_t>  m_qNotifications;

    AnimationHandler_t m_anchorAnim;
    void _CalcTargetAnchorPos();
    void _CalcAnchorPos();
    void _CalcOrigin();
    enum NotificationOrigin_t : int 
    {
        NotificationOrigin_TopLeft = 0, NotificationOrigin_TopRight, 
        NotificationOrigin_BottomLeft,  NotificationOrigin_BottomRight
    };

    
    // Notification dimensions.
    const float m_flPaddingInPxl =  5.0f;
    const float m_flHeightInPxl  = 30.0f;
    void _RemoveExpiredNtfsAndAnimate();

    void _DrawImGui();

    std::string _FormatString(const char* msg, va_list args);
};
///////////////////////////////////////////////////////////////////////////

DECLARE_CUSTOM_OBJECT(notificationSystem, NotificationSystem_t, Render)
DEFINE_SECTION(Notification, "Menu", 5)

DEFINE_FEATURE(RenderNotification,   "Enable", bool,          Notification, Menu, 1, true)
DEFINE_FEATURE(NotificationLifeTime, "Life",   FloatSlider_t, Notification, Menu, 2, FloatSlider_t(3.0f, 1.0f, 10.0f))

static const char* g_szNotificationOriginSettings[] = { "Top-Left", "Top-Right", "Bottom-Left", "Bottom-Right" };
DEFINE_FEATURE(NotificationOrigin, "Origin", DropDown_t, Notification, Menu, 3, DropDown_t(g_szNotificationOriginSettings, 4))
DEFINE_FEATURE(NotificationMax,    "Max Notifications", IntSlider_t, Notification, Menu, 4, IntSlider_t(30, 1, 30))