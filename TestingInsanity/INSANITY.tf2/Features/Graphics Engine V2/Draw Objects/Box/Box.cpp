#include "Box.h"

#include "../../../../Extra/math.h"
#include "../../Graphics.h"


///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
IBoxFilled_t::IBoxFilled_t()
{
    F::graphics.RegisterInTraingleList(this);
    InitRelativeUV();
}


///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
unsigned int IBoxFilled_t::GetVertexCount() const
{
    return 6;
}


///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
void IBoxFilled_t::SetBlur(const int iBlur)
{
    for(int i = 0; i < GetVertexCount(); i++)
        m_vertex[i].m_flBlurAmmount = static_cast<float>(iBlur);
}


///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
const Vertex* IBoxFilled_t::GetVertexData() const
{
    return m_vertex;
}


///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
void IBoxFilled_t::SetColor(unsigned char r, unsigned char g, unsigned char b, unsigned char a)
{
    for (int i = 0; i < 6; i++)
        m_vertex[i].m_clr.Set(r, g, b, a);
}


///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
void IBoxFilled_t::InvertColors(bool bInvert)
{
    for (int i = 0; i < 6; i++)
        m_vertex[i].m_flInvertColors = (bInvert == true ? 1.0f : 0.0f);
}


///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
void IBoxFilled_t::SetColor(RGBA_t clr, int vertex)
{
    vertex = std::clamp<int>(vertex, 0, GetVertexCount() - 1);

    switch (vertex)
    {
    case VertexType_t::VertexType_TopLeft:
    {
        m_vertex[VertexType_TopLeft].m_clr.Set(clr.r, clr.g, clr.b, clr.a);
        break;
    }
    case VertexType_t::VertexType_TopRight:
    case VertexType_t::VertexType_TopRight_Dup:
    {
        m_vertex[VertexType_TopRight].m_clr.Set(clr.r, clr.g, clr.b, clr.a);
        m_vertex[VertexType_TopRight_Dup].m_clr.Set(clr.r, clr.g, clr.b, clr.a);
        break;
    }
    case VertexType_t::VertexType_BottomLeft:
    case VertexType_t::VertexType_BottomLeft_Dup:
    {
        m_vertex[VertexType_BottomLeft].m_clr.Set(clr.r, clr.g, clr.b, clr.a);
        m_vertex[VertexType_BottomLeft_Dup].m_clr.Set(clr.r, clr.g, clr.b, clr.a);
        break;
    }
    case VertexType_t::VertexType_BottomRight:
    {
        m_vertex[VertexType_BottomRight].m_clr.Set(clr.r, clr.g, clr.b, clr.a);
        break;
    }
    default: break;
    }
}


///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
void IBoxFilled_t::SetRounding(float flRounding)
{
    flRounding = std::clamp<float>(flRounding, 0.0f, 1.0f);

    for (int i = 0; i < GetVertexCount(); i++)
        m_vertex[i].m_flRounding = flRounding;
}


///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
void IBoxFilled_t::InitRelativeUV()
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


///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
void BoxFilled3D_t::InitDimension()
{
    m_bIs3D = true;

    for (int i = 0; i < GetVertexCount(); i++)
        m_vertex[i].m_flStrictly2D = FLOAT_FALSE;
}


///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
void BoxFilled3D_t::SetVertex(const vec& vMin, const vec& vMax, const qangle& qNormal)
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
void BoxFilled2D_t::InitDimension()
{
    m_bIs3D = false;

    for (int i = 0; i < GetVertexCount(); i++)
        m_vertex[i].m_flStrictly2D = FLOAT_TRUE;
}


