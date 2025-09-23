#pragma once
#include <Windows.h>
#include <string>
#include <type_traits>
#include <vector>
#include <unordered_map>
#include <algorithm>
#include <functional>

#include "../Hooks/EndScene/EndScene.h"
#include "../SDK/class/Basic Structures.h"

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
    ColorData_t(float R, float G, float B, float A)
    {
        r = R / 255.0f;
        g = G / 255.0f;
        b = B / 255.0f;
        a = A / 255.0f;
    }
    ColorData_t(unsigned char R, unsigned char G, unsigned char B, unsigned char A)
    {
        r = static_cast<float>(R) / 255.0f;
        g = static_cast<float>(G) / 255.0f;
        b = static_cast<float>(B) / 255.0f;
        a = static_cast<float>(A) / 255.0f;
    }
    constexpr ColorData_t(RGBA_t clr)
    {
        r = static_cast<float>(clr.r) / 255.0f;
        g = static_cast<float>(clr.g) / 255.0f;
        b = static_cast<float>(clr.b) / 255.0f;
        a = static_cast<float>(clr.a) / 255.0f;
    }
    float r, g, b, a;

    inline RGBA_t GetAsBytes() const
    {
        return RGBA_t(
            static_cast<unsigned char>(std::clamp<float>(r * 255.0f, 0.0f, 255.0f)),
            static_cast<unsigned char>(std::clamp<float>(g * 255.0f, 0.0f, 255.0f)),
            static_cast<unsigned char>(std::clamp<float>(b * 255.0f, 0.0f, 255.0f)),
            static_cast<unsigned char>(std::clamp<float>(a * 255.0f, 0.0f, 255.0f))
        );
    }
};

struct DropDown_t
{
    DropDown_t() : m_pItems(nullptr), m_nItems(0) {}
    DropDown_t(const char** pItems, int nItems) : m_pItems(pItems), m_nItems(nItems) {}

    const char** m_pItems = nullptr;
    int          m_nItems = 0;
};


enum FeatureFlags
{
    FeatureFlag_None                 = 0,
    FeatureFlag_SupportKeyBind       = (1 << 0),
    FeatureFlag_OverrideCompatible   = (1 << 1),
    FeatureFlag_HoldOnlyKeyBind      = (1 << 2),
    FeatureFlag_ToggleOnlyKeyBind    = (1 << 3),
    FeatureFlag_DisableWhileMenuOpen = (1 << 4)
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
        int32_t     iKey,
        int         iFlags = FeatureFlag_None,
        std::string szToolTip = "");

    std::string m_szFeatureDisplayName;
    std::string m_szSectionName;
    std::string m_szTabName;
    int         m_iIndex;
    int         m_iFlags;
    int32_t     m_iKey = 0;
    std::string m_szToolTip = "";

    enum class DataType : int8_t
    {
        DT_INVALID,
        DT_BOOLEAN, DT_INTSLIDER, DT_FLOATSLIDER, DT_COLORDATA,
        DT_DROPDOWN
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
        std::is_same_v<T, std::vector<void*>*>,
        "DataType not supported");
    
    Feature(
        T defaultData,
        std::string szFeatureDisplayName,
        std::string szSectionName,
        std::string szTabName,
        int         iIndex,
        int32_t     iKey,
        int         iFlags,
        std::string szToolTip) :
        IFeature(szFeatureDisplayName, szSectionName, szTabName, iIndex, iKey, iFlags, szToolTip)
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
        else if (std::is_same_v<T, DropDown_t> == true)
        {
            m_iDataType = DataType::DT_DROPDOWN;
        }

        m_Data = defaultData;
    }

    T m_Data;
    T m_OverrideData; 

    inline const T& GetData()
    {
        // if this feature doesn't support key binds OR key not set, then return whatever the user has set it to!
        if (m_iKey == NULL || (m_iFlags & FeatureFlags::FeatureFlag_SupportKeyBind) == false)
            return m_Data;

        short iKeyStatus = GetAsyncKeyState(m_iKey);
        
        constexpr int KEY_HELD_DOWN     = (1 << 15);
        constexpr int KEY_PRESSED_SINCE = (1 << 0);

        if (m_iFlags & FeatureFlags::FeatureFlag_HoldOnlyKeyBind)
        {
            m_bIsOverrideActive = iKeyStatus & KEY_HELD_DOWN;
        }
        else if (m_iFlags & FeatureFlags::FeatureFlag_ToggleOnlyKeyBind)
        {
            if ((iKeyStatus & KEY_PRESSED_SINCE) == true)
                m_bIsOverrideActive = !m_bIsOverrideActive;
        }
        else
        {
            switch (m_iOverrideType)
            {
            case OverrideType::OVERRIDE_TOGGLE:
            {
                if ((iKeyStatus & KEY_PRESSED_SINCE) == true)
                    m_bIsOverrideActive = !m_bIsOverrideActive;
                break;
            }
            case OverrideType::OVERRIDE_HOLD:
            default:
                m_bIsOverrideActive = iKeyStatus & KEY_HELD_DOWN;
                break;
            }
        }

        if (m_bIsOverrideActive == true)
        {
            return m_OverrideData;
        }

        return m_Data;
    }
};

