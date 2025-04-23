#pragma once
#include <string>
#include <type_traits>
#include <assert.h>
#include <vector>
#include <unordered_map>

#define DEBUG_FEATURE_HANDLER

#define FEATURE_NAME_CONNECTOR "->"

constexpr const int ERROR_BOUND = 10;

class IFeature_t;
class AllFeatures_t;

struct IN_Color { float r, g, b, a; };
struct IN_FloatSlider { float m_data, m_min, m_max; };
struct IN_IntegerSlider { int m_data, m_min, m_max; };

// This allows us to only allow template of specific predefined types.
template <typename T> 
struct IsValidType : std::false_type {};

// Allowed types. boolean, int, float & colors.
template<> struct IsValidType<bool>              : std::true_type {};
template<> struct IsValidType<IN_IntegerSlider>  : std::true_type {};
template<> struct IsValidType<IN_FloatSlider>    : std::true_type {};
template<> struct IsValidType<IN_Color>          : std::true_type {};


//=========================================================================
//                     ALL FEATURE HANDLER
//=========================================================================
class AllFeatures_t
{
public:
    bool Initialize();
    void AddFeature(IFeature_t* pFeature) { m_vecAllFeatures.push_back(pFeature); }
    
    std::unordered_map<std::string, IFeature_t*> m_umBaseFeatures = {};

private:
    bool        _SortFeatures();
    std::string _GetBaseClass(std::string szInput);
    bool        _TrimBaseClass(std::string& szInput);
    uint32_t    _GetDepth(const std::string& szPath);
    std::string _GetPathTillDepth(const uint32_t iDepthWanted, const std::string& szPath);

    
    std::vector<IFeature_t*> m_vecAllFeatures = {};
};
inline AllFeatures_t allFeatures;


//=========================================================================
//                     CHILD FEATURE CLASS
//=========================================================================
enum FeatureType_t
{
    FEATURE_UNKNOWN = -1,
    FEATURE_BOOL = 0,
    FEATURE_FLOAT, 
    FEATURE_INT,
    FEATURE_COLOR
};

class IFeature_t
{
public:
    IFeature_t(std::string szPath, uint32_t iRenderID)
    {
        m_szPath = szPath;
        m_iRenderID = iRenderID;
    }
    
    FeatureType_t m_eFeatureType = FEATURE_UNKNOWN;
    std::string              m_szPath = "NULL";
    uint32_t                 m_iRenderID = 0;
    std::vector<IFeature_t*> m_vecChildFeature = {};
};

template <typename T>
class Feature_t : public IFeature_t
{
public:
    static_assert(IsValidType<T>::value , "Features of given type is not supported!");
    Feature_t(std::string szPath, uint32_t iRenderID, FeatureType_t eFeatureType, T data) : 
        IFeature_t(szPath, iRenderID)
    {
        m_data = data;
        m_eFeatureType = eFeatureType;
        allFeatures.AddFeature(this);
    }
    T m_data;
};


#define ADD_FEATURE_CUSTOM(name, type, spaceName) namespace spaceName {inline type name;}
#define ADD_FEATURE(name, type) namespace Features{inline type name;}


//=========================================================================
//                     MACROS :)
//=========================================================================

// BOOLEAN
#define MAKE_FEATURE_BOOL(name, path, renderID) \
namespace RenderFeature{\
    inline Feature_t<bool> UI_##name(path, renderID, FeatureType_t::FEATURE_BOOL ,false);\
}\
namespace Feature{\
    inline bool& name = RenderFeature::UI_##name.m_data;\
}

// INT SLIDER
#define MAKE_FEATURE_INTEGER(name, path, renderID, min, max)\
namespace RenderFeature{\
    inline Feature_t<IN_IntegerSlider> UI_##name(path, renderID, FeatureType_t::FEATURE_INT, IN_IntegerSlider{0.0f, min, max});\
}\
namespace Feature{\
    inline int& name = RenderFeature::UI_##name.m_data.m_data;\
}

// FLOAT SLIDER
#define MAKE_FEATURE_FLOAT(name, path, renderID, min, max)\
namespace RenderFeature{\
    inline Feature_t<IN_FloatSlider> UI_##name(path, renderID, FeatureType_t::FEATURE_FLOAT, IN_FloatSlider{0.0f, min, max});\
}\
namespace Feature{\
    inline float& name = RenderFeature::UI_##name.m_data.m_data;\
}

// COLOR
#define MAKE_FEATURE_COLOR(name, path, renderID)\
namespace RenderFeature{\
    inline Feature_t<IN_Color> UI_##name(path, renderID, FeatureType_t::FEATURE_COLOR, IN_Color{0.0f, 0.0f, 0.0f, 0.0f});\
}\
namespace Feature{\
    inline IN_Color& name = RenderFeature::UI_##name.m_data;\
}