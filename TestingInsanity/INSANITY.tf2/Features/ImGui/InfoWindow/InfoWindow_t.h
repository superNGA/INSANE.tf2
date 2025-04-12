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

#define MAX_INFOWINDOW_TEXT_LENGTH 40
union Data // 40 bytes
{
    int64_t iData;
    double flData;
    char szData[MAX_INFOWINDOW_TEXT_LENGTH];
};

struct DataInfoWindow_t
{
    Data m_uData;
    DataTypeInfoWindow_t dataType = DT_INFO_WINDOW_NOT_DEFINED;
};

class InfoWindow_t
{
public:
    void Draw();
    void AddToInfoWindow(std::string caller, std::string msg);
    
private:
    void _DrawInfoWindow();
    //void _DrawInfoBar(); // <- add this too.

    std::unordered_map <uint32_t, std::string> m_mapAllDataInfoWindow; // stores all the data for info window.
};

ADD_FEATURE_CUSTOM(InfoWindow, InfoWindow_t, Render);