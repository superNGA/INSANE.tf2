#include "MaterialGen.h"

#include "../ModelPreview/ModelPreview.h"

// SDK
#include "../../SDK/class/IVEngineClient.h"
#include "../../SDK/class/BaseEntity.h"
#include "../../SDK/class/IPanel.h"
#include "../../SDK/class/VGui_Panel.h"
#include "../../SDK/class/ISurface.h"
#include "../../SDK/class/Basic Structures.h"
#include "../../SDK/class/INetworkStringTable.h"

// Fonts n shits
#include "../../Resources/Fonts/FontManager.h"

// UTILITY
#include "../../Extra/math.h"
#include "../../Utility/ConsoleLogging.h"
#include "../../Utility/Hook Handler/Hook_t.h"

// External Dependencies
#include "../../External Libraries/ImGui/imgui.h"
#include "../../External Libraries/ImGui/imgui_internal.h"


///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
MaterialGen_t::MaterialGen_t()
{
    m_lastModelRotateTime = std::chrono::high_resolution_clock::now();
    m_iActiveMatBundleIndex.store(-1);
}


///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
void MaterialGen_t::Run()
{
    m_bVisible = Features::MaterialGen::MaterialGen::Enable.IsActive();
    if (m_bVisible == false)
        return;

    // clr
    F::modelPreview.SetRenderViewClr(40, 40, 40, 255);
    F::modelPreview.SetPanelClr(22, 22, 22, 255);

    // position
    F::modelPreview.SetPanelPos(0, 0);
    int iWidth = 0, iHeight = 0; I::iEngine->GetScreenSize(iWidth, iHeight);
    F::modelPreview.SetRenderViewPos(static_cast<int>((2.0f / 3.0f) * static_cast<float>(iWidth)), 0);

    // size
    F::modelPreview.SetPanelSize(iHeight, iWidth);
    F::modelPreview.SetRenderViewSize(iHeight, static_cast<int>((1.0f / 3.0f) * static_cast<float>(iWidth)));

    _DisableGameConsole();
    _AdjustCamera();
    _RotateModel();
    _DrawImGui();
    F::modelPreview.SetActiveModel(Features::MaterialGen::MaterialGen::Model.GetData().m_iVal);
}


///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
void MaterialGen_t::_DisableGameConsole()
{
    static vgui::VPANEL hGameConsole = NULL;
    if (hGameConsole == NULL)
    {
        hGameConsole = I::iPanel->FindChildByName(I::iSurface->GetEmbeddedPanel(), "GameConsole", true);

        if (hGameConsole == NULL)
        {
            FAIL_LOG("Failed to find console panel");
            return;
        }
        
        WIN_LOG("Found panel \"GameConsole\" @ ID : %llu", hGameConsole);
    }

    // Is console open ?
    if (I::iPanel->IsVisible(hGameConsole) == false)
        return;

    I::iPanel->SetVisible(hGameConsole, false);

    LOG("Stopped GameConsole from opening");
}