///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
void BoxFilled2D_t::SetVertex(const vec& vMin, const vec& vMax)
{
    vec vTopRight   = vec(vMax.x, vMin.y, 0.0f);
    vec vBottomLeft = vec(vMin.x, vMax.y, 0.0f);

    m_vertex[VertexType_TopLeft].m_vPos        = vMin;
    m_vertex[VertexType_TopRight].m_vPos       = vTopRight;
    m_vertex[VertexType_BottomLeft].m_vPos     = vBottomLeft;
    m_vertex[VertexType_TopRight_Dup].m_vPos   = vTopRight;
    m_vertex[VertexType_BottomLeft_Dup].m_vPos = vBottomLeft;
    m_vertex[VertexType_BottomRight].m_vPos    = vMax;

    // Setting up scale for this 2D box.
    float flHeight = fabsf(vMin.y - vMax.y);
    float flWidth  = fabsf(vMin.x - vMax.x);
    for (int i = 0; i < GetVertexCount(); i++)
        m_vertex[i].m_flScaleY = flHeight / flWidth;
}


///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
IBox_t::IBox_t()
{
    F::graphics.RegisterInLineList(this);
    InitRelativeUV();
}


///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
unsigned int IBox_t::GetVertexCount() const
{
    return 8;
}


///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
void IBox_t::SetBlur(const int iBlur)
{
    for (int i = 0; i < GetVertexCount(); i++)
        m_vertex[i].m_flBlurAmmount = static_cast<float>(iBlur);
}


///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
const Vertex* IBox_t::GetVertexData() const
{
    return m_vertex;
}


///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
void IBox_t::SetColor(unsigned char r, unsigned char g, unsigned char b, unsigned char a)
{
    for (int i = 0; i < GetVertexCount(); i++)
        m_vertex[i].m_clr.Set(r, g, b, a);
}


///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
void IBox_t::InvertColors(bool bInvert)
{
    for (int i = 0; i < GetVertexCount(); i++)
        m_vertex[i].m_flInvertColors = (bInvert == true ? 1.0f : 0.0f);
}


///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
void IBox_t::SetColor(RGBA_t clr, int vertex)
{
    vertex = std::clamp<int>(vertex, 0, GetVertexCount() - 1);

    switch (vertex)
    {
    case IBox_t::VertexType_TopLeft:
    case IBox_t::VertexType_TopLeft_Dup:
    {
        m_vertex[VertexType_TopLeft].m_clr.Set(clr.r, clr.g, clr.b, clr.a);
        m_vertex[VertexType_TopLeft_Dup].m_clr.Set(clr.r, clr.g, clr.b, clr.a);
        break;
    }
    case IBox_t::VertexType_BottomLeft:
    case IBox_t::VertexType_BottomLeft_Dup:
    {
        m_vertex[VertexType_BottomLeft].m_clr.Set(clr.r, clr.g, clr.b, clr.a);
        m_vertex[VertexType_BottomLeft_Dup].m_clr.Set(clr.r, clr.g, clr.b, clr.a);
        break;
    }
    case IBox_t::VertexType_BottomRight:
    case IBox_t::VertexType_BottomRight_Dup:
    {
        m_vertex[VertexType_BottomRight].m_clr.Set(clr.r, clr.g, clr.b, clr.a);
        m_vertex[VertexType_BottomRight_Dup].m_clr.Set(clr.r, clr.g, clr.b, clr.a);
        break;
    }
    case IBox_t::VertexType_TopRight:
    case IBox_t::VertexType_TopRight_Dup:
    {
        m_vertex[VertexType_TopRight].m_clr.Set(clr.r, clr.g, clr.b, clr.a);
        m_vertex[VertexType_TopRight_Dup].m_clr.Set(clr.r, clr.g, clr.b, clr.a);
        break;
    }
    default: break;
    }
}


///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
void IBox_t::InitRelativeUV()
{
    // Min
    m_vertex[VertexType_TopLeft].m_vPos         = vec(0.0f, 0.0f, 0.0f);
    
    // Bottom left
    m_vertex[VertexType_BottomLeft].m_vPos      = vec(0.0f, 1.0f, 0.0f);
    m_vertex[VertexType_BottomLeft_Dup].m_vPos  = m_vertex[VertexType_BottomLeft].m_vPos;

    // Max
    m_vertex[VertexType_BottomRight].m_vPos     = vec(1.0f, 1.0f, 0.0f);
    m_vertex[VertexType_BottomRight_Dup].m_vPos = m_vertex[VertexType_BottomRight].m_vPos;

    // Top right
    m_vertex[VertexType_TopRight].m_vPos        = vec(1.0f, 0.0f, 0.0f);
    m_vertex[VertexType_TopRight_Dup].m_vPos    = m_vertex[VertexType_TopRight].m_vPos;

    // Min
    m_vertex[VertexType_TopLeft_Dup].m_vPos     = m_vertex[VertexType_TopLeft].m_vPos;
}


