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
    void FreeBufferAndDeleteDrawObj();
    D3DPRIMITIVETYPE        m_iPrimitiveType;
    int                     m_iVertexPerPrimitive = 0;
    IDirect3DVertexBuffer9* m_pBuffer = nullptr;
    std::atomic<size_t>     m_iBufferSize;


    // Draw objs.
    void RegisterDrawObj(IDrawObj_t* pDrawObj);
    bool FindAndRemoveDrawObj(IDrawObj_t* pDrawObj);
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

#define MAX_BLUR 8
#define M_PI 3.14159265359f
#define SQRT_2 1.414213562373095f

#define INPUTFLAGS_INVERT_CLR  (1 << 0)
#define INPUTFLAGS_STRICTLY_2D (1 << 1)

// Input struct...
struct Input_t
{
    float3 m_vPos           : POSITION;
    float4 m_clr            : COLOR;
    float  m_flBlurAmmount  : TEXCOORD0;
    float  m_flRounding     : TEXCOORD1;
    float3 m_vRelativeUV    : TEXCOORD2;
    float  m_bStrictly2D    : TEXCOORD3;
    float  m_bInvertColor   : TEXCOORD4;
    float  m_flScaleY       : TEXCOORD5;
    float  m_flCircleThickness : TEXCOORD6; // for value > 0, we assume this draw obj as cirle, else not a circle.
};


struct Output_t
{
    float4 m_vPos           : POSITION;
    float4 m_clr            : COLOR;
    float3 m_vRelativeUV    : TEXCOORD0;
    float  m_flRounding     : TEXCOORD1;
    float  m_flBlurAmmount  : TEXCOORD2;
    float2 m_vAbsUV         : TEXCOORD3;
    float  m_bInvertColor   : TEXCOORD4;
    float  m_flScaleY       : TEXCOORD5;
    float  m_flCircleThickness : TEXCOORD6; // for value > 0, we assume this draw obj as cirle, else not a circle.
};

// Global Vars...
texture SceneTex;
sampler2D SceneSampler = sampler_state
{
    Texture   = <SceneTex>;
    MinFilter = POINT;
    MagFilter = POINT;
    MipFilter = NONE;
};

int      g_flIsInGame    = 0;
float    g_flScreenHeight = 0.0f;
float    g_flScreenWidth  = 0.0f;
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
Output_t VS(Input_t input)
{
    Output_t result;
    
    // If this is a 2D shape or we are not in game ( invalid W2S matrix )
    if (input.m_bStrictly2D == 1.0f || g_flIsInGame == 0.0f)
    {
        result.m_vPos.x = ((input.m_vPos.x / g_flScreenWidth) - 0.5f) * 2.0f;
        result.m_vPos.y = ((1.0f - (input.m_vPos.y / g_flScreenHeight)) - 0.5f) * 2.0f; // Y cords are inverted in here, so [-1, -1] is bottom left. and [1, 1] is top right.
        result.m_vPos.z = 0.0f;
        result.m_vPos.w = 1.0f;
        
        result.m_vAbsUV.x = input.m_vPos.x / g_flScreenWidth;
        result.m_vAbsUV.y = input.m_vPos.y / g_flScreenHeight;
    }
    else // if not stricly 2D shape, then world-to-screen matrix mult. is required.
    {
        result.m_vPos = W2S(input.m_vPos);
        
        result.m_vAbsUV.x = ((result.m_vPos.x / result.m_vPos.w) * 0.5f) + 0.5f;
        result.m_vAbsUV.y = (((result.m_vPos.y / result.m_vPos.w) * 0.5f) * -1.0f) + 0.5f; // again gotta invert y coords.
    }
    
    // Copy essential imformation to output ( required in pixel shader )
    result.m_bInvertColor  = input.m_bInvertColor;
    result.m_flRounding    = input.m_flRounding;
    result.m_flScaleY      = input.m_flScaleY;
    result.m_flBlurAmmount = input.m_flBlurAmmount;
    result.m_vRelativeUV   = input.m_vRelativeUV;
    result.m_clr           = input.m_clr;
    result.m_flCircleThickness = input.m_flCircleThickness;
    
    return result;
}


///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
float4 GetClrAverage(float2 uv, int iPixelOffset)
{ 
    float iPixelHeightInUV = 1.0f / g_flScreenHeight;
    float iPixelWidthInUV  = 1.0f / g_flScreenWidth;
    float iOffsetY = iPixelHeightInUV * iPixelOffset;
    float iOffsetX = iPixelWidthInUV  * iPixelOffset;
    
    // Moving uv coord to center of the pixel / texel
    uv.x += iPixelWidthInUV  / 2.0f;
    uv.y += iPixelHeightInUV / 2.0f;
    uv = saturate(uv);
    
    if(iPixelOffset <= 0)
    {
        //return tex2D(SceneSampler, float2(saturate(uv.x - iPixelWidthInUV), saturate(uv.y - iPixelHeightInUV)));
        return tex2D(SceneSampler, uv);
    }
    
    float4 right = tex2D(SceneSampler, float2(saturate(uv.x + iOffsetX), saturate(uv.y)));
    float4 left  = tex2D(SceneSampler, float2(saturate(uv.x - iOffsetX), saturate(uv.y)));
    float4 up    = tex2D(SceneSampler, float2(saturate(uv.x), saturate(uv.y - iOffsetY)));
    float4 down  = tex2D(SceneSampler, float2(saturate(uv.x), saturate(uv.y + iOffsetY)));
    
    return (up + down + left + right) / 4.0f;
}


