#pragma once
#include <cstdint>

namespace hook
{
    namespace sv_pure
    {
        // 48 89 5C 24 ? 48 89 74 24 ? 48 89 7C 24 ? 55 48 8D AC 24 ? ? ? ? 48 81 EC
        typedef int64_t(__fastcall* T_svPure)(void*);
        extern T_svPure O_svPure;
        void __fastcall H_svPure(void* pFile);
    }
}