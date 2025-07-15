#include "DirectXHandler.h"

#include <chrono>

#include "../Graphics Engine/GraphicsEngine.h"
#include "../Draw Objects/DrawObj.h"
#include "../../../Utility/ConsoleLogging.h"

#include "../../../SDK/class/IVEngineClient.h"

//=========================================================================
//                     PUBLIC METHODS
//=========================================================================

//-------------------------------------------------------------------------
// PURPOSE : Compiles shaders, Creates stateBlock & vertex decleration
//-------------------------------------------------------------------------
void DirectxHandler_t::Initialize(LPDIRECT3DDEVICE9 pDevice)
{
    // Compilling shaders
    if (m_pEffect == nullptr)
    {
        if (_CompileShaders(pDevice) == false)
        {
            FAIL_LOG("Failed to compile shaders");
            return;
        }
        WIN_LOG("Successfully compiled shaders");
    }


    // Creating StateBlock
    if (m_pStateBlock == nullptr)
    {
        if (FAILED(pDevice->CreateStateBlock(D3DSBT_ALL, &m_pStateBlock)) == true)
        {
            m_pStateBlock = nullptr;
            FAIL_LOG("Failed to compile shaders");
            return;
        }
        WIN_LOG("Successfully compiled shaders");
    }


    // Creating Vertex Declerations
    if (m_pVertexDecl == nullptr)
    {
        if (_DeclareVertex(pDevice) == false)
        {
            FAIL_LOG("Failed to declare vertex format.");
            return;
        }
        WIN_LOG("Declared vertex format");
    }

    m_bInitialized = (m_pEffect != nullptr) && (m_pVertexDecl != nullptr) && (m_pStateBlock != nullptr);
}



void DirectxHandler_t::SetW2SMatrix()
{
    if (m_pEffect == nullptr)
        return;

    m_pEffect->SetMatrix("g_worldToScreen", reinterpret_cast<D3DXMATRIX*>(&I::iEngine->WorldToScreenMatrix()));
}


void DirectxHandler_t::SetShaderTime()
{
    if (m_pEffect == nullptr)
        return;

    uint64_t iTimeSinceEpochInMs = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now().time_since_epoch()).count();
    m_pEffect->SetFloat("flTime", static_cast<float>(iTimeSinceEpochInMs) / 1000.0f);
}


bool DirectxHandler_t::_CompileShaders(LPDIRECT3DDEVICE9 pDevice)
{
    if (m_pEffect != nullptr)
        return true;

    LPD3DXBUFFER pErrorBuffer = nullptr;
    HRESULT      pShaderCompilationResult = D3DXCreateEffect(
        pDevice,
        szCommonShader,
        sizeof(szCommonShader),
        nullptr,
        NULL,
        NULL,
        nullptr,
        &m_pEffect,
        &pErrorBuffer
    );

    if (FAILED(pShaderCompilationResult) == true)
    {
        FAIL_LOG("Failed to compile shaders. ERROR : %s", (char*)pErrorBuffer->GetBufferPointer());
        return false;
    }

    return true;
}


bool DirectxHandler_t::_DeclareVertex(LPDIRECT3DDEVICE9 pDevice)
{
    auto declResult = pDevice->CreateVertexDeclaration(vertexDecl, &m_pVertexDecl);
    if (FAILED(declResult) == true)
        return false;

    return true;
}


void DirectxHandler_t::Draw(DrawObjList_t& drawlist, LPDIRECT3DDEVICE9 pDevice)
{
    // Safety checks
    assert(drawlist.m_iVertexPerObject > 0 && "Vertex per object is less than 0 you nigger!, this will cause bullshit");

    // Initialized yet?
    if (pDevice == nullptr || m_bInitialized == false)
        return;

    // Got anything to draw ?
    if (drawlist.m_nVertcies <= 0)
        return;

    // Fixxing size of vertex buffer before dumping data into the buffer
    for (int i = 0; i < 10 && drawlist.m_nBufferSize <= drawlist.m_nVertcies * drawlist.m_iStride; i++)
    {
        if (drawlist.DoubleBufferSize(pDevice) == false)
        {
            FAIL_LOG("FAILED TO INCEASE BUFFER SIZE");
            return;
        }
    }

    // If size is still not enough, then no drawing. ( we will again try to increase next frame & hopefully it will be enough next frame)
    if (drawlist.m_nBufferSize <= drawlist.m_nVertcies * drawlist.m_iStride)
        return;

    // Locking up our buffer
    Vertex_t* pBuffer     = nullptr;
    auto      bLockResult = drawlist.m_pVertexBuffer->Lock(0, drawlist.m_iStride * drawlist.m_nVertcies, reinterpret_cast<void**>(&pBuffer), D3DLOCK_DISCARD);
    if (FAILED(bLockResult) == true || pBuffer == nullptr)
    {
        FAIL_LOG("FAILED LOCKING BUFFER");
        return;
    }

    // Dumping data into the buffer.
    auto     now        = std::chrono::high_resolution_clock::now();
    uint32_t iCopyIndex = 0;
    std::vector<std::string> m_vecExpiredObjs = {};

    for (const auto& [ID, drawObj] : drawlist.m_mapIDToObj)
    {
        uint32_t iTimeSinceLastUpdate = std::chrono::duration_cast<std::chrono::milliseconds>(now - drawObj->m_lastUpdateTime).count();

        // Deleting this obj if life ended.
        if (iTimeSinceLastUpdate > drawObj->m_flLife)
        {
            m_vecExpiredObjs.push_back(ID);
            continue;
        }

        // Actual data copy here.
        memcpy(&pBuffer[iCopyIndex], drawObj->GetVertcies(), drawlist.m_iStride * drawObj->GetVertexCount());
        iCopyIndex += drawObj->GetVertexCount();
    }

    // Deleting expired draw objects ( if we delete them in the map loop (above), it will crash )
    for (const std::string& ID : m_vecExpiredObjs)
    {
        drawlist.RemoveDrawObj(ID);
    }

    // Unlocking buffer once filled up.
    drawlist.m_pVertexBuffer->Unlock();

    // Drawing
    m_pEffect->SetTechnique("HardRedPixels");
    m_pEffect->Begin(nullptr, 0);
    m_pEffect->BeginPass(0);

    pDevice->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);
    //pDevice->SetRenderState(D3DRS_MULTISAMPLEANTIALIAS, TRUE);
    //pDevice->SetRenderState(D3DRS_ANTIALIASEDLINEENABLE, TRUE);

    pDevice->SetStreamSource(0, drawlist.m_pVertexBuffer, 0, drawlist.m_iStride);
    pDevice->SetVertexDeclaration(m_pVertexDecl);
    pDevice->DrawPrimitive(drawlist.m_iDrawType, 0, drawlist.m_nVertcies / drawlist.m_iVertexPerObject);

    m_pEffect->EndPass();
    m_pEffect->End();
}
