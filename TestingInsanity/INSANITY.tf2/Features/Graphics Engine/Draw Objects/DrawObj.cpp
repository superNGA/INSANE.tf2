#include "DrawObj.h"

#include "../../../Extra/math.h"

constexpr RGBA_t ERROR_CLR(0);

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


void RectDrawObj_t::Set(const vec& vMin, const vec& vMax, const qangle& qNormal, GraphicInfo_t* pGraphicInfo)
{
    // NOTE : We draw a line using 2 traingles, and half of the traingle will be above the line and half below 
    //          to keep the scaling consistent. I hope you understand what I am trying to say here.
    UpdateTime();

    if (pGraphicInfo == nullptr)
    {
        // top left -> top right
        BuildQuad(vMin, vec{ vMax.x, vMax.y, vMin.z }, qNormal, 1.0f, m_vecVertcies, 0, ERROR_CLR, ERROR_CLR);

        // top right -> bottom right
        BuildQuad(vec{ vMax.x, vMax.y, vMin.z }, vMax, qNormal, 1.0f, m_vecVertcies, 6, ERROR_CLR, ERROR_CLR);

        // bottom right -> botton left
        BuildQuad(vMax, vec{ vMin.x, vMin.y, vMax.z }, qNormal, 1.0f, m_vecVertcies, 12, ERROR_CLR, ERROR_CLR);

        // bottom left -> top left
        BuildQuad(vec{ vMin.x, vMin.y, vMax.z }, vMin, qNormal, 1.0f, m_vecVertcies, 18, ERROR_CLR, ERROR_CLR);
    }
    else
    {
        // top left -> top right
        BuildQuad(vMin, vec{ vMax.x, vMax.y, vMin.z }, qNormal, pGraphicInfo->m_flThickness, m_vecVertcies, 0, pGraphicInfo->m_ULclr, pGraphicInfo->m_URclr);

        // top right -> bottom right
        BuildQuad(vec{ vMax.x, vMax.y, vMin.z }, vMax, qNormal, pGraphicInfo->m_flThickness, m_vecVertcies, 6, pGraphicInfo->m_URclr, pGraphicInfo->m_BRclr);

        // bottom right -> botton left
        BuildQuad(vMax, vec{ vMin.x, vMin.y, vMax.z }, qNormal, pGraphicInfo->m_flThickness, m_vecVertcies, 12, pGraphicInfo->m_BRclr, pGraphicInfo->m_BLclr);

        // bottom left -> top left
        BuildQuad(vec{ vMin.x, vMin.y, vMax.z }, vMin, qNormal, pGraphicInfo->m_flThickness, m_vecVertcies, 18, pGraphicInfo->m_BLclr, pGraphicInfo->m_ULclr);
    }

}