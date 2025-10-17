#include "GraphicsEngine.h"

#include <format>

#include "../../../Utility/ConsoleLogging.h"
#include "../../../Extra/math.h"

#include "../DirectX Handler/DirectXHandler.h"
#include "../../../Utility/Profiler/Profiler.h"


//=========================================================================
//                     GRAPHICS ENGINE
//=========================================================================

void GraphicsEngine_t::Run(LPDIRECT3DDEVICE9 pDevice)
{
    PROFILER_RECORD_FUNCTION(EndScene);

    // Setting up
    F::directxHandler.Initialize(pDevice);
    F::directxHandler.CaptureState();
    F::directxHandler.SetShaderTime();

    // Drawing line list
    //m_lineList.m_bBeingDrawn.store(true);
    //F::directxHandler.Draw(m_lineList, pDevice);
    //m_lineList.m_bBeingDrawn.store(false);

    // Drawing traingle list.
    m_triList.m_bBeingDrawn.store(true);
    F::directxHandler.Draw(m_triList, pDevice);
    m_triList.m_bBeingDrawn.store(false);

    // Restoring
    F::directxHandler.RestoreState();
}


BaseDrawObj_t* GraphicsEngine_t::DrawRect(std::string szID, const vec& vMin, const vec& vMax, const qangle& qNormal, const float flLife, GraphicInfo_t* pGraphicInfo)
{
    auto it = m_triList.m_mapIDToObj.find(szID);

    if (it == m_triList.m_mapIDToObj.end())
    {
        RectDrawObj_t* pDrawObj = CreateAndRegisterDrawObj<RectDrawObj_t>(szID, m_triList);
        if (pDrawObj == nullptr)
            return nullptr;

        pDrawObj->Set(vMin, vMax, qNormal);
        pDrawObj->SetLife(flLife);

        WIN_LOG("Created BOX with ID [ %s ]. Vertex Count -> [ %u ]", szID.c_str(), m_triList.m_nVertcies);

        return pDrawObj;
    }

    if (m_triList.m_bBeingDrawn.load() == true)
        return nullptr;

    if (it->second->IsLocked() == true)
        return it->second;

    reinterpret_cast<RectDrawObj_t*>(it->second)->Set( vMin, vMax, qNormal, pGraphicInfo);

    it->second->SetLife(flLife);
    return it->second;
}


BaseDrawObj_t* GraphicsEngine_t::DrawBox(std::string szID, const vec& vMin, const vec& vMax, const qangle& qNormal, const float flLife, GraphicInfo_t* pGraphicInfo)
{
    auto it = m_triList.m_mapIDToObj.find(szID);
    if (it == m_triList.m_mapIDToObj.end())
    {
        CuboidDrawObj_t* pDrawObj = CreateAndRegisterDrawObj<CuboidDrawObj_t>(szID, m_triList);
        if (pDrawObj == nullptr)
            return nullptr;

        pDrawObj->Set(vMin, vMax, qNormal, pGraphicInfo);
        pDrawObj->SetLife(flLife);
        return pDrawObj;
    }

    if (m_triList.m_bBeingDrawn.load() == true)
        return nullptr;

    if (it->second->IsLocked() == true)
        return it->second;

    reinterpret_cast<CuboidDrawObj_t*>(it->second)->Set(vMin, vMax, qNormal, pGraphicInfo);
    it->second->SetLife(flLife);
    return it->second;
}


BaseDrawObj_t* GraphicsEngine_t::DrawLine(std::string szID, const vec& vMin, const vec& vMax, const qangle& qNormal, const float flLife, GraphicInfo_t* pGraphicInfo)
{
    auto it = m_triList.m_mapIDToObj.find(szID);
    if (it == m_triList.m_mapIDToObj.end())
    {
        LineDrawObj_t* pDrawObj = CreateAndRegisterDrawObj<LineDrawObj_t>(szID, m_triList);
        if (pDrawObj == nullptr)
            return nullptr;

        pDrawObj->Set(vMin, vMax, qNormal, pGraphicInfo);
        pDrawObj->SetLife(flLife);
        return pDrawObj;
    }

    if (m_triList.m_bBeingDrawn.load() == true)
        return nullptr;

    if (it->second->IsLocked() == true)
        return it->second;

    reinterpret_cast<LineDrawObj_t*>(it->second)->Set(vMin, vMax, qNormal, pGraphicInfo);
    it->second->SetLife(flLife);
    return it->second;
}


void GraphicsEngine_t::FreeAllDrawObjs()
{
    m_lineList.Free();
    m_triList.Free();
}

void GraphicsEngine_t::FindAndDelete(std::string szID)
{
    // if not being drawn, then we can find & remove stuff from this map from anywhere without 
    // worrying about race conditions.
    if (m_lineList.m_bBeingDrawn.load() == false)
    {
        m_lineList.RemoveDrawObj(szID);
    }


    if (m_triList.m_bBeingDrawn.load() == false)
    {
        m_triList.RemoveDrawObj(szID);
    }
}



//=========================================================================
//                     DRAW OBJECT LIST
//=========================================================================

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



//-------------------------------------------------------------------------
// PURPOSE : Release the current vertex buffer & reallocates new space with 
//          double the size. Also checks for size validity & max size.
//-------------------------------------------------------------------------
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



//-------------------------------------------------------------------------
// PURPOSE : Release current vertex buffer & reallocates with half the size
//-------------------------------------------------------------------------
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



//-------------------------------------------------------------------------
// PURPOSE : Remove draw object from map & reduces vertex count accordingly.
//          Asserts vertex count.
//-------------------------------------------------------------------------
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



//-------------------------------------------------------------------------
// PURPOSE : Deletes all dynamically alloacted objects from the map,
//          ONLY sets necessary objects to NULL. Release vertex buffer.
//          Thread safe :)
//-------------------------------------------------------------------------
void DrawObjList_t::Free()
{
    while (m_bBeingDrawn.load() == true)
        FAIL_LOG("Waiting for frame end before deleting.");

    for (auto& [szID, drawObj] : m_mapIDToObj)
    {
        delete drawObj;
        m_mapIDToObj.erase(szID);
    }

    m_nVertcies = 0;
    m_nBufferSize = 0;

    if (m_pVertexBuffer != nullptr)
    {
        m_pVertexBuffer->Release();
    }
    m_pVertexBuffer = nullptr;

    LOG("Free draw object list [ %s ]", m_szName.c_str());
}