template <>
class Feature<bool> : public IFeature
{
public:
    Feature(
        bool        defaultData,
        std::string szFeatureDisplayName,
        std::string szSectionName,
        std::string szTabName,
        int         iIndex,
        int64_t     iKey,
        int         iFlags,
        std::string szToolTip) :
        IFeature(szFeatureDisplayName, szSectionName, szTabName, iIndex, iKey, iFlags, szToolTip)
    {
        m_iDataType = DataType::DT_BOOLEAN;
        m_Data     = defaultData;
    }

    bool m_Data;

    inline bool IsDisabled()
    {
        return (m_iKey == NULL && m_Data == false);
    }

    inline bool IsActive()
    {
        // if No key is set, then either run always, or don't run
        if (m_iKey == NULL || (m_iFlags & FeatureFlags::FeatureFlag_SupportKeyBind) == false)
            return m_Data;

        // Get key state
        int16_t iKeyStatus = GetAsyncKeyState(m_iKey);

        constexpr int KEY_HELD_DOWN     = (1 << 15);
        constexpr int KEY_PRESSED_SINCE = (1 << 0);

        if (m_iFlags & FeatureFlags::FeatureFlag_HoldOnlyKeyBind)
        {
            m_bIsOverrideActive = iKeyStatus & KEY_HELD_DOWN;
        }
        else if (m_iFlags & FeatureFlags::FeatureFlag_ToggleOnlyKeyBind)
        {
            if ((iKeyStatus & KEY_PRESSED_SINCE) == true)
                m_bIsOverrideActive = !m_bIsOverrideActive;
        }
        else
        {
            switch (m_iOverrideType)
            {
            case OverrideType::OVERRIDE_TOGGLE:
            {
                if ((iKeyStatus & KEY_PRESSED_SINCE) == true)
                    m_bIsOverrideActive = !m_bIsOverrideActive;
                break;
            }
            case OverrideType::OVERRIDE_HOLD:
            default:
                m_bIsOverrideActive = iKeyStatus & KEY_HELD_DOWN;
                break;
            }
        }

        m_Data = m_bIsOverrideActive;
        
        // Some feature like Rocket jumping shouldn't run while the menu is open. 
        // So this flag will return false when menu is open for such features. 
        // NOTE : PLACED IN THE END SO THAT m_Data GETS UPDATED cause IT WILL REFELCTED IN THE MENU!
        if (IsMenuOpen() == true && (m_iFlags & FeatureFlags::FeatureFlag_DisableWhileMenuOpen) == true)
            return false;
        
        return m_Data;
    }
};


