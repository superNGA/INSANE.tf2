#pragma once

#include "../BaseDrawObj.h"


///////////////////////////////////////////////////////////////////////////
class IBoxFilled_t : public IDrawObj_t
{
public:
    IBoxFilled_t();

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
    void         SetColor(RGBA_t clr, int vertex) override final;
    void         SetRGBAnimSpeed(const float flAnimSpeed) override final;

    // Functions unique to this class ( not inherited )
    virtual void SetRounding(float flRoundingInPixels);

protected:
    void InitRelativeUV() override final;

protected:
    Vertex m_vertex[6];
};
///////////////////////////////////////////////////////////////////////////


///////////////////////////////////////////////////////////////////////////
class IBox_t : public IDrawObj_t
{
public:
    IBox_t();

    unsigned int  GetVertexCount()          const override;
    void          SetBlur(const int iBlur)  override;
    const Vertex* GetVertexData()          const override;
    void          SetColor(unsigned char r, unsigned char g, unsigned char b, unsigned char a) override;
    void          InvertColors(bool bInvert) override;

    enum VertexType_t : int
    {
        VertexType_TopLeft,
        VertexType_BottomLeft,
        VertexType_BottomLeft_Dup,
        VertexType_BottomRight,
        VertexType_BottomRight_Dup,
        VertexType_TopRight,
        VertexType_TopRight_Dup,
        VertexType_TopLeft_Dup
    };
    void SetColor(RGBA_t clr, int vertex) override final; 
    void SetRGBAnimSpeed(const float flAnimSpeed) override final;

protected:
    void InitRelativeUV() override final;

protected:
    Vertex m_vertex[8];
};
///////////////////////////////////////////////////////////////////////////


///////////////////////////////////////////////////////////////////////////
class BoxFilled3D_t : public IBoxFilled_t
{
public:
    BoxFilled3D_t() : IBoxFilled_t()
    {
        InitDimension();
    }
    
    void InitDimension() override final;
    void SetVertex(const vec& vMin, const vec& vMax, const qangle& qNormal);
};
///////////////////////////////////////////////////////////////////////////


///////////////////////////////////////////////////////////////////////////
class BoxFilled2D_t : public IBoxFilled_t
{
public:
    BoxFilled2D_t() : IBoxFilled_t()
    {
        InitDimension();
    }

    void InitDimension() override final;
    void SetVertex(const vec& vMin, const vec& vMax);

    void SetRounding(float flRoundingInPixels) override final;
    void SetRoundingInPercentage(float flPercantage);
};
///////////////////////////////////////////////////////////////////////////


///////////////////////////////////////////////////////////////////////////
class Box2D_t : public IBox_t
{
public:
    Box2D_t() : IBox_t()
    {
        InitDimension();
    }

    void InitDimension() override final;
    void SetVertex(const vec& vMin, const vec& vMax);
};
///////////////////////////////////////////////////////////////////////////


///////////////////////////////////////////////////////////////////////////
class Box3D_t : public IBox_t
{
public:
    Box3D_t() : IBox_t()
    {
        InitDimension();
    }

    void InitDimension() override final;
    void SetVertex(const vec& vMin, const vec& vMax, const qangle& qNormal);
};
///////////////////////////////////////////////////////////////////////////