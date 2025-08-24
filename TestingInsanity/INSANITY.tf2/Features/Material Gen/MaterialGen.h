// 17 08 25
#pragma once

#include <chrono>
#include <vector>
#include <string>
#include <list>
#include <atomic>
#include "../FeatureHandler.h"


// Keyword list for .vmt file format. ( this does not include the proxy material Keywords. )
static std::vector<std::string> g_vecVMTKeyWords =
{
    "\"$additive\"",
    "\"$allowalphatocoverage\"",
    "\"$alpha\"",
    "\"$alphamodifiedbyproxy_do_not_set_in_vmt\"",
    "\"$alphatest\"",
    "\"$basealphaenvmapmask\"",
    "\"$basetexture\"",
    "\"$basetexturetransform\"",
    "\"$color\"",
    "\"$color2\"",
    "\"$debug\"",
    "\"$decal\"",
    "\"$envmapcameraspace\"",
    "\"$envmapmode\"",
    "\"$envmapsphere\"",
    "\"$flat\"",
    "\"$frame\"",
    "\"$halflambert\"",
    "\"$ignorez\"",
    "\"$model\"",
    "\"$no_draw\"",
    "\"$no_fullbright\"",
    "\"$nocull\"",
    "\"$nodecal\"",
    "\"$nofog\"",
    "\"$normalmapalphaenvmapmask\"",
    "\"$opaquetexture\"",
    "\"$selfillum\"",
    "\"$softwareskin\"",
    "\"$srgbtint\"",
    "\"$surfaceprop\"",
    "\"$translucent\"",
    "\"$use_in_fillrate_mode\"",
    "\"$vertexalpha\"",
    "\"$vertexcolor\"",
    "\"$vertexfog\"",
    "\"$wireframe\"",
    "\"$xxxxxunusedxxxxx\"",
    "\"$xxxxxxunusedxxxxx\"",
    "\"$znearer\""                                    
};


class MaterialGen_t
{
public:
    MaterialGen_t();

    void Run();

    void SetVisible(bool bVisible);
    bool IsVisible() const;

private:
    bool m_bVisible = false;
    void _DrawImGui();
    void _DrawTextEditor(float flWidth, float flHeight, float x, float y);
    void _DrawMaterialList(float flWidth, float flHeight, float x, float y);

    enum TokenType_t : int
    {
        TOKEN_UNDEFINED = -1,
        TOKEN_KEYWORD = 0, TOKEN_VALUE, 
        TOKEN_COMMENT,     TOKEN_PARENT,
    };
    struct TokenInfo_t
    {
        int         m_iLine      = 0;
        int         m_iCol       = 0;
        TokenType_t m_iTokenType = TokenType_t::TOKEN_UNDEFINED;
        std::string m_szToken    = "";

        void Reset();
    };

    void _ProcessBuffer(const char* szBuffer, uint32_t iBufferSize);
    void _SplitBuffer(std::list<TokenInfo_t>& listTokensOut, const char* szBuffer, uint32_t iBufferSize) const;
    void _ProcessTokens(std::list<TokenInfo_t>& listTokenOut) const;

    // It's rendering on top of my MatGen. so I removed it :).
    void _DisableGameConsole();
    
    // Handling model.
    void _AdjustCamera();
    void _RotateModel();

    std::chrono::high_resolution_clock::time_point m_lastModelRotateTime;

    struct Material_t
    {
        char m_materialData[2048];
        std::list<TokenInfo_t> m_listTokens;

        std::string m_szMatName;
        std::string m_szParentName;
    };
    struct MaterialBundle_t
    {
        std::string m_szMatBundleName;
        std::vector<Material_t*> m_vecMaterials;
        
        bool m_bExpanded     = false;
        bool m_bRenameActive = true;
        bool operator==(const MaterialBundle_t& other) const
        {
            return m_szMatBundleName == other.m_szMatBundleName;
        }
    };
    std::vector<MaterialBundle_t> m_vecMatBundles;
    std::atomic<int> m_iActiveMatBundleIndex;
    Material_t* m_pActiveTEMaterial = nullptr; // This is the material drawn to TextEditor

    void _CreateMaterialBundle();
    void _AddMaterialToBundle(MaterialBundle_t& matBundle);
    void _DeleteMaterialBundle(MaterialBundle_t& matBundle);
};

DECLARE_FEATURE_OBJECT(materialGen, MaterialGen_t)

DEFINE_TAB(MaterialGen, 11)
DEFINE_SECTION(MaterialGen, "MaterialGen", 1)
DEFINE_FEATURE(Enable, bool, MaterialGen, MaterialGen, 1, false)

// Rotation speed is defined as how much angle is covered ( in degrees ) 
// in one second.
DEFINE_FEATURE(RotationSpeed, FloatSlider_t, MaterialGen, MaterialGen, 2, 
    FloatSlider_t(65.0f, -360.0f, 360.0f))

DEFINE_FEATURE(Model, IntSlider_t, MaterialGen, MaterialGen, 3, 
    IntSlider_t(0, 0, 6))