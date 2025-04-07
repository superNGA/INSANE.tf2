#include "FX_FireBullets.h"
#include <iostream>

hook::FX_FireBullets::T_FireBullets hook::FX_FireBullets::O_FireBullets = nullptr;
int64_t __fastcall hook::FX_FireBullets::H_FireBulets(float* a1,
    unsigned int a2,
    int32_t* a3,
    int64_t a4,
    uint32_t a5,
    int32_t a6,
    uint32_t a7,
    float a8,
    float a9,
    char a10)
{
    //printf("fire bullet seed : %d | spread : %f\n", a7, a8);
    return O_FireBullets(a1, a2, a3, a4, a5, a6, a7, a8, a9, a10);
}