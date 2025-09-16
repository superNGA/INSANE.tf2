#include "Cube.h"


///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
Cube3D_t::Cube3D_t()
{
    F::graphics.RegisterInLineList(this);

    InitDimension();
    InitRelativeUV();
}


///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
unsigned int Cube3D_t::GetVertexCount() const
{
    return sizeof(m_vertex) / sizeof(Vertex);
}


///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
void Cube3D_t::SetBlur(const int iBlur)
{
    for (int i = 0; i < GetVertexCount(); i++)
        m_vertex[i].m_flBlurAmmount = static_cast<float>(iBlur);
}


///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
void Cube3D_t::SetColor(unsigned char r, unsigned char g, unsigned char b, unsigned char a)
{
    for (int i = 0; i < GetVertexCount(); i++)
        m_vertex[i].m_clr.Set(r, g, b, a);
}


///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
void Cube3D_t::InvertColors(bool bInvert)
{
    for (int i = 0; i < GetVertexCount(); i++)
        m_vertex[i].m_flInvertColors = (bInvert == true ? FLOAT_TRUE : FLOAT_FALSE);
}


///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
const Vertex* Cube3D_t::GetVertexData() const
{
    return m_vertex;
}


///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
void Cube3D_t::SetColor(RGBA_t clr, int vertex)
{
    vertex = std::clamp<int>(vertex, 0, GetVertexCount() - 1);

    switch (vertex)
    {
    case Cube3D_t::VertexType_TopLeftFront:
    case Cube3D_t::VertexType_TopLeftFront_Dup:
    case Cube3D_t::VertexType_TopLeftSideFront:
    {
        m_vertex[VertexType_TopLeftFront].m_clr.Set(clr.r, clr.g, clr.b, clr.a);
        m_vertex[VertexType_TopLeftFront_Dup].m_clr.Set(clr.r, clr.g, clr.b, clr.a);
        m_vertex[VertexType_TopLeftSideFront].m_clr.Set(clr.r, clr.g, clr.b, clr.a);
        break;
    }
    case Cube3D_t::VertexType_BottomLeftFront:
    case Cube3D_t::VertexType_BottomLeftFront_Dup:
    case Cube3D_t::VertexType_BottomLeftSideFront:
    {
        m_vertex[VertexType_BottomLeftFront].m_clr.Set(clr.r, clr.g, clr.b, clr.a);
        m_vertex[VertexType_BottomLeftFront_Dup].m_clr.Set(clr.r, clr.g, clr.b, clr.a);
        m_vertex[VertexType_BottomLeftSideFront].m_clr.Set(clr.r, clr.g, clr.b, clr.a);
        break;
    }
    case Cube3D_t::VertexType_BottomRightFront:
    case Cube3D_t::VertexType_BottomRightFront_Dup:
    case Cube3D_t::VertexType_BottomRightSideFront:
    {
        m_vertex[VertexType_BottomRightFront].m_clr.Set(clr.r, clr.g, clr.b, clr.a);
        m_vertex[VertexType_BottomRightFront_Dup].m_clr.Set(clr.r, clr.g, clr.b, clr.a);
        m_vertex[VertexType_BottomRightSideFront].m_clr.Set(clr.r, clr.g, clr.b, clr.a);
        break;
    }
    case Cube3D_t::VertexType_TopRightFront:
    case Cube3D_t::VertexType_TopRightFront_Dup:
    case Cube3D_t::VertexType_TopRightSideFront:
    {
        m_vertex[VertexType_TopRightFront].m_clr.Set(clr.r, clr.g, clr.b, clr.a);
        m_vertex[VertexType_TopRightFront_Dup].m_clr.Set(clr.r, clr.g, clr.b, clr.a);
        m_vertex[VertexType_TopRightSideFront].m_clr.Set(clr.r, clr.g, clr.b, clr.a);
        break;
    }
    case Cube3D_t::VertexType_TopLeftBack:
    case Cube3D_t::VertexType_TopLeftBack_Dup:
    case Cube3D_t::VertexType_TopLeftSideBack:
    {
        m_vertex[VertexType_TopLeftBack].m_clr.Set(clr.r, clr.g, clr.b, clr.a);
        m_vertex[VertexType_TopLeftBack_Dup].m_clr.Set(clr.r, clr.g, clr.b, clr.a);
        m_vertex[VertexType_TopLeftSideBack].m_clr.Set(clr.r, clr.g, clr.b, clr.a);
        break;
    }
    case Cube3D_t::VertexType_BottomLeftBack:
    case Cube3D_t::VertexType_BottomLeftBack_Dup:
    case Cube3D_t::VertexType_BottomLeftSideBack:
    {
        m_vertex[VertexType_BottomLeftBack].m_clr.Set(clr.r, clr.g, clr.b, clr.a);
        m_vertex[VertexType_BottomLeftBack_Dup].m_clr.Set(clr.r, clr.g, clr.b, clr.a);
        m_vertex[VertexType_BottomLeftSideBack].m_clr.Set(clr.r, clr.g, clr.b, clr.a);
        break;
    }
    case Cube3D_t::VertexType_BottomRightBack:
    case Cube3D_t::VertexType_BottomRightBack_Dup:
    case Cube3D_t::VertexType_BottomRightSideBack:
    {
        m_vertex[VertexType_BottomRightBack].m_clr.Set(clr.r, clr.g, clr.b, clr.a);
        m_vertex[VertexType_BottomRightBack_Dup].m_clr.Set(clr.r, clr.g, clr.b, clr.a);
        m_vertex[VertexType_BottomRightSideBack].m_clr.Set(clr.r, clr.g, clr.b, clr.a);
        break;
    }
    case Cube3D_t::VertexType_TopRightBack:
    case Cube3D_t::VertexType_TopRightBack_Dup:
    case Cube3D_t::VertexType_TopRightSideBack:
    {
        m_vertex[VertexType_TopRightBack].m_clr.Set(clr.r, clr.g, clr.b, clr.a);
        m_vertex[VertexType_TopRightBack_Dup].m_clr.Set(clr.r, clr.g, clr.b, clr.a);
        m_vertex[VertexType_TopRightSideBack].m_clr.Set(clr.r, clr.g, clr.b, clr.a);
        break;
    }
    default: break;
    }
}


