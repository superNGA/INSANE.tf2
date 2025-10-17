#pragma once

#include <unordered_map>
#include <vector>
#include <string>

#include "../../FeatureHandler.h"
#include "../../../External Libraries/ImGui/imgui.h"


struct InfoWindowWidget_t;
struct InfoWindowSliderWidget_t;
struct InfoWindowInstance_t;
class  InfoWindowV2_t;

/*
CONTEXT : 
    This is a multi-windowed, ImGui based, imformation display mechanism. 
    This is retention mode. This works using a grid based mechanism. 
    Every entry has a row x coloum ( alignment ), that works as x and y. ( int )
    coloum ( alignment ) must be either:
        0 -> Left   aligned
        1 -> Middle aligned
        2 -> Right  aligned
*/


///////////////////////////////////////////////////////////////////////////
class InfoWindowV2_t
{
public:
    InfoWindowV2_t();

    void Draw();
    void AddOrUpdate(std::string szKey, std::string&& szMessage, int iRow, int iAlignment, RGBA_t clrText);
    
    // This is just a wrapper for "AddOrUpdate", this will calculate the most visible text color for you.
    void AddOrUpdate(std::string szKey, std::string&& szMessage, int iRow, int iAlignment);
    void AddOrUpdate(std::string szKey, float flVal, float flMin, float flMax, int iRow);
    void AddOrUpdate(std::string szKey, int iVal,    int iMin,    int iMax,    int iRow);

private:
    void _DrawWindow(InfoWindowInstance_t& window, float flRowHeight);
    void _CheckWindowDocking(InfoWindowInstance_t& window);

    std::unordered_map<std::string, InfoWindowInstance_t> m_umAllWindows = {};

};
///////////////////////////////////////////////////////////////////////////


///////////////////////////////////////////////////////////////////////////
struct InfoWindowWidget_t
{
    enum Alignment_t : int
    {
        Alignment_Left = 0, Alignment_Middle, Alignment_Right
    };

    InfoWindowWidget_t(std::string& szMessage, int iHeight, Alignment_t iAlignment, RGBA_t clrText)
    {
        m_iRow = iHeight; m_iAlignment = iAlignment;
        m_szMessage = std::move(szMessage);
        m_clrText   = clrText;
    }

    Alignment_t m_iAlignment = Alignment_Left;
    int         m_iRow       = 0;
    std::string m_szMessage  = "";
    RGBA_t      m_clrText;
};
///////////////////////////////////////////////////////////////////////////


///////////////////////////////////////////////////////////////////////////
struct InfoWindowSliderWidget_t
{
    InfoWindowSliderWidget_t(int iRow = 0, float flVal = 0.0f, float flMin = 0.0f, float flMax = 0.0f)
    {
        m_iRow  = iRow; 
        m_flVal = flVal; 
        m_flMin = flMin; 
        m_flMax = flMax;
    }

    int    m_iRow  = 0; 
    float  m_flVal = 0.0f, m_flMin = 0.0f, m_flMax = 0.0f;
};
///////////////////////////////////////////////////////////////////////////


///////////////////////////////////////////////////////////////////////////
struct InfoWindowInstance_t
{
    InfoWindowInstance_t()
    {
        m_nRows = 0; m_vecWidgets.clear();
        m_vWindowPos.x  = 0.0f; m_vWindowPos.y  = 0.0f;
        m_vWindowSize.x = 0.0f; m_vWindowSize.y = 0.0f;
    }
           
    int    m_nRows             = 0;
    bool   m_bDraggedLastFrame = false; // Were we dragging this window last frame?
    ImVec2 m_vWindowPos;
    ImVec2 m_vWindowSize;
    std::vector<InfoWindowWidget_t>       m_vecWidgets;
    std::vector<InfoWindowSliderWidget_t> m_vecSliderWidgets;
};
///////////////////////////////////////////////////////////////////////////



DECLARE_CUSTOM_OBJECT(infoWindowV2, InfoWindowV2_t, Render)
DEFINE_SECTION(FeatureInfo, "Menu", 7)

DEFINE_FEATURE(FeatureInfo_Render,      "Enable", bool, FeatureInfo, Menu, 1, true)
DEFINE_FEATURE(FeatureInfo_WindowWidth, "Width",  FloatSlider_t, FeatureInfo, Menu, 2, FloatSlider_t(250.0f, 100.0f, 400.0f))
DEFINE_FEATURE(FeatureInfo_DockWindows, "Enable docking", bool, FeatureInfo, Menu, 3, true)