#include "RandomFloat.h"
#include <iostream>

hook::randomFloat::T_randomFloat hook::randomFloat::O_randomFloat = nullptr;
float __fastcall hook::randomFloat::H_randomFloat(/*void* CGausianStream, float m1, float m2*/)
{
    float result = hook::randomFloat::O_randomFloat(/*CGausianStream, m1, m2*/);
    //printf("random float : %.2f\n", result);
    return result;
}