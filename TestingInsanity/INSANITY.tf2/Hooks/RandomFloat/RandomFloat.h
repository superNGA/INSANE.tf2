#pragma once

namespace hook
{
    namespace randomFloat
    {
        typedef float(__fastcall* T_randomFloat)(/*void*, float, float*/);
        extern T_randomFloat O_randomFloat;
        float __fastcall H_randomFloat(/*void* CGausianStream, float m1, float m2*/);
    }
}