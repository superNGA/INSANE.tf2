#pragma once

#include "../BaseDrawObj.h"

///////////////////////////////////////////////////////////////////////////
class BoxFilled_t : public IDrawObj_t
{
public:
    BoxFilled_t();

    unsigned int GetVertexCount()          const override;
    void         SetBlur(const int iBlur)        override;
    void         SetAs2DObject(bool bIs2D)       override;
    const Vertex* GetVertexData()          const override;
    void         SetColor(unsigned char r, unsigned char g, unsigned char b, unsigned char a)      override;
    void         InvertColors()                  override;

    void         SetPoints(const vec& vMin, const vec& vMax);

private:
    Vertex m_vertex[6];
};
///////////////////////////////////////////////////////////////////////////