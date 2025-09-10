#pragma once

#include "../../../SDK/class/Basic Structures.h"
#include "../Graphics.h"


///////////////////////////////////////////////////////////////////////////
enum VertexFlags_t : int
{
    VertexFlags_InvertClr  = (1 << 0),
    VertexFlags_Strictly2D = (1 << 1)
};
///////////////////////////////////////////////////////////////////////////


///////////////////////////////////////////////////////////////////////////
struct Vertex
{
    Vertex()
    {
        m_vPos.Init();
        m_clr.Init();
        m_flBlurAmmount = 0; m_flRounding = 0.0f;
        m_vRelativeUV.Init();
        m_flStrictly2D = 0.0f; m_flInvertColors = 0.0f;
    }

    vec    m_vPos;
    Vec4   m_clr;
    float  m_flBlurAmmount = 0.0f;
    float  m_flRounding    = 0.0f;
    vec    m_vRelativeUV;
    float  m_flStrictly2D  = 0.0f;
    float  m_flInvertColors = 0.0f;
};
///////////////////////////////////////////////////////////////////////////


///////////////////////////////////////////////////////////////////////////
class IDrawObj_t
{
public:
    virtual unsigned int GetVertexCount()           const = 0;
    virtual void         SetBlur(const int iBlur)         = 0;
    virtual void         SetColor(unsigned char r, unsigned char g, unsigned char b, unsigned char a) = 0;
    virtual void         InvertColors()                   = 0;
    virtual void         SetAs2DObject(bool bIs2D)        = 0;
    virtual const Vertex* GetVertexData()           const = 0;

    inline void DeleteThis()
    {
        F::graphics.FindAndRemoveDrawObj(this);
        delete this;
    }
};
///////////////////////////////////////////////////////////////////////////