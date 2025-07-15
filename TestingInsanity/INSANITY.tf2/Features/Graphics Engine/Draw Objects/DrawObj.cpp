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
    Vertex_t* pStorage, 
    uint32_t iStartIndex, 
    RGBA_t clrMin = ERROR_CLR, 
    RGBA_t clrMax = ERROR_CLR,
    const float flThickness = 1.0f,
    const float flSpeed = 0.0f,
    const float flGlowPower = 0.0f)
{
    if (pStorage == nullptr)
        return;

    vec vForward;
    Maths::AngleVectors(qNormal, &vForward);

    vec vDirection = (vMax - vMin).NormalizeInPlace();
    vec vRight = vDirection.CrossProduct(vForward).NormalizeInPlace();

    pStorage[iStartIndex + 0]               = vMin + (vRight * flThickness * 0.5f) + (vDirection * flThickness * -0.5f);
    pStorage[iStartIndex + 0].clr           = clrMin;
    pStorage[iStartIndex + 0].m_flHeight    = 1.0f;
    pStorage[iStartIndex + 0].m_flSpeed     = flSpeed;
    pStorage[iStartIndex + 0].m_flGlowPower = flGlowPower;

    pStorage[iStartIndex + 1]               = vMin + (vRight * flThickness * -0.5f) + (vDirection * flThickness * -0.5f);
    pStorage[iStartIndex + 1].clr           = clrMin;
    pStorage[iStartIndex + 1].m_flHeight    = -1.0f;
    pStorage[iStartIndex + 1].m_flSpeed     = flSpeed;
    pStorage[iStartIndex + 1].m_flGlowPower = flGlowPower;

    pStorage[iStartIndex + 2]               = vMax + (vRight * flThickness * 0.5f) + (vDirection * flThickness * 0.5f);
    pStorage[iStartIndex + 2].clr           = clrMax;
    pStorage[iStartIndex + 2].m_flHeight    = 1.0f;
    pStorage[iStartIndex + 2].m_flSpeed     = flSpeed;
    pStorage[iStartIndex + 2].m_flGlowPower = flGlowPower;

    pStorage[iStartIndex + 3]               = pStorage[iStartIndex + 1];
    pStorage[iStartIndex + 4]               = pStorage[iStartIndex + 2];

    pStorage[iStartIndex + 5]               = vMax + (vRight * flThickness * -0.5f) + (vDirection * flThickness * 0.5f);
    pStorage[iStartIndex + 5].clr           = clrMax;
    pStorage[iStartIndex + 5].m_flHeight    = -1.0f;
    pStorage[iStartIndex + 5].m_flSpeed     = flSpeed;
    pStorage[iStartIndex + 5].m_flGlowPower = flGlowPower;
}



inline void BuildRectangle(
    const vec& vMin,
    const vec& vMax,
    const qangle& qNormal,
    Vertex_t* pStorage,
    uint32_t iStartIndex,
    RGBA_t ULclr = ERROR_CLR,
    RGBA_t URclr = ERROR_CLR,
    RGBA_t BLclr = ERROR_CLR,
    RGBA_t BRclr = ERROR_CLR, 
    const float flThickness = 1.0f,
    const float flSpeed     = 0.0f,
    const float flGlowPower = 0.0f)
{
    // top left -> top right
    BuildQuad(vMin, vec{ vMax.x, vMax.y, vMin.z }, qNormal, pStorage, iStartIndex + 0, ULclr, URclr, flThickness, flSpeed, flGlowPower);

    // top right -> bottom right
    BuildQuad(vec{ vMax.x, vMax.y, vMin.z }, vMax, qNormal, pStorage, iStartIndex + 6, URclr, BRclr, flThickness, flSpeed, flGlowPower);

    // bottom right -> botton left
    BuildQuad(vMax, vec{ vMin.x, vMin.y, vMax.z }, qNormal, pStorage, iStartIndex + 12, BRclr, BLclr, flThickness, flSpeed, flGlowPower);

    // bottom left -> top left
    BuildQuad(vec{ vMin.x, vMin.y, vMax.z }, vMin, qNormal, pStorage, iStartIndex + 18, BLclr, ULclr, flThickness, flSpeed, flGlowPower);
}



void RectDrawObj_t::Set(const vec& vMin, const vec& vMax, const qangle& qNormal, GraphicInfo_t* pGraphicInfo)
{
    // NOTE : We draw a line using 2 traingles, and half of the traingle will be above the line and half below 
    //          to keep the scaling consistent. I hope you understand what I am trying to say here.
    UpdateTime();

    if (pGraphicInfo == nullptr)
    {
        BuildRectangle(vMin, vMax, qNormal, m_vecVertcies, 0, ERROR_CLR, ERROR_CLR, ERROR_CLR, ERROR_CLR, 1.0f, 0.0f, 0.0f);
    }
    else
    {
        BuildRectangle(vMin, vMax, qNormal, m_vecVertcies, 0, 
            pGraphicInfo->m_ULclr, 
            pGraphicInfo->m_URclr, 
            pGraphicInfo->m_BLclr, 
            pGraphicInfo->m_BRclr,
            pGraphicInfo->m_flThickness,
            pGraphicInfo->m_flSpeed,
            pGraphicInfo->m_flGlowPower);
    }

}



