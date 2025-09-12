#pragma once

#include "../BaseDrawObj.h"


///////////////////////////////////////////////////////////////////////////
class IBoxFilled_t : public IDrawObj_t
{
public:
    IBoxFilled_t();

    unsigned int GetVertexCount()          const override;
    void         SetBlur(const int iBlur)        override;
    const Vertex* GetVertexData()          const override;
    void         SetColor(unsigned char r, unsigned char g, unsigned char b, unsigned char a)      override;
    void         InvertColors(bool bInvert)      override;

    void SetRounding(float flRounding);

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
        m_bIs3D = true;

        for (int i = 0; i < GetVertexCount(); i++)
            m_vertex[i].m_flStrictly2D = FLOAT_FALSE;
    }
    
    void SetVertex(const vec& vMin, const vec& vMax, const qangle& qNormal);
};
///////////////////////////////////////////////////////////////////////////


///////////////////////////////////////////////////////////////////////////
class BoxFilled2D_t : public IBoxFilled_t
{
public:
    BoxFilled2D_t() : IBoxFilled_t()
    {
        m_bIs3D = false;

        for (int i = 0; i < GetVertexCount(); i++)
            m_vertex[i].m_flStrictly2D = FLOAT_TRUE;
    }

    void SetVertex(const vec& vMin, const vec& vMax);
};
///////////////////////////////////////////////////////////////////////////


///////////////////////////////////////////////////////////////////////////
class Box2D_t : public IBox_t
{
public:
    Box2D_t() : IBox_t()
    {
        m_bIs3D = false;

        for (int i = 0; i < GetVertexCount(); i++)
            m_vertex[i].m_flStrictly2D = FLOAT_TRUE;
    }

    void SetVertex(const vec& vMin, const vec& vMax);
};
///////////////////////////////////////////////////////////////////////////


///////////////////////////////////////////////////////////////////////////
class Box3D_t : public IBox_t
{
public:
    Box3D_t() : IBox_t()
    {
        m_bIs3D = true;

        for (int i = 0; i < GetVertexCount(); i++)
            m_vertex[i].m_flStrictly2D = FLOAT_FALSE;
    }

    void SetVertex(const vec& vMin, const vec& vMax, const qangle& qNormal);
};
///////////////////////////////////////////////////////////////////////////