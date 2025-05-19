#pragma once
#include <Windows.h>
#include <string>
#include <type_traits>
#include <vector>
#include <unordered_map>

// DATA TYPES
struct IntSlider_t 
{ 
    IntSlider_t() : m_iVal(0), m_iMin(0), m_iMax(0) {}
    IntSlider_t(int iVal, int iMin, int iMax) : m_iVal(iVal), m_iMin(iMin), m_iMax(iMax) {}
    int32_t m_iVal, m_iMin, m_iMax;
};

struct FloatSlider_t
{
    FloatSlider_t() : m_flVal(0.0f), m_flMin(0.0f), m_flMax(0.0f) {}
    FloatSlider_t(float flVal, float flMin, float flMax) : m_flVal(flVal), m_flMin(flMin), m_flMax(flMax) {}
    float m_flVal, m_flMin, m_flMax;
};

struct ColorData_t
{
    ColorData_t() : r(0.0f), g(0.0f), b(0.0f), a(0.0f){}
    ColorData_t(float R, float G, float B, float A) : r(R), g(G), b(B), a(A){}
    float r, g, b, a;
};

enum FeatureFlags
{
    FeatureFlag_None               = 0,
    FeatureFlag_SupportKeyBind     = (1 << 0),
    FeatureFlag_OverrideCompatible = (1 << 1)
};

// Feature class
class IFeature
{
public:
    IFeature(
        std::string szFeatureDisplayName, 
        std::string szSectionName,
        std::string szTabName, 
        int         iIndex,
        int64_t     iKey,
        int         iFlags = FeatureFlag_None);

    std::string m_szFeatureDisplayName;
    std::string m_szSectionName;
    std::string m_szTabName;
    int         m_iIndex;
    int         m_iFlags;
    int64_t     m_iKey = 0;

    enum class DataType : int8_t
    {
        DT_INVALID,
        DT_BOOLEAN, DT_INTSLIDER, DT_FLOATSLIDER, DT_COLORDATA
    };
    DataType m_iDataType = DataType::DT_INVALID;

    enum class OverrideType : int8_t
    {
        OVERRIDE_HOLD = 0, OVERRIDE_TOGGLE = 1
    };
    OverrideType m_iOverrideType = OverrideType::OVERRIDE_HOLD;

    bool m_bIsOverrideActive = false;
};

template <typename T>
class Feature : public IFeature
{
public:
    // Restricting data types to selected few ones
    static_assert(
        std::is_same_v<T, IntSlider_t>   ||
        std::is_same_v<T, FloatSlider_t> ||
        std::is_same_v<T, ColorData_t>   ||
        std::is_same_v<T, bool>,
        "DataType not supported");
    
    Feature(
        T defaultData,
        std::string szFeatureDisplayName,
        std::string szSectionName,
        std::string szTabName,
        int         iIndex,
        int64_t     iKey,
        int         iFlags) :
        IFeature(szFeatureDisplayName, szSectionName, szTabName, iIndex, iKey, iFlags)
    {
        if (std::is_same_v<T, bool> == true)
        {
            m_iDataType = DataType::DT_BOOLEAN;
        }
        else if (std::is_same_v<T, IntSlider_t> == true)
        {
            m_iDataType = DataType::DT_INTSLIDER;
        }
        else if (std::is_same_v<T, FloatSlider_t> == true)
        {
            m_iDataType = DataType::DT_FLOATSLIDER;
        }
        else if (std::is_same_v<T, ColorData_t> == true)
        {
            m_iDataType = DataType::DT_COLORDATA;
        }

        m_Data = defaultData;
    }

    T m_Data;
    T m_OverrideData;

    inline T& GetData()
    {
        short iKeyStatus = GetAsyncKeyState(m_iKey);
        
        constexpr int KEY_HELD_DOWN     = (1 << 15);
        constexpr int KEY_PRESSED_SINCE = (1 << 0);

        switch (m_iOverrideType)
        {
        case OverrideType::OVERRIDE_TOGGLE:
            m_bIsOverrideActive = iKeyStatus & KEY_PRESSED_SINCE;
            break;
        case OverrideType::OVERRIDE_HOLD:
        default:
            m_bIsOverrideActive = iKeyStatus & KEY_HELD_DOWN;
            break;
        }

        if (m_bIsOverrideActive == true)
            return m_OverrideData;

        return m_Data;
    }
};

// Section class
class Section_t
{
public:
    Section_t(std::string szSectionDisplayName, std::string szTabName, int iIndex);
    void DumpNSort();
    
    std::string m_szSectionDisplayName;
    std::string m_szTabName;
    int         m_iIndex;

    std::vector<IFeature*> m_vecFeatures = {};
};

// TAB class
class Tab_t
{
public:
    Tab_t(std::string szTabDisplayName, int iIndex);

    std::string m_szTabDisplayName;
    int         m_iIndex;

    void DumpNSort();

    std::unordered_map <std::string, Section_t*> m_mapSections = {};
    std::vector<Section_t*>                      m_vecSections = {};
};

// MACROS
#define DECLARE_FEATURE_OBJECT(szName, Type) namespace FeatureObj{ inline Type szName; }
#define DECLARE_CUSTOM_OBJECT(szName, Type, spaceName) namespace spaceName{ inline Type szName; }

#define DEFINE_TAB(DisplayName, index) namespace Tabs{inline Tab_t DisplayName(#DisplayName, index); }
#define DEFINE_SECTION(DisplayName, TabName, index) namespace Sections{inline Section_t DisplayName(#DisplayName, TabName, index); }

#define DEFINE_FEATURE(DisplayName, type, SectionName, TabName, index, defaultData, Flags)\
namespace TempFeatureHelper{\
    inline Feature<type> DisplayName(defaultData, #DisplayName, \
                SectionName, TabName, index, 0, Flags);\
}\
namespace Features{\
    inline type& DisplayName = TempFeatureHelper::DisplayName.m_Data;\
}

class FeatureHandler_t
{
public:
    FeatureHandler_t();
    bool Initialize();
    void RegisterTab(Tab_t* pTab);
    void RegisterSection(Section_t* pSection);
    void RegisterFeature(IFeature* pFeature);

    const std::vector<Tab_t*>& GetFeatureMap() { return m_vecFeatureMap; }
    
private:
    std::vector<Tab_t*>     m_vecRegisteredTabs     = {};
    std::vector<Section_t*> m_vecRegisteredSections = {};
    std::vector<IFeature*>  m_vecRegisteredFeatures = {};

    bool m_bTabRegisterationFailed     = false;
    bool m_bSectionRegisterationFailed = false;
    bool m_bFeatureRegisterationFailed = false;

    void DumpNSort();
    std::unordered_map <std::string, Tab_t*> m_mapFeatureMap = {};
    std::vector<Tab_t*>                      m_vecFeatureMap = {};
};
inline FeatureHandler_t featureHandler;