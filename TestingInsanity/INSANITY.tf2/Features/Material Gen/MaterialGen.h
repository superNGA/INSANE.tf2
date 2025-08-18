// 17 08 25
#pragma once

#include "../FeatureHandler.h"


class MaterialGen_t
{
public:
    void Run();

private:

};

DECLARE_FEATURE_OBJECT(materialGen, MaterialGen_t)

DEFINE_TAB(MaterialGen, 11)
DEFINE_SECTION(MaterialGen, "MaterialGen", 1)
DEFINE_FEATURE(Enable, bool, MaterialGen, MaterialGen, 1, false)

DEFINE_FEATURE(X, FloatSlider_t, MaterialGen, MaterialGen, 2, FloatSlider_t(0.0f, -500.0f, 500.0f));
DEFINE_FEATURE(Y, FloatSlider_t, MaterialGen, MaterialGen, 3, FloatSlider_t(0.0f, -500.0f, 500.0f));
DEFINE_FEATURE(Z, FloatSlider_t, MaterialGen, MaterialGen, 4, FloatSlider_t(0.0f, -500.0f, 500.0f));