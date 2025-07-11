//=========================================================================
//                      INSANE OVERLAY
//=========================================================================
// by      : INSANE
// created : 07/07/2025
// 
// purpose : Draws stuff using shaders :)
//-------------------------------------------------------------------------
#pragma once

#include <unordered_map>
#include <vector>
#include <chrono>
#include <atomic>

#include "../FeatureHandler.h"
#include "../../SDK/class/Basic Structures.h"


// Forward declerations
class VisualEngine_t;
class InsaneOverlay_t;
class BaseDrawObj_t;
class RectDrawObj_t;
struct VEClrConfig_t;
struct VEGlowConfig_t;
struct DrawObjList_t;


//=========================================================================
//                     DIRECTX9 HANDLER
//=========================================================================
class VisualEngine_t
{
public:
    void Initialize(LPDIRECT3DDEVICE9 pDevice);
    inline void CaptureState() { if (m_pStateBlock != nullptr) m_pStateBlock->Capture(); }
    inline void RestoreState() { if (m_pStateBlock != nullptr) m_pStateBlock->Apply(); }

    void SetW2SMatrix();

    void Draw(DrawObjList_t& drawlist, LPDIRECT3DDEVICE9 pDevice);

private:
    bool m_bInitialized = false;

    bool _CompileShaders(LPDIRECT3DDEVICE9 pDevice);
    bool _DeclareVertex(LPDIRECT3DDEVICE9 pDevice);

    LPD3DXEFFECT                 m_pEffect     = nullptr;
    IDirect3DStateBlock9*        m_pStateBlock = nullptr;
    IDirect3DVertexDeclaration9* m_pVertexDecl = nullptr;
};

DECLARE_FEATURE_OBJECT(visualEngine, VisualEngine_t)


//=========================================================================
//                     INSANE OVERLAY
//=========================================================================
static _D3DVERTEXELEMENT9 vertexDecl[] =
{
    // Stream, Offset, Type,               Method,                Usage,                 Usage Index
      {0,      0,      D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION , 0},
      D3DDECL_END()
};

struct Vertex_t
{
    float x, y, z;

    Vertex_t& operator=(const vec& other)
    {
        x = other.x; y = other.y; z = other.z;
        return *this;
    }
};


struct DrawObjList_t
{
    DrawObjList_t(D3DPRIMITIVETYPE iDrawType, std::string szName, uint32_t iStride);

    // DirectX9 stuff
    D3DPRIMITIVETYPE m_iDrawType;
    uint32_t         m_iStride          = sizeof(Vertex_t);
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
    uint32_t                  m_nBufferSize   = 0;
    IDirect3DVertexBuffer9*   m_pVertexBuffer = nullptr;
    static constexpr uint32_t MAX_VERTEX_BUFFER_SIZE_InBytes = 5000;

    bool DoubleBufferSize(LPDIRECT3DDEVICE9 pDevice);
    bool HalfBufferSize(LPDIRECT3DDEVICE9 pDevice);

    void RemoveDrawObj(std::string szID);

    inline void Free()
    {
        for (auto& [szID, drawObj] : m_mapIDToObj)
        {
            delete drawObj;
            m_mapIDToObj.erase(szID);
        }

        m_nVertcies   = 0;
        m_nBufferSize = 0;
        
        if (m_pVertexBuffer != nullptr)
        {
            m_pVertexBuffer->Release();
        }
        m_pVertexBuffer = nullptr;
    }

    std::string m_szName = "UNDEFINED";
};

class InsaneOverlay_t
{
public:
    InsaneOverlay_t() :
        m_lineList(D3DPT_LINELIST,    "LineList",      sizeof(Vertex_t)),
        m_triList(D3DPT_TRIANGLELIST, "Traingle List", sizeof(Vertex_t))
    {}
    

    // All rendering happens here...
    void Run(LPDIRECT3DDEVICE9 pDevice);


    // Drawing different stuff
    void DrawRect(std::string szID, const vec& vMin, const vec& vMax, float flLife = -1.0f);
    

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


    DrawObjList_t m_lineList;
    DrawObjList_t m_triList;

};
DECLARE_FEATURE_OBJECT(insaneOverlay, InsaneOverlay_t)


//=========================================================================
//                     DRAW OBJECTS
//=========================================================================
struct VEClrConfig_t
{

};
struct VEGlowConfig_t
{

};

class BaseDrawObj_t
{
public:
    BaseDrawObj_t()
    {
        m_lastUpdateTime = std::chrono::high_resolution_clock::now();
    }

    virtual inline Vertex_t* GetVertcies()    { return nullptr; }
    virtual inline uint32_t  GetVertexCount() { return 0; }

    inline void SetLife(float flTimeInMs) { m_flLife = flTimeInMs; }
    inline void Lock()   { m_bLocked = true;  }
    inline void Unlock() { m_bLocked = false; }
    inline bool IsLocked() const { return m_bLocked; }

    // Storing update time
    inline void UpdateTime() { m_lastUpdateTime = std::chrono::high_resolution_clock::now(); }
    std::chrono::high_resolution_clock::time_point m_lastUpdateTime;
    
    float m_flLife = 1.0f;

protected:
    
    bool           m_bLocked    = false;
    VEClrConfig_t  m_clrConfig  = {};
    VEGlowConfig_t m_glowConfig = {};

};

class RectDrawObj_t : public BaseDrawObj_t
{
public:
    RectDrawObj_t() : BaseDrawObj_t() {}
    RectDrawObj_t(const vec& vMin, const vec& vMax);

    void Set(const vec& vMin, const vec& vMax, float flLife);

    // BaseDrawObj_t virtual fn overrides.
    inline Vertex_t* GetVertcies()    override { return m_vecVertcies; }
    inline uint32_t  GetVertexCount() override { return m_iVertexCount; }

private:
    vec m_vMin;
    vec m_vMax;

    static constexpr uint16_t m_iVertexCount = 8;
    Vertex_t m_vecVertcies[m_iVertexCount]   = {};
};


/*

so I can call render and update stuff.
I can change boxes for 1 specific draw object.
I can lock up some shit so it won't take updates for a set ammount of time.

LocknKill : so as the lock ends, it will fade out & erase it self.
Kill : Immediatly delete this draw object. won't be drawn no more.
Batching : All draw calls with same draw topology must be drawn at once. to leverage the 
            parallel computing of the GPU.

*/