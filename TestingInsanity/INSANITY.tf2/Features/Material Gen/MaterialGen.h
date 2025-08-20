// 17 08 25
#pragma once

#include "../FeatureHandler.h"


class MaterialGen_t
{
public:
    void Run();

private:
    void _DisableGameConsole();
    void _AdjustCamera();

};

DECLARE_FEATURE_OBJECT(materialGen, MaterialGen_t)

DEFINE_TAB(MaterialGen, 11)
DEFINE_SECTION(MaterialGen, "MaterialGen", 1)
DEFINE_FEATURE(Enable, bool, MaterialGen, MaterialGen, 1, false)

DEFINE_FEATURE(z, FloatSlider_t, MaterialGen, MaterialGen, 2, 
    FloatSlider_t(0.0f, -200.0f, 200.0f))

DEFINE_FEATURE(Model, IntSlider_t, MaterialGen, MaterialGen, 3, 
    IntSlider_t(0, 0, 6))