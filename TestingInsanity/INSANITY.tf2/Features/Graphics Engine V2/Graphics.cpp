#include "Graphics.h"

// Utility
#include "../../Utility/ConsoleLogging.h"

// SDK
#include "../../SDK/class/IVEngineClient.h"

//DrawObjs
#include "Draw Objects/BaseDrawObj.h"


///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
void Graphics_t::Run(LPDIRECT3DDEVICE9 pDevice)
{
    // Initialzing ( compiling shader & more ).
    if (_Initialize(pDevice) == false)
        return;
    
    _InitShaderVariables(pDevice);

    m_pStateBlock->Capture();
    
    _DrawList(&m_lines, pDevice);
    _DrawList(&m_traingles, pDevice);

    m_pStateBlock->Apply();
}


///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
void Graphics_t::Free()
{
    m_lines.FreeBufferAndDeleteDrawObj();
    m_traingles.FreeBufferAndDeleteDrawObj();

    if (m_pEffect != nullptr)
    {
        m_pEffect->Release();
        m_pEffect = nullptr;
        LOG("Released shader.");
    }

    if (m_pStateBlock != nullptr)
    {
        m_pStateBlock->Release();
        m_pStateBlock = nullptr;
        LOG("Released state block.");
    }

    if (m_pVertexDecl != nullptr)
    {
        m_pVertexDecl->Release();
        m_pVertexDecl = nullptr;
        LOG("Release vertex decleration.");
    }

    if (m_pSceneTexture != nullptr)
    {
        m_pSceneTexture->Release();
        m_pSceneTexture = nullptr;
        LOG("Released scene texture.");
    }

    if (m_pSceneSurface != nullptr)
    {
        m_pSceneSurface->Release();
        m_pSceneSurface = nullptr;
        LOG("Released scene surface.");
    }
}


///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
void Graphics_t::CaptureW2SMatrix()
{
    if (I::iEngine->IsInGame() == false)
        return;

    m_vW2SMatrix = I::iEngine->WorldToScreenMatrix();
}


///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
void Graphics_t::RegisterInLineList(IDrawObj_t* pDrawObj)
{
    m_lines.RegisterDrawObj(pDrawObj);
    
    LOG("Registered traingle list object. Verticies [ %d ]", pDrawObj->GetVertexCount());
}


///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
void Graphics_t::RegisterInTraingleList(IDrawObj_t* pDrawObj)
{
    m_traingles.RegisterDrawObj(pDrawObj);

    LOG("Registered traingle list object. Verticies [ %d ]", pDrawObj->GetVertexCount());
}


///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
void Graphics_t::FindAndRemoveDrawObj(IDrawObj_t* pDrawObj)
{
    // Searching in line list first.
    if (m_lines.FindAndRemoveDrawObj(pDrawObj) == true)
        return;

    // If not found in line list, try to find in Traingle list.
    m_traingles.FindAndRemoveDrawObj(pDrawObj);
}


///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
bool Graphics_t::_Initialize(LPDIRECT3DDEVICE9 pDevice)
{
    // Compile Shader...
    if (m_bShaderCompiled == false)
    {
        m_bShaderCompiled = _CompileShader(pDevice);
        
        if (m_bShaderCompiled == false)
            return false;
    }

    // Declare Vertex...
    if (m_bVertexDecl == false)
    {
        m_bVertexDecl = _DeclareVertex(pDevice);

        if (m_bVertexDecl == false)
            return false;
    }

    // Initialize texture / surface n shit
    if (m_bTextureInit == false)
    {
        m_bTextureInit = _CreateTexture(pDevice);

        if (m_bTextureInit == false)
            return false;
    }

    // Creating state block
    if (m_bStateBlockInit == false)
    {
        m_bStateBlockInit = _CreateStateBlock(pDevice);

        if (m_bStateBlockInit == false)
            return false;
    }

    return true;
}