///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
void MaterialGen_t::_AdjustCamera()
{
    // NOTE : FOV here works in a weird way. the FOV we set in the viewsetup is always good FOV 
    //        for horizontal view ( i.e. always good horizontal FOV ) , but we must use it to find the correct vertical FOV.
    BaseEntity* pEnt   = F::modelPreview.GetModelEntity();
    model_t*    pModel = F::modelPreview.GetActiveModel();

    if (pEnt == nullptr || pModel == nullptr || F::modelPreview.GetActiveModelIndex() == -1)
        return;
    
    pEnt->GetAbsOrigin() = vec(0.0f);

    // Model dimensions.
    float flModelHeight = fabsf(pModel->maxs.z - pModel->mins.z);
    float flModelWidth  = pModel->mins.Dist2Dto(pModel->maxs);

    // Scaling the dimensions so the model has some padding around it.
    flModelHeight *= 1.50f;
    flModelWidth  *= 1.10f;

    // Calculating best distance for perfectly fitting height & width.
    float flHorizontalFOVInRad = DEG2RAD(F::modelPreview.GetBaseCameraFOV());
    float flVerticalFOVInRad   = DEG2RAD(F::modelPreview.GetVerticalFOV());
    float flIdealDistForWidth  = 0.0f, flIdealDistForHieght = 0.0f;
    
    flIdealDistForWidth  = (flModelWidth  / 2.0f) / tanf(flHorizontalFOVInRad / 2.0f);
    flIdealDistForHieght = (flModelHeight / 2.0f) / tanf(flVerticalFOVInRad   / 2.0f);
    
    float flIdealDist = Maths::MAX<float>(flIdealDistForWidth, flIdealDistForHieght);
    F::modelPreview.SetCameraPos(vec(-flIdealDist, 0.0f, 0.0f));


    // Trying to calculate position of entity origin on the screen.
    // NOTE : We are calculating position from the botton of the screen here. i.e. 
    //        0 at botton & 1080 ( or whatever screen height is ) at the top.
    vec vCameraOrigin = F::modelPreview.GetCameraPos();
    vec vModelOrigin  = F::modelPreview.GetModelEntity()->GetAbsOrigin();
    float flModelDist2D = vCameraOrigin.Dist2Dto(vModelOrigin);

    // this is max visible range at the model origin.
    float flFrustumHeight = 2.0f * tanf(flVerticalFOVInRad / 2.0f) * flModelDist2D;
    int iWidth = 0, iHeight = 0; F::modelPreview.GetRenderViewSize(iHeight, iWidth);

    // How much height the model origin is from camera.
    float flDeltaZ = vModelOrigin.z - vCameraOrigin.z;
    
    // This is the target point's height from the bottom of the frustum. We will do trignometry on this
    // to find out where our point lies on the screen.
    float flPosOnFrustum = flDeltaZ + (flFrustumHeight / 2.0f);
    
    // This is how we calculate the y coordinates of model on screen, just do it the other way to find camera height for 
    // desired model y coordinates. That's what I have done below. :) ( cause I am smart ass ) [ There is a -1 error, and IDK where. ]
    //float flPos = (flPosOnFrustum / flFrustumHeight) * static_cast<float>(iHeight);

    constexpr float MODEL_PADDDING_BOTTOM = 100.0f;
    float flIdealCameraHeight = ((MODEL_PADDDING_BOTTOM / static_cast<float>(iHeight)) * flFrustumHeight) - (flFrustumHeight / 2.0f) - vModelOrigin.z * -1.0f;

    F::modelPreview.SetCameraPos(vec(-flIdealDist, 0.0f, -flIdealCameraHeight));
}


///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
void MaterialGen_t::_RotateModel()
{
    BaseEntity* pEnt = F::modelPreview.GetModelEntity();
    if (pEnt == nullptr)
        return;

    // Calculate time since last rotating model & set last update time to current time.
    auto now                    = std::chrono::high_resolution_clock::now();
    int64_t iTimeSinceStartInMs = std::chrono::duration_cast<std::chrono::milliseconds>(now - m_lastModelRotateTime).count();
    m_lastModelRotateTime = now;

    float flTimeSinceLastUpdateInSec = static_cast<float>(iTimeSinceStartInMs) / 1000.0f;
    pEnt->GetAbsAngles().yaw += flTimeSinceLastUpdateInSec * Features::MaterialGen::MaterialGen::RotationSpeed.GetData().m_flVal;
}


///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
void MaterialGen_t::_DrawImGui()
{
    ImGui::PushFont(Resources::Fonts::JetBrains_SemiBold_NL_Mid);

    int iHeight = 0, iWidth = 0; F::modelPreview.GetPanelSize(iHeight, iWidth);
    float flScreenWidth  = static_cast<float>(iWidth);
    float flScreenHeight = static_cast<float>(iHeight);
    ImVec2 vWindowSize(flScreenWidth * (2.0f / 3.0f), flScreenHeight); // 2 / 3 of the total screen goes to this.

    // Styling
    {
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
        ImGui::PushStyleVar(ImGuiStyleVar_FramePadding,  ImVec2(0.0f, 0.0f));
        ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(6.0f / 255.0f, 6.0f / 255.0f, 6.0f / 255.0f, 1.0f)); // This is causing the text editor color
        ImGui::PushStyleColor(ImGuiCol_Border,  ImVec4(0.0f, 0.0f, 0.0f, 0.0f));
    }

    ImGui::SetNextWindowSize(vWindowSize);
    ImGui::SetNextWindowPos({ 0.0f, 0.0f });
    ImGui::SetNextWindowBgAlpha(0.0f);
    bool bOpen        = true;
    int  iWindowFlags = 
        ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoScrollbar;
    if (ImGui::Begin("MaterialGen", &bOpen, iWindowFlags) == true)
    {
        // Drawing Text Editor.
        constexpr float flTEWidthFraction = 0.5f, flTELineCounterWidthFraction = 0.02f;
        constexpr float flTEHeightFraction = 0.9f;
        ImVec2 vTESize(flScreenWidth * (flTEWidthFraction - flTELineCounterWidthFraction), flScreenHeight * flTEHeightFraction);
        ImVec2 vTEPos(flScreenWidth  * flTELineCounterWidthFraction, flScreenHeight * (1.0f - flTEHeightFraction) * 0.5f);
        _DrawTextEditor(vTESize.x, vTESize.y, vTEPos.x, vTEPos.y);

        _DrawMaterialList(flScreenWidth * (2.0f / 3.0f - 0.5f), flScreenHeight, flScreenWidth * 0.5f, 0.0f);

        ImGui::End();
    }

    // Removing style vars
    {
        ImGui::PopStyleVar(2);
        ImGui::PopStyleColor(2);
    }

    ImGui::PopFont();
}