///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
void Cube3D_t::SetRGBAnimSpeed(const float flAnimSpeed)
{
    for (int i = 0; i < GetVertexCount(); i++)
        m_vertex[i].m_flRGBAnimSpeed = flAnimSpeed;
}


///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
void Cube3D_t::InitDimension()
{
    m_bIs3D = true;

    for (int i = 0; i < GetVertexCount(); i++)
        m_vertex[i].m_flStrictly2D = FLOAT_FALSE;
}


///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
void Cube3D_t::SetVertex(const vec& vMin, const vec& vMax)
{
    vec vTopLeftBack(vMin.x, vMin.y, vMax.z);
    vec vTopRightFront(vMax.x, vMin.y, vMin.z);   vec vTopRightBack(vMax.x, vMin.y, vMax.z);
    vec vBottomLeftFront(vMin.x, vMax.y, vMin.z); vec vBottomLeftBack(vMin.x, vMax.y, vMax.z);
    vec vBottomRightFront(vMax.x, vMax.y, vMin.z);

    // Top Left Front.
    m_vertex[VertexType_TopLeftFront].m_vPos         = vMin;
    m_vertex[VertexType_TopLeftFront_Dup].m_vPos     = vMin;
    m_vertex[VertexType_TopLeftSideFront].m_vPos     = vMin;
    
    // Top Left Back
    m_vertex[VertexType_TopLeftBack].m_vPos          = vTopLeftBack;
    m_vertex[VertexType_TopLeftBack_Dup].m_vPos      = vTopLeftBack;
    m_vertex[VertexType_TopLeftSideBack].m_vPos      = vTopLeftBack;

    // Bottom Right Back
    m_vertex[VertexType_BottomRightBack].m_vPos      = vMax;
    m_vertex[VertexType_BottomRightBack_Dup].m_vPos  = vMax;
    m_vertex[VertexType_BottomRightSideBack].m_vPos  = vMax;
    
    // Bottom Right Front
    m_vertex[VertexType_BottomRightFront].m_vPos     = vBottomRightFront;
    m_vertex[VertexType_BottomRightFront_Dup].m_vPos = vBottomRightFront;
    m_vertex[VertexType_BottomRightSideFront].m_vPos = vBottomRightFront;

    // Top Right Front
    m_vertex[VertexType_TopRightFront].m_vPos        = vTopRightFront;
    m_vertex[VertexType_TopRightFront_Dup].m_vPos    = vTopRightFront;
    m_vertex[VertexType_TopRightSideFront].m_vPos    = vTopRightFront;
                                                            
    // Top Right Back                                       
    m_vertex[VertexType_TopRightBack].m_vPos         = vTopRightBack;
    m_vertex[VertexType_TopRightBack_Dup].m_vPos     = vTopRightBack;
    m_vertex[VertexType_TopRightSideBack].m_vPos     = vTopRightBack;
                                                            
    // Bottom Left Front                                    
    m_vertex[VertexType_BottomLeftFront].m_vPos      = vBottomLeftFront;
    m_vertex[VertexType_BottomLeftFront_Dup].m_vPos  = vBottomLeftFront;
    m_vertex[VertexType_BottomLeftSideFront].m_vPos  = vBottomLeftFront;
                                                            
    // Bottom Left Back                                     
    m_vertex[VertexType_BottomLeftBack].m_vPos       = vBottomLeftBack;
    m_vertex[VertexType_BottomLeftBack_Dup].m_vPos   = vBottomLeftBack;
    m_vertex[VertexType_BottomLeftSideBack].m_vPos   = vBottomLeftBack;
}