///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
void Box2D_t::InitDimension()
{
    m_bIs3D = false;

    for (int i = 0; i < GetVertexCount(); i++)
        m_vertex[i].m_flStrictly2D = FLOAT_TRUE;
}


///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
void Box2D_t::SetVertex(const vec& vMin, const vec& vMax)
{
    vec vTopRight   = vec(vMax.x, vMin.y, 0.0f);
    vec vBottomLeft = vec(vMin.x, vMax.y, 0.0f);

    m_vertex[VertexType_TopLeft].m_vPos         = vMin;
    m_vertex[VertexType_BottomLeft].m_vPos      = vBottomLeft;
    m_vertex[VertexType_BottomLeft_Dup].m_vPos  = vBottomLeft;
    m_vertex[VertexType_BottomRight].m_vPos     = vMax;
    m_vertex[VertexType_BottomRight_Dup].m_vPos = vMax;
    m_vertex[VertexType_TopRight].m_vPos        = vTopRight;
    m_vertex[VertexType_TopRight_Dup].m_vPos    = vTopRight;
    m_vertex[VertexType_TopLeft_Dup].m_vPos     = vMin;
}


///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
void Box3D_t::InitDimension()
{
    m_bIs3D = true;

    for (int i = 0; i < GetVertexCount(); i++)
        m_vertex[i].m_flStrictly2D = FLOAT_FALSE;
}


///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
void Box3D_t::SetVertex(const vec& vMin, const vec& vMax, const qangle& qNormal)
{
    vec vForward; Maths::AngleVectors(qNormal, &vForward); vForward.NormalizeInPlace();
    vec vDiagonal = vMax - vMin; vDiagonal.NormalizeInPlace();

    if (vForward.HasSameDirection(vDiagonal) == true)
        return;

    vec vDiagonal2 = vDiagonal.CrossProduct(vForward); vDiagonal2.NormalizeInPlace();

    // Calculating both axis perpendicular in the plane of the rectangle.
    vec vAxis1 = vDiagonal2 + vDiagonal;        vAxis1.NormalizeInPlace();
    vec vAxis2 = vAxis1.CrossProduct(vForward); vAxis2.NormalizeInPlace();

    // magnitude along each axis of the half daigonal.
    vec   vHalfDaigonal = (vMax - vMin) / 2.0f;
    float flOffset1     = vHalfDaigonal.Dot(vAxis1);
    float flOffset2     = vHalfDaigonal.Dot(vAxis2);

    // Adding magnitude along each axis in opposite order to get the other 2 points.
    vec vCenter     = (vMin + vMax) / 2.0;
    vec vTopRight   = vCenter + (vAxis1 * flOffset1 * -1.0f) + (vAxis2 * flOffset2);
    vec vBottomLeft = vCenter + (vAxis1 * flOffset1) + (vAxis2 * flOffset2 * -1.0f);

    m_vertex[VertexType_TopLeft].m_vPos         = vMin;
    m_vertex[VertexType_BottomLeft].m_vPos      = vBottomLeft;
    m_vertex[VertexType_BottomLeft_Dup].m_vPos  = vBottomLeft;
    m_vertex[VertexType_BottomRight].m_vPos     = vMax;
    m_vertex[VertexType_BottomRight_Dup].m_vPos = vMax;
    m_vertex[VertexType_TopRight].m_vPos        = vTopRight;
    m_vertex[VertexType_TopRight_Dup].m_vPos    = vTopRight;
    m_vertex[VertexType_TopLeft_Dup].m_vPos     = vMin;
}