template<>
class Feature<DropDown_t> : public IFeature
{
public:
    Feature(
        DropDown_t  data,
        std::string szFeatureDisplayName,
        std::string szSectionName,
        std::string szTabName,
        int         iIndex,
        int64_t     iKey,
        int         iFlags,
        std::string szToolTip) :
        IFeature(szFeatureDisplayName, szSectionName, szTabName, iIndex, iKey, iFlags, szToolTip)
    {
        m_iDataType = DataType::DT_DROPDOWN;
        m_data      = data;
    }

    DropDown_t m_data;
    int        m_iActiveData = -1;

    inline void SetItems(const char** pItems, int iSize)
    {
        m_data.m_pItems = pItems; 
        m_data.m_nItems = iSize;
    }

    inline int GetData()
    {
        return m_iActiveData;
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
#define DECLARE_FEATURE_OBJECT(szName, Type) namespace F{ inline Type szName; }
#define DECLARE_CUSTOM_OBJECT(szName, Type, spaceName) namespace spaceName{ inline Type szName; }

#define DEFINE_TAB(DisplayName, index) namespace Tabs{inline Tab_t DisplayName(#DisplayName, index); }
#define DEFINE_SECTION(DisplayName, TabName, index) namespace Sections{inline Section_t DisplayName(#DisplayName, TabName, index); }

#define DEFINE_FEATURE_NOFLAG(DisplayName, type, SectionName, TabName, index, defaultData)\
namespace Features{\
    namespace TabName{\
        namespace SectionName{\
            inline Feature<type> DisplayName(defaultData, #DisplayName, \
                #SectionName, #TabName, index, 0, FeatureFlags::FeatureFlag_None , "");\
        }\
    }\
}

#define DEFINE_FEATURE_FLAG(DisplayName, type, SectionName, TabName, index, defaultData, Flags)\
namespace Features{\
    namespace TabName{\
        namespace SectionName{\
            inline Feature<type> DisplayName(defaultData, #DisplayName, \
                #SectionName, #TabName, index, 0, Flags, "");\
        }\
    }\
}

#define DEFINE_FEATURE_TOOLTIP(DisplayName, type, SectionName, TabName, index, defaultData, Flags, szToolTip)\
namespace Features{\
    namespace TabName{\
        namespace SectionName{\
            inline Feature<type> DisplayName(defaultData, #DisplayName, \
                #SectionName, #TabName, index, 0, Flags, szToolTip);\
        }\
    }\
}

#define GET_9TH_ARGUMENT(arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8, arg9, ...) arg9

#define EXPAND(x) x
// A Macro won't expand if its the result / output of another macro unless its being passed in as argument into a Macro ?!
// (DisplayName, type, SectionName, TabName, index, defaultData, Flags, szToolTip)
#define DEFINE_FEATURE(...)\
        EXPAND(GET_9TH_ARGUMENT(__VA_ARGS__, DEFINE_FEATURE_TOOLTIP, DEFINE_FEATURE_FLAG, DEFINE_FEATURE_NOFLAG)(__VA_ARGS__))

class FeatureHandler_t
{
public:
    FeatureHandler_t();
    bool Initialize();
    void RegisterTab(Tab_t* pTab);
    void RegisterSection(Section_t* pSection);
    void RegisterFeature(IFeature* pFeature);

    const std::vector<Tab_t*>& GetFeatureMap() { return m_vecFeatureMap; }
    
    const std::unordered_map<uint64_t, IFeature*>& GetConfigLinkerMap() { return m_mapFeatureToConfigLinkerMap; }
    
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

    struct FeaturePathHash_t
    {
        FeaturePathHash_t() :
            m_iTabNameHash(0), m_iSectionNameHash(0), m_iFeatureNameHash(0) { }
        uint16_t m_iTabNameHash;
        uint16_t m_iSectionNameHash;
        uint32_t m_iFeatureNameHash;
    };
    std::unordered_map<uint64_t, IFeature*> m_mapFeatureToConfigLinkerMap = {};
    bool _ConstructFeatureToConfigLinkerMap();
};

DECLARE_CUSTOM_OBJECT(featureHandler, FeatureHandler_t, Config)