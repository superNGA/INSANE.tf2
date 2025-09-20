#include "Graphics.h"

// Utility
#include "../../Utility/ConsoleLogging.h"
#include "../ModelPreview/ModelPreview.h"
#include "../Material Gen/MaterialGen.h"

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
    
    _DrawList(&m_lines,     pDevice);
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
        LOG("Released vertex decleration.");
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

    m_renderTargetDup0.Free();
    m_renderTargetDup1.Free();
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
bool Graphics_t::FindAndRemoveDrawObj(IDrawObj_t* pDrawObj)
{
    // Searching in line list first.
    if (m_lines.FindAndRemoveDrawObj(pDrawObj) == true)
        return true;

    // If not found in line list, try to find in Traingle list.
    return m_traingles.FindAndRemoveDrawObj(pDrawObj);
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

    if (m_renderTargetDup0.m_bInit == false)
    {
        bool bSucceded = m_renderTargetDup0.Init(pDevice);
        if (bSucceded == false)
            return false;
    }

    if (m_renderTargetDup1.m_bInit == false)
    {
        bool bSucceded = m_renderTargetDup1.Init(pDevice);
        if (bSucceded == false)
            return false;
    }

    return true;
}


///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
bool Graphics_t::_InitShaderVariables(LPDIRECT3DDEVICE9 pDevice)
{
    if (m_bStateBlockInit == false || m_bShaderCompiled == false || m_bStateBlockInit == false || m_bVertexDecl == false)
        return false;


    D3DSurface* pBackBuffer = nullptr;  
    pDevice->GetBackBuffer(0, 0, D3DBACKBUFFER_TYPE_MONO, &pBackBuffer);
    
    RenderTargetDup_t* pBestCapture = I::iEngine->IsInGame() == false ? &m_renderTargetDup0 : &m_renderTargetDup1;
    pBestCapture->DumpCapture(pDevice, m_pSceneSurface);

    if(directX::UI::UI_visble == true || F::materialGen.IsVisible() == true)
    {
        _DumpCaptureAndMaskModelPanel(pDevice, pBestCapture->m_pSurface, pBackBuffer);
    }
    else
    {
        pBestCapture->DumpCapture(pDevice, pBackBuffer);
    }

    pBackBuffer->Release();
    m_pEffect->SetTexture("SceneTex", m_pSceneTexture);


    int iScreenHeight = 0; int iScreenWidth = 0; I::iEngine->GetScreenSize(iScreenWidth, iScreenHeight);
    m_pEffect->SetFloat("g_flScreenHeight", static_cast<float>(iScreenHeight));
    m_pEffect->SetFloat("g_flScreenWidth",  static_cast<float>(iScreenWidth));
    m_pEffect->SetFloat("g_flIsInGame",    I::iEngine->IsInGame() == true ? 1.0f : 0.0f); // Let the shader know if we are in game or not.


    // Here I am trying to retain some precision cause "static_cast<double>(iTimeSinceEpochInMs)" is a big number and storing in a float, will cause
    // poor accuracy cause of poor step size, so we reduce the convert it to seconds first, so its a smaller number and then store it in float, so
    // we don't lose as much accuracy. me a smart ass nigga :)
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

    std::vector<IDrawObj_t*>* vecDrawObjs = pData->m_vecDrawObj.GetReadBuffer(true);
    VECTHREADSAFE_AUTO_RELEASE_READ_BUFFER(&pData->m_vecDrawObj, vecDrawObjs);

    if (vecDrawObjs->empty() == true)
        return;

    pData->m_free.store(false);

    bool bIsInGame = I::iEngine->IsInGame();
    // Total number of vertex to dump to buffer.
    uint64_t nVertex = 0;
    {
        for (const IDrawObj_t* pDrawObj : *vecDrawObjs)
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
    if (pData->AdjustBufferSize(iVertexDataSize, pDevice, vecDrawObjs->size()) == false)
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
    for (IDrawObj_t* pDrawObj : *vecDrawObjs)
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
        pDevice->SetRenderState(D3DRS_CULLMODE,              D3DCULL_NONE);
        pDevice->SetRenderState(D3DRS_MULTISAMPLEANTIALIAS,  TRUE);
        pDevice->SetRenderState(D3DRS_ANTIALIASEDLINEENABLE, TRUE);
        pDevice->SetRenderState(D3DRS_ALPHABLENDENABLE,      TRUE);

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
void Graphics_t::_DumpCaptureAndMaskModelPanel(D3DDevice* pDevice, D3DSurface* pSource, D3DSurface* pBackBuffer) const
{
    // Dumping "RenderTargetIndex" 0's data ( that we captur each frame starting from Present Fn ( PostCall )) into the back buffer,
    // such that the Model Preview Panel doesn't get hidden ( cause the model gets draw from Satan's ass somewhere before
    // Present call and the doesn't get captured in the data we captured ). This method is ass and probably 
    // not very good for performance, but its the only thing I could get to work so... its part of the final build. :)
    int iScreenWidth = 0, iScreenHeight = 0; I::iEngine->GetScreenSize(iScreenWidth, iScreenHeight);
    int x            = 0, y             = 0; F::modelPreview.GetRenderViewPos(x, y);
    int iWidth       = 0, iHeight       = 0; F::modelPreview.GetRenderViewSize(iHeight, iWidth);
    
    RECT leftMask  { 0,                    0,  x,             iScreenHeight };
    RECT RightMask { x + iWidth,           0,  iScreenWidth,  iScreenHeight };
    RECT TopMask   { x,                    0,  x + iWidth,    y };
    RECT BottomMask{ x,          y + iHeight,  x + iWidth,    iScreenHeight };
    
    pDevice->StretchRect(pSource, &leftMask,   pBackBuffer, &leftMask,   D3DTEXF_NONE);
    pDevice->StretchRect(pSource, &RightMask,  pBackBuffer, &RightMask,  D3DTEXF_NONE);
    pDevice->StretchRect(pSource, &TopMask,    pBackBuffer, &TopMask,    D3DTEXF_NONE);
    pDevice->StretchRect(pSource, &BottomMask, pBackBuffer, &BottomMask, D3DTEXF_NONE);
}


///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
bool VertexBuffer_t::AdjustBufferSize(uint64_t iSizeRequired, LPDIRECT3DDEVICE9 pDevice, int nObjects)
{
    // if we already got this much space.
    if (m_iBufferSize.load() > iSizeRequired)
        return true;

    // Calculating the smallest power of 2 that is more than size required. Incrementing size like this reduces reallocation times.
    iSizeRequired = static_cast<uint64_t>(powf(2.0f, ceilf(static_cast<float>(log2(iSizeRequired))) ));

    // Warning size.
    if (iSizeRequired >= 10ull * 0x400ull)
        FAIL_LOG("Buffer size more than 10KiBs, we are drawing [ %d ] objects. is this normal", nObjects);

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
    std::vector<IDrawObj_t*>* pDrawObjs = m_vecDrawObj.GetWriteBuffer(true);
    VECTHREADSAFE_AUTO_RELEASE_WRITE_BUFFER(&m_vecDrawObj, pDrawObjs);

    for (IDrawObj_t* pDrawObj : *pDrawObjs)
    {
        pDrawObj->DeleteThis();
        FAIL_LOG("Deleted draw obj [ %p ]", pDrawObj);
    }

    pDrawObjs->clear();
    pDrawObjs->shrink_to_fit();
}


///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
void VertexBuffer_t::RegisterDrawObj(IDrawObj_t* pDrawObj)
{
    m_vecDrawObj.PushBack(pDrawObj);
}


///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
bool VertexBuffer_t::FindAndRemoveDrawObj(IDrawObj_t* pDrawObj)
{
    std::vector<IDrawObj_t*>* pDrawObjs = m_vecDrawObj.GetWriteBuffer(true);
    VECTHREADSAFE_AUTO_RELEASE_WRITE_BUFFER(&m_vecDrawObj, pDrawObjs);

    auto it = std::find(pDrawObjs->begin(), pDrawObjs->end(), pDrawObj);
    if (it == pDrawObjs->end())
        return false;

    pDrawObjs->erase(it);
    FAIL_LOG("Removed draw object [ %p ]", pDrawObj);
    return true;
}


///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
bool RenderTargetDup_t::Init(D3DDevice* pDevice)
{
    if (pDevice == nullptr)
        return false;

    if (m_bInit == true)
        return true;

    int iWidth = 0, iHeight = 0; I::iEngine->GetScreenSize(iWidth, iHeight);
    HRESULT iTextureResult = pDevice->CreateTexture(
        iWidth, iHeight,
        1,
        D3DUSAGE_RENDERTARGET,
        D3DFMT_A8R8G8B8,
        D3DPOOL_DEFAULT,
        &m_pTexture,
        nullptr
    );

    // Did texture creation failed?
    if (FAILED(iTextureResult) == true || m_pTexture == nullptr)
    {
        FAIL_LOG("Failed to create Render Texture");
        m_pTexture = nullptr;
        return false;
    }

    // Getting surface from texture.
    HRESULT iSurfaceResult = m_pTexture->GetSurfaceLevel(0, &m_pSurface);
    if (FAILED(iSurfaceResult) == true || m_pSurface == nullptr)
    {
        FAIL_LOG("Failed to create Render Surface");
        m_pSurface = nullptr;
        return false;
    }

    WIN_LOG("Successfully create Render_Texture & Render_Surface for LEVEL [ %d ]", m_iLevel);
    m_bInit = true;
    return true;
}


///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
void RenderTargetDup_t::Free()
{
    if (m_bInit == false)
        return;

    if (m_pSurface != nullptr)
    {
        m_pSurface->Release();
        m_pSurface = nullptr;
        LOG("Release m_pSurface for RenderTargetDup Level [ %d ]", m_iLevel);
    }

    if (m_pTexture != nullptr)
    {
        m_pTexture->Release();
        m_pTexture = nullptr;
        LOG("Release m_pTexture for RenderTargetDup Level [ %d ]", m_iLevel);
    }
}


///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
void RenderTargetDup_t::StartCapture(D3DDevice* pDevice)
{
    if (m_bInit == false)
        return;

    m_pOriginalSurface = nullptr;
    pDevice->GetRenderTarget(m_iLevel, &m_pOriginalSurface);
    pDevice->SetRenderTarget(m_iLevel, m_pSurface);

    pDevice->Clear(0, nullptr, D3DCLEAR_TARGET, D3DCOLOR_RGBA(0, 0, 0, 0), 1.0f, 0);
}


///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
void RenderTargetDup_t::EndCapture(D3DDevice* pDevice)
{
    if (m_bInit == false || m_pOriginalSurface == nullptr)
        return;

    pDevice->SetRenderTarget(m_iLevel, m_pOriginalSurface);
    m_pOriginalSurface->Release(); // We got this in StartCapture() so to level the reference counter we gotta release this.
    m_pOriginalSurface = nullptr;
}


///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
void RenderTargetDup_t::DumpCapture(D3DDevice* pDevice, IDirect3DSurface9* pDest) const
{
    if (m_bInit == false)
        return;

    pDevice->StretchRect(m_pSurface, nullptr, pDest, nullptr, D3DTEXF_NONE);
}
