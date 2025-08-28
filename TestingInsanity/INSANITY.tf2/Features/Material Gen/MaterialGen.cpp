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
#include "../../SDK/class/IMaterial.h"
#include "../../SDK/class/IMaterialSystem.h"
#include "../../SDK/class/IKeyValuesSystem.h"

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
    m_activeToken.Reset();
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
        constexpr float flPaddingFraction = 0.01f;
        constexpr float flTEWidthFraction = 0.5f, flTELineCounterWidthFraction = 0.02f;
        constexpr float flTEHeightFraction = 0.9f;
        ImVec2 vTESize(flScreenWidth * (flTEWidthFraction - flTELineCounterWidthFraction), flScreenHeight * flTEHeightFraction);
        ImVec2 vTEPos(flScreenWidth  * flTELineCounterWidthFraction, flScreenHeight * (1.0f - flTEHeightFraction) * 0.5f);
        //_DrawTextEditor(vTESize.x, vTESize.y, vTEPos.x, vTEPos.y);

        float flTitleHeight = flScreenHeight * 0.03f;
        constexpr float flRouding = 5.0f;

        _DrawTextEditor(
            flScreenWidth  * (0.5f - (flPaddingFraction * 1.25f)),
            flScreenHeight - (2.0f * flPaddingFraction * flScreenWidth) - flTitleHeight,
            flScreenWidth  * flPaddingFraction,
            flScreenWidth  * (flPaddingFraction * 1.5f) + flTitleHeight,
            flRouding);

        _DrawMaterialList(
            flScreenWidth  * (2.0f / 3.0f - 0.5f) - (1.25f * flPaddingFraction * flScreenWidth),
            flScreenHeight - (2.0f * flPaddingFraction * flScreenWidth) - flTitleHeight,
            flScreenWidth  * 0.5f + (flPaddingFraction * 0.25f * flScreenWidth),
            flScreenWidth  * (flPaddingFraction * 1.5f) + flTitleHeight,
            flRouding);

        _DrawTitleBar(
            flScreenWidth  * (2.0f / 3.0f) - (flPaddingFraction * 2.0f * flScreenWidth),
            flScreenHeight * 0.03f,
            flScreenWidth  * flPaddingFraction,
            flScreenWidth  * flPaddingFraction,
            flRouding);

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
int TextEditorCallback(ImGuiInputTextCallbackData* pData)
{
    static bool bCharAdded = false;
    if (pData->EventFlag == ImGuiInputTextFlags_CallbackCharFilter)
    {
        bCharAdded = true;
        return 0;
    }

    if (bCharAdded == false)
        return 0;

    // Only if we have valid buffer and cursor pos.
    if (pData->BufTextLen > 0 || pData->CursorPos > 0)
    {
        char input = pData->Buf[pData->CursorPos - 1];

        switch (input)
        {
        case '"': pData->InsertChars(pData->CursorPos, "\""); pData->CursorPos -= 1; break;
        case '[': pData->InsertChars(pData->CursorPos, "]");  pData->CursorPos -= 1; break;
        case '(': pData->InsertChars(pData->CursorPos, ")");  pData->CursorPos -= 1; break;
        case '{': pData->InsertChars(pData->CursorPos, "}");  pData->CursorPos -= 1; break;
        case '\t':
        {
            pData->DeleteChars(pData->CursorPos - 1, 1);

            if (F::materialGen.GetBestKeywordMatch() == -1)
            {
                pData->InsertChars(pData->CursorPos, "    ");
                break;
            }

            const std::string& szKeyWord     = g_vecVMTKeyWords[F::materialGen.GetBestKeywordMatch()];
            const std::string& szActiveToken = F::materialGen.GetActiveToken().m_szToken;
            int                nChars        = szActiveToken.size();
            int                iChar         = 0;
            for (iChar = 0; iChar < nChars; iChar++)
            {
                if (szKeyWord[iChar] != szActiveToken[iChar])
                    break;
            }

            pData->InsertChars(pData->CursorPos, szKeyWord.c_str() + iChar, szKeyWord.c_str() + szKeyWord.size() - 1); // -1 so we don't get that fucking quote character in auto complete.

            // And we did auto complete and the buffer size allows, move the cursor out of the quotes.
            if (pData->BufTextLen < pData->BufSize - 2)
                pData->CursorPos += 1;

            break;
        }
        default: break;
        }
    }
    bCharAdded = false;

    return 1;
}


