//=========================================================================
//                      MODEL PREVIEW
//=========================================================================
// by      : INSANE
// created : 31/07/2025
// 
// purpose : Renders any model. Anywhere.
//-------------------------------------------------------------------------
#include "ModelPreview.h"

// SDK
#include "../../SDK/class/BaseEntity.h"
#include "../../SDK/class/IModelLoader.h"
#include "../../SDK/class/IVModelRender.h"
#include "../../SDK/class/IVEngineClient.h"
#include "../../SDK/class/IMaterialSystem.h"
#include "../../SDK/class/IVRenderView.h"
#include "../../SDK/class/IMatRenderContext.h"
#include "../../SDK/class/IVEngineServer.h"
#include "../../SDK/class/INetworkStringTable.h"
#include "../../SDK/class/IEngineTool.h"
#include "../../SDK/class/ISurface.h"
#include "../../SDK/class/IEngineVGui.h"
#include "../../SDK/class/VGui_Panel.h"
#include "../../SDK/class/IPanel.h"
#include "../../SDK/class/VPanel.h"
#include "../../SDK/class/IViewPort.h"
#include "../../SDK/class/CMDL.h"
#include "../../SDK/class/IStudioRender.h"
#include "../../SDK/class/CClientState.h"
#include "../../SDK/class/IVModelInfo.h"
#include "../../SDK/class/CClientLeafSystem.h"

// UTILITY
#include "../../SDK/TF object manager/TFOjectManager.h"
#include "../../Extra/math.h"
#include "../../Utility/ConsoleLogging.h"
#include "../../Utility/Hook Handler/Hook_t.h"
#include "../../Utility/Interface Handler/Interface.h"
#include "../../Utility/Signature Handler/signatures.h"
#include "../../Utility/PullFromAssembly.h"
#include "../../Utility/Profiler/Profiler.h"
#include "../Graphics Engine V2/Graphics.h"
#include "../../Resources/Fonts/FontManager.h"
#include "../ImGui/MenuV2/MenuV2.h"


// Debugging macros
#define ENABLE_DEBUGGING_HOOKS  false
#define DISABLE_ESSENTIAL_HOOKS false

// FUNCTIONS
MAKE_SIG(VGui_Panel_Constructor,      "48 89 5C 24 ? 48 89 74 24 ? 57 48 83 EC ? 48 8B F2 48 8B D9 45 85 C0",                          CLIENT_DLL, void*, Panel*, void*, const char*) // arguments : VTable, parent panel pointer ( Get form engineVGui ), name.
MAKE_SIG(CBaseFlex_Constructor,       "48 89 5C 24 ? 48 89 6C 24 ? 48 89 74 24 ? 57 41 56 41 57 48 83 EC ? 48 8B D9 E8 ? ? ? ? 33 ED", CLIENT_DLL, void*, void*)
MAKE_SIG(CBaseEntity_SetModelByIndex, "48 89 5C 24 ? 57 48 83 EC ? 66 89 91",                                                          CLIENT_DLL, void*, BaseEntity*, unsigned short)
//MAKE_SIG(CMatRenderContext_SetLightingOrigin, "48 83 EC ? 8B 42 ? F2 0F 10 02", MATERIALSYSTEM_DLL, void*, void*, vec*)

// Should be 0x410.
GET_ADRS_FROM_ASSEMBLY(PaintFnOffset,            int, "FF 90 ? ? ? ? 48 8B 0D ? ? ? ? 49 8B D7 48 8B 01 FF 50 ? 48 8B 0D ? ? ? ? 49 8B D7", 2, 6, CLIENT_DLL)
GET_ADRS_FROM_ASSEMBLY(ModelPrecacheTableOffset, int, "48 89 BB ? ? ? ? 48 89 BB ? ? ? ? 48 89 BB ? ? ? ? 8D 57", 3, 7, ENGINE_DLL)


#if (DISABLE_ESSENTIAL_HOOKS == false)

MAKE_HOOK(CModelInfoClient_GetModel, "83 FA ? 0F 8C ? ? ? ? 48 8D 0D ? ? ? ? E9 ? ? ? ? CC CC CC CC CC CC CC CC CC CC CC 48 89 5C 24", __fastcall, ENGINE_DLL, void*, void* a1, int iModelIndex)
{
    // If in main menu, then just always return. it doesn't matter, nothing's gonna call that shit.
    if (I::iEngine->IsConnected() == false)
        return F::modelPreview.GetActiveModel();

    return Hook::CModelInfoClient_GetModel::O_CModelInfoClient_GetModel(a1, iModelIndex);
}
#endif


//=========================================================================
//                     PUBLIC METHODS
//=========================================================================
void ModelPreview_t::Run()
{
    PROFILER_RECORD_FUNCTION(EndScene);

    if (m_bVisible == false)
        return;

    // Make sure to not create string tables while in game or loading in a game.
    if (_ShouldCreateStringTable() == true)
    {
        _VerifyOrCreateStringTable();
    }

    // Precache models.
    if (m_bModelPrecached == false)
    {
        m_bModelPrecached = _PrecacheModels();
    }

    // Inititalize entity & panel object
    if (_Initialize() == false)
        return;

    // Doing this here, has a lot of benifits, like when the size is 0, Panel's paint never gets called,
    // and earlier I was doing this in paint, so when game's tabbed out ( size set to 0 ) its never gets 
    // set to correct size again ever.
    if (m_pPanel != nullptr)
    {
        I::iPanel->SetSize(m_pPanel->GetVPanel(), m_iPanelWidth, m_iPanelHeight);
    }

    _AdjustCamera();
}