void CuboidDrawObj_t::Set(const vec& vMin, const vec& vMax, const qangle& qNormal, GraphicInfo_t* pGraphicInfo)
{
    UpdateTime();

    if (pGraphicInfo == nullptr)
    {
        uint32_t iStoreIndex = 0;
        BuildQuad(vMin, vec(vMin.x, vMax.y, vMin.z), qNormal, m_vecVertcies, iStoreIndex);
        iStoreIndex += 3u * 2u;
        BuildQuad(vec(vMax.x, vMin.y, vMin.z), vec(vMax.x, vMax.y, vMin.z), qNormal, m_vecVertcies, iStoreIndex);
        iStoreIndex += 3u * 2u;
        BuildQuad(vec(vMax.x, vMin.y, vMax.z), vMax, qNormal, m_vecVertcies, iStoreIndex);
        iStoreIndex += 3u * 2u;
        BuildQuad(vec(vMin.x, vMin.y, vMax.z), vec(vMin.x, vMax.y, vMax.z), qNormal, m_vecVertcies, iStoreIndex);
        iStoreIndex += 3u * 2u;

        BuildRectangle(vMin, vec(vMax.x, vMin.y, vMax.z), qNormal, m_vecVertcies, iStoreIndex);

        iStoreIndex += 3u * 2u * 4u;

        BuildRectangle(vec(vMin.x, vMax.y, vMin.z), vMax, qNormal, m_vecVertcies, iStoreIndex);
    }
    else
    {
        uint32_t iStoreIndex = 0;

        BuildQuad(vMin, vec(vMin.x, vMax.y, vMin.z), 
            qNormal, m_vecVertcies, iStoreIndex, 
            pGraphicInfo->m_ULclr, pGraphicInfo->m_URclr, 
            pGraphicInfo->m_flThickness, 
            pGraphicInfo->m_flSpeed, 
            pGraphicInfo->m_flGlowPower);

        iStoreIndex += 3u * 2u;
        BuildQuad(vec(vMax.x, vMin.y, vMin.z), vec(vMax.x, vMax.y, vMin.z), 
            qNormal, m_vecVertcies, iStoreIndex, 
            pGraphicInfo->m_ULclr, pGraphicInfo->m_URclr, 
            pGraphicInfo->m_flThickness, 
            pGraphicInfo->m_flSpeed, 
            pGraphicInfo->m_flGlowPower);

        iStoreIndex += 3u * 2u;
        BuildQuad(vec(vMax.x, vMin.y, vMax.z), vMax, 
            qNormal, m_vecVertcies, iStoreIndex, 
            pGraphicInfo->m_BLclr, pGraphicInfo->m_BRclr, 
            pGraphicInfo->m_flThickness, 
            pGraphicInfo->m_flSpeed, 
            pGraphicInfo->m_flGlowPower);

        iStoreIndex += 3u * 2u;
        BuildQuad(vec(vMin.x, vMin.y, vMax.z), vec(vMin.x, vMax.y, vMax.z), 
            qNormal, m_vecVertcies, iStoreIndex, 
            pGraphicInfo->m_BLclr, pGraphicInfo->m_BRclr, 
            pGraphicInfo->m_flThickness, 
            pGraphicInfo->m_flSpeed, 
            pGraphicInfo->m_flGlowPower);

        iStoreIndex += 3u * 2u;

        BuildRectangle(vMin, vec(vMax.x, vMin.y, vMax.z), qNormal, m_vecVertcies, iStoreIndex,
            pGraphicInfo->m_ULclr, pGraphicInfo->m_ULclr, pGraphicInfo->m_BLclr, pGraphicInfo->m_BLclr,
            pGraphicInfo->m_flThickness, pGraphicInfo->m_flSpeed, pGraphicInfo->m_flGlowPower);

        iStoreIndex += 3u * 2u * 4u;
        
        BuildRectangle(vec(vMin.x, vMax.y, vMin.z), vMax, qNormal, m_vecVertcies, iStoreIndex,
            pGraphicInfo->m_URclr, pGraphicInfo->m_URclr, pGraphicInfo->m_BRclr, pGraphicInfo->m_BRclr,
            pGraphicInfo->m_flThickness, pGraphicInfo->m_flSpeed, pGraphicInfo->m_flGlowPower);

    }
}


void LineDrawObj_t::Set(const vec& vMin, const vec& vMax, const qangle& qNormal, GraphicInfo_t* pGraphicInfo)
{
    UpdateTime();

    if (pGraphicInfo == nullptr)
    {
        BuildQuad(vMin, vMax, qNormal, m_vecVertcies, 0);
    }
    else
    {
        BuildQuad(vMin, vMax, qNormal, m_vecVertcies, 0, 
            pGraphicInfo->m_ULclr, pGraphicInfo->m_URclr, 
            pGraphicInfo->m_flThickness, 
            pGraphicInfo->m_flSpeed, 
            pGraphicInfo->m_flGlowPower);
    }
}
