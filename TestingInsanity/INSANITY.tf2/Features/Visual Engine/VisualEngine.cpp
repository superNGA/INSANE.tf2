#include "VisualEngine.h"

#include <d3d9.h>
#include <d3dx9.h>

// SDK
#include "../../SDK/class/Basic Structures.h"
#include "../../SDK/class/IVEngineClient.h"

// UTILITY
#include "../../Utility/ConsoleLogging.h"
#include "../../Extra/math.h"


struct Vertex
{
    float x, y, z;
};


struct ScreenPos
{
    float x, y, z;
};


bool WorldToScreen(const vec& in, vec& out, const float m[4][4], int screenWidth, int screenHeight);

//=========================================================================
//                     PUBLIC METHODS
//=========================================================================
void VisualEngine_t::Run(LPDIRECT3DDEVICE9 pDevice)
{
    if (m_pEffect == nullptr || m_pVertexBuffer == nullptr || m_pStateBlock == nullptr || m_pVertexDecl == nullptr)
    {
        // COMPILLING SHADERS
        if (_CompileShaders(pDevice) == false)
        {
            FAIL_LOG("Failed to compile shaders");
            return;
        }
        WIN_LOG("Successfully compiled shaders");


        // Allocating vertex buffer & temporary vertex data
        if (_AllocBuffer(pDevice) == false)
        {
            FAIL_LOG("Failed to allocate vertex buffer");
            return;
        }
        WIN_LOG("Allocated buffer successfully");

        // Creating state block object.
        pDevice->CreateStateBlock(D3DSBT_ALL, &m_pStateBlock);

        
        if (_DeclareVertex(pDevice) == false)
        {
            FAIL_LOG("Failed to declare vertex format.");
            return;
        }
        WIN_LOG("Declared vertex format");
    }

    if (!I::iEngine->IsInGame())
        return;

    // Checking Z Buffer format
    {
        IDirect3DSurface9* pSurf = nullptr;
        auto stencilResult = pDevice->GetDepthStencilSurface(&pSurf);
        if (pSurf != nullptr || FAILED(stencilResult) == true)
        {
            D3DSURFACE_DESC surfaceDesc;
            if(SUCCEEDED(pSurf->GetDesc(&surfaceDesc)) == true)
                printf("Z Buffer format : %d\n", surfaceDesc.Format);
            pSurf->Release();
        }
    }

    m_pStateBlock->Capture();

    {
        // Render
        float arrClr[4] = { 0.0f, 1.0f, 1.0f, 1.0f };
        m_pEffect->SetFloatArray("defaultClr", arrClr, 4);
        m_pEffect->SetFloat("zCustom", Features::VisualEngine::VisualEngine::Z.GetData().m_flVal);
        m_pEffect->SetTechnique("HardRedPixels");
        m_pEffect->Begin(nullptr, 0);
        m_pEffect->BeginPass(0);

        // Disabling flags to help out drawing show in real game
        {
            pDevice->SetRenderState(D3DRS_ZENABLE, D3DZB_TRUE);
            pDevice->SetRenderState(D3DRS_ZWRITEENABLE, TRUE); 
            pDevice->SetRenderState(D3DRS_ZFUNC, D3DCMP_LESS);
        }

        pDevice->SetVertexDeclaration(m_pVertexDecl);
        pDevice->SetStreamSource(0, m_pVertexBuffer, 0, sizeof(ScreenPos));
        pDevice->DrawPrimitive(D3DPT_TRIANGLELIST, 0, 1);

        m_pEffect->EndPass();
        m_pEffect->End();
    }

    m_pStateBlock->Apply();
}


void VisualEngine_t::SetW2SMatrix()
{
    if (m_pEffect == nullptr)
        return;

    m_pEffect->SetMatrix("g_worldToScreen",reinterpret_cast<D3DXMATRIX*>(&I::iEngine->WorldToScreenMatrix()));
}


//=========================================================================
//                     PRIVATE METHODS
//=========================================================================
bool VisualEngine_t::_CompileShaders(LPDIRECT3DDEVICE9 pDevice)
{
    if (m_pEffect != nullptr)
        return true;

    LPD3DXBUFFER pErrorBuffer = nullptr;

    HRESULT pCompiledEffect = D3DXCreateEffectFromFileA(
        pDevice,
        "SimpleShader.fx",
        nullptr,
        nullptr,
        D3DXSHADER_DEBUG,
        nullptr,
        &m_pEffect,
        &pErrorBuffer
    );

    if (FAILED(pCompiledEffect) == true)
    {
        FAIL_LOG("Failed to compile shaders. ERROR : %s", (char*)pErrorBuffer);
        return false;
    }

    return true;
}



bool VisualEngine_t::_AllocBuffer(LPDIRECT3DDEVICE9 pDevice)
{
    if (m_pVertexBuffer != nullptr)
        return true;

    auto allocResult = pDevice->CreateVertexBuffer(sizeof(ScreenPos) * 3, D3DUSAGE_WRITEONLY, 0, D3DPOOL_DEFAULT, &m_pVertexBuffer, nullptr);
    
    if (FAILED(allocResult) == true)
        return false;   

    if (m_pVertexBuffer == nullptr)
        return false;

    void* pLockedData = nullptr;
    auto  lockResult  = m_pVertexBuffer->Lock(0, sizeof(ScreenPos) * 3, &pLockedData, 0); // might wanna discard here?
    if (FAILED(lockResult) == true || pLockedData == nullptr)
        return false;

    ScreenPos vecScreenPos[3] =
    {  
       // x        y       z
        {0.0f,   0.0f, 0.0f},
        {100.0f, 0.0f, 0.0f},
        {0.0f,   0.0f, 100.0f}
    };
    memcpy(pLockedData, vecScreenPos, sizeof(ScreenPos) * 3);

    m_pVertexBuffer->Unlock();

    return true;
}


bool VisualEngine_t::_DeclareVertex(LPDIRECT3DDEVICE9 pDevice)
{
    if (m_pVertexDecl != nullptr)
        return true;

    // this usage index is used when we have mutilple objects of the same schematic in the vertex struct.
    _D3DVERTEXELEMENT9 vertexDecl[] =
    {
      // Stream, Offset, Type,               Method,                Usage,                 Usage Index
        {0,      0,      D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION , 0},
        D3DDECL_END()
    };

    auto declResult = pDevice->CreateVertexDeclaration(vertexDecl, &m_pVertexDecl);
    if (FAILED(declResult) == true)
        return false;

    return true;
}


bool WorldToScreen(const vec& in, vec& out, const float m[4][4], int screenWidth, int screenHeight)
{
    float w =
        in.x * m[3][0] +
        in.y * m[3][1] +
        in.z * m[3][2] +
        m[3][3];

    if (w < 0.001f) // behind camera or invalid
        return false;

    float x =
        in.x * m[0][0] +
        in.y * m[0][1] +
        in.z * m[0][2] +
        m[0][3];

    float y =
        in.x * m[1][0] +
        in.y * m[1][1] +
        in.z * m[1][2] +
        m[1][3];

    float z = 
        in.x * m[2][0] + 
        in.y * m[2][1] + 
        in.z * m[2][2] + 
        m[2][3];

    out.x = x / w; // in range [-1, +1]
    out.y = y / w;
    out.z = w;

    //out.x = (screenWidth / 2.0f) * (1.0f + (x / w));
    //out.y = (screenHeight / 2.0f) * (1.0f - (y / w));

    return true;
}