///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
void ModelPreview_t::DrawOverlay(float flRounding)
{
    int x      = 0, y       = 0; GetRenderViewPos(x, y);
    int iWidth = 0, iHeight = 0; GetRenderViewSize(iHeight, iWidth);

    {
        ImGui::PushFont(Resources::Fonts::JetBrainsMonoNerd_Mid);

        ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.0f, 0.0f, 0.0f, 0.0f));
        ImGui::PushStyleColor(ImGuiCol_Border,   ImVec4(0.0f, 0.0f, 0.0f, 0.0f));
    }

    // Size & Pos
    ImVec2 vWindowPos (static_cast<float>(x),      static_cast<float>(y));
    ImVec2 vWindowSize(static_cast<float>(iWidth), static_cast<float>(iHeight));
    ImGui::SetNextWindowPos(vWindowPos); ImGui::SetNextWindowSize(vWindowSize);

    int iWindowFlags = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoMove;
    if(ImGui::Begin("##ModelPreviewOverlay", nullptr, iWindowFlags) == true)
    {
        ImDrawList* pDrawList = ImGui::GetWindowDrawList();

        float flThickness         = flRounding - (flRounding / sqrtf(2.0f));
        float flRoundingEffective = flRounding - (flThickness / 2.0f);

        pDrawList->AddRect(
            ImVec2(vWindowPos.x + (flThickness / 2.0f),                 vWindowPos.y + (flThickness / 2.0f)), 
            ImVec2(vWindowPos.x + vWindowSize.x - (flThickness / 2.0f), vWindowPos.y + vWindowSize.y - (flThickness / 2.0f)), 
            ImColor(GetRenderViewClr().GetAsImVec4()), flRoundingEffective, 0, flThickness
        );

        // we are increasing thickness by one just to be safe, cause casting it might take away some precision & we might see some points n shit around the corners.
        int iSafeThickness = static_cast<int>(flThickness) + 1;
        SetRenderViewPos (x       + iSafeThickness,        y      + iSafeThickness);
        SetRenderViewSize(iHeight - (2 * iSafeThickness),  iWidth - (2 * iSafeThickness));
        SetPanelPos      (x       + iSafeThickness,        y      + iSafeThickness);
        SetPanelSize     (iHeight - (2 * iSafeThickness),  iWidth - (2 * iSafeThickness));

        // New size & pos.
        ImVec2 vRenderViewSize(static_cast<float>(iWidth - (2.0f * iSafeThickness)), static_cast<float>(iHeight - (2.0f * iSafeThickness)));
        ImVec2 vRenderViewPos (static_cast<float>(x + iSafeThickness)              , static_cast<float>(y + iSafeThickness));

        constexpr float PADDING_IN_PXL        =  5.0f; // Padding between render view and content drawin within it.
        constexpr float BUTTON_PADDING_ON_TOP = 20.0f; // Padding on top of setting buttons ( so it doesn't point into the rounded corner ).

        {
            ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding,   WIDGET_ROUNDING);
            // ImGui::PushStyleVar(ImGuiStyleVar_FramePadding,    ImVec2(0.0f,0.0f));
            ImGui::PushStyleVar(ImGuiStyleVar_PopupRounding,   POPUP_ROUNDING);
            ImGui::PushStyleVar(ImGuiStyleVar_PopupBorderSize, 0.0f);

            ImGui::PushStyleColor(ImGuiCol_Button,        Render::menuGUI.GetPrimaryClr().GetAsImVec4());
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, Render::menuGUI.GetSecondaryClr().GetAsImVec4());
            ImGui::PushStyleColor(ImGuiCol_ButtonActive,  Render::menuGUI.GetSecondaryClr().GetAsImVec4());

            ImGui::PushStyleColor(ImGuiCol_PopupBg,       Render::menuGUI.GetSecondaryClr().GetAsImVec4());

            RGBA_t clrText; Render::menuGUI.CalcTextClrForBg(clrText, Render::menuGUI.GetSecondaryClr());
            ImGui::PushStyleColor(ImGuiCol_Text,          clrText.GetAsImVec4());
        }

        float flFrameHeight = ImGui::GetFrameHeight();
        ImVec2 vButtonPos(vRenderViewPos.x + vRenderViewSize.x - PADDING_IN_PXL - flFrameHeight, vRenderViewPos.y + PADDING_IN_PXL + BUTTON_PADDING_ON_TOP);
        ImGui::SetCursorScreenPos(vButtonPos);

        if(ImGui::Button(reinterpret_cast<const char*>(u8"\uf400"), ImVec2(flFrameHeight, flFrameHeight)) == true)
        {
            ImGui::OpenPopup("##ModelLightingSettings");
        }
        _DrawLightingSettings();
        

        vButtonPos.y += flFrameHeight + PADDING_IN_PXL;
        ImGui::SetCursorScreenPos(vButtonPos);
        if(ImGui::Button(reinterpret_cast<const char*>(u8"\uef0c"), ImVec2(flFrameHeight, flFrameHeight)) == true)
        {
            ImGui::OpenPopup("ModelPreviewSettings");
        }
        _DrawModelSettings();
        _RotateModel();
        

        {
            ImGui::PopStyleVar(3);
            ImGui::PopStyleColor(5);
        }

        ImGui::End();
    }


    ImGui::PopStyleColor(2);
    ImGui::PopFont();
}


///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
void ModelPreview_t::_DrawLightingSettings()
{
    int iPopupFlags = ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoTitleBar;
    if (ImGui::BeginPopup("##ModelLightingSettings", iPopupFlags) == true)
    {
        ImGui::PushFont(Resources::Fonts::JetBrainsMonoNerd_Small);
        ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, WIDGET_BORDER_THICKNESS);
        ImGui::PushStyleColor(ImGuiCol_Border, Render::menuGUI.GetThemeClr().GetAsImVec4());
            
        static float clrUp[3]    = { 0.4f, 0.4f, 0.4f }; static float clrBottom[3] = { 0.4f, 0.4f, 0.4f };
        static float clrRight[3] = { 0.4f, 0.4f, 0.4f }; static float clrLeft[3]   = { 0.4f, 0.4f, 0.4f };
        static float clrFront[3] = { 0.4f, 0.4f, 0.4f }; static float clrBack[3]   = { 0.4f, 0.4f, 0.4f };

        int iColorPickerFlags = ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoLabel | ImGuiColorEditFlags_PickerHueWheel;

        // Top light color
        ImGui::Text("Top    : "); ImGui::SameLine();
        if (ImGui::ColorEdit3("##Up", clrUp, iColorPickerFlags) == true)
        {
            SetAmbientLight(vec(clrUp[0], clrUp[1], clrUp[2]), ModelPreview_t::AmbientLight_t::LIGHT_TOP);
        }

        // Bottom light color
        ImGui::Text("Bottom : "); ImGui::SameLine();
        if (ImGui::ColorEdit3("##Down", clrBottom, iColorPickerFlags) == true)
        {
            SetAmbientLight(vec(clrBottom[0], clrBottom[1], clrBottom[2]), ModelPreview_t::AmbientLight_t::LIGHT_BOTTON);
        }

        // Right light color
        ImGui::Text("Right  : "); ImGui::SameLine();
        if (ImGui::ColorEdit3("##Right", clrRight, iColorPickerFlags) == true)
        {
            SetAmbientLight(vec(clrRight[0], clrRight[1], clrRight[2]), ModelPreview_t::AmbientLight_t::LIGHT_RIGHT);
        }

        // Left light color
        ImGui::Text("Left   : "); ImGui::SameLine();
        if (ImGui::ColorEdit3("##Left", clrLeft, iColorPickerFlags) == true)
        {
            SetAmbientLight(vec(clrLeft[0], clrLeft[1], clrLeft[2]), ModelPreview_t::AmbientLight_t::LIGHT_LEFT);
        }

        // Front light color
        ImGui::Text("Front  : "); ImGui::SameLine();
        if (ImGui::ColorEdit3("##Front", clrFront, iColorPickerFlags) == true)
        {
            SetAmbientLight(vec(clrFront[0], clrFront[1], clrFront[2]), ModelPreview_t::AmbientLight_t::LIGHT_FRONT);
        }

        // Back light color
        ImGui::Text("Back   : "); ImGui::SameLine();
        if (ImGui::ColorEdit3("##Back", clrBack, iColorPickerFlags) == true)
        {
            SetAmbientLight(vec(clrBack[0], clrBack[1], clrBack[2]), ModelPreview_t::AmbientLight_t::LIGHT_BACK);
        }

        ImGui::PopStyleVar(); ImGui::PopStyleColor();
        ImGui::PopFont();
        ImGui::EndPopup();
    }
}


