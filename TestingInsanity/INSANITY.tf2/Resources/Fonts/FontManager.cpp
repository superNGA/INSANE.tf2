#include "FontManager.h"
#include "../../Utility/ConsoleLogging.h"

#include "../../External Libraries/ImGui/imgui.h"

///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
bool FontManager_t::Initialize()
{
    if (m_bInitialized == true)
        return true;

    ImFontConfig fontConfig;
    fontConfig.FontDataOwnedByAtlas = false;

    // Registering each font.
    ImGuiIO io = ImGui::GetIO();
    for (const Font_t* pFont : m_vecFonts)
    {
        *pFont->m_pDestination = io.Fonts->AddFontFromMemoryTTF(pFont->m_pFontData, pFont->m_iFontDataSize, pFont->m_flSize, &fontConfig);

        if (*pFont->m_pDestination == nullptr)
        {
            FAIL_LOG("Failed to load font \"%s\"", pFont->m_szFontName);
            return false;
        }

        WIN_LOG("Initialized font ( %s ) size : %.2f", pFont->m_szFontName, pFont->m_flSize);
    }

    ImGui_ImplDX9_InvalidateDeviceObjects();
    ImGui_ImplDX9_CreateDeviceObjects();

    m_bInitialized = true;

    WIN_LOG("All fonts initialized succesfully");
    return true;
}


///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
void FontManager_t::RegisterFont(Font_t* pFont)
{
    // No repeating font name, even with different sizes of same font, change the fucking name.
    for (const Font_t* pRegisteredFont : m_vecFonts)
    {
        if (std::string(pRegisteredFont->m_szFontName) == std::string(pFont->m_szFontName))
        {
            FAIL_LOG("Font \"%s\" is repeating", pFont->m_szFontName);
            return;
        }
    }

    m_vecFonts.push_back(pFont);
    LOG("Registered font \"%s\" [ %d KiBs ]", pFont->m_szFontName, pFont->m_iFontDataSize / (1 << 10));
}


///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
Font_t::Font_t(const char* szFontName, unsigned char* pFontData, unsigned int iFontDataSize, float flSize, ImFont** pDestination)
{
    assert(
        pFontData != nullptr    && // No bullshit data
        sizeof(pFontData) > 0   && // No empty fonts
        flSize > 0.0f           && // Size must be good
        pDestination != nullptr && // Must have a valid place to store
        iFontDataSize > 100     && // ImGui also asserts at lessthan 100 bytes.
        "Fucked up font data"); // MSG

    m_flSize        = flSize;
    m_szFontName    = szFontName;
    m_pDestination  = pDestination;
    m_pFontData     = pFontData;
    m_iFontDataSize = iFontDataSize;

    Resources::Fonts::fontManager.RegisterFont(this);
}
