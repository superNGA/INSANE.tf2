#pragma once

#include <chrono>

#include "../../../SDK/class/Basic Structures.h"


struct GraphicInfo_t
{
    GraphicInfo_t() : 
        m_ULclr(0), m_URclr(0), m_BRclr(0), m_BLclr(0), m_flThickness(1.0f) {}
    GraphicInfo_t(
        RGBA_t ULclr, RGBA_t URclr, RGBA_t BLclr, RGBA_t BRclr, float flThickness) :
        m_ULclr(ULclr), m_URclr(URclr), m_BRclr(BRclr), m_BLclr(BLclr), m_flThickness(flThickness)
    {}

    RGBA_t m_ULclr;
    RGBA_t m_URclr;
    RGBA_t m_BLclr;
    RGBA_t m_BRclr;

    float m_flThickness = 1.0f;
};


struct Vertex_t
{
    float x, y, z;
    RGBA_t clr;

    float flHeight = 1.0f;

    Vertex_t& operator=(const vec& other)
    {
        x = other.x; y = other.y; z = other.z;
        return *this;
    }
    Vertex_t& operator=(const Vertex_t& other)
    {
        x = other.x; y = other.y; z = other.z;
        clr = other.clr;
        flHeight = other.flHeight;
        return *this;
    }
};

//=========================================================================
//                     BASE DRAW OBJECT
//=========================================================================
class BaseDrawObj_t
{
public:
    BaseDrawObj_t()
    {
        m_lastUpdateTime = std::chrono::high_resolution_clock::now();
    }
    virtual ~BaseDrawObj_t() {}

    virtual inline Vertex_t* GetVertcies() { return nullptr; }
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

    bool m_bLocked = false;

};


//=========================================================================
//                     DERIVED DRAW OBJECTS
//=========================================================================
class RectDrawObj_t : public BaseDrawObj_t
{
public:
    RectDrawObj_t() : BaseDrawObj_t() {}

    void Set(
        const vec& vMin,
        const vec& vMax,
        const qangle& qNormal, 
        GraphicInfo_t* pGraphicInfo = nullptr);

    // BaseDrawObj_t virtual fn overrides.
    inline Vertex_t* GetVertcies()    override { return m_vecVertcies; }
    inline uint32_t  GetVertexCount() override { return m_iVertexCount; }

private:
    static constexpr uint16_t m_iVertexCount = 3U * 2U * 4U; // vertex per triangle * triangle per side * number of sides.
    Vertex_t m_vecVertcies[m_iVertexCount]   = {};
};


class CuboidDrawObj_t : public BaseDrawObj_t
{
public:
    CuboidDrawObj_t() : BaseDrawObj_t() {}

    void Set(
        const vec& vMin,
        const vec& vMax,
        const qangle& qNormal,
        GraphicInfo_t* pGraphicInfo = nullptr);

    // BaseDrawObj_t virtual fn overrides.
    inline Vertex_t* GetVertcies()    override { return m_vecVertcies; }
    inline uint32_t  GetVertexCount() override { return m_iVertexCount; }

private:
    static constexpr uint16_t m_iVertexCount = 3U * 2U * 12U; // Vertex per traingle * traingle per side * total sides count
    Vertex_t m_vecVertcies[m_iVertexCount] = {};
};


class LineDrawObj_t : public BaseDrawObj_t
{
public:
    LineDrawObj_t() : BaseDrawObj_t() {}

    void Set(
        const vec& vMin,
        const vec& vMax,
        const qangle& qNormal,
        GraphicInfo_t* pGraphicInfo = nullptr);

    // BaseDrawObj_t virtual fn overrides.
    inline Vertex_t* GetVertcies()    override { return m_vecVertcies; }
    inline uint32_t  GetVertexCount() override { return m_iVertexCount; }

private:
    static constexpr uint16_t m_iVertexCount = 3U * 2U; // Vertex per traingle * traingle per side
    Vertex_t m_vecVertcies[m_iVertexCount] = {};
};