///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
void ModelPreview_t::_DrawModelSettings()
{
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing,   ImVec2(0.0f, 0.0f));
    
    static float flFeatureCount = 3.0f;
    static float flFeatureWidth = 300.0f;
    ImGui::SetNextWindowSize(ImVec2(flFeatureWidth + (2.0f * SECTION_PADDING_PXL), (2.0f * SECTION_PADDING_PXL) + (flFeatureCount - 1.0f) * INTER_FEATURE_PADDING_PXL + (FEATURE_HEIGHT * flFeatureCount)));
        
    int iPopupFlags = ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoTitleBar;
    if(ImGui::BeginPopup("ModelPreviewSettings", iPopupFlags) == true)
    {
        ImGui::PushFont(Resources::Fonts::JetBrainsMonoNerd_Small);

        ImVec2 vWindowPos(ImGui::GetWindowPos());
        ImVec2 vCursorPos(vWindowPos.x + SECTION_PADDING_PXL, vWindowPos.y + SECTION_PADDING_PXL);

        bool bDataModified = Render::menuGUI.DrawFloatInputWidget(
            Features::MaterialGen::ModelPreview::ModelAbsAngle.m_szFeatureDisplayName.c_str(), "##ModelYawMyNigga",
            vCursorPos, ImVec2(vCursorPos.x + flFeatureWidth, vCursorPos.y + FEATURE_HEIGHT),
            &Features::MaterialGen::ModelPreview::ModelAbsAngle.m_Data.m_flVal,
            Features::MaterialGen::ModelPreview::ModelAbsAngle.m_Data.m_flMin,
            Features::MaterialGen::ModelPreview::ModelAbsAngle.m_Data.m_flMax,
            Render::menuGUI.GetSecondaryClr(), 0.4f, 0.2f);

        vCursorPos.y += FEATURE_HEIGHT + INTER_FEATURE_PADDING_PXL;

        Render::menuGUI.DrawFloatInputWidget(
            Features::MaterialGen::ModelPreview::RotationSpeed.m_szFeatureDisplayName.c_str(), "##ModelRotationSpeed",
            vCursorPos, ImVec2(vCursorPos.x + flFeatureWidth, vCursorPos.y + FEATURE_HEIGHT),
            &Features::MaterialGen::ModelPreview::RotationSpeed.m_Data.m_flVal,
            Features::MaterialGen::ModelPreview::RotationSpeed.m_Data.m_flMin,
            Features::MaterialGen::ModelPreview::RotationSpeed.m_Data.m_flMax,
            Render::menuGUI.GetSecondaryClr(), 0.4f, 0.2f);

        vCursorPos.y += FEATURE_HEIGHT + INTER_FEATURE_PADDING_PXL;

        Render::menuGUI.DrawIntInputWidget(
            Features::MaterialGen::ModelPreview::AnimSquence.m_szFeatureDisplayName.c_str(), "##ModelAnimSquence",
            vCursorPos, ImVec2(vCursorPos.x + flFeatureWidth, vCursorPos.y + FEATURE_HEIGHT),
            &Features::MaterialGen::ModelPreview::AnimSquence.m_Data.m_iVal,
            Features::MaterialGen::ModelPreview::AnimSquence.m_Data.m_iMin,
            Features::MaterialGen::ModelPreview::AnimSquence.m_Data.m_iMax,
            Render::menuGUI.GetSecondaryClr(), 0.4f, 0.2f);

        BaseEntity* pModelEnt = GetModelEntity();
        if(pModelEnt != nullptr)
        {
            qangle& qModelAngles = pModelEnt->GetAbsAngles();

            // Only update when modified, else it won't let the model rotate by resetting shit every frame ( below )
            if(bDataModified == true)
            {
                qModelAngles.yaw = Features::MaterialGen::ModelPreview::ModelAbsAngle.GetData().m_flVal;
            }
        }

        ImGui::PopFont();
        ImGui::EndPopup();
    }
    ImGui::PopStyleVar(2);
}


///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
void ModelPreview_t::_RotateModel()
{
    if (m_pEnt == nullptr)
        return;

    static std::chrono::high_resolution_clock::time_point s_lastModelRotateTime;

    auto now = std::chrono::high_resolution_clock::now();
    int64_t iTimeSinceStartInMs = std::chrono::duration_cast<std::chrono::milliseconds>(now - s_lastModelRotateTime).count();
    s_lastModelRotateTime = now;

    float flTimeSinceLastUpdateInSec = static_cast<float>(iTimeSinceStartInMs) / 1000.0f;
    m_pEnt->GetAbsAngles().yaw += flTimeSinceLastUpdateInSec * Features::MaterialGen::ModelPreview::RotationSpeed.GetData().m_flVal;
}


///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
void ModelPreview_t::_AdjustCamera()
{
    // NOTE : FOV here works in a weird way. the FOV we set in the viewsetup is always good FOV 
    //        for horizontal view ( i.e. always good horizontal FOV ) , but we must use it to find the correct vertical FOV.
    BaseEntity* pEnt   = GetModelEntity();
    model_t*    pModel = GetActiveModel();

    if (pEnt == nullptr || pModel == nullptr || GetActiveModelIndex() == -1)
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
    vec vCameraOrigin = GetCameraPos();
    vec vModelOrigin  = GetModelEntity()->GetAbsOrigin();
    float flModelDist2D = vCameraOrigin.Dist2Dto(vModelOrigin);

    // this is max visible range at the model origin.
    float flFrustumHeight = 2.0f * tanf(flVerticalFOVInRad / 2.0f) * flModelDist2D;
    int iWidth = 0, iHeight = 0; GetRenderViewSize(iHeight, iWidth);

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

    SetCameraPos(vec(-flIdealDist, 0.0f, -flIdealCameraHeight));
}


///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
bool ModelPreview_t::_Initialize()
{
    if (m_bPanelInitilized == false)
    {
        if (_InitializePanel() == false)
        {
            FAIL_LOG("panel init failed");
            return false;
        }
    }


    if (m_bEntInit == false)
    {
        if (_InitializeEntity() == false)
        {
            FAIL_LOG("Entity init failed");
            return false;
        }
        
        m_pEnt->GetAbsAngles().yaw = Features::MaterialGen::ModelPreview::ModelAbsAngle.GetData().m_flVal;
    }


    return true;
}


///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
bool ModelPreview_t::_ShouldCreateStringTable()
{
    // Only when we are in lobby, do we need to create string tables ourselves.
    if (m_bJoiningMatch == false)
        return I::iEngine->IsConnected() == false;

    // else never create string tables, the engine will create it for us.
    return false;
}


///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
void ModelPreview_t::DiscardStringTables() const
{
    I::iNetworkStringTableContainer->RemoveAllTables();
    _SetTablePointer(nullptr);

    FAIL_LOG("Deleted MODEL_PRECACHE_TABLE and cleanup refrences.");
}


///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
void ModelPreview_t::SetActiveModel(int iIndex)
{
    if (m_bPanelInitilized == false || m_bEntInit == false)
        return;

    // Don't set model pointer if model name not added to string table.
    if (m_bModelPrecached == false)
        return;

    // If this model is the current model.
    if (iIndex == m_iActiveModelIndex)
        return;

    iIndex = std::clamp<int>(iIndex, 0, m_vecModels.size() - 1);

    m_iActiveModelIndex = iIndex;
    m_pActiveModel      = I::iModelLoader->GetModelForName(m_vecModels[iIndex].c_str(), IModelLoader::FMODELLOADER_CLIENT);

    if (m_pActiveModel == nullptr)
    {
        FAIL_LOG("Failed to load model \"%s\". Setting model to default ( %s )", m_vecModels[iIndex].c_str(), m_vecModels[0].c_str());
        SetActiveModel(0);
    }

    _UpdateEntityModel(m_iActiveModelIndex);
    WIN_LOG("Set active model to \"%s\" @ index : %d. [ %p ]", m_vecModels[m_iActiveModelIndex].c_str(), m_iActiveModelIndex, m_pActiveModel);
}


///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
void ModelPreview_t::SetActiveModel(std::string& szModelName)
{
    auto it = std::find(m_vecModels.begin(), m_vecModels.end(), szModelName);
    if (it == m_vecModels.end())
    {
        FAIL_LOG("Model not found in our model name list.");
        return;
    }

    // This should get the index. ( chatGPT )
    int iModelIndex = it - m_vecModels.begin();

    SetActiveModel(iModelIndex);
}


///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
bool ModelPreview_t::_InitializeEntity()
{
    assert(m_bEntInit == false);

    if (m_bEntInit == true)
    {
        FAIL_LOG("Entity already intitialized!!");
        return true;
    }


    constexpr uint64_t iEntSize = 0x400ull * 10ull;
    if (m_pEnt == nullptr)
    {
        m_pEnt = reinterpret_cast<BaseEntity*>(malloc(iEntSize));
        if (m_pEnt == nullptr)
        {
            FAIL_LOG("Bad malloc");
            return false;
        }

        memset(m_pEnt, 0, iEntSize);
        WIN_LOG("Model Entity allcoated @ %p", m_pEnt);
    }


    Sig::CBaseFlex_Constructor(m_pEnt);

    // Model we set here doesn't matter, We can change it later.
    auto* pTable = I::iNetworkStringTableContainer->FindTable(MODEL_PRECACHE_TABLENAME);
    if (pTable != nullptr)
    {
        int iIndex = pTable->FindStringIndex(m_vecModels[0].c_str());
        Sig::CBaseEntity_SetModelByIndex(m_pEnt, iIndex);
        LOG("Set default model to \"%s\" @ index [ %d ]", m_vecModels[0].c_str(), 0);
    }

    m_bEntInit = true;
    return true;
}


