#pragma once

#include "../BaseDrawObj.h"


///////////////////////////////////////////////////////////////////////////
class Cube3D_t : public IDrawObj_t
{
public:
    Cube3D_t();

    unsigned int  GetVertexCount()                         const override final;
    void          SetBlur(const int iBlur)                 override final;
    void          SetColor(unsigned char r, unsigned char g, unsigned char b, unsigned char a) override final;
    void          InvertColors(bool bInvert)               override final;
    const Vertex* GetVertexData()                          const override final;
    void          SetColor(RGBA_t clr, int vertex)         override final;
    void          SetRGBAnimSpeed(const float flAnimSpeed) override final;

    void          SetVertex(const vec& vMin, const vec& vMax, qangle qOrientation);
    void          SetVertex(const vec& vMin, const vec& vMax, const matrix3x4_t& matrix);
protected:
    void          InitDimension()  override final; // Handle m_flStrictly2D & m_bIs3D parameter for vertex. ( called in constructor )
    void          InitRelativeUV() override final; // Set up the relative UV coords for this draw obj.      ( called in constructor )

private:
    Vertex m_vertex[24];

public:
    enum VertexType_t : int
    {
        // Front face
        VertexType_TopLeftFront = 0,
        VertexType_BottomLeftFront,
        VertexType_BottomLeftFront_Dup,
        VertexType_BottomRightFront,
        VertexType_BottomRightFront_Dup,
        VertexType_TopRightFront,
        VertexType_TopRightFront_Dup,
        VertexType_TopLeftFront_Dup,

        // Lines connecting Front & Back face.
        VertexType_TopLeftSideFront,
        VertexType_TopLeftSideBack,
        VertexType_BottomLeftSideFront,
        VertexType_BottomLeftSideBack,
        VertexType_BottomRightSideFront,
        VertexType_BottomRightSideBack,
        VertexType_TopRightSideFront,
        VertexType_TopRightSideBack,

        // Back face
        VertexType_TopLeftBack,
        VertexType_BottomLeftBack,
        VertexType_BottomLeftBack_Dup,
        VertexType_BottomRightBack,
        VertexType_BottomRightBack_Dup,
        VertexType_TopRightBack,
        VertexType_TopRightBack_Dup,
        VertexType_TopLeftBack_Dup,
    };
};
///////////////////////////////////////////////////////////////////////////