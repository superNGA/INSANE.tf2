// Utility
#include "../Utility/Hook Handler/Hook_t.h"
#include "../Utility/ConsoleLogging.h"

// SDK
#include "../SDK/class/IMaterialSystem.h"
#include "../SDK/class/IStudioRender.h"
#include "../SDK/class/IMaterial.h"
#include "../SDK/class/BaseEntity.h"

#include "../Features/Chams/Chams.h"
#include "../Features/ModelPreview/ModelPreview.h"
#include "EndScene/EndScene.h"


MAKE_SIG(KeyValues_LoadFromBuffer, "4C 89 4C 24 ? 48 89 4C 24 ? 55 56 48 81 EC", CLIENT_DLL, bool, void*, const char*, const char*, const char*, const char*)
MAKE_SIG(ForcedMaterialOverride_NIGGA, "48 89 91 ? ? ? ? 44 89 81", STUDIORENDER_DLL, void, void*, IMaterial*, int)
MAKE_SIG(CreateMaterial_NIGGA, "48 89 5C 24 ? 57 48 83 EC ? 48 8B C2", MATERIALSYSTEM_DLL, IMaterial*, IMaterialSystem*, const char*, KeyValues*)


static const char szMat[] = R"(
VertexLitGeneric
    {
        "$basetexture" "vgui/white"
        "$envmap" "env_cubemap"
        "$envmaptint" "[1 1 1]"
        "$ignorez" "1"
        "$envmapfresnel" "1"
        "$phong" "1"
        "$phongexponent" "20"
        "$phongboost" "2"
    }
)";


MAKE_HOOK(DrawModelExecute, "4C 89 4C 24 ? 48 89 4C 24 ? 55 53 56 57 41 54", __fastcall, ENGINE_DLL, int64_t, 
    void* pVTable, DrawModelState_t* modelState, ModelRenderInfo_t* renderInfo, matrix3x4_t* boneMatrix)
{
    // Don't do material altering at unstable states, else causes crahes.
    if(directX::UI::UI_has_been_shutdown == true)
        return Hook::DrawModelExecute::O_DrawModelExecute(pVTable, modelState, renderInfo, boneMatrix);

    static KeyValues* pKeyValues = nullptr;
    static bool bMatInit = false;
    if (bMatInit == false)
    {
        pKeyValues = new KeyValues;
        pKeyValues->Init();

        bMatInit = Sig::KeyValues_LoadFromBuffer(pKeyValues, "NigaMaterial", szMat, NULL, NULL);
        
        if (bMatInit == true)
            WIN_LOG("Material loaded into KV successfully");
        else
            FAIL_LOG("Failed to load material from buffer");

        printf("IKeyName : %d & DataType : %d | IKeyName : %d & DataType : %d\n", pKeyValues->m_iKeyName, pKeyValues->m_iDataType, pKeyValues->m_pSub->m_iKeyName, pKeyValues->m_pSub->m_iDataType);
    }

    if (pKeyValues != nullptr)
    {
        static IMaterial* pMat = nullptr;

        if (pMat == nullptr)
            pMat = Sig::CreateMaterial_NIGGA(I::iMaterialSystem, "NigaMaterialMaxxing", pKeyValues);

        if (pMat != nullptr)
        {
            BaseEntity* pPreviewModelEntity = F::modelPreview.GetModelEntity();
            if (pPreviewModelEntity != nullptr)
            {
                if (modelState->m_pRenderable == pPreviewModelEntity->GetClientRenderable())
                {
                    Sig::ForcedMaterialOverride_NIGGA(I::iStudioRender, pMat, 0);
                    auto result = Hook::DrawModelExecute::O_DrawModelExecute(pVTable, modelState, renderInfo, boneMatrix);
                    Sig::ForcedMaterialOverride_NIGGA(I::iStudioRender, nullptr, 0);
                    return result;
                }
            }

        }
    }

    return Hook::DrawModelExecute::O_DrawModelExecute(pVTable, modelState, renderInfo, boneMatrix);
}