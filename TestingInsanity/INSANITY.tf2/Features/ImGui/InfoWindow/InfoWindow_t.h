#pragma once
#include <string>
#include <unordered_map>
#include "../../features.h"

enum DataTypeInfoWindow_t
{
    DT_INFO_WINDOW_NOT_DEFINED = -1,
    DT_INFOWINDOW_INT =0,
    DT_INFOWINDOW_FLOAT,
    DT_INFOWINDOW_STRING
};

enum TextMsgClr_t
{
    DEFAULT = -1,
    RED, BLUE, YELLOW, GREEN
};

struct TextMsg_t
{
    TextMsgClr_t m_clr = DEFAULT;
    std::string m_sTextMsg;
};

struct ImVec4;

class InfoWindow_t
{
public:
    void Draw();
    void AddToInfoWindow(std::string caller, std::string msg);
    void AddToCenterConsole(std::string caller, std::string msg, TextMsgClr_t clr = DEFAULT);
    
private:
    void _DrawInfoWindow();
    void _DrawCenterWindow();
    const ImVec4& _GetTextClr(TextMsgClr_t clr);
    //void _DrawInfoBar(); // <- add this too.

    std::unordered_map <uint32_t, std::string> m_mapAllDataInfoWindow; // stores all the data for info window.
    std::unordered_map <uint32_t, TextMsg_t> m_mapAllDataCenterWindow; // stores all the data for info window.
};

ADD_FEATURE_CUSTOM(InfoWindow, InfoWindow_t, Render);