///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
void ModelPreview_t::_FreeEntity()
{
    if (m_pEnt == nullptr || m_bEntInit == false)
        return;

    free(m_pEnt);
    m_pEnt = nullptr;
    m_bEntInit = false;
    LOG("Free entity successfully");
}


///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
static void __fastcall PaintHijack(Panel* a1)
{
    // Setting panel's size
    int iWidth = 0, iHeight = 0;
    F::modelPreview.GetPanelSize(iHeight, iWidth);
    I::iPanel->SetSize(a1->GetVPanel(), iWidth, iHeight);
    RGBA_t panelClr = F::modelPreview.GetPanelClr(); I::iSurface->DrawSetint(panelClr.r, panelClr.g, panelClr.b, panelClr.a);
    I::iSurface->DrawFilledRect(0, 0, iWidth, iHeight);

    // Setting panel's pos
    int x = 0, y = 0; F::modelPreview.GetPanelPos(x, y);
    I::iPanel->SetPos(a1->GetVPanel(), x, y);

    BaseEntity* pEnt = F::modelPreview.GetModelEntity();

    // if model entity not initialized call original & leave.
    if (pEnt == nullptr || F::modelPreview.GetActiveModel() == nullptr || F::modelPreview.GetActiveModelIndex() == -1)
    {
        ((void(__fastcall*)(Panel*))(F::modelPreview.GetOriginalPaintFn()))(a1);
        return;
    }


    static void* pCubeMap{ nullptr };
    if (pCubeMap == nullptr)
    {
        pCubeMap = I::iMaterialSystem->FindTexture("editor/cubemap.hdr", TEXTURE_GROUP_CUBE_MAP, true);
        if (pCubeMap == nullptr) return;
        WIN_LOG("CubeMap good");
    }


    static CViewSetup view; static bool bViewInit = false;
    if (bViewInit == false)
    {
        bViewInit = true;
        memset(&view, 0, sizeof(CViewSetup));

        view.fov = 30.0f; view.m_bOrtho = false; view.zFar = 1000; view.zNear = 7;
        view.x = 100; view.y = 100;
        view.height = 100; view.width = 100;

        view.origin = vec(-220.0f, -4.0f, 35.0f); view.angles.Init();
        WIN_LOG("View good");
    }

    view.fov = F::modelPreview.GetBaseCameraFOV();

    // View setup origin
    view.origin = F::modelPreview.GetCameraPos();

    // Setting view setup pos & size.
    int iRenderViewHeight = 0, iRenderViewWidth = 0; F::modelPreview.GetRenderViewSize(iRenderViewHeight, iRenderViewWidth);
    int iRenderViewX      = 0, iRenderViewY     = 0; F::modelPreview.GetRenderViewPos(iRenderViewX, iRenderViewY);

    view.x      = iRenderViewX;      view.y     = iRenderViewY;
    view.height = iRenderViewHeight; view.width = iRenderViewWidth;

    // Animation
    {
        pEnt->m_flAnimTime(pEnt->m_flAnimTime() + tfObject.pGlobalVar->frametime / 2.0f);
        pEnt->m_flCycle(pEnt->m_flCycle() + 0.002f);
        pEnt->m_nSequence(Features::MaterialGen::ModelPreview::AnimSquence.GetData().m_iVal);
    }


    // Rendering model
    {
        IMatRenderContext* pRenderCtx = I::iMaterialSystem->GetRenderContext();
        pRenderCtx->BindLocalCubemap(pCubeMap);

        // Fix lighting n shit.
        vec vLightOrigin(0.0f); pRenderCtx->SetLightingOrigin(&vLightOrigin); // We also have signature for this function.
        pRenderCtx->SetAmbientLight(1.0f, 1.0f, 1.0f);
        I::iStudioRender->SetAmbientLightColors(F::modelPreview.GetAmbientLighting());
        I::iStudioRender->SetLocalLights(0, nullptr);


        Frustum dummyFrustum;
        I::iVRenderView->Push3DView(&view, 1 | 2 | 32, NULL, dummyFrustum);
        vec vClrModulation(1.0f); I::iVRenderView->SetColorModulation(&vClrModulation);
        I::iVRenderView->SetBlend(1.0f);

        RGBA_t clr = F::modelPreview.GetRenderViewClr(); pRenderCtx->ClearColor4ub(clr.r, clr.g, clr.b, clr.a);
        pRenderCtx->ClearBuffers(true, false);

        I::iVModelRender->SuppressEngineLighting(true);
        pEnt->DrawModel(STUDIO_RENDER);
        I::iVModelRender->SuppressEngineLighting(false);

        I::iVRenderView->PopView(dummyFrustum);

        pRenderCtx->BindLocalCubemap(nullptr);
    }


    // calling original paint.
    ((void(__fastcall*)(Panel*))(F::modelPreview.GetOriginalPaintFn()))(a1);
}


///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
void ModelPreview_t::_SetTablePointer(INetworkStringTable* pTable) const
{
    *reinterpret_cast<INetworkStringTable**>(reinterpret_cast<uintptr_t>(I::cClientState) + static_cast<uint64_t>(*ASM::ModelPrecacheTableOffset)) = pTable;
    //*reinterpret_cast<INetworkStringTable**>(reinterpret_cast<uintptr_t>(I::CGameServer) + 0x54298LLU)  = pTable;
}


///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
void ModelPreview_t::_VerifyOrCreateStringTable() const
{
    // If in game, then the table must be there.
    if (I::iEngine->IsInGame() == true)
        return;

    // already made
    INetworkStringTable* pTable = I::iNetworkStringTableContainer->FindTable(MODEL_PRECACHE_TABLENAME);
    if (pTable != nullptr)
        return;

    I::iNetworkStringTableContainer->m_bAllowCreation = true;
    pTable = I::iNetworkStringTableContainer->CreateStringTableEx(MODEL_PRECACHE_TABLENAME, (1 << 12), 1, 2, false);
    I::iNetworkStringTableContainer->m_bAllowCreation = false;

    _SetTablePointer(pTable);

    if (pTable == nullptr)
        FAIL_LOG("Failed to create string table");

    WIN_LOG("Created \"%s\" string table successfully", MODEL_PRECACHE_TABLENAME);
}


///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
bool ModelPreview_t::_PrecacheModels() const
{
    INetworkStringTable* pTable = I::iNetworkStringTableContainer->FindTable(MODEL_PRECACHE_TABLENAME);
    if (pTable == nullptr)
    {
        FAIL_LOG("MODEL_PRECACHE_TABLE not found");
        return false;
    }


    for (const std::string& szModelname : m_vecModels)
    {
        int iMDLIndex = pTable->FindStringIndex(szModelname.c_str());

        // already added to table
        if (iMDLIndex != INVALID_TABLE_INDEX)
        {
            LOG("Model \"%s\" is already precached", szModelname.c_str());
            continue;
        }

        pTable->AddString(false, szModelname.c_str());

        int iModelIndex = pTable->FindStringIndex(szModelname.c_str());

        LOG("Precached model \"%s\" @ index : %d", szModelname.c_str(), iModelIndex);
    }

    WIN_LOG("Model Precache done!");
    return true;
}


