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

    const TokenInfo_t& operator=(TokenInfo_t& other)
    {
        m_szToken    = other.m_szToken;
        m_iTokenType = other.m_iTokenType;
        m_iCol       = other.m_iCol;
        m_iLine      = other.m_iLine;

        return *this;
    }

    void Reset();
};
///////////////////////////////////////////////////////////////////////////


///////////////////////////////////////////////////////////////////////////
struct Material_t
{
    char m_materialData[2048]   = "";
    std::vector<TokenInfo_t> m_vecTokens;
    IMaterial* m_pMaterial      = nullptr;
    KeyValues* m_pKeyValues     = nullptr;
    bool       m_bSaved         = true;

    std::string m_szMatName     = "( null )";
    std::string m_szParentName  = "( null )";

    char m_szRenameBuffer[128]  = "";
    bool m_bRenameActive        = true;
};
struct MaterialBundle_t
{
    std::string m_szMatBundleName = "( null )";
    std::vector<Material_t*> m_vecMaterials;

    char m_szRenameBuffer[128] = "";
    bool m_bExpanded           = false;
    bool m_bRenameActive       = true;

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

    int                GetBestKeywordMatch() const;
    const TokenInfo_t& GetActiveToken() const;

    char*              GetModelSearchBuffer();
    int                GetBestModelName() const;
    void               MoveBestModelNameSuggIndex(int iOffset);
    const std::string& GetModelNameAtIndex(int iIndex) const;

private:

    // Drawing...
    bool m_bVisible = false;
    void _DrawImGui();
    void _DrawTextEditor(float flWidth, float flHeight, float x, float y, float flRounding);
    void _DrawMaterialList(float flWidth, float flHeight, float x, float y, float flRounding);
    void _DrawTitleBar(float flWidth, float flHeight, float x, float y, float flRounding);
    void _DrawModelPanelOverlay(float flWidth, float flHeight, float x, float y);

    // Hanlding text buffers here...
    void _ConstuctModelNameSuggestions(const char* szBuffer, uint32_t iBufferSize);
    char m_szSearchBoxBuffer[512] = "";
    std::vector<int> m_vecModelNameSuggestions;
    int m_iBestModelNameSuggestion = -1;

    void _ProcessBuffer(const char* szBuffer, uint32_t iBufferSize);
    void _SplitBuffer(std::vector<TokenInfo_t>& vecTokensOut, const char* szBuffer, uint32_t iBufferSize) const;
    void _ProcessTokens(std::vector<TokenInfo_t>& vecTokenOut, TokenInfo_t & activeTokenOut);
    void _CreateSuggestionList(const std::string& szToken);
    std::vector<int> m_vecSuggestions;

    // It's rendering on top of my MatGen. so I removed it :).
    void _DisableGameConsole();
    
    // Handling model.
    void _AdjustCamera();
    void _RotateModel();

    std::chrono::high_resolution_clock::time_point m_lastModelRotateTime;
    
    // Rotation speed is defined as how much angle is covered ( in degrees ) 
    // in one second.
    float m_flModelRotationSpeed = 65.0f;

    // Handling Materials here...
    std::vector<MaterialBundle_t> m_vecMatBundles;
    std::atomic<int> m_iActiveMatBundleIndex;
    TokenInfo_t m_activeToken;
    Material_t* m_pActiveTEMaterial = nullptr; // This is the material drawn to TextEditor

    void _CreateMaterialBundle();
    void _AddMaterialToBundle(MaterialBundle_t& matBundle);
    void _DeleteMaterial(Material_t* pMat, MaterialBundle_t& matBundle);
    void _DeleteMaterialBundle(MaterialBundle_t& matBundle);

    void _RefreshMaterial(Material_t* pMaterial);

    void _MakeMaterialNameUnique(std::string& szNameOut, const std::string & szBaseName, MaterialBundle_t& parentBundle) const;
    void _MakeMaterialBundleNameUnique(std::string& szNameOut, const std::string & szBaseName) const;
};
///////////////////////////////////////////////////////////////////////////


// VMT Keyword
extern std::vector<std::string> g_vecVMTKeyWords;


///////////////////////////////////////////////////////////////////////////


DECLARE_FEATURE_OBJECT(materialGen, MaterialGen_t)

DEFINE_TAB(MaterialGen, 11)
DEFINE_SECTION(MaterialGen, "MaterialGen", 1)
DEFINE_FEATURE(Enable, bool, MaterialGen, MaterialGen, 1, false)