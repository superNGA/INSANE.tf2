#include "../Utility/Hook_t.h"
#include "../Utility/ConsoleLogging.h"

#include "../Features/config.h"
#include "../Features/NoSpread/NoSpread.h"

MAKE_HOOK(FX_FireBullet, "48 89 5C 24 ? 48 89 74 24 ? 4C 89 4C 24 ? 55", __fastcall, CLIENT_DLL, int64_t, float* a1,
    unsigned int a2, int32_t* a3, int64_t a4,
    uint32_t a5, int32_t a6, uint32_t iSeed, float flBaseSpread, float a9, char a10)
{
    if(config.aimbotConfig.bNoSpread == true)
        iSeed = Features::noSpread.m_iSeed.load();
    
    return Hook::FX_FireBullet::O_FX_FireBullet(a1, a2, a3, a4, a5, a6, iSeed, flBaseSpread, a9, a10);
}