///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
void MaterialGen_t::_DrawTextEditor(float flWidth, float flHeight, float x, float y, float flRounding)
{
    ImGui::PushFont(Resources::Fonts::JetBrains_SemiBold_NL_Mid);

    // Rounding text editor size to line height multiple. We the line count stays in sync nicely.
    flHeight = Maths::RoundToFloor(flHeight, ImGui::GetTextLineHeight());

    // Text editor size & pos.
    ImVec2 vTextEditorSize(flWidth, flHeight);
    ImVec2 vTextEditorPos(x, y);

    ImGui::SetCursorPos(vTextEditorPos);
    static float s_flScrollInPixels = 0.0f;
    float        flLineCounterWidth = 0.0f;

    // Drawing line count on the left side.
    {
        ImVec2      vCursorPos   = ImGui::GetCursorPos();
        ImDrawList* pDrawList    = ImGui::GetWindowDrawList();
        float       flLineHeight = ImGui::GetTextLineHeight();

        flLineCounterWidth = Resources::Fonts::JetBrains_SemiBold_NL_Mid->GetCharAdvance('0') * 4;
        pDrawList->AddRectFilled(ImVec2(x, y), ImVec2(x + flLineCounterWidth, y + vTextEditorSize.y), ImColor(10, 10, 10, 255), flRounding, ImDrawFlags_RoundCornersBottomLeft);

        int nLines = static_cast<int>(Maths::RoundToCeil(vTextEditorSize.y, flLineHeight));
        int nScrolledLines = static_cast<int>(s_flScrollInPixels / flLineHeight);
        for (int iLineIndex = nScrolledLines; iLineIndex < nLines + nScrolledLines; iLineIndex++)
        {
            pDrawList->AddText(vCursorPos, ImColor(255, 255, 255, 255), std::format("{}", iLineIndex + 1).c_str());
            vCursorPos.y += flLineHeight;

            if (vCursorPos.y >= vTextEditorSize.y + vTextEditorPos.y)
                break;
        }

        // Move the cursor we Text Editor doesn't get drawn over the line counter.
        ImGui::SetCursorPosX(x + flLineCounterWidth);
        vTextEditorSize.x -= flLineCounterWidth; // reduce width by "LineCounter"
    }

    
    if(m_pActiveTEMaterial != nullptr)
    {
        ImGui::PushFont(Resources::Fonts::JetBrains_Light_NL_MID);

        ImGuiInputTextFlags iFlags = ImGuiInputTextFlags_CallbackAlways | ImGuiInputTextFlags_CallbackCharFilter | ImGuiInputTextFlags_AllowTabInput;

        // Text editor input widget.
        if (ImGui::InputTextMultiline(
            "##MaterialGen_TextEditor", 
            m_pActiveTEMaterial->m_materialData, sizeof(m_pActiveTEMaterial->m_materialData), 
            vTextEditorSize, iFlags, TextEditorCallback, nullptr) == true)
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
            int nMaxLines      = static_cast<int>(vTextEditorSize.y / ImGui::GetTextLineHeight());

            float flEmptySpaceSize = Resources::Fonts::JetBrains_Light_NL_MID->GetCharAdvance(' ');
            for (const auto& token : m_pActiveTEMaterial->m_vecTokens)
            {
                // Calculating text X & Y coordinates ( compensating for scroll & spaces )
                ImVec2 vPos(
                    flLineCounterWidth + vTextEditorPos.x + (static_cast<float>(token.m_iCol) * flEmptySpaceSize),
                    vTextEditorPos.y + (token.m_iLine * ImGui::GetTextLineHeight()) - s_flScrollInPixels);

                // Checking if it has gone out of text input area.
                if (vPos.y + ImGui::GetTextLineHeight() > vTextEditorPos.y + vTextEditorSize.y || vPos.y < vTextEditorPos.y)
                    continue;

                ImColor clr(255, 255, 255, 255);
                switch (token.m_iTokenType)
                {
                case TokenType_t::TOKEN_COMMENT:
                    clr = ImColor(105, 105, 105, 255);  break;
                case TokenType_t::TOKEN_KEYWORD:
                    clr = ImColor(31, 191, 186, 255);  break;
                case TokenType_t::TOKEN_VALUE:
                    clr = ImColor(255, 255, 255, 255); break;
                case TokenType_t::TOKEN_PARENT:
                    clr = ImColor(255, 255, 0, 255);   break;
                default:
                    break;
                }

                pDrawList->AddText(vPos, clr, token.m_szToken.c_str());
            }
        }


        // Drawing suggestions...
        if(m_vecSuggestions.size() > 0/* && m_activeToken.m_iTokenType == TokenType_t::TOKEN_KEYWORD*/)
        {
            // Calculating longest suggestion string size so we can size the suggestion box accordingly.
            int nSuggestions   = m_vecSuggestions.size() > 5 ? 5 : m_vecSuggestions.size();
            int iMaxStringSize = 0;
            for (int iSuggestionIndex = 0; iSuggestionIndex < nSuggestions; iSuggestionIndex++)
            {
                std::string szSuggestion = g_vecVMTKeyWords[m_vecSuggestions[iSuggestionIndex]];
                if (szSuggestion.size() > iMaxStringSize)
                    iMaxStringSize = szSuggestion.size();
            }

            ImDrawList* pDrawList = ImGui::GetForegroundDrawList();

            float flSpaceWidth = Resources::Fonts::JetBrains_SemiBold_NL_Mid->GetCharAdvance(' ');
            ImVec2 vSuggestionPos(
                vTextEditorPos.x + flLineCounterWidth + (m_activeToken.m_iCol * flSpaceWidth),
                vTextEditorPos.y + (m_activeToken.m_iLine * ImGui::GetTextLineHeight()) - s_flScrollInPixels);

            vSuggestionPos.y += ImGui::GetTextLineHeight(); // Just so we draw it a line below where we are actually writting.

            pDrawList->AddRectFilled(vSuggestionPos, ImVec2(vSuggestionPos.x + (flSpaceWidth * (iMaxStringSize + 2)), vSuggestionPos.y + (ImGui::GetTextLineHeight() * static_cast<float>(nSuggestions))), ImColor(14, 14, 14, 255));            

            ImGui::PushFont(Resources::Fonts::JetBrains_SemiBold_NL_Mid);
            ImVec2 vTextPos = vSuggestionPos;
            for (int iSuggestionIndex = 0; iSuggestionIndex < nSuggestions; iSuggestionIndex++)
            {
                pDrawList->AddText(vTextPos, iSuggestionIndex == 0 ? ImColor(0, 180, 100, 255) : ImColor(255, 255, 255, 255), g_vecVMTKeyWords[m_vecSuggestions[iSuggestionIndex]].c_str());
                vTextPos.y += ImGui::GetTextLineHeight();
            }
            ImGui::PopFont();
        }

        ImGui::PopFont();
    }
    else // Drawing a simple message when no material is selected.
    {
        ImDrawList* pDrawList = ImGui::GetWindowDrawList();
        pDrawList->AddRectFilled(ImVec2(x + flLineCounterWidth, y), ImVec2(x + vTextEditorSize.x + flLineCounterWidth, y + vTextEditorSize.y), ImColor(12, 12, 12, 125));

        static const char* szMsg = "No material selected";
        ImVec2 vMsgSize = ImGui::CalcTextSize(szMsg);
        ImVec2 vMsgPos(x + (vTextEditorSize.x / 2.0f) - (vMsgSize.x / 2.0f), y + (vTextEditorSize.y / 2.0f) - (vMsgSize.y / 2.0f));
        pDrawList->AddText(vMsgPos, ImColor(255, 255, 255, 255), szMsg);
    }

    ImGui::PopFont();
}


