#include "FX_FireBullets.h"
#include <iostream>
#include "../../Features/NoSpread/NoSpread.h"


hook::FX_FireBullets::T_FireBullets hook::FX_FireBullets::O_FireBullets = nullptr;
int64_t __fastcall hook::FX_FireBullets::H_FireBulets(float* a1,
    unsigned int a2,
    int32_t* a3,
    int64_t a4,
    uint32_t a5,
    int32_t a6,
    uint32_t iSeed,
    float flBaseSpread,
    float a9,
    char a10)
{
    iSeed = Features::noSpread.m_iSeed.load();
    //iSeed = Features::noSpread.GetSeed();
    printf("spread : %f W/ seed : %d\n", flBaseSpread, iSeed);
    return O_FireBullets(a1, a2, a3, a4, a5, a6, iSeed, flBaseSpread, a9, a10);
}