///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
void MaterialGen_t::_DrawTextEditor(float flWidth, float flHeight, float x, float y)
{
    ImGui::PushFont(Resources::Fonts::JetBrains_SemiBold_NL_Mid);

    // Text editor size & pos.
    ImVec2 vTextEditorSize(flWidth, flHeight);
    ImVec2 vTextEditorPos(x, y);

    // Rounding text editor size to line height multiple. We the line count stays in sync nicely.
    vTextEditorSize.y = Maths::RoundToFloor(vTextEditorSize.y, ImGui::GetTextLineHeight());

    ImGui::SetCursorPos(vTextEditorPos);
    static float s_flScrollInPixels = 0.0f;

    // Drawing line count on the left side.
    {
        ImVec2      vCursorPos = ImGui::GetCursorPos(); vCursorPos.x = 0.0f;
        ImDrawList* pDrawList = ImGui::GetWindowDrawList();
        float       flLineHeight = ImGui::GetTextLineHeight();

        pDrawList->AddRectFilled(vCursorPos, ImVec2(vTextEditorPos.x, vTextEditorPos.y + vTextEditorSize.y), ImColor(10, 10, 10, 255));

        int nLines = vTextEditorSize.y / flLineHeight;
        int nScrolledLines = static_cast<int>(s_flScrollInPixels / flLineHeight);
        for (int iLineIndex = nScrolledLines; iLineIndex < nLines + nScrolledLines; iLineIndex++)
        {
            pDrawList->AddText(vCursorPos, ImColor(255, 255, 255, 255), std::format("{}", iLineIndex + 1).c_str());
            vCursorPos.y += flLineHeight;

            if (vCursorPos.y >= vTextEditorSize.y + vTextEditorPos.y)
                break;
        }
    }

    
    if(m_pActiveTEMaterial != nullptr)
    {
        // Text editor input widget.
        if (ImGui::InputTextMultiline("MaterialGen_TextEditor", m_pActiveTEMaterial->m_materialData, sizeof(m_pActiveTEMaterial->m_materialData), vTextEditorSize))
        {
            _ProcessBuffer(m_pActiveTEMaterial->m_materialData, sizeof(m_pActiveTEMaterial->m_materialData));
        }

        // Getting scroll ammount in pixel. NOTE: The child name must be the same as the textInput widget.
        ImGui::BeginChild("##MaterialGen_TextEditor");
        s_flScrollInPixels = ImGui::GetScrollY();
        ImGui::EndChild();

        // Drawing highlighted syntax
        {
            ImDrawList* pDrawList = ImGui::GetForegroundDrawList();
            int nLinesScrolled = static_cast<int>(s_flScrollInPixels / ImGui::GetTextLineHeight());
            int nMaxLines = static_cast<int>(vTextEditorSize.y / ImGui::GetTextLineHeight());

            float flEmptySpaceSize = Resources::Fonts::JetBrains_SemiBold_NL_Mid->GetCharAdvance(' ');
            for (const auto& token : m_pActiveTEMaterial->m_listTokens)
            {
                // Calculating text X & Y coordinates ( compensating for scroll & spaces )
                ImVec2 vPos(
                    vTextEditorPos.x + (static_cast<float>(token.m_iCol) * flEmptySpaceSize),
                    vTextEditorPos.y + (token.m_iLine * ImGui::GetTextLineHeight()) - s_flScrollInPixels);

                // Checking if it has gone out of text input area.
                if (vPos.y + ImGui::GetTextLineHeight() > vTextEditorPos.y + vTextEditorSize.y || vPos.y < vTextEditorPos.y)
                    continue;

                ImColor clr(255, 255, 255, 255);
                switch (token.m_iTokenType)
                {
                case TokenType_t::TOKEN_COMMENT:
                    clr = ImColor(83, 252, 165, 255);  break;
                case TokenType_t::TOKEN_KEYWORD:
                    clr = ImColor(31, 191, 186, 255);  break;
                case TokenType_t::TOKEN_VALUE:
                    clr = ImColor(255, 255, 255, 255); break;
                case TokenType_t::TOKEN_PARENT:
                    clr = ImColor(255, 255, 0, 255); break;
                default:
                    break;
                }

                pDrawList->AddText(vPos, clr, token.m_szToken.c_str());
            }
        }
    }
    else
    {
        ImDrawList* pDrawList = ImGui::GetWindowDrawList();
        pDrawList->AddRectFilled(ImVec2(x, y), ImVec2(x + flWidth, y + flHeight), ImColor(12, 12, 12, 125));
        static const char* szMsg = "No material selected";
        ImVec2 vMsgSize = ImGui::CalcTextSize(szMsg);
        ImVec2 vMsgPos(x + (flWidth / 2.0f) - (vMsgSize.x / 2.0f), y + (flHeight / 2.0f) - (vMsgSize.y / 2.0f));
        pDrawList->AddText(vMsgPos, ImColor(255, 255, 255, 255), szMsg);
    }

    ImGui::PopFont();
}


