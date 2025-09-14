#include "Circle.h"

// SDK
#include "../../../../SDK/class/IVEngineClient.h"

#include "../../../../Extra/math.h"


///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
ICircle_t::ICircle_t()
{
    F::graphics.RegisterInTraingleList(this);
    InitRelativeUV();

    for (int i = 0; i < GetVertexCount(); i++)
        m_vertex[i].m_flCircleThinkess = 0.01f;
}


///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
unsigned int ICircle_t::GetVertexCount() const
{
    return sizeof(m_vertex) / sizeof(Vertex);
}


///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
void ICircle_t::SetBlur(const int iBlur)
{
    for (int i = 0; i < GetVertexCount(); i++)
        m_vertex[i].m_flBlurAmmount = static_cast<float>(iBlur);
}


///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
const Vertex* ICircle_t::GetVertexData() const
{
    return m_vertex;
}


///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
void ICircle_t::SetColor(unsigned char r, unsigned char g, unsigned char b, unsigned char a)
{
    for (int i = 0; i < GetVertexCount(); i++)
        m_vertex[i].m_clr.Set(r, g, b, a);
}


///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
void ICircle_t::InvertColors(bool bInvert)
{
    for (int i = 0; i < GetVertexCount(); i++)
        m_vertex[i].m_flInvertColors = (bInvert == true ? FLOAT_TRUE : FLOAT_FALSE);
}


///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
void ICircle_t::SetColor(RGBA_t clr, int vertex)
{
    vertex = std::clamp<int>(vertex, 0, GetVertexCount() - 1);

    switch (vertex)
    {
    case ICircle_t::VertexType_TopLeft:
    {
        m_vertex[VertexType_TopLeft].m_clr.Set(clr.r, clr.g, clr.b, clr.a);
        break;
    }
    case ICircle_t::VertexType_TopRight:
    case ICircle_t::VertexType_TopRight_Dup:
    {
        m_vertex[VertexType_TopRight].m_clr.Set(clr.r, clr.g, clr.b, clr.a);
        m_vertex[VertexType_TopRight_Dup].m_clr.Set(clr.r, clr.g, clr.b, clr.a);
        break;
    }
    case ICircle_t::VertexType_BottomLeft:
    case ICircle_t::VertexType_BottomLeft_Dup:
    {
        m_vertex[VertexType_BottomLeft].m_clr.Set(clr.r, clr.g, clr.b, clr.a);
        m_vertex[VertexType_BottomLeft_Dup].m_clr.Set(clr.r, clr.g, clr.b, clr.a);
        break;
    }
    case ICircle_t::VertexType_BottomRight:
    {
        m_vertex[VertexType_BottomRight].m_clr.Set(clr.r, clr.g, clr.b, clr.a);
        break;
    }
    default: break;
    }
}


///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
void ICircle_t::InitRelativeUV()
{
    m_vertex[VertexType_TopLeft].m_vRelativeUV        = vec(0.0f, 0.0f, 0.0f);
    
    // Top right.
    m_vertex[VertexType_TopRight].m_vRelativeUV       = vec(1.0f, 0.0f, 0.0f);
    m_vertex[VertexType_TopRight_Dup].m_vRelativeUV   = m_vertex[VertexType_TopRight].m_vRelativeUV;
    
    // Bottom left.
    m_vertex[VertexType_BottomLeft].m_vRelativeUV     = vec(0.0f, 1.0f, 0.0f);
    m_vertex[VertexType_BottomLeft_Dup].m_vRelativeUV = m_vertex[VertexType_BottomLeft].m_vRelativeUV;

    m_vertex[VertexType_BottomRight].m_vRelativeUV    = vec(1.0f, 1.0f, 0.0f);
}


////////////////////////////// DERIVED CIRCLE CLASSES /////////////////////
///////////////////////////////////////////////////////////////////////////
void Circle2D_t::SetVertex(const vec& vMin, const vec& vMax)
{
    vec vTopRight   = vec(vMax.x, vMin.y, 0.0f);
    vec vBottomLeft = vec(vMin.x, vMax.y, 0.0f);

    m_vertex[VertexType_TopLeft].m_vPos        = vMin;
    m_vertex[VertexType_TopRight].m_vPos       = vTopRight;
    m_vertex[VertexType_BottomLeft].m_vPos     = vBottomLeft;
    m_vertex[VertexType_TopRight_Dup].m_vPos   = vTopRight;
    m_vertex[VertexType_BottomLeft_Dup].m_vPos = vBottomLeft;
    m_vertex[VertexType_BottomRight].m_vPos    = vMax;

    // Setting up scale.
    float flHeight = fabsf(vMin.y - vMax.y);
    float flWidth  = fabsf(vMin.x - vMax.x);
    for (int i = 0; i < GetVertexCount(); i++)
        m_vertex[i].m_flScaleY = flHeight / flWidth;
}


