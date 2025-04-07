#pragma once
#include <cstdint>

namespace hook
{
    namespace FX_FireBullets
    {
        typedef int64_t (__fastcall* T_FireBullets)(
            float* a1,
            unsigned int a2,
            int32_t* a3,
            int64_t a4,
            uint32_t a5,
            int32_t a6,
            uint32_t a7,
            float a8,
            float a9,
            char a10);
        extern T_FireBullets O_FireBullets;
        int64_t __fastcall H_FireBulets(float* a1,
            unsigned int a2,
            int32_t* a3,
            int64_t a4,
            uint32_t a5,
            int32_t a6,
            uint32_t a7,
            float a8,
            float a9,
            char a10);
    }
}