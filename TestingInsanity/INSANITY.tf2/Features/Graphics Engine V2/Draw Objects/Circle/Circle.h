#pragma once

#include "../BaseDrawObj.h"


///////////////////////////////////////////////////////////////////////////
class ICircle_t : public IDrawObj_t
{
public:
    ICircle_t();

    unsigned int  GetVertexCount()          const override;
    void          SetBlur(const int iBlur)        override;
    const Vertex* GetVertexData()           const override;
    void          SetColor(unsigned char r, unsigned char g, unsigned char b, unsigned char a)      override;
    void          InvertColors(bool bInvert)      override;

    enum VertexType_t : int
    {
        VertexType_TopLeft = 0,
        VertexType_TopRight,
        VertexType_BottomLeft,
        VertexType_TopRight_Dup,
        VertexType_BottomLeft_Dup,
        VertexType_BottomRight
    };

    virtual void SetColor(RGBA_t clr, int vertex) override final;

protected:
    void InitRelativeUV() override final;

protected:
    Vertex m_vertex[6];
};
///////////////////////////////////////////////////////////////////////////


///////////////////////////////////////////////////////////////////////////
class Circle2D_t : public ICircle_t
{
public:
    Circle2D_t() : ICircle_t()
    {
        InitDimension();
    }

    void InitDimension() override final;
    void SetVertex(const vec& vMin, const vec& vMax);
    void SetVertex(const vec& vCenter, const float flRadius);

    void SetThickness(const float flThicknessInPixels);
};
///////////////////////////////////////////////////////////////////////////


///////////////////////////////////////////////////////////////////////////
class Circle3D_t : public ICircle_t
{
public:
    Circle3D_t() : ICircle_t()
    {
        InitDimension();
    }

    void InitDimension() override final;
    void SetVertex(const vec& vMin, const vec& vMax, const qangle& qNormal);
    void SetThickness(const float flThickenssInPrcnt);
};
///////////////////////////////////////////////////////////////////////////