///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
void Circle2D_t::SetVertex(const vec& vCenter, const float flRadius)
{
    vec vMax(vCenter.x + flRadius, vCenter.y + flRadius, 0.0f);
    vec vMin(vCenter.x - flRadius, vCenter.y - flRadius, 0.0f);
    vec vTopRight   = vec(vMax.x, vMin.y, 0.0f);
    vec vBottomLeft = vec(vMin.x, vMax.y, 0.0f);

    m_vertex[VertexType_TopLeft].m_vPos        = vMin;
    m_vertex[VertexType_TopRight].m_vPos       = vTopRight;
    m_vertex[VertexType_BottomLeft].m_vPos     = vBottomLeft;
    m_vertex[VertexType_TopRight_Dup].m_vPos   = vTopRight;
    m_vertex[VertexType_BottomLeft_Dup].m_vPos = vBottomLeft;
    m_vertex[VertexType_BottomRight].m_vPos    = vMax;

    // Setting up scale.
    float flHeight = fabsf(vMin.y - vMax.y);
    float flWidth  = fabsf(vMin.x - vMax.x);
    for (int i = 0; i < GetVertexCount(); i++)
        m_vertex[i].m_flScaleY = flHeight / flWidth;
}


///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
void Circle2D_t::SetThickness(const float flThicknessInPixels)
{
    float flObjWidth = fabsf(m_vertex[VertexType_TopLeft].m_vPos.x - m_vertex[VertexType_BottomRight].m_vPos.x);

    float flThicknessInUV = (1.0f / flObjWidth) * flThicknessInPixels;
    
    for (int i = 0; i < GetVertexCount(); i++)
        m_vertex[i].m_flCircleThinkess = flThicknessInUV;
}


///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
void Circle2D_t::InitDimension()
{
    m_bIs3D = false;

    for (int i = 0; i < GetVertexCount(); i++)
        m_vertex[i].m_flStrictly2D = FLOAT_TRUE;
}


///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
void Circle3D_t::InitDimension()
{
    m_bIs3D = true;

    for (int i = 0; i < GetVertexCount(); i++)
        m_vertex[i].m_flStrictly2D = FLOAT_FALSE;
}


///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
void Circle3D_t::SetVertex(const vec& vMin, const vec& vMax, const qangle& qNormal)
{
    vec vForward; Maths::AngleVectors(qNormal, &vForward); vForward.NormalizeInPlace();
    vec vDiagonal = vMax - vMin; vDiagonal.NormalizeInPlace();

    if (vForward.HasSameDirection(vDiagonal) == true)
        return;

    vec vDiagonal2 = vDiagonal.CrossProduct(vForward); vDiagonal2.NormalizeInPlace();

    // Calculating both axis perpendicular in the plane of the rectangle.
    vec vAxisY = vDiagonal2 + vDiagonal;        vAxisY.NormalizeInPlace();
    vec vAxisX = vAxisY.CrossProduct(vForward); vAxisX.NormalizeInPlace();

    // magnitude along each axis of the half daigonal.
    vec   vHalfDaigonal = (vMax - vMin) / 2.0f;
    float flOffsetY = vHalfDaigonal.Dot(vAxisY);
    float flOffsetX = vHalfDaigonal.Dot(vAxisX);

    // Adding magnitude along each axis in opposite order to get the other 2 points.
    vec vCenter     = (vMin + vMax) / 2.0;
    vec vTopRight   = vCenter + (vAxisY * flOffsetY * -1.0f) + (vAxisX * flOffsetX);
    vec vBottomLeft = vCenter + (vAxisY * flOffsetY) + (vAxisX * flOffsetX * -1.0f);


    m_vertex[VertexType_TopLeft].m_vPos        = vMin;
    m_vertex[VertexType_TopRight].m_vPos       = vTopRight;
    m_vertex[VertexType_BottomLeft].m_vPos     = vBottomLeft;
    m_vertex[VertexType_TopRight_Dup].m_vPos   = vTopRight;
    m_vertex[VertexType_BottomLeft_Dup].m_vPos = vBottomLeft;
    m_vertex[VertexType_BottomRight].m_vPos    = vMax;

    // We are using flOffsetX and flOffsetY here, cause they are calculated based upon the normal
    // so its the true height and width ratio of the box and it doesn't depend on 
    // the orientation of the box. [ most probably. I'm not very mathematical ]:)
    for (int i = 0; i < GetVertexCount(); i++)
        m_vertex[i].m_flScaleY = fabsf(flOffsetY / flOffsetX);
}


///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
void Circle3D_t::SetThickness(const float flThickenssInPrcnt)
{
    for (int i = 0; i < GetVertexCount(); i++)
        m_vertex[i].m_flCircleThinkess = flThickenssInPrcnt;
}