///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
void MaterialGen_t::_DrawMaterialList(float flWidth, float flHeight, float x, float y)
{
    ImDrawList* pDrawList = ImGui::GetWindowDrawList();
    pDrawList->AddRectFilled(ImVec2(x, y), ImVec2(x + flWidth, y + flHeight), ImColor(12, 12, 12, 255));

    ImGui::SetNextWindowPos(ImVec2(x, y));
    ImGui::BeginChild("##MaterialList", ImVec2(flWidth, flHeight));

    // Styling
    {
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.0f, 0.0f, 0.0f, 0.0f));
    }



    ImVec2 vTopButtonSize(Resources::Fonts::JetBrains_SemiBold_NL_Small->GetCharAdvance('+') * 4.0f, 20.0f);
    if (ImGui::Button("+", vTopButtonSize) == true)
    {
        _CreateMaterialBundle();
    }
    ImGui::SameLine();
    if (ImGui::Button("save") == true)
    {
        LOG("Saved all material to file.");
    }

    ImGui::SetCursorPosY(20.0f);
    int nMatBundles = m_vecMatBundles.size();
    for (int iMatBundleIndex = 0; iMatBundleIndex < nMatBundles; iMatBundleIndex++)
    {
        MaterialBundle_t& matBundle = m_vecMatBundles[iMatBundleIndex];

        ImVec2 vBundlePos = ImGui::GetCursorPos();
        if (ImGui::Selectable(matBundle.m_szMatBundleName.c_str(), &matBundle.m_bExpanded, ImGuiSelectableFlags_AllowItemOverlap, ImVec2(flWidth, 20.0f)) == true)
        {   
            // This is now the active material.
            m_iActiveMatBundleIndex.store(iMatBundleIndex);

            LOG("Set active material as \"%s\"", matBundle.m_szMatBundleName.c_str());
        }

        
        // Drawing buttons for this mat bundle
        ImGui::SameLine(flWidth - (vTopButtonSize.x * 2.0f));
        std::string szAddButtonID = "+##" + matBundle.m_szMatBundleName;
        if (ImGui::Button(szAddButtonID.c_str(), vTopButtonSize) == true)
        {
            _AddMaterialToBundle(matBundle);
        }
        ImGui::SameLine(flWidth - vTopButtonSize.x);
        std::string szDeleteButtonID = "x##" + matBundle.m_szMatBundleName;
        if (ImGui::Button(szDeleteButtonID.c_str(), vTopButtonSize) == true)
        {
            _DeleteMaterialBundle(matBundle);
        }


        // Drawing all of its materials for this material bundle.
        if(matBundle.m_bExpanded == true)
        {
            for (Material_t* mat : matBundle.m_vecMaterials)
            {
                if (ImGui::Button(std::string(mat->m_szMatName + "##" + matBundle.m_szMatBundleName).c_str(), ImVec2(flWidth, 20.0f)) == true)
                {
                    m_pActiveTEMaterial = mat;
                }
            }
        }
    }

    // Removing style
    {
        ImGui::PopStyleColor();
    }

    ImGui::EndChild();
}