///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
void MaterialGen_t::_DrawMaterialList(float flWidth, float flHeight, float x, float y, float flRounding)
{
    // This needs to be done to match the height of the text editor.
    ImGui::PushFont(Resources::Fonts::JetBrains_SemiBold_NL_Mid);
    flHeight = Maths::RoundToFloor(flHeight, ImGui::GetTextLineHeight());
    ImGui::PopFont();

    ImDrawList* pDrawList = ImGui::GetWindowDrawList();
    pDrawList->AddRectFilled(ImVec2(x, y), ImVec2(x + flWidth, y + flHeight), ImColor(12, 12, 12, 255), flRounding, ImDrawFlags_RoundCornersBottomRight);

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
        if (ImGui::Selectable(std::string(" " + matBundle.m_szMatBundleName + "##Parent").c_str(), &matBundle.m_bExpanded, ImGuiSelectableFlags_AllowItemOverlap, ImVec2(flWidth, 20.0f)) == true)
        {   
            // This is now the active material.
            m_iActiveMatBundleIndex.store(iMatBundleIndex);

            LOG("Set active material as \"%s\"", matBundle.m_szMatBundleName.c_str());
        }

        // Rename logic
        if (matBundle.m_bRenameActive == true)
        {
            ImGui::SetCursorPos(vBundlePos);
            ImGui::SetKeyboardFocusHere();
            if (ImGui::InputText(std::string("##RenamePanel_" + matBundle.m_szMatBundleName).c_str(), matBundle.m_szRenameBuffer, sizeof(matBundle.m_szRenameBuffer), ImGuiInputTextFlags_EnterReturnsTrue) == true)
            {
                _MakeMaterialBundleNameUnique(matBundle.m_szMatBundleName, std::string(matBundle.m_szRenameBuffer));
                matBundle.m_bRenameActive = false;

                LOG("Renamed material bundle to \"%s\"", matBundle.m_szMatBundleName.c_str());
            }
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
            for (Material_t* pMat : matBundle.m_vecMaterials)
            {
                ImVec2 vMatPos = ImGui::GetCursorPos();
                if (ImGui::Button(std::string(pMat->m_szMatName + "##" + matBundle.m_szMatBundleName + "_Child").c_str(), ImVec2(flWidth, 20.0f)) == true)
                {
                    m_pActiveTEMaterial = pMat;
                }

                if (pMat->m_bRenameActive == true)
                {
                    ImGui::SetCursorPos(vMatPos);
                    ImGui::SetKeyboardFocusHere();
                    if (ImGui::InputText(std::string("##RenamePanel_" + pMat->m_szMatName).c_str(), pMat->m_szRenameBuffer, sizeof(pMat->m_szRenameBuffer), ImGuiInputTextFlags_EnterReturnsTrue) == true)
                    {
                        _MakeMaterialNameUnique(pMat->m_szMatName, std::string(pMat->m_szRenameBuffer), matBundle);

                        pMat->m_bRenameActive = false;

                        LOG("Renamed material to \"%s\"->\"%s\"", pMat->m_szParentName.c_str(), pMat->m_szMatName.c_str());
                    }
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
void MaterialGen_t::_DrawTitleBar(float flWidth, float flHeight, float x, float y, float flRounding)
{
    ImGui::SetCursorScreenPos({ x, y });

    ImDrawList* pDrawList = ImGui::GetWindowDrawList();
    pDrawList->AddRectFilled(ImVec2(x, y), ImVec2(x + flWidth, y + flHeight), ImColor(45, 45, 45, 255), flRounding, ImDrawFlags_RoundCornersTop);

    if(m_pActiveTEMaterial != nullptr)
    {
        std::string szActiveFileName = '[' + m_pActiveTEMaterial->m_szParentName + "]->[" + m_pActiveTEMaterial->m_szMatName + ']';

        ImGui::TextColored(
            m_pActiveTEMaterial->m_bSaved == true ? ImVec4(0.0f, 1.0f, 0.23f, 1.0f) : ImVec4(1.0f, 1.0f, 0.0f, 1.0f),
            szActiveFileName.c_str());

        ImGui::SameLine();
        if (ImGui::Button("Reload") == true)
        {
            _RefreshMaterial(m_pActiveTEMaterial);
        }
    }
}


///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
void MaterialGen_t::_CreateMaterialBundle()
{
    MaterialBundle_t mat;
    mat.m_bRenameActive = true; // Rename flag, this will put a text input panel so we change change name.
    mat.m_bExpanded     = true;

    _MakeMaterialBundleNameUnique(mat.m_szMatBundleName, std::string("NewMaterial"));

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
    pMat->m_szParentName    = matBundle.m_szMatBundleName;
    pMat->m_vecTokens.clear();
    pMat->m_bSaved          = true;
    pMat->m_pKeyValues      = new KeyValues;
    pMat->m_pMaterial       = nullptr;
    pMat->m_szRenameBuffer[0] = '\0';
    pMat->m_bRenameActive   = true;

    _MakeMaterialNameUnique(pMat->m_szMatName, std::string("NewMat"), matBundle);

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

        // Delete keyvalue pair for this mateiral object too.
        if(pMat->m_pKeyValues != nullptr)
        {
            pMat->m_pKeyValues->DeleteAllChildren();
            delete pMat->m_pKeyValues;
        }

        // Release the IMaterial
        if (pMat->m_pMaterial != nullptr)
            pMat->m_pMaterial->Release();

        delete pMat;
    }

    LOG("Deleted material bundle \"%s\" and all its materials.", matBundle.m_szMatBundleName.c_str());

    // Deleting this material bundle from the main list.
    auto it = std::find(m_vecMatBundles.begin(), m_vecMatBundles.end(), matBundle);
    if (it != m_vecMatBundles.end())
        m_vecMatBundles.erase(it);

    // Adjusting active material bundle.
    if (m_vecMatBundles.size() <= 0)
    {
        m_iActiveMatBundleIndex.store(-1);
    }
    else
    {
        m_iActiveMatBundleIndex.store(std::clamp<int>(m_iActiveMatBundleIndex.load() - 1, 0, m_vecMatBundles.size() - 1));
    }
}


///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
void MaterialGen_t::_RefreshMaterial(Material_t* pMaterial)
{
    if (pMaterial == nullptr)
        return;

    if (pMaterial->m_bSaved == true)
        return;

    if (pMaterial->m_pKeyValues == nullptr)
    {
        FAIL_LOG("Material has uninitialized keyvalue pair");
        return;
    }

    // First we gota free the current KeyValues ( all of them )
    pMaterial->m_pKeyValues->DeleteAllChildren();
    pMaterial->m_pKeyValues->Init();

    // Release the current material to prevent any memory leaks.
    if (pMaterial->m_pMaterial)
        pMaterial->m_pMaterial->Release();

    // Setting up the KeyValue pair for new material data.
    bool bGoodMaterial = pMaterial->m_pKeyValues->LoadFromBuffer(pMaterial->m_szMatName.c_str(), pMaterial->m_materialData);
    if (bGoodMaterial == false)
    {
        FAIL_LOG("You have written a bullshit material my nigga. [ %s ]->[ %s ]", pMaterial->m_szParentName.c_str(), pMaterial->m_szMatName.c_str());
        return;
    }

    // Creating new material.
    pMaterial->m_pMaterial = I::iMaterialSystem->CreateMaterial(pMaterial->m_szMatName.c_str(), pMaterial->m_pKeyValues);

    pMaterial->m_bSaved = true;
    WIN_LOG("Succesfully refreshed material \"%s\"->\"%s\"", pMaterial->m_szParentName.c_str(), pMaterial->m_szMatName.c_str());
}


///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
void MaterialGen_t::_MakeMaterialNameUnique(std::string& szNameOut, const std::string& szBaseName, MaterialBundle_t& parentBundle) const
{
    std::string szNameToTest = szBaseName;

    for (int iOffset = 0; iOffset < 10; iOffset++)
    {
        szNameToTest = szBaseName;

        // Adding offset to name
        if (iOffset > 0)
            szNameToTest = szNameToTest + "(" + std::to_string(iOffset) + ")";

        bool bDuplicateFound = false;
        for (Material_t* pMat : parentBundle.m_vecMaterials)
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

    szNameOut = szNameToTest;
}


///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
void MaterialGen_t::_MakeMaterialBundleNameUnique(std::string& szNameOut, const std::string& szBaseName) const
{
    std::string szNameToTest = szBaseName;

    for (int iOffset = 0; iOffset < 10; iOffset++)
    {
        szNameToTest = szBaseName;

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

    szNameOut = szNameToTest;
}


///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
void MaterialGen_t::_ProcessBuffer(const char* szBuffer, uint32_t iBufferSize)
{
    if (szBuffer == NULL)
        return;

    std::vector<TokenInfo_t> vecTokens; 
    vecTokens.clear();

    // cut up the buffer at each ' ' and '\n' and keeping quoted & comments intact.
    _SplitBuffer(vecTokens, szBuffer, iBufferSize);

    // Iterator over all tokens & determine what catagory they fall into.
    TokenInfo_t activeToken;
    // NOTE : if _ProcessToken found a valid active token, then its type will be set to TokenType_t::Token_Keyword. Else no active token has been found.
    _ProcessTokens(vecTokens, activeToken); 

    // Prefix matching from all keyword list to find potential matches.
    m_vecSuggestions.clear();
    if(activeToken.m_iTokenType == TokenType_t::TOKEN_KEYWORD)
    {
        _CreateSuggestionList(activeToken.m_szToken);
        m_activeToken = activeToken;
    }

    m_pActiveTEMaterial->m_vecTokens = std::move(vecTokens);
    m_pActiveTEMaterial->m_bSaved    = false;
}


///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
void MaterialGen_t::_SplitBuffer(std::vector<TokenInfo_t>& vecTokensOut, const char* szBuffer, uint32_t iBufferSize) const
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
                    vecTokensOut.push_back(token);
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
        vecTokensOut.push_back(token);
        token.Reset();
    }
}


///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
void MaterialGen_t::_ProcessTokens(std::vector<TokenInfo_t>& vecTokenOut, TokenInfo_t& activeTokenOut)
{
    int  iLastCommentLine  = -1;
    bool bExpectingValue   = false;
    bool bMateiralStarted  = false;
    bool bActiveTokenFound = false; // Found the token that the user if currently messing with.

    int nTokens = vecTokenOut.size();
    for (int iTokenIndex = 0; iTokenIndex < nTokens; iTokenIndex++)
    {
        TokenInfo_t& token = vecTokenOut[iTokenIndex];
        int          iTokenSize = token.m_szToken.size();

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
            else
            {
                bool bValidKeyword = false;
                for (const std::string& szValidKeyword : g_vecVMTKeyWords) // Yea its a fucking linear search, and no I don't need to make a fucking OS or a fucking game engine in O(-1) time. Its not required, this check is only done once per change.
                {
                    if (token.m_szToken == szValidKeyword)
                    {
                        bValidKeyword = true;
                        break;
                    }
                }

                // Only if not a valid keyword, we setup suggestions.
                if (bValidKeyword == false)
                {
                    // Gotta store this for showing suggestions.
                    if (bActiveTokenFound == false)
                    {
                        bool bTokenModified =
                            m_pActiveTEMaterial->m_vecTokens.size()                 >= vecTokenOut.size() && // Do the old token list even has this many tokens.
                            m_pActiveTEMaterial->m_vecTokens[iTokenIndex].m_szToken != token.m_szToken;      // has this token been modified.

                        // Found the current active token.
                        if (bTokenModified == true)
                        {
                            bActiveTokenFound = true;
                            activeTokenOut    = token;
                            activeTokenOut.m_iTokenType = TokenType_t::TOKEN_KEYWORD; // It might be a keyword.
                        }
                    }

                    continue;
                }

                token.m_iTokenType = TokenType_t::TOKEN_KEYWORD;
                bExpectingValue    = !bExpectingValue;
            }
            //else if (token.m_szToken[1] == '$') // All keywords start with a $ sign ( except for maybe the proxy ones )
            //{
            //    // Checking if its a valid .vmt keyword or not.
            //    bool bValidKeyword = false;
            //    for (const std::string& szValidKeyword : g_vecVMTKeyWords) // Yea its a fucking linear search, and no I don't need to make a fucking OS or a fucking game engine in O(-1) time. Its not required, this check is only done once per change.
            //    {
            //        if (token.m_szToken == szValidKeyword)
            //        {
            //            bValidKeyword = true;
            //            break;
            //        }
            //    }

            //    if (bValidKeyword == false)
            //        continue;

            //    token.m_iTokenType = TokenType_t::TOKEN_KEYWORD;
            //    bExpectingValue    = true;
            //}
            //else if(bExpectingValue == true)
            //{
            //    token.m_iTokenType = TokenType_t::TOKEN_VALUE;
            //}
        }
    }
}


///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
void MaterialGen_t::_CreateSuggestionList(const std::string& szToken)
{
    int nKeyWords = g_vecVMTKeyWords.size();
    for (int iIndex = 0; iIndex < nKeyWords; iIndex++)
    {
        const std::string& szKeyWord = g_vecVMTKeyWords[iIndex];

        // if keyword size is less than token, then this can't be the match.
        if (szToken.size() > szKeyWord.size() || szToken.size() <= 2)
            continue;
        
        bool bPrefixMatched = true;

        // Only matching character 1 2 & 3. for efficiency purposes.
        int nChars = szToken.size();// < 4 ? szToken.size() : 4;
        for (int iChar = 1; iChar < nChars - 1; iChar++) // NOTE : We are starting form 1 till size - 1 to not consider the quotes at all.
        {
            if (szToken[iChar] != szKeyWord[iChar])
            {
                bPrefixMatched = false;
                break;
            }
        }

        if (bPrefixMatched == false)
            continue;

        m_vecSuggestions.push_back(iIndex);
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
std::vector<Material_t*>* MaterialGen_t::GetModelMaterials()
{
    int iActiveMatBundleIndex = m_iActiveMatBundleIndex.load();

    // Bullshit active material ?
    if (iActiveMatBundleIndex < 0 || iActiveMatBundleIndex >= m_vecMatBundles.size())
        return nullptr;

    return &(m_vecMatBundles[iActiveMatBundleIndex].m_vecMaterials);
}


///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
int MaterialGen_t::GetBestKeywordMatch() const
{
    if (m_vecSuggestions.size() <= 0)
        return -1;

    return m_vecSuggestions[0];
}


///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
const TokenInfo_t& MaterialGen_t::GetActiveToken() const
{
    return m_activeToken;
}


///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
void TokenInfo_t::Reset()
{
    m_iLine = 0; m_iCol = 0;
    m_szToken    = "";
    m_iTokenType = TokenType_t::TOKEN_UNDEFINED;
}


///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
void MaterialGen_t::Free()
{
    for (MaterialBundle_t& matBundle : m_vecMatBundles)
    {
        _DeleteMaterialBundle(matBundle);
    }

    // Note that this is not gauranteed to free the heap allocated data. peak STL containers.
    m_vecMatBundles.clear();
    m_vecMatBundles.shrink_to_fit();

    LOG("Succesfully Free'ed all material & material bundles");
}


/*
TODO : 
"VertexLitGeneric"
{
"$additive" 1
"$color2" "[0 0.2 0.2]"
"$model" 1
"$envmap" "env_cubemap"
"$envmaptint" "[0 0.5 0.5]"
"$envmapsaturation" "0.5"
"$envmapcontrast" "0.5"
"$selfillum" 1

"Proxies"
{

// ENV Map saturation
"Sine"
{
"resultvar" "$envmapsaturation"
"sineperiod" 1
"sinemin" 0
"sinemax" 1
}

// ENV MAP contrast
   "Sine"
{
"resultvar" "$envmapcontrast"
"sineperiod" 1
"sinemin" 0
"sinemax" 1
}

}
}
*/