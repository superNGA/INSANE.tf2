#pragma once

#include <d3d9.h>
#include <d3dx9.h>

#include "../../FeatureHandler.h"


struct DrawObjList_t;


//=========================================================================
//                     DIRECTX9 HANDLER
//=========================================================================
class DirectxHandler_t
{
public:
    void Initialize(LPDIRECT3DDEVICE9 pDevice);
    inline void CaptureState() { if (m_pStateBlock != nullptr) m_pStateBlock->Capture(); }
    inline void RestoreState() { if (m_pStateBlock != nullptr) m_pStateBlock->Apply(); }

    void SetW2SMatrix();
    void SetShaderTime();

    void Draw(DrawObjList_t& drawlist, LPDIRECT3DDEVICE9 pDevice);

private:
    bool m_bInitialized = false;

    bool _CompileShaders(LPDIRECT3DDEVICE9 pDevice);
    bool _DeclareVertex(LPDIRECT3DDEVICE9 pDevice);

    LPD3DXEFFECT                 m_pEffect     = nullptr;
    IDirect3DStateBlock9*        m_pStateBlock = nullptr;
    IDirect3DVertexDeclaration9* m_pVertexDecl = nullptr;
};

DECLARE_FEATURE_OBJECT(directxHandler, DirectxHandler_t)


////////////////////////////// CONSTANTs //////////////////////////////

static _D3DVERTEXELEMENT9 vertexDecl[] =
{
    // Stream, Offset,                                                         Type,               Method,                Usage,                 Usage Index
      {0,      0,                                                              D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION , 0},
      {0,      sizeof(float) * 3,                                              D3DDECLTYPE_UBYTE4N, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_COLOR ,   0},
      {0,      (sizeof(float) * 3) + (sizeof(char) * 4),                       D3DDECLTYPE_FLOAT1, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD , 0},
      {0,      (sizeof(float) * 3) + (sizeof(char) * 4) + sizeof(float),       D3DDECLTYPE_FLOAT1, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD , 1},
      {0,      (sizeof(float) * 3) + (sizeof(char) * 4) + (sizeof(float) * 2), D3DDECLTYPE_FLOAT1, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD , 2},
      D3DDECL_END()
};


// This is the common shader that does all the shit.
static const char szCommonShader[] = R"(


// INPUT STRUCT ( screen pos coordinates )
struct vs_input {
    float3 Pos : POSITION;
    float4 clr : COLOR;
    float flWidth : TEXCOORD0;
    float flSpeed : TEXCOORD1;
    float flGlowPower : TEXCOORD2;
};

// OUTPUT STRUCT
struct vs_output {
    float4 Pos : POSITION;
    float4 clr : COLOR;
    float flWidth : TEXCOORD0;
    float flSpeed : TEXCOORD1;
    float flGlowPower : TEXCOORD2;
};

// World to screen matrix which gets loaded here form frame state NET_UPDATE_END.
float4x4 g_worldToScreen;
float    flTime;

float4 clrDefault = (1.0, 0.0, 0.0, 1.0);

float4 W2S(float3 worldPos)
{
    float4 screenPos;

    // World-to-screen
    screenPos.x = (worldPos.x * g_worldToScreen._11) + (worldPos.y * g_worldToScreen._12) + (worldPos.z * g_worldToScreen._13) + g_worldToScreen._14;
    screenPos.y = (worldPos.x * g_worldToScreen._21) + (worldPos.y * g_worldToScreen._22) + (worldPos.z * g_worldToScreen._23) + g_worldToScreen._24;
    screenPos.z = (worldPos.x * g_worldToScreen._31) + (worldPos.y * g_worldToScreen._32) + (worldPos.z * g_worldToScreen._33) + g_worldToScreen._34;
    screenPos.w = (worldPos.x * g_worldToScreen._41) + (worldPos.y * g_worldToScreen._42) + (worldPos.z * g_worldToScreen._43) + g_worldToScreen._44;

    return screenPos;
}

// VERTEX SHADER ( calculates coordintes & clrs )
vs_output VS(vs_input input)
{
    vs_output output;

    output.Pos = W2S(input.Pos);

    output.clr     = input.clr;
    output.flWidth = input.flWidth;
    output.flSpeed = input.flSpeed;
    output.flGlowPower = input.flGlowPower;

    return output;
}


struct HSV
{
    float h,s,v;
};

HSV RGB2HSV(float4 clr)
{
    HSV clrHSV;

    float flCMax = max(clr.r, max(clr.g, clr.b));
    float flCMin = min(clr.r, min(clr.g, clr.b));
    float flDif = flCMax - flCMin;

    // HUE
    if(flCMax == flCMin)
    {
        clrHSV.h = 0.0;
    }
    else if(flCMax == clr.r)
    {
        clrHSV.h = fmod((60 * ((clr.g - clr.b) / flDif) + 360), 360);
    }
    else if (flCMax == clr.g)
    {
        clrHSV.h = fmod((60 * ((clr.b - clr.r) / flDif) + 120), 360);
    }
    else if(flCMax == clr.b)
    {
        clrHSV.h = fmod((60 * ((clr.r - clr.g) / flDif) + 240), 360);
    }

    // SATURATION
    clrHSV.s = (flCMax == 0.0 ? 0.0 : (flDif / flCMax));

    // VIBRANCE
    clrHSV.v = flCMax;

    return clrHSV;
}

float3 HSV2RGB(HSV hsv)
{
    float C = hsv.v * hsv.s;
    float X = C * (1 - abs(fmod(hsv.h / 60.0, 2.0) - 1));
    float m = hsv.v - C;

    float3 rgb;

    if (hsv.h < 60)       rgb = float3(C, X, 0);
    else if (hsv.h < 120) rgb = float3(X, C, 0);
    else if (hsv.h < 180) rgb = float3(0, C, X);
    else if (hsv.h < 240) rgb = float3(0, X, C);
    else if (hsv.h < 300) rgb = float3(X, 0, C);
    else                  rgb = float3(C, 0, X);

    return rgb + float3(m, m, m);
}


// PIXEL SHADER
float4 PS(vs_output output) : COLOR
{
    HSV clrHSV = RGB2HSV(output.clr);

    // RGB-ing
    clrHSV.h = fmod(clrHSV.h + (flTime * output.flSpeed), 360.0);

    float flAlpha = exp(-pow(output.flWidth * output.flGlowPower, 2));
    flAlpha = min(flAlpha, output.clr.a);

    clrHSV.v = saturate(clrHSV.v + (0.3 * flAlpha));
    clrHSV.s = saturate(clrHSV.s + (0.2 * flAlpha));

    return float4(HSV2RGB(clrHSV), flAlpha);
}


// Our Technique & pass
technique HardRedPixels
{
    pass P0
    {
        VertexShader = compile vs_3_0 VS();
        PixelShader  = compile ps_3_0 PS();
    }
}


)";