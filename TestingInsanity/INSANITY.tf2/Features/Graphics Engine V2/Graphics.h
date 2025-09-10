#pragma once

#include <d3d9.h>
#include <d3dx9.h>
#include <atomic>
#include <vector>

#include "../FeatureHandler.h"


class IDrawObj_t;


///////////////////////////////////////////////////////////////////////////
struct VertexBuffer_t
{
    VertexBuffer_t(D3DPRIMITIVETYPE iPrimitiveType, int iVertexPerPrimitive) :
        m_iPrimitiveType(iPrimitiveType), 
        m_iVertexPerPrimitive(iVertexPerPrimitive)
    {
        m_free.store(true);
        m_iBufferSize.store(0ull);
    }

    std::atomic<bool> m_free;

    
    // Vertex buffer...
    bool AdjustBufferSize(uint64_t iSizeRequired, LPDIRECT3DDEVICE9 pDevice);
    void FreeBuffer();
    D3DPRIMITIVETYPE        m_iPrimitiveType;
    int                     m_iVertexPerPrimitive = 0;
    IDirect3DVertexBuffer9* m_pBuffer = nullptr;
    std::atomic<size_t>     m_iBufferSize;


    // Draw objs.
    void RegisterDrawObj(IDrawObj_t* pDrawObj);
    bool RemoveDrawObj(IDrawObj_t* pDrawObj);
    std::vector<IDrawObj_t*> m_vecDrawObjs = {};
    std::vector<IDrawObj_t*> m_vecTempBuffer = {};
};
///////////////////////////////////////////////////////////////////////////


///////////////////////////////////////////////////////////////////////////
class Graphics_t
{
public:
    Graphics_t():
        m_lines(D3DPRIMITIVETYPE::D3DPT_LINELIST, 2),
        m_traingles(D3DPRIMITIVETYPE::D3DPT_TRIANGLELIST, 3)
    {}

    void Run(LPDIRECT3DDEVICE9 pDevice);
    void Free();

    void CaptureW2SMatrix();

    void RegisterInLineList(IDrawObj_t* pDrawObj);
    void RegisterInTraingleList(IDrawObj_t* pDrawObj);
    void FindAndRemoveDrawObj(IDrawObj_t* pDrawObj);

private:
    bool _Initialize(LPDIRECT3DDEVICE9 pDevice);
    bool _InitShaderVariables(LPDIRECT3DDEVICE9 pDevice);

    void _DrawList(VertexBuffer_t* pData, LPDIRECT3DDEVICE9 pDevice);

    bool m_bShaderCompiled = false;
    bool _CompileShader(LPDIRECT3DDEVICE9 pDevice);

    bool m_bVertexDecl = false;
    bool _DeclareVertex(LPDIRECT3DDEVICE9 pDevice);

    bool m_bTextureInit = false;
    bool _CreateTexture(LPDIRECT3DDEVICE9 pDevice);

    bool m_bStateBlockInit = false;
    bool _CreateStateBlock(LPDIRECT3DDEVICE9 pDevice);

    LPD3DXEFFECT                 m_pEffect       = nullptr;
    IDirect3DStateBlock9*        m_pStateBlock   = nullptr;
    IDirect3DVertexDeclaration9* m_pVertexDecl   = nullptr;
    IDirect3DTexture9*           m_pSceneTexture = nullptr;
    IDirect3DSurface9*           m_pSceneSurface = nullptr;

    view_matrix m_vW2SMatrix;

    // Vertex Buffer handlers
    VertexBuffer_t m_lines;
    VertexBuffer_t m_traingles;
};
///////////////////////////////////////////////////////////////////////////

DECLARE_FEATURE_OBJECT(graphics, Graphics_t)


///////////////////////////////////////////////////////////////////////////
static const char szShader[] = 
R"(
#define INPUTFLAGS_INVERT_CLR  (1 << 0)
#define INPUTFLAGS_STRICTLY_2D (1 << 1)

// Input struct...
struct Input_t
{
    float3 m_vPos          : POSITION;
    float4 m_clr           : COLOR;
    float  m_flBlurAmmount : TEXCOORD0;
    float  m_flRounding    : TEXCOORD1;
    float3 m_vRelativeUV   : TEXCOORD2;
    float  m_bStrictly2D   : TEXCOORD3;
    float  m_bInvertColor  : TEXCOORD4;
};


