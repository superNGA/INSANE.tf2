#pragma once

#include <vector>
#include "../../Features/FeatureHandler.h"

// Fonts
#include "JetBrains Mono NL/JetBrains Mono NL SemiBold.h"
#include "JetBrains Mono NL/JetBrains Mono NL Light.h"

class Font_t;
struct ImFont;


///////////////////////////////////////////////////////////////////////////
class FontManager_t
{
public:
    bool Initialize();
    void RegisterFont(Font_t* pFont);

private:
    bool m_bInitialized = false;
    std::vector<Font_t*> m_vecFonts;
};
///////////////////////////////////////////////////////////////////////////


///////////////////////////////////////////////////////////////////////////
class Font_t
{
public:
    Font_t(const char* szFontName, unsigned char* pFontData, unsigned int iFontDataSize, float flSize, ImFont** pDestination);

    const char*    m_szFontName    = "";
    unsigned char* m_pFontData     = nullptr;
    float          m_flSize        = -1.0f;
    ImFont**       m_pDestination  = nullptr;
    unsigned int   m_iFontDataSize = 0;
};
DECLARE_CUSTOM_OBJECT(fontManager, FontManager_t, Resources::Fonts)
///////////////////////////////////////////////////////////////////////////


////////////////////////////// FONTS ////////////////////////////////////
#define REGISTER_FONT(szFontName, pFontData, flFontSize)\
namespace Resources::Fonts{\
    inline ImFont* szFontName = nullptr;\
}\
namespace Resources::Fonts::Helper{\
    static Font_t szFontName(#szFontName, pFontData, sizeof(pFontData), flFontSize, &Resources::Fonts::szFontName);\
}


////////////////////////////// REGISTERED FONTS ////////////////////////////////////
REGISTER_FONT(JetBrains_SemiBold_NL_Small, Resources::Fonts::jetBrainsMono_SemiBoldNL, 16.0f)
REGISTER_FONT(JetBrains_SemiBold_NL_Mid,   Resources::Fonts::jetBrainsMono_SemiBoldNL, 20.0f)
REGISTER_FONT(JetBrains_Light_NL_MID,      Resources::Fonts::JetBrainsMono_LightNL,    20.0f)