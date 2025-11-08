#pragma once
#include "Basic Structures.h"


///////////////////////////////////////////////////////////////////////////
struct dworldlight_t
{
    vec		    origin;
    vec		    intensity;
    vec		    normal;			
    int			cluster;
    int       	type;
    int			style;
    float		stopdot;		
    float		stopdot2;		
    float		exponent;		
    float		radius;			
    float		constant_attn;
    float		linear_attn;
    float		quadratic_attn;
    int			flags;			
    int			texinfo;		
    int			owner;			
};
///////////////////////////////////////////////////////////////////////////