struct Output_t
{
    float4 m_vPos          : POSITION;
    float4 m_clr           : COLOR;
    float3 m_vRelativeUV   : TEXCOORD0;
    float  m_flRounding    : TEXCOORD1;
    float  m_flBlurAmmount : TEXCOORD2;
    float2 m_vAbsUV        : TEXCOORD3;
    float  m_bInvertColor  : TEXCOORD4;
};

// Global Vars...
texture SceneTex;
sampler2D SceneSampler = sampler_state
{
    Texture   = <SceneTex>;
    MinFilter = LINEAR;
    MagFilter = LINEAR;
    MipFilter = NONE;
};

int g_iScreenHeight = 0;
int g_iScreenWidth  = 0;
float4x4 g_worldToScreen; // world to screen matrix.


///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
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


///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
bool IsBitEnabled(float flags, float bitMask)
{
    return fmod(floor(flags / bitMask), 2.0) == 1.0;
}


///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
Output_t VS(Input_t input)
{
    Output_t result;
    
    // If strictly 2D shape.
    if (input.m_bStrictly2D == 1.0f)
    {
        result.m_vPos.x = ((input.m_vPos.x / g_iScreenWidth)           - 0.5f) * 2.0f;
        result.m_vPos.y = ((1.0f - (input.m_vPos.y / g_iScreenHeight)) - 0.5f) * 2.0f; // Y cords are inverted in here, so [-1, -1] is bottom left. and [1, 1] is top right.
        result.m_vPos.z = 0.0f;
        result.m_vPos.w = 1.0f;
        
        result.m_vAbsUV.x = input.m_vPos.x / g_iScreenWidth;
        result.m_vAbsUV.y = input.m_vPos.y / g_iScreenHeight;
    }
    else // if not stricly 2D shape, then world-to-screen matrix mult. is required.
    {
        result.m_vPos = W2S(input.m_vPos);
        
        result.m_vAbsUV.x = ((result.m_vPos.x / result.m_vPos.w) * 0.5f) + 0.5f;
        result.m_vAbsUV.y = ((result.m_vPos.y / result.m_vPos.w) * 0.5f) + 0.5f;
    }
    
    // Copy essential imformation to output ( required in pixel shader )
    result.m_bInvertColor  = input.m_bInvertColor;
    result.m_flRounding    = input.m_flRounding;
    result.m_flBlurAmmount = input.m_flBlurAmmount;
    result.m_vRelativeUV   = input.m_vRelativeUV;
    result.m_clr           = input.m_clr;
    
    return result;
}


///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
float4 GetClrAverage(float2 uv, int iPixelOffset)
{
    float iPixelHeightInUV = 1.0f / g_iScreenHeight;
    float iPixelWidthInUV  = 1.0f / g_iScreenWidth;
    iPixelHeightInUV *= iPixelOffset;
    iPixelWidthInUV  *= iPixelOffset;
    
    float4 up    = tex2D(SceneSampler, float2(saturate(uv.x + iPixelHeightInUV), saturate(uv.y)));
    float4 down  = tex2D(SceneSampler, float2(saturate(uv.x - iPixelHeightInUV), saturate(uv.y)));
    float4 left  = tex2D(SceneSampler, float2(saturate(uv.x), saturate(uv.y - iPixelWidthInUV)));
    float4 right = tex2D(SceneSampler, float2(saturate(uv.x), saturate(uv.y + iPixelWidthInUV)));
    
    return (up + down + left + right) / 4.0f;
}


///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
float4 PS(Output_t output) : COLOR
{
    float4 resultClr = output.m_clr;
    
    // Blur-ing
    if ((int)output.m_flBlurAmmount > 0)
    {
        float4 vAverageClr = GetClrAverage(output.m_vAbsUV, (int)output.m_flBlurAmmount);
        vAverageClr.a      = 1.0f;

        // "returning" averaged out clr.
        resultClr = vAverageClr;
    }
    
    // Inverting colors.
    if (output.m_bInvertColor == 1.0f)
    {
        resultClr.r = 1.0f - resultClr.r;
        resultClr.g = 1.0f - resultClr.g;
        resultClr.b = 1.0f - resultClr.b;
    }
    
    return resultClr;
}


///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
technique simple1
{
    pass p0
    {
        VertexShader = compile vs_3_0 VS();
        PixelShader  = compile ps_3_0 PS();
    }
}
)";