///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
bool Graphics_t::_InitShaderVariables(LPDIRECT3DDEVICE9 pDevice)
{
    IDirect3DSurface9* pBackBuffer = nullptr;
    pDevice->GetBackBuffer(0, 0, D3DBACKBUFFER_TYPE_MONO, &pBackBuffer);
    pDevice->StretchRect(pBackBuffer, nullptr, m_pSceneSurface, nullptr, D3DTEXF_NONE);
    pBackBuffer->Release();

    m_pEffect->SetTexture("SceneTex", m_pSceneTexture);

    int iScreenHeight = 0; int iScreenWidth = 0; I::iEngine->GetScreenSize(iScreenWidth, iScreenHeight);
    m_pEffect->SetFloat("g_flScreenHeight", static_cast<float>(iScreenHeight));
    m_pEffect->SetFloat("g_flScreenWidth",  static_cast<float>(iScreenWidth));
    m_pEffect->SetFloat("g_flIsInGame",    I::iEngine->IsInGame() == true ? 1.0f : 0.0f); // Let the shader know if we are in game or not.

    // Here I am trying to retain some precision cause static_cast<double>(iTimeSinceEpochInMs) is a big number and storing in a float, will cause
    // poor accuracy cause of poor step size, so we reduce the convert it to seconds first, so its a smaller number and then store it in float, so
    // we don't lose as much accuracy cause me a smart ass nigga :)
    int64_t iTimeSinceEpochInMs = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now().time_since_epoch()).count();
    m_pEffect->SetFloat("g_flTimeSinceEpochInSec", static_cast<float>(static_cast<double>(iTimeSinceEpochInMs) / 1000.0));

    if (I::iEngine->IsInGame() == true)
    {
        m_pEffect->SetMatrix("g_worldToScreen", reinterpret_cast<D3DXMATRIX*>(&m_vW2SMatrix));
    }

    return true;
}


///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
void Graphics_t::_DrawList(VertexBuffer_t* pData, LPDIRECT3DDEVICE9 pDevice)
{
    if (pData->m_free.load() == false)
        return;

    if (pData->m_vecDrawObjs.empty() == true)
        return;

    pData->m_free.store(false);

    bool bIsInGame = I::iEngine->IsInGame();
    // Total number of vertex to dump to buffer.
    uint64_t nVertex = 0;
    {
        for (const IDrawObj_t* pDrawObj : pData->m_vecDrawObjs)
        {
            // Don't draw objects that don't wanna be drawn in game.
            if (bIsInGame == true && pDrawObj->ShouldDrawInGame() == false)
                continue;

            // Don't draw 3D objects & objects don't wanna be drawn in lobby ( like FOV circles n shit )
            if (bIsInGame == false && pDrawObj->ShouldDrawInLobby() == false)
                continue;
            
            nVertex += pDrawObj->GetVertexCount();
        }
    }

    if (nVertex <= 0)
    {
        pData->m_free.store(true);
        return;
    }

    // Make sure buffer size is sufficient.
    uint64_t iVertexDataSize = sizeof(Vertex) * nVertex;
    if (pData->AdjustBufferSize(iVertexDataSize, pDevice) == false)
        return;

    // Lock buffer.
    void* pBuffer     = nullptr;
    auto  bLockResult = pData->m_pBuffer->Lock(0, iVertexDataSize, reinterpret_cast<void**>(&pBuffer), D3DLOCK_DISCARD);
    if (FAILED(bLockResult) == true || pBuffer == nullptr)
    {
        FAIL_LOG("FAILED LOCKING BUFFER");
        pData->m_free.store(true);
        return;
    }

    // Write data to buffer.
    for (IDrawObj_t* pDrawObj : pData->m_vecDrawObjs)
    {
        // Don't draw objects that don't wanna be drawn in game.
        if (bIsInGame == true && pDrawObj->ShouldDrawInGame() == false)
            continue;

        // Don't draw 3D objects & objects don't wanna be drawn in lobby ( like FOV circles n shit )
        if (bIsInGame == false && pDrawObj->ShouldDrawInLobby() == false)
            continue;

        uint64_t nBytesToWrite = sizeof(Vertex) * pDrawObj->GetVertexCount();
        memcpy(pBuffer, pDrawObj->GetVertexData(), nBytesToWrite);
    
        pBuffer = reinterpret_cast<void*>(reinterpret_cast<uintptr_t>(pBuffer) + nBytesToWrite);
    }

    pData->m_pBuffer->Unlock();

    // Finally, draw whatever is in the buffer.
    {
        pDevice->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);
        pDevice->SetVertexDeclaration(m_pVertexDecl);
        pDevice->SetStreamSource(0, pData->m_pBuffer, 0, sizeof(Vertex));

        m_pEffect->SetTechnique("simple1");
        UINT numPasses = 0;
        m_pEffect->Begin(&numPasses, 0);

        m_pEffect->BeginPass(0);
        pDevice->DrawPrimitive(pData->m_iPrimitiveType, 0, nVertex / pData->m_iVertexPerPrimitive);
        m_pEffect->EndPass();

        m_pEffect->End();
    }

    //LOG("Drew [ %d ] objects", nVertex / pData->m_iVertexPerPrimitive);

    pData->m_free.store(true);
}


