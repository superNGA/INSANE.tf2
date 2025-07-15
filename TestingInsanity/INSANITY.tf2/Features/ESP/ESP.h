#pragma once

#include "../FeatureHandler.h"


class CUserCmd;
class BaseEntity;


class ESP_t
{
public:
    void Run(BaseEntity* pLocalPlayer, CUserCmd* pCmd);
};
DECLARE_FEATURE_OBJECT(esp, ESP_t)

DEFINE_TAB(ESP, 1111);
DEFINE_SECTION(PLAYER, "ESP", 1);


DEFINE_FEATURE(Speed, FloatSlider_t, PLAYER, ESP,
    1, FloatSlider_t(100.0f, 1.0f, 500.0f), FeatureFlag_None,
    "ESP SPEED")
DEFINE_FEATURE(Thickness, FloatSlider_t, PLAYER, ESP,
    2, FloatSlider_t(5.0f, 1.0f, 100.0f), FeatureFlag_None,
    "ESP border thickness")

DEFINE_FEATURE(TOP_LEFT, ColorData_t, PLAYER, ESP,
    3, ColorData_t(0.0f, 0.0f, 0.0f, 0.0f), FeatureFlag_None,
    "TOP_LEFT corner clr")
DEFINE_FEATURE(TOP_RIGHT, ColorData_t, PLAYER, ESP,
    4, ColorData_t(0.0f, 0.0f, 0.0f, 0.0f), FeatureFlag_None,
    "TOP_RIGHT corner clr")
DEFINE_FEATURE(BOTTOM_LEFT, ColorData_t, PLAYER, ESP,
    5, ColorData_t(0.0f, 0.0f, 0.0f, 0.0f), FeatureFlag_None,
    "BOTTON_LEFT corner clr")
DEFINE_FEATURE(BOTTOM_RIGHT, ColorData_t, PLAYER, ESP,
    6, ColorData_t(0.0f, 0.0f, 0.0f, 0.0f), FeatureFlag_None,
    "BOTTOM_RIGHT corner clr")

DEFINE_FEATURE(glowPower, FloatSlider_t, PLAYER, ESP,
    7, FloatSlider_t(3.0f, 0.0f, 25), FeatureFlag_None,
    "ESP SPEED")