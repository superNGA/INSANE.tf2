#pragma once

#include "../../FeatureHandler.h"

class BoxFilled2D_t;


///////////////////////////////////////////////////////////////////////////
class MenuGUI_t
{
public:
    void Draw();

private:
    BoxFilled2D_t* m_pMainMenu = nullptr;

};
///////////////////////////////////////////////////////////////////////////

DECLARE_CUSTOM_OBJECT(menuGUI, MenuGUI_t, Render)

DEFINE_TAB(Menu, 7)
DEFINE_SECTION(Menu, "Menu", 1)
DEFINE_FEATURE(Scale, FloatSlider_t, Menu, Menu, 1, FloatSlider_t(1.0f, 0.25f, 2.0f))
DEFINE_FEATURE(Blur,  FloatSlider_t, Menu, Menu, 2, FloatSlider_t(0.0f, 0.0f, 8.0f))

DEFINE_FEATURE(ColorTopRight,    ColorData_t, Menu, Menu, 3, ColorData_t(RGBA_t(255, 255, 255, 255)))
DEFINE_FEATURE(ColorTopLeft,     ColorData_t, Menu, Menu, 4, ColorData_t(RGBA_t(255, 255, 255, 255)))
DEFINE_FEATURE(ColorBottomRight, ColorData_t, Menu, Menu, 5, ColorData_t(RGBA_t(255, 255, 255, 255)))
DEFINE_FEATURE(ColorBottomLeft,  ColorData_t, Menu, Menu, 6, ColorData_t(RGBA_t(255, 255, 255, 255)))

DEFINE_FEATURE(rgb,      bool,          Menu, Menu, 7, false)
DEFINE_FEATURE(RGBSpeed, FloatSlider_t, Menu, Menu, 8, FloatSlider_t(0.0f, 0.0f, 10.0f))

DEFINE_FEATURE(Rounding, FloatSlider_t, Menu, Menu, 9, FloatSlider_t(15.0f, 0.0f, 100.0f)) 