///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
bool Graphics_t::_CompileShader(LPDIRECT3DDEVICE9 pDevice)
{
    LPD3DXBUFFER pErrorBuffer = nullptr;
    HRESULT      pShaderCompilationResult = D3DXCreateEffect(
        pDevice,
        szShader,
        sizeof(szShader),
        nullptr,
        NULL,
        NULL,
        nullptr,
        &m_pEffect,
        &pErrorBuffer
    );

    if (FAILED(pShaderCompilationResult) == true)
    {
        m_pEffect = nullptr;
        FAIL_LOG("Failed to compile shaders. ERROR : %s", (char*)pErrorBuffer->GetBufferPointer());
        return false;
    }

    WIN_LOG("Compile shader successfully");
    return m_pEffect != nullptr;
}


///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
bool Graphics_t::_DeclareVertex(LPDIRECT3DDEVICE9 pDevice)
{
    static _D3DVERTEXELEMENT9 vertexDecl[] =
    {
        {0, 0,                                                              D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION, 0}, // pos     
        {0, sizeof(vec),                                                    D3DDECLTYPE_FLOAT4, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_COLOR   , 0}, // color   
        {0, sizeof(vec) + (sizeof(float) * 4),                              D3DDECLTYPE_FLOAT1, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 0}, // Blur Amm
        {0, sizeof(vec) + (sizeof(float) * 4) + 4,                          D3DDECLTYPE_FLOAT1, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 1}, // Rounding
        {0, sizeof(vec) + (sizeof(float) * 4) + 4 + 4,                      D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 2}, // rel. UV 
        {0, sizeof(vec) + (sizeof(float) * 4) + 4 + 4 + 12,                 D3DDECLTYPE_FLOAT1, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 3}, // 2D  
        {0, sizeof(vec) + (sizeof(float) * 4) + 4 + 4 + 12 + 4 ,            D3DDECLTYPE_FLOAT1, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 4}, // Invert colors  
        {0, sizeof(vec) + (sizeof(float) * 4) + 4 + 4 + 12 + 4 + 4,         D3DDECLTYPE_FLOAT1, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 5}, // scale Y 
        {0, sizeof(vec) + (sizeof(float) * 4) + 4 + 4 + 12 + 4 + 4 + 4,     D3DDECLTYPE_FLOAT1, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 6}, // circle thickness.
        {0, sizeof(vec) + (sizeof(float) * 4) + 4 + 4 + 12 + 4 + 4 + 4 + 4, D3DDECLTYPE_FLOAT1, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 7}, // RGB anim speed.
        D3DDECL_END()
    };

    if (FAILED(pDevice->CreateVertexDeclaration(vertexDecl, &m_pVertexDecl)) == true)
    {
        m_pVertexDecl = nullptr;
        FAIL_LOG("Failed to create vertex decleratoin");
        return false;
    }

    WIN_LOG("Create vertex decleration successfully");
    return m_pVertexDecl != nullptr;
}


///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
bool Graphics_t::_CreateTexture(LPDIRECT3DDEVICE9 pDevice)
{
    int iHeight = 0, iWidth = 0; I::iEngine->GetScreenSize(iWidth, iHeight);
    HRESULT bResult = pDevice->CreateTexture(
        iWidth,
        iHeight,
        1,
        D3DUSAGE_RENDERTARGET,
        D3DFMT_A8R8G8B8,
        D3DPOOL_DEFAULT,
        &m_pSceneTexture,
        nullptr
    );

    if (FAILED(bResult) == true)
    {
        m_pSceneTexture = nullptr;
        FAIL_LOG("Faild to create texture");
        return false;
    }

    // Grab surface level from the texture.
    HRESULT bSurfaceResult = m_pSceneTexture->GetSurfaceLevel(0, &m_pSceneSurface);
    if (FAILED(bSurfaceResult) == true)
    {
        m_pSceneSurface = nullptr; m_pSceneTexture = nullptr;
        FAIL_LOG("Failed to get surface level");
        return false;
    }

    return m_pSceneSurface != nullptr && m_pSceneTexture != nullptr;
}