///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
void ModelPreview_t::_UpdateEntityModel(int iIndex)
{
    // Does table even exist yet?
    auto* pTable = I::iNetworkStringTableContainer->FindTable(MODEL_PRECACHE_TABLENAME);
    if (pTable == nullptr)
        return;

    iIndex = std::clamp<int>(iIndex, 0, m_vecModels.size() - 1);
    int iModelIndex = pTable->FindStringIndex(m_vecModels[iIndex].c_str());

    Sig::CBaseEntity_SetModelByIndex(m_pEnt, iModelIndex);
}


///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
bool ModelPreview_t::_InitializePanel()
{
    if (m_bPanelInitilized == true)
        return true;

    assert(m_pSpoofedVTable == nullptr && m_pOriginalPaint == nullptr && m_pOriginalVTable == nullptr && "Trying to reinitialize panel object.");

    // Checking if the panel already exists. ( i.e. if we are injecting. )
    // EDIT : It seems like the panel gets auto deleted, cause I am unable to find it in reinjection. I'm just gonna leave it here.
    vgui::VPANEL vDesiredPanel = I::iPanel->FindChildByName(I::iSurface->GetEmbeddedPanel(), std::string(m_szPanelName), true);
    if (vDesiredPanel != NULL)
    {
        if (_SpoofVTable() == false)
        {
            FAIL_LOG("Failed to spoof VTable");
            return false;
        }

        I::iPanel->SetParent(vDesiredPanel, I::iSurface->GetEmbeddedPanel());
        I::iPanel->SetEnabled(m_pPanel->GetVPanel(), true);
        I::iPanel->SetVisible(m_pPanel->GetVPanel(), true);
        I::iPanel->SetPos(m_pPanel->GetVPanel(), 0, 0);
        I::iPanel->SetSize(m_pPanel->GetVPanel(), m_iPanelWidth, m_iPanelHeight);

        LOG("Panel \"%s\" already exists, reusing it.", m_szPanelName);

        m_bPanelInitilized = true;
        return true;
    }
    LOG("Panel \"%s\" doesn't exist yet. Creating one.", m_szPanelName);


    // Allocate our panel object some memory.
    if (m_pPanel == nullptr)
    {
        constexpr size_t iPanelSize = 0x400ULL * 2; //  It only needed 400 bytes, but its valve we are talking about.
        m_pPanel = reinterpret_cast<Panel*>(malloc(iPanelSize));
        if (m_pPanel == nullptr)
        {
            FAIL_LOG("Failed memory allocation for panel");
            return false;
        }

        memset(m_pPanel, 0, iPanelSize);
    }

    // Check if the parent panel is good or not
    VPanel* pParentVPanel = reinterpret_cast<VPanel*>(I::iSurface->GetEmbeddedPanel());
    if (pParentVPanel == nullptr)
    {
        FAIL_LOG("Bad parent VPanel!");
        return false;
    }
    Panel* pParentPanel = pParentVPanel->Client();
    if (pParentPanel == nullptr)
    {
        FAIL_LOG("Parent panel* is invalid ( NULL )");
        return false;
    }


    // Run our panel object throught the constructor so VTables-n-shit gets setup properly
    Sig::VGui_Panel_Constructor(m_pPanel, pParentPanel, m_szPanelName);
    Sig::VGui_Panel_MakeReadyToUse(m_pPanel);

    // spoof the VTable, replace the paint function with ours.
    if (m_pSpoofedVTable == nullptr)
    {
        _SpoofVTable();
    }

    // Enable the panel.
    I::iPanel->SetEnabled(m_pPanel->GetVPanel(), true);
    I::iPanel->SetVisible(m_pPanel->GetVPanel(), true);
    I::iPanel->SetPos(m_pPanel->GetVPanel(), 0, 0);
    I::iPanel->SetSize(m_pPanel->GetVPanel(), m_iPanelWidth, m_iPanelHeight);

    m_bPanelInitilized = true;
    WIN_LOG("Showcase panel initialized! @[ %p ]", m_pPanel);
    return true;
}


///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
bool ModelPreview_t::_SpoofVTable()
{
    if (m_pSpoofedVTable != nullptr)
        return false;


    constexpr size_t iVTableSize = 213ULL * sizeof(void*); // Its 257 fn for CTFPlayerModelPanel
    m_pSpoofedVTable = malloc(iVTableSize);
    if (m_pSpoofedVTable == nullptr)
    {
        FAIL_LOG("Failed to allocate mem for spoofed vtable");
        return false;
    }

    // just to be safe, check if paint fn index gets changed.
    if (*ASM::PaintFnOffset != 0x410)
        FAIL_LOG("Panel::Paint fn index has been modified by devs. We are using index : %d offset : 0x%X", *ASM::PaintFnOffset / 8, *ASM::PaintFnOffset);

    // copy original vtable to our memory.
    m_pOriginalVTable = *reinterpret_cast<void***>(m_pPanel);
    memcpy(m_pSpoofedVTable, m_pOriginalVTable, iVTableSize);

    // Store the original paint fn adrs.
    m_pOriginalPaint = *reinterpret_cast<void**>(reinterpret_cast<uintptr_t>(m_pOriginalVTable) + *ASM::PaintFnOffset);

    // Sawp out the paint function.
    *reinterpret_cast<uintptr_t*>(reinterpret_cast<uintptr_t>(m_pSpoofedVTable) + *ASM::PaintFnOffset) = reinterpret_cast<uintptr_t>(PaintHijack);

    // Swap out the VTable with our modified VTable.
    *reinterpret_cast<void**>(m_pPanel) = m_pSpoofedVTable;

    WIN_LOG("Spoofed VTable successfully");
}


///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
void ModelPreview_t::Free()
{
    // Remove this panel out of the way else it will block mouse clicks
    if(m_pPanel != nullptr)
    {
        I::iPanel->SetVisible(m_pPanel->GetVPanel(), false);
        I::iPanel->SetEnabled(m_pPanel->GetVPanel(), false);
        I::iPanel->SetSize(m_pPanel->GetVPanel(), 1, 1); // This seems to fix it.
    }

    g_vecModelNames.clear();       g_vecModelNames.shrink_to_fit();
    g_vecModelNamesInGame.clear(); g_vecModelNamesInGame.shrink_to_fit();

    _FreeVTable();
    _FreeEntity();
}


///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
void ModelPreview_t::_FreeVTable()
{
    if (m_pSpoofedVTable == nullptr || m_pOriginalVTable == nullptr)
        return;
    
    // Restore the original VTable for our panel object.
    *reinterpret_cast<uintptr_t*>(m_pPanel) = reinterpret_cast<uintptr_t>(m_pOriginalVTable);

    free(m_pSpoofedVTable);
    WIN_LOG("Freed & restored VTable succesfully");
}


/////////////////////// GETTERS & SETTERS /////////////////////////////////
///////////////////////////////////////////////////////////////////////////
void ModelPreview_t::SetPanelClr(unsigned char r, unsigned char g, unsigned char b, unsigned char a)
{
    m_panelClr.r = r; m_panelClr.g = g; m_panelClr.b = b; m_panelClr.a = a;
}


///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
void ModelPreview_t::SetPanelClr(RGBA_t clr)
{
    m_panelClr = clr;
}


///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
RGBA_t ModelPreview_t::GetPanelClr() const
{
    return m_panelClr;
}


///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
void ModelPreview_t::SetRenderViewClr(unsigned char r, unsigned char g, unsigned char b, unsigned char a)
{
    m_renderViewClr.r = r; m_renderViewClr.g = g; m_renderViewClr.b = b; m_renderViewClr.a = a;
}


///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
void ModelPreview_t::SetRenderViewClr(RGBA_t clr)
{
    m_renderViewClr = clr;
}

///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
RGBA_t ModelPreview_t::GetRenderViewClr() const
{
    return m_renderViewClr;
}


///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
void ModelPreview_t::SetCameraPos(const vec& vCameraPos)
{
    m_vCameraPos = vCameraPos;
}


