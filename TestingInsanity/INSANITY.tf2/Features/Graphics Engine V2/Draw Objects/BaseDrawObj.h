#pragma once

#include "../../../Utility/ConsoleLogging.h"
#include "../../../SDK/class/Basic Structures.h"
#include "../Graphics.h"

constexpr float FLOAT_TRUE  = 1.0f;
constexpr float FLOAT_FALSE = 0.0f;

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
        m_flBlurAmmount    = -1.0f;
        m_flRounding       = 0.0f;
        m_vRelativeUV.Init();
        m_flStrictly2D     = 0.0f; 
        m_flInvertColors   = 0.0f;
        m_flScaleY         = -1.0f;
        m_flCircleThinkess = -1.0f;
    }

    vec    m_vPos;
    Vec4   m_clr;
    float  m_flBlurAmmount    = -1.0f;
    float  m_flRounding       = 0.0f;
    vec    m_vRelativeUV;
    float  m_flStrictly2D     = 0.0f;
    float  m_flInvertColors   = 0.0f;
    float  m_flScaleY         = -1.0f;
    float  m_flCircleThinkess = -1.0f; // for value > 0, we assume this draw obj as cirle, else not a circle.
    float  m_flRGBAnimSpeed   = -1.0f;
};
///////////////////////////////////////////////////////////////////////////


///////////////////////////////////////////////////////////////////////////
class IDrawObj_t
{
public:
    virtual unsigned int  GetVertexCount() const = 0;
    virtual void          SetBlur(const int iBlur) = 0;
    virtual void          SetColor(unsigned char r, unsigned char g, unsigned char b, unsigned char a) = 0;
    virtual void          InvertColors(bool bInvert) = 0;
    virtual const Vertex* GetVertexData() const = 0;
    virtual void          SetColor(RGBA_t clr, int vertex) = 0;
    virtual void          SetRGBAnimSpeed(const float flAnimSpeed) = 0;
    
protected:
    virtual void          InitDimension()  = 0; // Handle m_flStrictly2D & m_bIs3D parameter for vertex. ( called in constructor )
    virtual void          InitRelativeUV() = 0; // Set up the relative UV coords for this draw obj.      ( called in constructor )

public:
    inline void DeleteThis()
    {
        delete this;
    }
    void SetDrawInLobby(bool bDrawInLobby);
    void SetDrawInGame(bool bDrawInGame);
    void SetVisible(bool bVisible); // just sets both drawInLobby & drawInGame to false.

    bool Is3D()              const;
    bool ShouldDrawInLobby() const;
    bool ShouldDrawInGame()  const;

protected:
    bool m_bIs3D        = false;
    bool m_bDrawInLobby = true;
    bool m_bDrawInGame  = true;
};
///////////////////////////////////////////////////////////////////////////