///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
float Vec2Length(float2 vec)
{
    return sqrt((vec.x * vec.x) + (vec.y * vec.y));
}


///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
float Vec2LengthSqr(float2 vec)
{
    return (vec.x * vec.x) + (vec.y * vec.y);
}


///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
float4 PS(Output_t output) : COLOR
{
    float4 resultClr = output.m_clr;
    
    // Blur-ing
    // a object, with blur set to 0, when drawn will draw like nothings draw below it.
    // We can use it to create "windows" in objects that are already drawn.
    if ((int)output.m_flBlurAmmount == 0)
    {
        float4 vAverageClr = float4(0.0f, 0.0f, 0.0f, 0.0f);
        
        // Getting this pixels color from "back buffer" or whatever TF that shit is called.
        vAverageClr = GetClrAverage(output.m_vAbsUV, 0);

        // Moving it towards actual color set by caller.        
        float3 tint     = float3(output.m_clr.rgb);
        vAverageClr.rgb = lerp(vAverageClr.rgb, tint, output.m_clr.a);
        
        vAverageClr.a = 1.0f;
        resultClr     = vAverageClr;
    }
    else if ((int)output.m_flBlurAmmount > 0)
    {
        float4 vAverageClr = float4(0.0f, 0.0f, 0.0f, 0.0f);
        
        for (int i = 1; i < MAX_BLUR; i++)
        {
            vAverageClr += GetClrAverage(output.m_vAbsUV, i);
            
            if (i >= (int)output.m_flBlurAmmount)
                break;
        }
        
        vAverageClr /= output.m_flBlurAmmount;
            
        // Lerping rgb toward the actual color of this pixel set by the caller.
        float3 tint     = float3(output.m_clr.rgb);
        vAverageClr.rgb = lerp(vAverageClr.rgb, tint, output.m_clr.a);
        vAverageClr.a   = 1.0f;
        
        
        resultClr = vAverageClr;
    }
    
    // Inverting colors.
    if (output.m_bInvertColor == 1.0f)
    {
        resultClr.r = 1.0f - resultClr.r;
        resultClr.g = 1.0f - resultClr.g;
        resultClr.b = 1.0f - resultClr.b;
    }
    
    // Rounding Corners
    if (output.m_flScaleY > 0.0f && output.m_flRounding > 0.0f && output.m_flCircleThickness <= 0.0f) // must not be a circle.
    {
        float2 vUVScaled = float2(output.m_vRelativeUV.x, output.m_vRelativeUV.y * output.m_flScaleY);
        
        float2 vClosestCorner = float2(step(0.5f, output.m_vRelativeUV.x), step(0.5f, output.m_vRelativeUV.y));        
        float2 vCenter        = float2(0.5f, 0.5f);
        
        float2 vDirection     = (vClosestCorner - vCenter);
        // Since the center is constant, and closest corner has coordinates either 1 or 0. the vector magnitude
        // from the center to any of the corner is gaurantted to be 0.5 * sqrt(2). and sqrt fns in shader are (maybe) 
        // expensive in shaders, so I just slapped in the constant value here.
        vDirection           /= 0.5f * SQRT_2; //Vec2Length(vDirection); // Unit vector from center to closest corner.
        float2 vAnchor        = vCenter + (vDirection * ((0.5f - output.m_flRounding) * SQRT_2));
        float2 vAnchorScaled  = float2(vAnchor.x, vAnchor.y - (vClosestCorner.y - (vClosestCorner.y * output.m_flScaleY))); // for top left & right corners, this won't move the anchor.y at all, cause closetCorner.y is 0
        
        // now scaling closest corner
        vClosestCorner.y *= output.m_flScaleY;
        
        // Distance of point from closest corner.
        float2 vCornerToPoint      = vUVScaled - vClosestCorner;
        float  flDistSqrFromCorner = Vec2LengthSqr(vCornerToPoint);
        
        // Distance of point from "best" anchor point.
        float2 vAnchorToPoint      = vUVScaled - vAnchorScaled;
        float  flDistSqrFromAnchor = Vec2LengthSqr(vAnchorToPoint);
        
        if(flDistSqrFromAnchor > (output.m_flRounding * output.m_flRounding) && flDistSqrFromCorner < (output.m_flRounding * output.m_flRounding))
            discard;

    }
    else if(output.m_flCircleThickness > 0.0f) // doing circle math here.
    {
        float2 vCenter = float2(0.5f, 0.5f);
        float2 vCenterToPoint = output.m_vRelativeUV.xy - vCenter;
        
        if (Vec2LengthSqr(vCenterToPoint) > 0.5f * 0.5f)
            discard;
    
        if(Vec2LengthSqr(vCenterToPoint) < (0.5f * 0.5f) - output.m_flCircleThickness)
            discard;
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