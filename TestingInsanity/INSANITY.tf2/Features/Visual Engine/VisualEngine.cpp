//=========================================================================
//                      INSANE OVERLAY
//=========================================================================
// by      : INSANE
// created : 07/07/2025
// 
// purpose : Draws stuff using shaders :)
//-------------------------------------------------------------------------
#include "VisualEngine.h"

#include <d3d9.h>
#include <d3dx9.h>
#include <vector>

// SDK
#include "../../SDK/class/Basic Structures.h"
#include "../../SDK/class/IVEngineClient.h"

// UTILITY
#include "../../Utility/ConsoleLogging.h"
#include "../../Extra/math.h"


//=========================================================================
//                     PUBLIC METHODS
//=========================================================================
void VisualEngine_t::Initialize(LPDIRECT3DDEVICE9 pDevice)
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


void VisualEngine_t::SetW2SMatrix()
{
    if (m_pEffect == nullptr)
        return;

    m_pEffect->SetMatrix("g_worldToScreen",reinterpret_cast<D3DXMATRIX*>(&I::iEngine->WorldToScreenMatrix()));
}


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


bool VisualEngine_t::_DeclareVertex(LPDIRECT3DDEVICE9 pDevice)
{
    auto declResult = pDevice->CreateVertexDeclaration(vertexDecl, &m_pVertexDecl);
    if (FAILED(declResult) == true)
        return false;

    return true;
}


void VisualEngine_t::Draw(DrawObjList_t& drawlist, LPDIRECT3DDEVICE9 pDevice)
{

    // Initialized yet?
    if (pDevice == nullptr || m_bInitialized == false)
        return;

    // Got anything to draw
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


    // Locking up our buffer
    Vertex_t* pBuffer     = nullptr;
    auto      bLockResult = drawlist.m_pVertexBuffer->Lock(0, drawlist.m_iStride * drawlist.m_nVertcies, reinterpret_cast<void**>(&pBuffer), D3DLOCK_DISCARD);
    if (FAILED(bLockResult) == true || pBuffer == nullptr)
    {
        FAIL_LOG("FAILED LOCKING BUFFER");
        return;
    }


    // Dumping data into the buffer.
    auto now = std::chrono::high_resolution_clock::now();
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

    pDevice->SetStreamSource(0, drawlist.m_pVertexBuffer, 0, drawlist.m_iStride);
    pDevice->SetVertexDeclaration(m_pVertexDecl);
    pDevice->DrawPrimitive(drawlist.m_iDrawType, 0, drawlist.m_nVertcies / drawlist.m_iVertexPerObject);

    m_pEffect->EndPass();
    m_pEffect->End();
}


//=========================================================================
//                     DRAWING HANDLER
//=========================================================================
void InsaneOverlay_t::Run(LPDIRECT3DDEVICE9 pDevice)
{
    // Setting up
    F::visualEngine.Initialize(pDevice);
    F::visualEngine.CaptureState();

    // Drawing
    m_lineList.m_bBeingDrawn.store(true);
    F::visualEngine.Draw(m_lineList, pDevice);
    m_lineList.m_bBeingDrawn.store(false);

    // Restoring
    F::visualEngine.RestoreState();
}


void InsaneOverlay_t::DrawRect(std::string szID, const vec& vMin, const vec& vMax, float flLife)
{
    auto it = m_lineList.m_mapIDToObj.find(szID);
    if (it == m_lineList.m_mapIDToObj.end())
    {
        RectDrawObj_t* drawObj = CreateAndRegisterDrawObj<RectDrawObj_t>(szID, m_lineList);
        drawObj->Set(vMin, vMax, flLife);

        WIN_LOG("Created new BOX with ID [ %s ]", szID.c_str());

        return;
    }

    // No updating if locked
    if (it->second->IsLocked() == true)
        return;

    auto* drawObj = static_cast<RectDrawObj_t*>(it->second);
    drawObj->Set(vMin, vMax, flLife);
}


void InsaneOverlay_t::FreeAllDrawObjs()
{
    m_lineList.Free();
}

void InsaneOverlay_t::FindAndDelete(std::string szID)
{
    // if not being drawn, then we can find & remove stuff from this map from anywhere without 
    // worrying about race conditions.
    if(m_lineList.m_bBeingDrawn.load() == false)
    {
        m_lineList.RemoveDrawObj(szID);
    }
}



