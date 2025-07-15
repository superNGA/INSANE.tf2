#include "DrawObj.h"

#include "../../../Extra/math.h"
#include "../../../Utility/ConsoleLogging.h"

constexpr RGBA_t ERROR_CLR(0);
constexpr uint32_t VERTEX_PER_QUAD = 6u;

//=========================================================================
//                     DRAW OBJECTS
//=========================================================================
inline void BuildQuad(
    const vec& vMin, 
    const vec& vMax, 
    const qangle& qNormal, 
    const float flThickness, 
    Vertex_t* pStorage, 
    uint32_t iStartIndex, 
    RGBA_t clrMin, 
    RGBA_t clrMax)
{
    if (pStorage == nullptr)
        return;

    vec vForward;
    Maths::AngleVectors(qNormal, &vForward);

    vec vDirection = (vMax - vMin).NormalizeInPlace();
    vec vRight = vDirection.CrossProduct(vForward).NormalizeInPlace();

    pStorage[iStartIndex + 0]           = vMin + (vRight * flThickness * 0.5f) + (vDirection * flThickness * -0.5f);
    pStorage[iStartIndex + 0].clr       = clrMin;
    pStorage[iStartIndex + 0].flHeight  = 1.0f;

    pStorage[iStartIndex + 1]           = vMin + (vRight * flThickness * -0.5f) + (vDirection * flThickness * -0.5f);
    pStorage[iStartIndex + 1].clr       = clrMin;
    pStorage[iStartIndex + 1].flHeight  = -1.0f;

    pStorage[iStartIndex + 2]           = vMax + (vRight * flThickness * 0.5f) + (vDirection * flThickness * 0.5f);
    pStorage[iStartIndex + 2].clr       = clrMax;
    pStorage[iStartIndex + 2].flHeight  = 1.0f;

    pStorage[iStartIndex + 3] = pStorage[iStartIndex + 1];
    pStorage[iStartIndex + 4] = pStorage[iStartIndex + 2];

    pStorage[iStartIndex + 5]           = vMax + (vRight * flThickness * -0.5f) + (vDirection * flThickness * 0.5f);
    pStorage[iStartIndex + 5].clr       = clrMax;
    pStorage[iStartIndex + 5].flHeight  = -1.0f;
}

inline void BuildQuad(
    const vec& vMin, 
    const vec& vMax, 
    const qangle& qNormal, 
    const float flThickness, 
    std::vector<Vertex_t>* pStorage, 
    RGBA_t clrMin, 
    RGBA_t clrMax)
{
    if (pStorage == nullptr)
        return;

    vec vForward;
    Maths::AngleVectors(qNormal, &vForward);

    vec vDirection = (vMax - vMin).NormalizeInPlace();
    vec vRight = vDirection.CrossProduct(vForward).NormalizeInPlace();

    Vertex_t vertex;
    vertex          = vMin + (vRight * flThickness * 0.5f) + (vDirection * flThickness * -0.5f);
    vertex.clr      = clrMin;
    vertex.flHeight = 1.0f;
    pStorage->push_back(vertex);

    vertex           = vMin + (vRight * flThickness * -0.5f) + (vDirection * flThickness * -0.5f);
    vertex.clr       = clrMin;
    vertex.flHeight  = -1.0f;
    pStorage->push_back(vertex);

    vertex          = vMax + (vRight * flThickness * 0.5f) + (vDirection * flThickness * 0.5f);
    vertex.clr      = clrMax;
    vertex.flHeight = 1.0f;
    pStorage->push_back(vertex);

    vertex           = vMin + (vRight * flThickness * -0.5f) + (vDirection * flThickness * -0.5f);
    vertex.clr       = clrMin;
    vertex.flHeight  = -1.0f;
    pStorage->push_back(vertex);

    vertex          = vMax + (vRight * flThickness * 0.5f) + (vDirection * flThickness * 0.5f);
    vertex.clr      = clrMax;
    vertex.flHeight = 1.0f;
    pStorage->push_back(vertex);

    vertex           = vMax + (vRight * flThickness * -0.5f) + (vDirection * flThickness * 0.5f);
    vertex.clr       = clrMax;
    vertex.flHeight  = -1.0f;
    pStorage->push_back(vertex);
}


inline void BuildRectangle(
    const vec& vMin,
    const vec& vMax,
    const qangle& qNormal,
    Vertex_t* pStorage,
    uint32_t iStartIndex,
    const float flThickness,
    RGBA_t ULclr,
    RGBA_t URclr,
    RGBA_t BLclr,
    RGBA_t BRclr)
{
    // top left -> top right
    BuildQuad(vMin, vec{ vMax.x, vMax.y, vMin.z }, qNormal, flThickness, pStorage, iStartIndex + 0, ULclr, URclr);

    // top right -> bottom right
    BuildQuad(vec{ vMax.x, vMax.y, vMin.z }, vMax, qNormal, flThickness, pStorage, iStartIndex + 6, URclr, BRclr);

    // bottom right -> botton left
    BuildQuad(vMax, vec{ vMin.x, vMin.y, vMax.z }, qNormal, flThickness, pStorage, iStartIndex + 12, BRclr, BLclr);

    // bottom left -> top left
    BuildQuad(vec{ vMin.x, vMin.y, vMax.z }, vMin, qNormal, flThickness, pStorage, iStartIndex + 18, BLclr, ULclr);
}


