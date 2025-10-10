#pragma once

#include <vector>
#include "../../Features/FeatureHandler.h"

// Fonts
#include "JetBrains Mono NL/JetBrains Mono NL SemiBold.h"
#include "JetBrains Mono NL/JetBrains Mono NL Light.h"
#include "JetBrains Mono Nerd/JetBrainsMonoNerd.h"
#include "ShadowIntoLight/ShadowIntoLight.h"
#include "Montserrat/MontserratBlack.h"

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
    Font_t(const char* szFontName, unsigned char* pFontData, unsigned int iFontDataSize, float flSize, ImFont** pDestination, bool bNerdFont = false);

    const char*    m_szFontName    = "";
    unsigned char* m_pFontData     = nullptr;
    float          m_flSize        = -1.0f;
    ImFont**       m_pDestination  = nullptr;
    unsigned int   m_iFontDataSize = 0;
    bool           m_bNerdFont     = false;
};
DECLARE_CUSTOM_OBJECT(fontManager, FontManager_t, Resources::Fonts)
///////////////////////////////////////////////////////////////////////////


////////////////////////////// FONTS ////////////////////////////////////
#define REGISTER_FONT(szFontName, pFontData, flFontSize, bNerdFont)\
namespace Resources::Fonts{\
    inline ImFont* szFontName = nullptr;\
}\
namespace Resources::Fonts::Helper{\
    static Font_t szFontName(#szFontName, pFontData, sizeof(pFontData), flFontSize, &Resources::Fonts::szFontName, bNerdFont);\
}


////////////////////////////// REGISTERED FONTS ////////////////////////////////////
REGISTER_FONT(JetBrains_SemiBold_NL_Small, Resources::Fonts::jetBrainsMono_SemiBoldNL, 16.0f, false)
REGISTER_FONT(JetBrains_SemiBold_NL_Mid,   Resources::Fonts::jetBrainsMono_SemiBoldNL, 20.0f, false)
REGISTER_FONT(JetBrains_Light_NL_MID,      Resources::Fonts::JetBrainsMono_LightNL,    20.0f, false)

REGISTER_FONT(MontserratBlack,             Resources::Fonts::MontserratBlack_Data,     31.0f, false)
REGISTER_FONT(ShadowIntoLight,             Resources::Fonts::ShadowIntoLight_Data,     30.0f, false)

REGISTER_FONT(JetBrainsMonoNerd_Small,     Resources::Fonts::jetBrainsMonoNerd_Data,   16.0f, true)
REGISTER_FONT(JetBrainsMonoNerd_Mid,       Resources::Fonts::jetBrainsMonoNerd_Data,   22.0f, true)
REGISTER_FONT(JetBrainsMonoNerd_Large,     Resources::Fonts::jetBrainsMonoNerd_Data,   40.0f, true)