///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
vec ModelPreview_t::GetCameraPos() const
{
    return m_vCameraPos;
}


///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
float ModelPreview_t::GetBaseCameraFOV() const
{
    return m_flCameraFOV;
}


///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
float ModelPreview_t::GetVerticalFOV() const
{
    int iHeight = 0, iWidth = 0; GetRenderViewSize(iHeight, iWidth);

    float flScale = static_cast<float>(iHeight) / static_cast<float>(iWidth);

    float flHorizontalFOV = DEG2RAD(GetBaseCameraFOV());
    
    // Frustum width at 1 unit from camera. with correct horizontal FOV ofcourse.
    float flFrustumWidth = 2.0f * (tanf(flHorizontalFOV / 2.0f) * 1.0f);

    // Size the height according to how much bigger the height is from the width.
    float flFrustumHeight = flFrustumWidth * flScale;

    return RAD2DEG(2.0f * atanf((flFrustumHeight / 2.0f) / 1.0f));
}


///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
float ModelPreview_t::GetHorizontalFOV() const
{
    float flBaseFOV = DEG2RAD(GetBaseCameraFOV());

    int iHeight = 0, iWidth = 0; GetRenderViewSize(iHeight, iWidth);
    float flWidthCorrect = static_cast<float>(iHeight) * (4.0f / 3.0f);
    
    // This is the distance at which the frustum width will be equal to 4:3 of the height.
    // cause that is what the FOV is correct for.
    float flDist = (static_cast<float>(flWidthCorrect) / 2.0f) / tanf(flBaseFOV / 2.0f);

    // at the same distance ( flDist ) what angle will result in the frustum being equal to 
    // our renderview's size ( i.e. custom width )
    return RAD2DEG(2.0f * atanf((static_cast<float>(iWidth) / 2.0f) / flDist));
}


///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
void ModelPreview_t::SetCameraFOV(const float flCameraFOV)
{
    m_flCameraFOV = flCameraFOV;
}


///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
void ModelPreview_t::InvalidateModelPrecache()
{
    m_bModelPrecached = false;

    m_vecModels.clear();
    m_vecModels.push_back("models/player/soldier.mdl");
}


///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
void ModelPreview_t::JoiningMatch(bool bJoining)
{
    m_bJoiningMatch = bJoining;
}


///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
model_t* ModelPreview_t::GetActiveModel() const
{
    return m_pActiveModel;
}


///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
bool ModelPreview_t::AddModel(std::string& szModelName)
{
    // Checking if model name is valid or not.
    std::vector<std::string>& vecValidModelNames = GetModelNameList();
    auto it = std::find(vecValidModelNames.begin(), vecValidModelNames.end(), szModelName);
    if (it == vecValidModelNames.end())
    {
        FAIL_LOG("Model name [ %s ] is invalid", szModelName.c_str());
        return false;
    }


    // Add model name in our model name list.
    auto it2 = std::find(m_vecModels.begin(), m_vecModels.end(), szModelName);
    if (it2 == m_vecModels.end())
    {
        m_vecModels.push_back(szModelName);
        WIN_LOG("Model name [ %s ] not found in our model name list. so I Added it", szModelName.c_str());
    }


    // Don't add to network string table if in game.
    if (I::iEngine->IsConnected() == true)
        return true;

    INetworkStringTable* pModelTable = I::iNetworkStringTableContainer->FindTable(MODEL_PRECACHE_TABLENAME);
    if (pModelTable == nullptr)
    {
        FAIL_LOG("Model precache table doesn't exist yet.");
        return false;
    }

    // Add model name in ModelPrecache table.
    if (pModelTable->FindStringIndex(szModelName.c_str()) == INVALID_STRING_INDEX)
    {
        pModelTable->AddString(false, szModelName.c_str());
        WIN_LOG("Model name [ %s ] not found in model string table. so I Added it", szModelName.c_str());
    }

    return true;
}


///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
void ModelPreview_t::CaptureAllEngineModels()
{
    // if in game, get all the names for all the models loaded by the engine.
    if (I::iEngine->IsConnected() == false)
        return;

    INetworkStringTable* pTable = I::iNetworkStringTableContainer->FindTable(MODEL_PRECACHE_TABLENAME);
    if (pTable == nullptr)
        return;

    g_vecModelNamesInGame.clear();

    int nModels = pTable->GetNumStrings();
    for (int iModelIndex = 0; iModelIndex < nModels; iModelIndex++)
    {
        g_vecModelNamesInGame.push_back(std::string(pTable->GetString(iModelIndex)));
    }

    WIN_LOG("Done capturing [ %d ] models", nModels);
}


///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
int ModelPreview_t::GetActiveModelIndex() const
{
    return m_iActiveModelIndex;
}


///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
BaseEntity* ModelPreview_t::GetModelEntity() const
{
    return m_pEnt;
}


///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
std::vector<std::string>& ModelPreview_t::GetModelNameList() const
{
    // if not in any game & not trying to join any game, Use the predefined model list.
    if (m_bJoiningMatch == false && I::iEngine->IsConnected() == false)
        return g_vecModelNames;

    // else use engine's models. ( se we don't have to load a million more models. )
    return g_vecModelNamesInGame;
}


///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
void ModelPreview_t::SetVisible(bool bVisible)
{
    // Change panel visibility
    if (m_pPanel != nullptr)
    {
        I::iPanel->SetVisible(m_pPanel->GetVPanel(), bVisible);
    }

    // Make sure initialization is done.
    if (m_pEnt == nullptr || m_iActiveModelIndex == -1 || m_pActiveModel == nullptr)
        return;

    bool bRefreshModel = bVisible == true && bVisible != m_bVisible;
    m_bVisible = bVisible;

    if (bRefreshModel == false)
        return;

    // Refresh model once if opened ( to prevent bullshit )
    _UpdateEntityModel(m_iActiveModelIndex);
    LOG("Refreshed entity model to \"%s\"", m_vecModels[m_iActiveModelIndex].c_str());
}


///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
bool ModelPreview_t::IsVisible() const
{
    return m_bVisible;
}


///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
void ModelPreview_t::SetPanelSize(int iHeight, int iWidth)
{
    m_iPanelHeight = iHeight;
    m_iPanelWidth  = iWidth;
}


///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
void ModelPreview_t::GetPanelSize(int& iHeight, int& iWidth) const
{
    iHeight = m_iPanelHeight; 
    iWidth  = m_iPanelWidth;
}


///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
void ModelPreview_t::SetPanelPos(int x, int y)
{
    m_iPanelX = x; m_iPanelY = y;
}


///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
void ModelPreview_t::GetPanelPos(int& x, int& y) const
{
    x = m_iPanelX; y = m_iPanelY;
}


///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
int ModelPreview_t::GetDefaultWidth() const
{
    return DEFAULT_WIDTH;
}


///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
void ModelPreview_t::SetRenderViewSize(int iHeight, int iWidth)
{
    m_iRenderViewHeight = iHeight; 
    m_iRenderViewWidth  = iWidth;
}


///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
void ModelPreview_t::GetRenderViewSize(int& iHeight, int& iWidth) const
{
    iHeight = m_iRenderViewHeight; 
    iWidth  = m_iRenderViewWidth;
}


///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
void ModelPreview_t::SetRenderViewPos(int x, int y)
{
    m_iRenderViewX = x; m_iRenderViewY = y;
}


///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
void ModelPreview_t::GetRenderViewPos(int& x, int& y) const
{
    x = m_iRenderViewX; y = m_iRenderViewY;
}


///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
vec* ModelPreview_t::GetAmbientLighting()
{
    return m_vAmbientLight;
}


///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
void ModelPreview_t::SetAmbientLight(const vec& vLight, AmbientLight_t iLightIndex)
{
    m_vAmbientLight[iLightIndex] = vLight;
}


