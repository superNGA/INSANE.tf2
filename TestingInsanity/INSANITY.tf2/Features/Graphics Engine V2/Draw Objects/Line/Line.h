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
    
    enum VertexType_t : int 
    { 
        VertexType_Min = 0, 
        VertexType_Max 
    };

protected:
    void          InitRelativeUV() override final;


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
        InitDimension();
    }

protected:
    void InitDimension() override final;
};
///////////////////////////////////////////////////////////////////////////


///////////////////////////////////////////////////////////////////////////
class Line3D_t : public ILine_t
{
public:
    Line3D_t() : ILine_t()
    {
        InitDimension();
    }

    void InitDimension() override final;
};
///////////////////////////////////////////////////////////////////////////