void RectDrawObj_t::Set(const vec& vMin, const vec& vMax, const qangle& qNormal, GraphicInfo_t* pGraphicInfo)
{
    // NOTE : We draw a line using 2 traingles, and half of the traingle will be above the line and half below 
    //          to keep the scaling consistent. I hope you understand what I am trying to say here.
    UpdateTime();

    if (pGraphicInfo == nullptr)
    {
        BuildRectangle(vMin, vMax, qNormal, m_vecVertcies, 0, 1.0f, ERROR_CLR, ERROR_CLR, ERROR_CLR, ERROR_CLR);
    }
    else
    {
        BuildRectangle(vMin, vMax, qNormal, m_vecVertcies, 0, pGraphicInfo->m_flThickness, 
            pGraphicInfo->m_ULclr, 
            pGraphicInfo->m_URclr, 
            pGraphicInfo->m_BLclr, 
            pGraphicInfo->m_BRclr);
    }

}



void CuboidDrawObj_t::Set(const vec& vMin, const vec& vMax, const qangle& qNormal, GraphicInfo_t* pGraphicInfo)
{
    UpdateTime();

    if (pGraphicInfo == nullptr)
    {
        uint32_t iStoreIndex = 0;
        BuildQuad(vMin, vec(vMin.x, vMax.y, vMin.z), qNormal, 1.0f, m_vecVertcies, iStoreIndex, ERROR_CLR, ERROR_CLR);
        iStoreIndex += 3u * 2u;
        BuildQuad(vec(vMax.x, vMin.y, vMin.z), vec(vMax.x, vMax.y, vMin.z), qNormal, 1.0f, m_vecVertcies, iStoreIndex, ERROR_CLR, ERROR_CLR);
        iStoreIndex += 3u * 2u;
        BuildQuad(vec(vMax.x, vMin.y, vMax.z), vMax, qNormal, 1.0f, m_vecVertcies, iStoreIndex, ERROR_CLR, ERROR_CLR);
        iStoreIndex += 3u * 2u;
        BuildQuad(vec(vMin.x, vMin.y, vMax.z), vec(vMin.x, vMax.y, vMax.z), qNormal, 1.0f, m_vecVertcies, iStoreIndex, ERROR_CLR, ERROR_CLR);
        iStoreIndex += 3u * 2u;

        BuildRectangle(vMin, vec(vMax.x, vMin.y, vMax.z), qNormal, m_vecVertcies, iStoreIndex, 1.0f,
            ERROR_CLR, ERROR_CLR, ERROR_CLR, ERROR_CLR);

        iStoreIndex += 3u * 2u * 4u;

        BuildRectangle(vec(vMin.x, vMax.y, vMin.z), vMax, qNormal, m_vecVertcies, iStoreIndex, 1.0f,
            ERROR_CLR, ERROR_CLR, ERROR_CLR, ERROR_CLR);
    }
    else
    {
        uint32_t iStoreIndex = 0;
        BuildQuad(vMin, vec(vMin.x, vMax.y, vMin.z), qNormal, pGraphicInfo->m_flThickness, m_vecVertcies, iStoreIndex, pGraphicInfo->m_ULclr, pGraphicInfo->m_URclr);
        iStoreIndex += 3u * 2u;
        BuildQuad(vec(vMax.x, vMin.y, vMin.z), vec(vMax.x, vMax.y, vMin.z), qNormal, pGraphicInfo->m_flThickness, m_vecVertcies, iStoreIndex, pGraphicInfo->m_ULclr, pGraphicInfo->m_URclr);
        iStoreIndex += 3u * 2u;
        BuildQuad(vec(vMax.x, vMin.y, vMax.z), vMax, qNormal, pGraphicInfo->m_flThickness, m_vecVertcies, iStoreIndex, pGraphicInfo->m_BLclr, pGraphicInfo->m_BRclr);
        iStoreIndex += 3u * 2u;
        BuildQuad(vec(vMin.x, vMin.y, vMax.z), vec(vMin.x, vMax.y, vMax.z), qNormal, pGraphicInfo->m_flThickness, m_vecVertcies, iStoreIndex, pGraphicInfo->m_BLclr, pGraphicInfo->m_BRclr);
        iStoreIndex += 3u * 2u;

        BuildRectangle(vMin, vec(vMax.x, vMin.y, vMax.z), qNormal, m_vecVertcies, iStoreIndex, pGraphicInfo->m_flThickness,
            pGraphicInfo->m_ULclr, pGraphicInfo->m_ULclr, pGraphicInfo->m_BLclr, pGraphicInfo->m_BLclr);

        iStoreIndex += 3u * 2u * 4u;
        
        BuildRectangle(vec(vMin.x, vMax.y, vMin.z), vMax, qNormal, m_vecVertcies, iStoreIndex, pGraphicInfo->m_flThickness,
            pGraphicInfo->m_URclr, pGraphicInfo->m_URclr, pGraphicInfo->m_BRclr, pGraphicInfo->m_BRclr);

    }
}


void LineDrawObj_t::Set(const vec& vMin, const vec& vMax, const qangle& qNormal, GraphicInfo_t* pGraphicInfo)
{
    UpdateTime();

    if (pGraphicInfo == nullptr)
    {
        BuildQuad(vMin, vMax, qNormal, 1.0f, m_vecVertcies, 0, ERROR_CLR, ERROR_CLR);
    }
    else
    {
        BuildQuad(vMin, vMax, qNormal, pGraphicInfo->m_flThickness, m_vecVertcies, 0, pGraphicInfo->m_ULclr, pGraphicInfo->m_URclr);
    }
}