///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
void MaterialGen_t::_CreateMaterialBundle()
{
    MaterialBundle_t mat;
    mat.m_bRenameActive = true; // Rename flag, this will put a text input panel so we change change name.
    mat.m_bExpanded     = false;

    static const char s_szDefaultMatBundleName[] = "NewMaterial";
    std::string szNameToTest = s_szDefaultMatBundleName;

    for (int iOffset = 0; iOffset < 10; iOffset++)
    {
        szNameToTest = s_szDefaultMatBundleName;

        // Adding offset to name
        if (iOffset > 0)
            szNameToTest = szNameToTest + "(" + std::to_string(iOffset) + ")";

        bool bDuplicateFound = false;
        for (const MaterialBundle_t& matBundle : m_vecMatBundles)
        {
            // if we got a unique name, we are done
            if (matBundle.m_szMatBundleName == szNameToTest)
            {
                bDuplicateFound = true;
                break;
            }
        }

        // No duplicates found.
        if (bDuplicateFound == false)
            break;
    }

    mat.m_szMatBundleName = szNameToTest;

    m_vecMatBundles.push_back(mat);

    LOG("Added material bundle \"%s\"", mat.m_szMatBundleName.c_str());
}


///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
void MaterialGen_t::_AddMaterialToBundle(MaterialBundle_t& matBundle)
{
    // Creating new material
    Material_t* pMat = new Material_t;
    if (pMat == nullptr)
    {
        FAIL_LOG("Failed to allocate material");
        return;
    }

    memset(pMat->m_materialData, 0, sizeof(pMat->m_materialData));
    pMat->m_materialData[0] = '\0';
    pMat->m_szParentName = matBundle.m_szMatBundleName;

    // Finding a unique name for our newly created material
    static const char s_szDefaultMatBundleName[] = "NewMaterial";
    std::string szNameToTest = s_szDefaultMatBundleName;

    for (int iOffset = 0; iOffset < 10; iOffset++)
    {
        szNameToTest = s_szDefaultMatBundleName;

        // Adding offset to name
        if (iOffset > 0)
            szNameToTest = szNameToTest + "(" + std::to_string(iOffset) + ")";

        bool bDuplicateFound = false;
        for (const Material_t* pMat : matBundle.m_vecMaterials)
        {
            // if we got a unique name, we are done
            if (pMat->m_szMatName == szNameToTest)
            {
                bDuplicateFound = true;
                break;
            }
        }

        // No duplicates found.
        if (bDuplicateFound == false)
            break;
    }

    pMat->m_szMatName = szNameToTest;

    matBundle.m_vecMaterials.push_back(pMat);
    LOG("Created new mateiral \"%s\"->\"%s\"", pMat->m_szParentName.c_str(), pMat->m_szMatName.c_str());
}


///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
void MaterialGen_t::_DeleteMaterialBundle(MaterialBundle_t& matBundle)
{
    // First deleting all materials in this Material bundle
    for (Material_t* pMat : matBundle.m_vecMaterials)
    {
        // Sanity checks
        if (pMat == nullptr)
            continue;

        LOG("Deleted mateiral \"%s\"->\"%s\"", pMat->m_szParentName.c_str(), pMat->m_szMatName.c_str());
        
        // if we happen to remove the current Text Editor material, we must mark it as null ( else the Text Editor will try to load it & crash )
        if (pMat == m_pActiveTEMaterial)
            m_pActiveTEMaterial = nullptr;

        delete pMat;
    }

    // Deleting this material bundle from the main list.
    auto it = std::find(m_vecMatBundles.begin(), m_vecMatBundles.end(), matBundle);
    if (it != m_vecMatBundles.end())
        m_vecMatBundles.erase(it);

    LOG("Deleted material bundle \"%s\" and all its materials.", matBundle.m_szMatBundleName.c_str());
}


///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
void MaterialGen_t::_ProcessBuffer(const char* szBuffer, uint32_t iBufferSize)
{
    if (szBuffer == NULL)
        return;

    std::list<TokenInfo_t> listTokens; 
    listTokens.clear();

    // cut up the buffer at each ' ' and '\n' and keeping quoted & comments intact.
    _SplitBuffer(listTokens, szBuffer, iBufferSize);

    // Iterator over all tokens & determine what catagory they fall into.
    _ProcessTokens(listTokens);

    m_pActiveTEMaterial->m_listTokens = std::move(listTokens);
}


