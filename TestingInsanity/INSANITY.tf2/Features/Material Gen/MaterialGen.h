// 17 08 25
#pragma once

#include <chrono>
#include "../FeatureHandler.h"

class MaterialGen_t
{
public:
    MaterialGen_t();

    void Run();

private:
    void _DisableGameConsole();
    void _AdjustCamera();
    void _AdjustModel();

    std::chrono::high_resolution_clock::time_point m_lastUpdateTime;
    
    // Rotation speed is defined as how much angle is covered ( in degrees ) 
    // in one second.
    // float m_flRotationSpeed = 45.0f;
};

DECLARE_FEATURE_OBJECT(materialGen, MaterialGen_t)

DEFINE_TAB(MaterialGen, 11)
DEFINE_SECTION(MaterialGen, "MaterialGen", 1)
DEFINE_FEATURE(Enable, bool, MaterialGen, MaterialGen, 1, false)

DEFINE_FEATURE(RotationSpeed, FloatSlider_t, MaterialGen, MaterialGen, 2, 
    FloatSlider_t(0.0f, -360.0f, 360.0f))

DEFINE_FEATURE(Model, IntSlider_t, MaterialGen, MaterialGen, 3, 
    IntSlider_t(0, 0, 6))