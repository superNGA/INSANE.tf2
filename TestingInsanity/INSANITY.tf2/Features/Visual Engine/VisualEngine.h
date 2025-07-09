#pragma once

#include "../FeatureHandler.h"
#include "../../SDK/class/Basic Structures.h"

class VisualEngine_t
{
public:
    void Run(LPDIRECT3DDEVICE9 pDevice);
    void SetW2SMatrix();

private:
    bool _CompileShaders(LPDIRECT3DDEVICE9 pDevice);
    bool _AllocBuffer(LPDIRECT3DDEVICE9 pDevice);
    bool _DeclareVertex(LPDIRECT3DDEVICE9 pDevice);

    LPD3DXEFFECT            m_pEffect       = nullptr;
    IDirect3DVertexBuffer9* m_pVertexBuffer = nullptr;
    IDirect3DStateBlock9*   m_pStateBlock   = nullptr;

    IDirect3DVertexDeclaration9* m_pVertexDecl = nullptr;
};

DECLARE_FEATURE_OBJECT(visualEngine, VisualEngine_t)


DEFINE_TAB(VisualEngine, 1000);
DEFINE_SECTION(VisualEngine, "VisualEngine", 1);

DEFINE_FEATURE(Z, FloatSlider_t, VisualEngine, VisualEngine, 1, 
    FloatSlider_t(0.0f, -100.0f, 100.0))

/*

DirectX9 Graphics Rendering pipeline : 
Vertex Buffer -> Vertex shader -> Rasterizer -> Pixel shader -> Output

NOTE:
    DirectX9 shader structs must be schematics. Else illegal.
*/