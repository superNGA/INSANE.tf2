#pragma once
#include "../BaseDrawObj.h"


///////////////////////////////////////////////////////////////////////////
class ILine_t : public IDrawObj_t
{
public:
    ILine_t();

    unsigned int  GetVertexCount() const override;
    void          SetBlur(const int iBlur) override;
    const Vertex* GetVertexData() const override;
    void          SetColor(unsigned char r, unsigned char g, unsigned char b, unsigned char a) override;
    void          InvertColors(bool bInvert) override;

    void          SetPoints(const vec& vMin, const vec& vMax);
    void          SetColor(RGBA_t clr, int vertex) override final;

    void          SetRGBAnimSpeed(const float flAnimSpeed) override final;

protected:
    void InitRelativeUV() override final;

    enum VertexType_t : int 
    { 
        VertexType_Min = 0, 
        VertexType_Max 
    };

protected:
    Vertex m_vertex[2];
};
///////////////////////////////////////////////////////////////////////////


///////////////////////////////////////////////////////////////////////////
class Line2D_t : public ILine_t
{
public:
    Line2D_t() : ILine_t()
    {
        F::graphics.RegisterInLineList(this);
        m_vertex[0].m_flStrictly2D = FLOAT_TRUE;
        m_vertex[1].m_flStrictly2D = FLOAT_TRUE;

        m_bIs3D = false;
    }
};
///////////////////////////////////////////////////////////////////////////


///////////////////////////////////////////////////////////////////////////
class Line3D_t : public ILine_t
{
public:
    Line3D_t() : ILine_t()
    {
        F::graphics.RegisterInLineList(this);
        m_vertex[0].m_flStrictly2D = FLOAT_FALSE;
        m_vertex[1].m_flStrictly2D = FLOAT_FALSE;

        m_bIs3D = true;
    }
};
///////////////////////////////////////////////////////////////////////////