///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
bool Graphics_t::_CreateStateBlock(LPDIRECT3DDEVICE9 pDevice)
{
    if (FAILED(pDevice->CreateStateBlock(D3DSBT_ALL, &m_pStateBlock)) == true)
    {
        m_pStateBlock = nullptr;
        FAIL_LOG("Bad state block");
        return false;
    }

    WIN_LOG("Created state block successfully");
    return m_pStateBlock != nullptr;
}


///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
bool VertexBuffer_t::AdjustBufferSize(uint64_t iSizeRequired, LPDIRECT3DDEVICE9 pDevice)
{
    // if we already got this much space.
    if (m_iBufferSize.load() > iSizeRequired)
        return true;

    // Calculating the smallest power of 2 that is more than size required. Incrementing size like this reduces reallocation times.
    iSizeRequired = static_cast<uint64_t>(powf(2.0f, ceilf(static_cast<float>(log2(iSizeRequired))) ));

    // Warning size.
    if (iSizeRequired >= 10ull * 0x400ull)
        FAIL_LOG("Buffer size more than 10KiBs, we are drawing [ %d ] objects. is this normal", m_vecDrawObjs.size());

    // Release old buffer first.
    if (m_pBuffer != nullptr)
        m_pBuffer->Release();

    HRESULT bAllocResult = pDevice->CreateVertexBuffer(
        iSizeRequired, // size to allocate
        D3DUSAGE_DYNAMIC | D3DUSAGE_WRITEONLY,
        0,
        D3DPOOL_DEFAULT,
        &m_pBuffer,
        nullptr
    );

    if (FAILED(bAllocResult) == true)
    {
        FAIL_LOG("Failed vertex buffer allocation for size [ %llu ]", iSizeRequired);
        m_pBuffer = nullptr;
        return false;
    }

    WIN_LOG("Updated buffer size to [ %llu ]", iSizeRequired);
    m_iBufferSize.store(iSizeRequired);
    return true;
}


///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
void VertexBuffer_t::FreeBufferAndDeleteDrawObj()
{
    while (m_free.load() == false)
    {
        FAIL_LOG("Waiting for vertex buffer data to be free-ed.");
    }

    m_free.store(false);
    
    if (m_pBuffer != nullptr)
    {
        m_pBuffer->Release();
        m_pBuffer = nullptr;
    }

    // Delete and clear all draw objects.
    for (IDrawObj_t* pDrawObj : m_vecDrawObjs)
    {
        pDrawObj->DeleteThis();
        FAIL_LOG("Delete draw obj : %p", pDrawObj);
    }
    m_vecDrawObjs.clear();
    m_vecDrawObjs.shrink_to_fit();
    
    // Delete and clear all temp draw obj.
    for (IDrawObj_t* pDrawObj : m_vecTempBuffer)
    {
        pDrawObj->DeleteThis();
        FAIL_LOG("Delete temp draw obj : %p", pDrawObj);
    }
    m_vecTempBuffer.clear();
    m_vecTempBuffer.shrink_to_fit();

    // No setting the mutex to true, cause we don't want it to be used anymore.
}


///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
void VertexBuffer_t::RegisterDrawObj(IDrawObj_t* pDrawObj)
{
    // if draw obj list is not free, the leave obj in temp buffer.
    if (m_free.load() == false)
    {
        m_vecTempBuffer.push_back(pDrawObj);
        return;
    }
    
    // if draw obj list is free, then store it and if anythings left in temp buffer.
    m_free.store(false);
    m_vecDrawObjs.push_back(pDrawObj);

    // Dump everything from temp storage to main obj list.
    for (IDrawObj_t* pObj : m_vecTempBuffer)
        m_vecDrawObjs.push_back(pObj);

    m_vecTempBuffer.clear();

    m_free.store(true);
}


///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
bool VertexBuffer_t::FindAndRemoveDrawObj(IDrawObj_t* pDrawObj)
{
    auto it = std::find(m_vecDrawObjs.begin(), m_vecDrawObjs.end(), pDrawObj);

    // Trying to find obj in draw obj list.
    if (it != m_vecDrawObjs.end())
    {
        LOG("Removed draw obj @ %p", pDrawObj);
        m_vecDrawObjs.erase(it);
        return true;
    }

    // Trying to find obj in temp list.
    auto itTempList = std::find(m_vecTempBuffer.begin(), m_vecTempBuffer.end(), pDrawObj);
    if (itTempList != m_vecTempBuffer.end())
    {
        LOG("Removed draw obj from temp list @ %p", pDrawObj);
        m_vecTempBuffer.erase(itTempList);
        return true;
    }

    // Draw obj is not present in this vertexBuffer obj.
    return false;
}