DrawObjList_t::DrawObjList_t(D3DPRIMITIVETYPE iDrawType, std::string szName, uint32_t iStride)
{
    assert((iDrawType == D3DPT_LINELIST || iDrawType == D3DPT_TRIANGLELIST) && "Draw primitive type ain't supported my nigga");

    m_iDrawType = iDrawType;
    m_szName    = szName;
    m_iStride   = iStride;

    switch (m_iDrawType)
    {
    case D3DPT_LINELIST:     m_iVertexPerObject = 2; break;
    case D3DPT_TRIANGLELIST: m_iVertexPerObject = 3; break;
    default: break;
    }

    m_bBeingDrawn.store(false);
}


bool DrawObjList_t::DoubleBufferSize(LPDIRECT3DDEVICE9 pDevice)
{
    assert(m_nBufferSize >= 0 && "Vertex buffer size is negative!");

    if (m_pVertexBuffer != nullptr)
    {
        m_pVertexBuffer->Release();
    }

    uint32_t iTargetSize = m_nBufferSize == 0 ? 1 : (m_nBufferSize * 2U);

    if (FAILED(pDevice->CreateVertexBuffer(iTargetSize, D3DUSAGE_WRITEONLY, 0, D3DPOOL_DEFAULT, &m_pVertexBuffer, nullptr)) == true)
        return false;

    m_nBufferSize = iTargetSize;

    // Delete this
    LOG("Updated buffer size : %u", m_nBufferSize);
    
    if (m_nBufferSize >= MAX_VERTEX_BUFFER_SIZE_InBytes)
    {
        FAIL_LOG("%s  Buffer size is getting too big. [ %u ]", m_szName.c_str(), m_nBufferSize);
    }

    return true;
}

bool DrawObjList_t::HalfBufferSize(LPDIRECT3DDEVICE9 pDevice)
{
    assert(m_nBufferSize >= 0 && "Vertex buffer size is negative!");

    if (m_pVertexBuffer != nullptr)
    {
        m_pVertexBuffer->Release();
        m_pVertexBuffer = nullptr;
    }

    if (FAILED(pDevice->CreateVertexBuffer(m_nBufferSize / 2u, D3DUSAGE_WRITEONLY, 0, D3DPOOL_DEFAULT, &m_pVertexBuffer, nullptr)) == true)
        return false;

    m_nBufferSize /= 2u;

    // Delete this
    LOG("Updated buffer size : %u", m_nBufferSize);

    return true;
}


void DrawObjList_t::RemoveDrawObj(std::string szID)
{
    auto it = m_mapIDToObj.find(szID);
    if (it == m_mapIDToObj.end())
        return;

    m_nVertcies -= it->second->GetVertexCount();
    
    FAIL_LOG("Deleted draw obj [ %s ]", szID.c_str());

    delete it->second;
    m_mapIDToObj.erase(it);

    assert(m_nVertcies >= 0 && "Vertices counter is negative, something went wrong");
}


//=========================================================================
//                     DRAW OBJECTS
//=========================================================================
RectDrawObj_t::RectDrawObj_t(const vec& vMin, const vec& vMax)
{
    m_vMin = vMin;
    m_vMax = vMax;

    m_flLife  = -1.0f;
    m_bLocked = false;

    // Base constructor set the creation time.
    BaseDrawObj_t();
}

void RectDrawObj_t::Set(const vec& vMin, const vec& vMax, float flLife)
{
    UpdateTime();

    m_vMin = vMin;
    m_vMax = vMax;

    // Setting up all 8 coordintes of this box.
    m_vecVertcies[0] = vMin;
    m_vecVertcies[7] = vMin; // Last index
    
    m_vecVertcies[3] = vMax;
    m_vecVertcies[4] = vMax;

    // NOTE : Z is from max & X-Y are from min
    m_vecVertcies[1] = { vMin.x, vMin.y, vMax.z };
    m_vecVertcies[2] = { vMin.x, vMin.y, vMax.z };

    // NOTE : Z is from max & X-Y are from min
    m_vecVertcies[5] = { vMax.x, vMax.y, vMin.z };
    m_vecVertcies[6] = { vMax.x, vMax.y, vMin.z };

    // storing new life
    if (flLife >= 0.0f)
        m_flLife = flLife;
}
