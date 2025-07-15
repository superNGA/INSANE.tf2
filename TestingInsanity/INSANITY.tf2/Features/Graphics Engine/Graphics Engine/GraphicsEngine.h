#pragma once

#include <d3d9.h>
#include <d3dx9.h>
#include <atomic>
#include <string>
#include <unordered_map>


#include "../Draw Objects/DrawObj.h"
#include "../../FeatureHandler.h"
#include "../../../SDK/class/Basic Structures.h"


struct DrawObjList_t
{
    DrawObjList_t(D3DPRIMITIVETYPE iDrawType, std::string szName, uint32_t iStride);

    // DirectX9 stuff
    D3DPRIMITIVETYPE m_iDrawType;
    uint32_t         m_iStride = sizeof(Vertex_t);
    uint32_t         m_iVertexPerObject = 0;

    std::atomic<bool> m_bBeingDrawn;

    // Stores all objects
    std::unordered_map<std::string, BaseDrawObj_t*> m_mapIDToObj = {};

    // NOTE : its very important that this Verticies count is correct. else 
    //          a nothing will draw correctly.
    // This is only incremented when an object is created & 
    // only decremented in the RemoveDrawObj fn.
    uint32_t m_nVertcies = 0;

    // Vertex buffer & its stats
    uint32_t                  m_nBufferSize = 0;
    IDirect3DVertexBuffer9*   m_pVertexBuffer = nullptr;
    static constexpr uint32_t MAX_VERTEX_BUFFER_SIZE_InBytes = 10 * 1024; // 10 KiBs

    bool DoubleBufferSize(LPDIRECT3DDEVICE9 pDevice);
    bool HalfBufferSize(LPDIRECT3DDEVICE9 pDevice);

    void RemoveDrawObj(std::string szID);
    void Free();

    std::string m_szName = "UNDEFINED";
};



//=========================================================================
//                     GRAPHICS ENGINE
//=========================================================================
class GraphicsEngine_t
{
public:
    GraphicsEngine_t() :
        m_lineList(D3DPT_LINELIST,    "LineList",      sizeof(Vertex_t)),
        m_triList(D3DPT_TRIANGLELIST, "Traingle List", sizeof(Vertex_t))
    {}

    // All rendering happens here...
    void Run(LPDIRECT3DDEVICE9 pDevice);

    // Drawing different stuff    
    BaseDrawObj_t* DrawRect(std::string szID,
        const vec& vMin,
        const vec& vMax,
        const qangle& qNormal,
        const float flLife = DEFAULT_LIFE_IN_MS, 
        GraphicInfo_t* pGraphicInfo = nullptr);

    BaseDrawObj_t* DrawBox(std::string szID,
        const vec& vMin,
        const vec& vMax,
        const qangle& qNormal,
        const float flLife = DEFAULT_LIFE_IN_MS,
        GraphicInfo_t* pGraphicInfo = nullptr);
    
    BaseDrawObj_t* DrawLine(std::string szID,
        const vec& vMin,
        const vec& vMax,
        const qangle& qNormal,
        const float flLife = DEFAULT_LIFE_IN_MS,
        GraphicInfo_t* pGraphicInfo = nullptr);

    void DrawLineList(std::string szID,
        const std::vector<vec&>& vecPoints,
        const qangle& qNormal, const bool bLock,
        const float flLife = DEFAULT_LIFE_IN_MS, GraphicInfo_t* pGraphicInfo = nullptr);

    // Deleting stuff
    void FreeAllDrawObjs();
    void FindAndDelete(std::string szID);

private:
    template <typename T>
    inline T* CreateAndRegisterDrawObj(std::string& szID, DrawObjList_t& drawObjList)
    {
        T* pNewDrawObj = new T;
        drawObjList.m_mapIDToObj.insert({ szID, pNewDrawObj });
        drawObjList.m_nVertcies += reinterpret_cast<BaseDrawObj_t*>(pNewDrawObj)->GetVertexCount();

        return pNewDrawObj;
    }

    std::vector<std::vector<LineDrawObj_t*>> m_vecLineList = {};

    DrawObjList_t m_lineList;
    DrawObjList_t m_triList;


    static constexpr float DEFAULT_LIFE_IN_MS = 1000.0f;
};
DECLARE_FEATURE_OBJECT(graphicsEngine, GraphicsEngine_t)


/*

should I use lines to make line list or quads? quads will look cool. Like rally cool.
wiht RGB n shit. but lines will be cheaper. I think imma have to go with quads. but 
I must know the size at runtime.

I think I will have to create a shit tone of these line draw objs for my line list. 
this way I can very nicely manage the vertex count n stuff

*/