///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
void Cube3D_t::InitRelativeUV()
{
    // Top Left Front.
    m_vertex[VertexType_TopLeftFront].m_vRelativeUV         = vec(0.0f, 0.0f, 0.0f);
    m_vertex[VertexType_TopLeftFront_Dup].m_vRelativeUV     = vec(0.0f, 0.0f, 0.0f);
    m_vertex[VertexType_TopLeftSideFront].m_vRelativeUV     = vec(0.0f, 0.0f, 0.0f);
    
    // Top Left Back
    m_vertex[VertexType_TopLeftBack].m_vRelativeUV          = vec(0.0f, 0.0f, 1.0f);
    m_vertex[VertexType_TopLeftBack_Dup].m_vRelativeUV      = vec(0.0f, 0.0f, 1.0f);
    m_vertex[VertexType_TopLeftSideBack].m_vRelativeUV      = vec(0.0f, 0.0f, 1.0f);

    // Bottom Right Front
    m_vertex[VertexType_BottomRightFront].m_vRelativeUV     = vec(1.0f, 1.0f, 0.0f);
    m_vertex[VertexType_BottomRightFront_Dup].m_vRelativeUV = vec(1.0f, 1.0f, 0.0f);
    m_vertex[VertexType_BottomRightSideFront].m_vRelativeUV = vec(1.0f, 1.0f, 0.0f);

    // Bottom Right Back
    m_vertex[VertexType_BottomRightBack].m_vRelativeUV      = vec(1.0f, 1.0f, 1.0f);
    m_vertex[VertexType_BottomRightBack_Dup].m_vRelativeUV  = vec(1.0f, 1.0f, 1.0f);
    m_vertex[VertexType_BottomRightSideBack].m_vRelativeUV  = vec(1.0f, 1.0f, 1.0f);

    // Top Right Front
    m_vertex[VertexType_TopRightFront].m_vRelativeUV        = vec(1.0f, 0.0f, 0.0f);
    m_vertex[VertexType_TopRightFront_Dup].m_vRelativeUV    = vec(1.0f, 0.0f, 0.0f);
    m_vertex[VertexType_TopRightSideFront].m_vRelativeUV    = vec(1.0f, 0.0f, 0.0f);
                                                            
    // Top Right Back                                       
    m_vertex[VertexType_TopRightBack].m_vRelativeUV         = vec(1.0f, 0.0f, 1.0f);
    m_vertex[VertexType_TopRightBack_Dup].m_vRelativeUV     = vec(1.0f, 0.0f, 1.0f);
    m_vertex[VertexType_TopRightSideBack].m_vRelativeUV     = vec(1.0f, 0.0f, 1.0f);
                                                            
    // Bottom Left Front                                    
    m_vertex[VertexType_BottomLeftFront].m_vRelativeUV      = vec(0.0f, 1.0f, 0.0f);
    m_vertex[VertexType_BottomLeftFront_Dup].m_vRelativeUV  = vec(0.0f, 1.0f, 0.0f);
    m_vertex[VertexType_BottomLeftSideFront].m_vRelativeUV  = vec(0.0f, 1.0f, 0.0f);
                                                            
    // Bottom Left Back                                     
    m_vertex[VertexType_BottomLeftBack].m_vRelativeUV       = vec(0.0f, 1.0f, 1.0f);
    m_vertex[VertexType_BottomLeftBack_Dup].m_vRelativeUV   = vec(0.0f, 1.0f, 1.0f);
    m_vertex[VertexType_BottomLeftSideBack].m_vRelativeUV   = vec(0.0f, 1.0f, 1.0f);
}
