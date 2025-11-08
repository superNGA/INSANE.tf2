#pragma once
#include "Basic Structures.h"


///////////////////////////////////////////////////////////////////////////
enum LightType_t
{
    MATERIAL_LIGHT_DISABLE = 0,
    MATERIAL_LIGHT_POINT,
    MATERIAL_LIGHT_DIRECTIONAL,
    MATERIAL_LIGHT_SPOT,
};
///////////////////////////////////////////////////////////////////////////


///////////////////////////////////////////////////////////////////////////
enum AmbientLight_t : int32_t
{
    LIGHT_BACK = 0, LIGHT_FRONT,
    LIGHT_LEFT,     LIGHT_RIGHT,
    LIGHT_TOP,      LIGHT_BOTTON
};
///////////////////////////////////////////////////////////////////////////


///////////////////////////////////////////////////////////////////////////
struct LightDesc_t
{
    LightType_t     m_Type;										//< MATERIAL_LIGHT_xxx
    vec             m_Color;									//< color+intensity 
    vec             m_Position;									//< light source center position
    vec             m_Direction;								//< for SPOT, direction it is pointing
    float           m_Range;									//< distance range for light.0=infinite
    float           m_Falloff;									//< angular falloff exponent for spot lights
    float           m_Attenuation0;								//< constant distance falloff term
    float           m_Attenuation1;								//< linear term of falloff
    float           m_Attenuation2;								//< quadatic term of falloff
    float           m_Theta;									//< inner cone angle. no angular falloff 
    float           m_Phi;										//< outer cone angle
    float           m_ThetaDot;
    float           m_PhiDot;
    unsigned int    m_Flags;
    float           OneOver_ThetaDot_Minus_PhiDot;
    float           m_RangeSquared;
};
///////////////////////////////////////////////////////////////////////////