///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
void MaterialGen_t::_SplitBuffer(std::list<TokenInfo_t>& listTokensOut, const char* szBuffer, uint32_t iBufferSize) const
{
    TokenInfo_t token;
    bool bTokenActive        = false;
    bool bQuoteActive        = false;
    bool bCommentActive      = false;
    bool bTokenBreakOverride = false;

    int iLine = 0, iCol = 0;

    for (int i = 0; i < iBufferSize; i++)
    {
        char c = szBuffer[i];

        if (c == '\0')
            break;

        // if line changes, reset col counter & increment line counter. ( these are used for drawing later on )
        iCol++;
        if (c == '\n')
        {
            iLine++; iCol = 0;
            bCommentActive = false;
        }

        if (c == '"')
            bQuoteActive = !bQuoteActive;

        if (c == '/' && i + 1 < iBufferSize && szBuffer[i + 1] == '/')
            bCommentActive = true;

        bTokenBreakOverride = bQuoteActive == true || bCommentActive == true;

        bool bShouldBreakToken = c == ' ' || c == '\n';
        if (bShouldBreakToken == true && bTokenBreakOverride == false)
        {
            if (bTokenActive == true)
            {
                // if somethings written in token, then store it.
                if (token.m_szToken.size() > 0)
                {
                    listTokensOut.push_back(token);
                    token.Reset();
                }

                bTokenActive = false;
            }
        }
        else // Store this character.
        {
            if (bTokenActive == false)
            {
                token.m_iLine = iLine;
                token.m_iCol = iCol - 1;
            }
            token.m_szToken.push_back(c);
            bTokenActive = true;
        }
    }

    // Clear out if something is remaining.
    if (token.m_szToken.size() > 0)
    {
        listTokensOut.push_back(token);
        token.Reset();
    }
}


///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
void MaterialGen_t::_ProcessTokens(std::list<TokenInfo_t>& listTokenOut) const
{
    int  iLastCommentLine = -1;
    bool bExpectingValue  = false;
    bool bMateiralStarted = false;

    for (TokenInfo_t& token : listTokenOut)
    {
        // Comments
        if (token.m_szToken.size() > 2 && token.m_szToken[0] == '/' && token.m_szToken[1] == '/')
        {
            iLastCommentLine   = token.m_iLine;
            token.m_iTokenType = TokenType_t::TOKEN_COMMENT;
            continue;
        }
        // Bracket open
        else if (token.m_szToken == "{")
        {
            bMateiralStarted = true;
            bExpectingValue  = false;
        }
        // Bracket close
        else if (token.m_szToken == "}")
        {
            bMateiralStarted = false;
        }
        // Parameters & Values
        else if (token.m_szToken[0] == '"' && token.m_szToken[token.m_szToken.size() - 1] == '"')
        {
            if (bMateiralStarted == false)
            {
                token.m_iTokenType = TokenType_t::TOKEN_PARENT;
            }
            else if (token.m_szToken[1] == '$') // All keywords start with a $ sign ( except for maybe the proxy ones )
            {
                // Checking if its a valid .vmt keyword or not.
                bool bValidKeyword = false;
                for (const std::string& szValidKeyword : g_vecVMTKeyWords) // Yea its a fucking linear search, and no I don't need to make a fucking OS or a fucking game engine in O(-1) time. Its not required, this check is only done once per change.
                {
                    if (token.m_szToken == szValidKeyword)
                    {
                        bValidKeyword = true;
                        break;
                    }
                }

                if (bValidKeyword == false)
                    continue;

                token.m_iTokenType = TokenType_t::TOKEN_KEYWORD;
                bExpectingValue    = true;
            }
            else if(bExpectingValue == true)
            {
                token.m_iTokenType = TokenType_t::TOKEN_VALUE;
            }
        }
    }
}


///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
void MaterialGen_t::SetVisible(bool bVisible)
{
    m_bVisible = bVisible;
}


///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
bool MaterialGen_t::IsVisible() const
{
    return m_bVisible;
}


///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
void MaterialGen_t::TokenInfo_t::Reset()
{
    m_iLine = 0; m_iCol = 0;
    m_szToken    = "";
    m_iTokenType = TokenType_t::TOKEN_UNDEFINED;
}

/*
TODO : 
-> Material priority change mechanism.
-> Maybe add a default material bundle & material.
-> Get these materials to the DME & draw using this mateiral.

-> Auto complete quotes & brackets.
-> Suggestions for keywords. ( prefix match )
*/