///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
void ModelPreview_t::SetDefaultLight()
{
    constexpr vec vDefaultLight(0.4f, 0.4f, 0.4f);

    m_vAmbientLight[AmbientLight_t::LIGHT_TOP]    = vDefaultLight;
    m_vAmbientLight[AmbientLight_t::LIGHT_BOTTON] = vDefaultLight;
    m_vAmbientLight[AmbientLight_t::LIGHT_BACK]   = vDefaultLight;
    m_vAmbientLight[AmbientLight_t::LIGHT_FRONT]  = vDefaultLight;
    m_vAmbientLight[AmbientLight_t::LIGHT_RIGHT]  = vDefaultLight;
    m_vAmbientLight[AmbientLight_t::LIGHT_LEFT]   = vDefaultLight;
}




//=========================================================================
//                     DEBUGGING
//=========================================================================
#if (DISABLE_ESSENTIAL_HOOKS == false)

struct LightingState_t
{
    vec         r_boxcolor[6];		// ambient, and lights that aren't in locallight[]
    int			numlights;
    void* locallight[4];
};

MAKE_HOOK(LightcacheGetDynamic, "48 89 5C 24 ? 44 89 4C 24 ? 4C 89 44 24 ? 55 56 57 41 54 41 55 41 56 41 57 48 81 EC ? ? ? ? 48 8B 05", __fastcall, ENGINE_DLL,
    void*, void* pThis, vec* vOrigin, LightingState_t* pLightState, void* a1, int a2, bool a3)
{
    if (I::iEngine->IsInGame() == true)
        return Hook::LightcacheGetDynamic::O_LightcacheGetDynamic(pThis, vOrigin, pLightState, a1, a2, a3);


    //WIN_LOG("Returned spoofed light state for Origin { %.2f %.2f %.2f }", vOrigin->x, vOrigin->y, vOrigin->z);
    for (int i = 0; i < 6; i++)
    {
        pLightState->r_boxcolor[i] = vec(1.0f, 1.0f, 1.0f);

        if (i < 4)
            pLightState->locallight[i] = nullptr;
    }
    pLightState->numlights = 0;
    return nullptr;
}


MAKE_HOOK(CBaseEntity_UpdateVisibility, "48 89 5C 24 ? 48 89 74 24 ? 57 48 83 EC ? 48 8B D9 48 8B 0D", __fastcall, CLIENT_DLL, void*,
    void* pEnt)
{
    if (pEnt == F::modelPreview.GetModelEntity())
    {
        FAIL_LOG("Skipped visibility update for ModelEntity.");
        return nullptr;
    }

    return Hook::CBaseEntity_UpdateVisibility::O_CBaseEntity_UpdateVisibility(pEnt);
}

#endif


///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
std::vector<std::string> g_vecModelNamesInGame = {};


