// 17 08 25
#pragma once

#include <chrono>
#include <vector>
#include <string>
#include <list>
#include <atomic>
#include "../FeatureHandler.h"


class IMaterial;
struct KeyValues;
struct Material_t;
struct MaterialBundle_t;
struct TokenInfo_t;


///////////////////////////////////////////////////////////////////////////
enum TokenType_t : int
{
    TOKEN_UNDEFINED = -1,
    TOKEN_KEYWORD = 0, TOKEN_VALUE,
    TOKEN_COMMENT, TOKEN_PARENT,
};
struct TokenInfo_t
{
    int         m_iLine = 0;
    int         m_iCol = 0;
    TokenType_t m_iTokenType = TokenType_t::TOKEN_UNDEFINED;
    std::string m_szToken = "";

    void Reset();
};
///////////////////////////////////////////////////////////////////////////


///////////////////////////////////////////////////////////////////////////
struct Material_t
{
    char m_materialData[2048] = "";
    std::list<TokenInfo_t> m_listTokens;
    IMaterial* m_pMaterial = nullptr;
    KeyValues* m_pKeyValues = nullptr;
    bool m_bSaved = true;

    std::string m_szMatName = "( null )";
    std::string m_szParentName = "( null )";

    char m_szRenameBuffer[128] = "";
    bool m_bRenameActive = true;
};
struct MaterialBundle_t
{
    std::string m_szMatBundleName = "( null )";
    std::vector<Material_t*> m_vecMaterials;

    char m_szRenameBuffer[128] = "";
    bool m_bExpanded = false;
    bool m_bRenameActive = true;
    bool operator==(const MaterialBundle_t& other) const
    {
        return m_szMatBundleName == other.m_szMatBundleName;
    }
};
///////////////////////////////////////////////////////////////////////////


///////////////////////////////////////////////////////////////////////////
class MaterialGen_t
{
public:
    MaterialGen_t();

    void Run();
    void Free();

    void SetVisible(bool bVisible);
    bool IsVisible() const;

    std::vector<Material_t*>* GetModelMaterials();

private:

    // Drawing...
    bool m_bVisible = false;
    void _DrawImGui();
    void _DrawTextEditor(float flWidth, float flHeight, float x, float y, float flRounding);
    void _DrawMaterialList(float flWidth, float flHeight, float x, float y, float flRounding);
    void _DrawTitleBar(float flWidth, float flHeight, float x, float y, float flRounding);


    // Hanlding text buffers here...
    void _ProcessBuffer(const char* szBuffer, uint32_t iBufferSize);
    void _SplitBuffer(std::list<TokenInfo_t>& listTokensOut, const char* szBuffer, uint32_t iBufferSize) const;
    void _ProcessTokens(std::list<TokenInfo_t>& listTokenOut) const;

    // It's rendering on top of my MatGen. so I removed it :).
    void _DisableGameConsole();
    
    // Handling model.
    void _AdjustCamera();
    void _RotateModel();

    std::chrono::high_resolution_clock::time_point m_lastModelRotateTime;


    // Handling Materials here...
    std::vector<MaterialBundle_t> m_vecMatBundles;
    std::atomic<int> m_iActiveMatBundleIndex;
    Material_t* m_pActiveTEMaterial = nullptr; // This is the material drawn to TextEditor

    void _CreateMaterialBundle();
    void _AddMaterialToBundle(MaterialBundle_t& matBundle);
    void _DeleteMaterialBundle(MaterialBundle_t& matBundle);

    void _RefreshMaterial(Material_t* pMaterial);

    void _MakeMaterialNameUnique(std::string& szNameOut, const std::string & szBaseName, MaterialBundle_t& parentBundle) const;
    void _MakeMaterialBundleNameUnique(std::string& szNameOut, const std::string & szBaseName) const;
};
///////////////////////////////////////////////////////////////////////////


// VMT Keyword
static std::vector<std::string> g_vecVMTKeyWords = {
    "\"$basetexture\"",
    "\"$texture2\"",
    "\"$basetexture2\"",
    "\"$basetexturetransform\"",
    "\"$texture2transform\"",
    "\"$bumpmap\"",
    "\"$normalmap\"",
    "\"$normalmapalphaenvmapmask\"",
    "\"$basealphaenvmapmask\"",
    "\"$detail\"",
    "\"$detailscale\"",
    "\"$detailblendfactor\"",
    "\"$detailblendmode\"",
    "\"$alphatest\"",
    "\"$alpha\"",
    "\"$additive\"",
    "\"$translucent\"",
    "\"$vertexalpha\"",
    "\"$vertexcolor\"",
    "\"$vertexfog\"",
    "\"$color\"",
    "\"$color2\"",
    "\"$cloakcolortint\"",
    "\"$cloakfactor\"",
    "\"$frame\"",
    "\"$frameRate\"",
    "\"$framecount\"",
    "\"$model\"",
    "\"$surfaceprop\"",
    "\"$decal\"",
    "\"$ignorez\"",
    "\"$nofog\"",
    "\"$flat\"",
    "\"$wireframe\"",
    "\"$nocull\"",
    "\"$nodecal\"",
    "\"$no_fullbright\"",
    "\"$no_draw\"",
    "\"$halflambert\"",
    "\"$selfillum\"",
    "\"$selfillumtint\"",
    "\"$softwareskin\"",
    "\"$srgbtint\"",
    "\"$opaquetexture\"",
    "\"$phong\"",
    "\"$phongboost\"",
    "\"$phongexponent\"",
    "\"$phongexponenttexture\"",
    "\"$phongalbedotint\"",
    "\"$envmap\"",
    "\"$envmapsphere\"",
    "\"$envmapcameraspace\"",
    "\"$envmapmode\"",
    "\"$envmapcontrast\"",
    "\"$envmapsaturation\"",
    "\"$envmaptint\"",
    "\"$reflecttexture\"",
    "\"$reflectamount\"",
    "\"$reflecttint\"",
    "\"$reflectskyboxonly\"",
    "\"$reflectonlymarkedentities\"",
    "\"$receiveflashlight\"",
    "\"$singlepassflashlight\"",
    "\"$linearwrite\"",
    "\"$allowalphatocoverage\"",
    "\"$alphaenvmapmask\"",
    "\"$basealphaenvmapmask\"",
    "\"$fogcolor\"",
    "\"$fogenable\"",
    "\"$fogstart\"",
    "\"$fogend\"",
    "\"$lightmapwaterfog\"",
    "\"$abovewater\"",
    "\"$bottommaterial\"",
    "\"$compilewater\"",
    "\"%keywords\"",
    "\"%tooltexture\"",
    "\"%notooltexture\"",
    "\"$decalBlendMode\"",
    "\"$decalblendfactor\"",
    "\"$readlowres\"",
    "\"$texturetransform\"",
    "\"$texoffset\"",
    "\"$scale\"",
    "\"$scale2\"",
    "\"$scale_ofs\"",
    "\"$midofs\"",
    "\"$texresolution\"",
    "\"$surfaceparm\"",
    "\"$flags\"",
    "\"$decalorientation\"",
    "\"$srgb\"",
    "\"$deferred\"",
    "\"$detailblendopacity\"",
    "\"$detailblendpower\"",
    "\"$detailblendfactor\""

    // Proxy Keywords
    "\"Sine\"",
    "\"LinearRamp\"",
    "\"Add\"",
    "\"Subtract\"",
    "\"Multiply\"",
    "\"Divide\"",
    "\"TextureTransform\"",
    "\"TextureScroll\"",
    "\"TextureScale\"",
    "\"TextureRotate\"",
    "\"TextureTransform\"",
    "\"TextureCrop\"",
    "\"TextureFlip\"",
    "\"TextureMask\"",
    "\"TextureBlend\"",
    "\"TransformColor\"",
    "\"Random\"",
    "\"Time\"",
    "\"CWave\"",
    "\"Lightwarp\"",
    "\"Noise\"",
    "\"LightmappedGeneric_Ambient\"",
    "\"Periscope\"",
    "\"LessThan\"",
    "\"GreaterThan\"",
    "\"LessOrEqual\"",
    "\"GreaterOrEqual\"",
    "\"Sequence\"",
    "\"Clamp\"",
    "\"RemapVal\"",
    "\"RemapValClamped\"",
    "\"Switch\"",
    "\"TextureAtlas\"",
    "\"ParticleAge\"",
    "\"PlayerColor\"",
    "\"EntityColor\"",
    "\"VMTGlobalVarProxy\"",
    "\"Skin\"",
    "\"Fresnel\"",
    "\"Compare\"",
    "\"CRC32\"",
    "\"DivideBy" // (some engine forks have variations)

    // Proxy Parameters ( used inside proxy brakets )
    "\"resultVar\"",
    "\"srcVar1\"",
    "\"srcVar2\"",
    "\"srcVar3\"",
    "\"sineperiod\"",
    "\"sinemin\"",
    "\"sinemax\"",
    "\"rate\"",
    "\"initialValue\"",
    "\"scaleVar\"",
    "\"translateVar\"",
    "\"rotateVar\"",
    "\"textureVar\"",
    "\"texture\"",
    "\"xformVar\"",
    "\"texture2transform\"",
    "\"texturetransform\"",
    "\"resultVarIndex\"",
    "\"min\"",
    "\"max\"",
    "\"lower\"",
    "\"upper\"",
    "\"dstVar\"",
    "\"src\"",
    "\"dest\"",
    "\"srcVar\"",
    "\"dstVar\"",
    "\"minvar\"",
    "\"maxvar\"",
    "\"invert\"",
    "\"period\"",
    "\"speed\"",
    "\"startframe\"",
    "\"endframe\"",
    "\"frame\"",
    "\"tile\"",
    "\"rows\"",
    "\"columns\"",
    "\"src1\"",
    "\"src2\""
};

///////////////////////////////////////////////////////////////////////////


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