std::vector<std::string> g_vecModelNames = {
    "models/player/spy.mdl",
    "models/player/heavy.mdl",
    "models/props_gameplay/tombstone_specialdelivery.mdl",
    "models/props_gameplay/tombstone_crocostyle.mdl",
    "models/props_gameplay/tombstone_tankbuster.mdl",
    "models/props_gameplay/tombstone_gasjockey.mdl",
    "models/weapons/c_models/c_engineer_gunslinger.mdl",
    "models/weapons/w_models/w_stickybomb.mdl",
    "models/weapons/w_models/w_stickybomb_gib1.mdl",
    "models/weapons/w_models/w_stickybomb_gib2.mdl",
    "models/weapons/w_models/w_stickybomb_gib3.mdl",
    "models/weapons/w_models/w_stickybomb2.mdl",
    "models/weapons/w_models/w_stickybomb_d.mdl",
    "models/weapons/w_models/w_grenade_grenadelauncher.mdl",
    "models/weapons/w_models/w_cannonball.mdl",
    "models/weapons/c_models/c_engineer_gunslinger.mdl",
    "models/weapons/w_models/w_stickybomb.mdl",
    "models/weapons/w_models/w_stickybomb_gib1.mdl",
    "models/weapons/w_models/w_stickybomb_gib2.mdl",
    "models/weapons/w_models/w_stickybomb_gib3.mdl",
    "models/weapons/w_models/w_stickybomb2.mdl",
    "models/weapons/w_models/w_stickybomb_d.mdl",
    "models/weapons/w_models/w_grenade_grenadelauncher.mdl",
    "models/weapons/w_models/w_cannonball.mdl",
    "models/workshop/weapons/c_models/c_kingmaker_sticky/w_kingmaker_stickybomb.mdl",
    "models/workshop/weapons/c_models/c_quadball/w_quadball_grenade.mdl",
    "models/weapons/shells/shell_sniperrifle.mdl",
    "models/weapons/shells/shell_pistol.mdl",
    "models/weapons/shells/shell_shotgun.mdl",
    "models/player/items/engineer/guitar.mdl",
    "models/player/items/engineer/guitar_gib1.mdl",
    "models/player/items/engineer/guitar_gib2.mdl",
    "models/player/items/engineer/guitar_gib3.mdl",
    "models/weapons/v_models/v_pda_spy.mdl",
    "models/weapons/c_models/c_buffbanner/c_buffbanner.mdl",
    "models/workshop/weapons/c_models/c_battalion_buffbanner/c_battalion_buffbanner.mdl",
    "models/workshop_partner/weapons/c_models/c_shogun_warbanner/c_shogun_warbanner.mdl",
    "models/workshop/weapons/c_models/c_paratooper_pack/c_paratrooper_parachute.mdl",
    "models/workshop/weapons/c_models/c_paratooper_pack/c_paratrooper_pack_open.mdl",
    "models/weapons/shells/shell_minigun.mdl",
    "models/props_mvm/mvm_player_shield.mdl",
    "models/props_mvm/mvm_player_shield2.mdl",
    "models/effects/sentry1_muzzle/sentry1_muzzle.mdl",
    "models/weapons/w_models/w_rocket.mdl",
    "models/weapons/w_models/w_rocket_airstrike/w_rocket_airstrike.mdl",
    "models/weapons/w_models/w_drg_ball.mdl",
    "models/items/medkit_medium.mdl",
    "models/items/medkit_medium_bday.mdl",
    "models/items/plate.mdl",
    "models/workshop/weapons/c_models/c_buffalo_steak/plate_buffalo_steak.mdl",
    "models/items/plate_robo_sandwich.mdl",
    "models/items/plate_sandwich_xmas.mdl",
    "models/workshop/weapons/c_models/c_chocolate/plate_chocolate.mdl",
    "models/items/banana/plate_banana.mdl",
    "models/workshop/weapons/c_models/c_fishcake/plate_fishcake.mdl",
    "models/workshop_partner/weapons/c_models/c_sd_cleaver/v_sd_cleaver.mdl",
    "models/weapons/v_models/v_grenadelauncher_demo.mdl",
    "models/weapons/w_models/w_grenadelauncher.mdl",
    "models/player/items/pyro/mtp_bubble_wand.mdl",
    "models/items/ammopack_small.mdl",
    "models/workshop/weapons/c_models/c_caber/c_caber_exploded.mdl",
    "models/weapons/c_models/c_xms_festive_ornament.mdl",
    "models/weapons/w_models/w_baseball.mdl",
    "models/weapons/v_models/v_baseball.mdl",
    "models/weapons/w_models/w_flaregun_shell.mdl",
    "models/weapons/w_models/w_arrow.mdl",
    "models/weapons/w_models/w_repair_claw.mdl",
    "models/weapons/w_models/w_arrow_xmas.mdl",
    "models/workshop/weapons/c_models/c_crusaders_crossbow/c_crusaders_crossbow_xmas_proj.mdl",
    "models/weapons/w_models/w_arrow_gib1.mdl",
    "models/weapons/w_models/w_arrow_gib2.mdl",
    "models/weapons/w_models/w_repair_claw_gib1.mdl",
    "models/weapons/w_models/w_repair_claw_gib2.mdlmodels/weapons/w_models/w_arrow_xmas_gib2.mdl",
    "models/player/items/all_class/ring_Scout.mdl",
    "models/player/items/all_class/ring_Sniper.mdl",
    "models/player/items/all_class/ring_Soldier.mdl",
    "models/player/items/all_class/ring_demo.mdl",
    "models/player/items/all_class/ring_Medic.mdl",
    "models/player/items/all_class/ring_Heavy.mdl",
    "models/player/items/all_class/ring_Pyro.mdl",
    "models/player/items/all_class/ring_Spy.mdl",
    "models/player/items/all_class/ring_Engineer.mdl",
    "models/player/items/all_class/hwn_spellbook_diary.mdl",
    "models/player/items/scout/scout_zombie.mdl",
    "models/player/items/soldier/soldier_zombie.mdl",
    "models/player/items/heavy/heavy_zombie.mdl",
    "models/player/items/demo/demo_zombie.mdl",
    "models/player/items/engineer/engineer_zombie.mdl",
    "models/player/items/medic/medic_zombie.mdl",
    "models/player/items/spy/spy_zombie.mdl",
    "models/player/items/pyro/pyro_zombie.mdl",
    "models/player/items/sniper/sniper_zombie.mdl",
    "models/player/items/crafting/weapons_case.mdl",
    "models/player/items/crafting/coin_summer2015_gravel.mdl",
    "models/player/items/crafting/coin_summer2015_bronze.mdl",
    "models/player/items/crafting/coin_summer2015_silver.mdl",
    "models/player/items/crafting/coin_summer2015_gold.mdl",
    "models/player/items/crafting/cosmetic_case.mdl",
    "models/player/items/crafting/coin_invasion.mdl",
    "models/workshop/cases/invasion_case/invasion_case.mdl",
    "models/workshop/cases/invasion_case/invasion_case_rare.mdl",
    "models/props_halloween/gargoyle_backpack.mdl",
    "models/player/items/crafting/halloween2015_case.mdl",
    "models/player/items/crafting/stamp_winter2016.mdl",
    "models/items/gift_festive.mdl",
    "models/items/festivizer.mdl",
    "models/weapons/c_models/stattrack.mdl",
    "models/player/items/crafting/halloween_case.mdl",
    "models/player/items/crafting/cosmetic_case_ribbon.mdl",
    "models/player/items/crafting/community_cosmetic_case.mdl",
    "models/player/items/cyoa_pda/cyoa_pda.mdl",
    "models/player/items/crafting/halloween2021_case.mdl",
    "models/player/items/all_class/hwn_spellbook_complete.mdl",
    "models/weapons/c_models/c_xms_double_barrel.mdl",
    "models/workshop/weapons/c_models/c_crusaders_crossbow/c_crusaders_crossbow_xmas.mdl",
    "models/weapons/c_models/c_sapper/c_sapper_xmas.mdl",
    "models/weapons/c_models/c_xms_flaregun/c_xms_flaregun.mdl",
    "models/weapons/c_models/c_claymore/c_claymore_xmas.mdl",
    "models/weapons/c_models/c_xms_urinejar.mdl",
    "models/weapons/c_models/c_boxing_gloves/c_boxing_gloves_xmas.mdl",
    "models/workshop/weapons/c_models/c_blackbox/c_blackbox_xmas.mdl",
    "models/weapons/c_models/c_wrangler_xmas.mdl",
    "models/workshop_partner/weapons/c_models/c_bow_thief/c_bow_thief.mdl",
    "models/weapons/c_models/c_tfc_sniperrifle/c_tfc_sniperrifle.mdl",
    "models/workshop/weapons/c_models/c_wheel_shield/c_wheel_shield.mdl",
    "models/weapons/c_models/c_breadmonster_gloves/c_breadmonster_gloves.mdl",
    "models/workshop/weapons/c_models/c_paratooper_pack/c_paratrooper_pack.mdl",
    "models/weapons/c_models/c_breadmonster_sapper/c_breadmonster_sapper.mdl",
    "models/workshop/weapons/c_models/c_scatterdrum/c_scatterdrum.mdl",
    "models/workshop/weapons/c_models/c_atom_launcher/c_atom_launcher.mdl",
    "models/workshop/weapons/c_models/c_atom_launcher/c_atom_launcher_festivizer.mdl",
    "models/weapons/c_models/c_breadmonster/c_breadmonster.mdl",
    "models/player/items/taunts/cash_wad.mdl",
    "models/player/items/taunts/medic_xray_taunt.mdl",
    "models/player/items/taunts/balloon_animal_pyro/balloon_animal_pyro.mdl",
    "models/player/items/taunts/wupass_mug/wupass_mug.mdl",
    "models/player/items/taunts/demo_nuke_bottle/demo_nuke_bottle.mdl",
    "models/player/items/taunts/engys_new_chair/engys_new_chair_articlulated.mdl",
    "models/player/items/taunts/chicken_bucket/chicken_bucket.mdl",
    "models/player/items/taunts/beer_crate/beer_crate.mdl",
    "models/weapons/c_models/c_breadmonster/c_breadmonster_milk.mdl",
    "models/weapons/c_models/c_carnival_mallet/c_carnival_mallet.mdl",
    "models/workshop/player/items/pyro/threea_nabler/threea_nabler.mdl",
    "models/workshop/player/items/all_class/all_class_badge_bonusd/all_class_badge_bonusd.mdl",
    "models/workshop/weapons/c_models/c_crossing_guard/c_crossing_guard.mdl",
    "models/weapons/c_models/c_shotgun/c_shotgun_xmas.mdl",
    "models/weapons/c_models/c_revolver/c_revolver_xmas.mdl",
    "models/weapons/c_models/c_bonesaw/c_bonesaw_xmas.mdl",
    "models/weapons/c_models/c_targe/c_targe_xmas.mdl",
    "models/weapons/c_models/c_xms_energy_drink/c_xms_energy_drink.mdl",
    "models/weapons/c_models/c_flamethrower/c_backburner_xmas.mdl",
    "models/weapons/c_models/c_smg/c_smg_xmas.mdl",
    "models/workshop/weapons/c_models/c_kingmaker_sticky/c_kingmaker_sticky.mdl",
    "models/workshop/weapons/c_models/c_quadball/c_quadball.mdl",
    "models/workshop/weapons/c_models/c_quadball/c_quadball_festivizer.mdl",
    "models/workshop/weapons/c_models/c_trenchgun/c_trenchgun.mdl",
    "models/workshop/weapons/c_models/c_trenchgun/c_trenchgun_festivizer.mdl",
    "models/passtime/ball/passtime_ball.mdl",
    "models/player/items/taunts/bumpercar/parts/bumpercar.mdl",
    "models/player/items/heavy/heavy_table_flip_prop.mdl",
    "models/player/items/heavy/heavy_table_flip_joule_prop.mdl",
    "models/weapons/c_models/c_flameball/c_flameball.mdl",
    "models/weapons/c_models/c_flameball/c_flameball_festivizer.mdl",
    "models/weapons/c_models/c_rocketpack/c_rocketpack.mdl",
    "models/player/items/taunts/matchbox/matchbox.mdl",
    "models/weapons/c_models/c_gascan/c_gascan.mdl",
    "models/weapons/c_models/c_slapping_glove/c_slapping_glove.mdl",
    "models/weapons/c_models/c_slapping_glove/w_slapping_glove.mdl",
    "models/player/items/taunts/yeti_standee/yeti_standee.mdl",
    "models/player/items/taunts/yeti/yeti.mdl",
    "models/weapons/c_models/c_banana/c_banana.mdl",
    "models/player/items/taunts/tank/tank.mdl",
    "models/player/items/taunts/scooter/scooter.mdl",
    "models/items/paintkit_tool.mdl"
};
