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
#include "../Features/Material Gen/MaterialGen.h"
#include "EndScene/EndScene.h"


MAKE_HOOK(DrawModelExecute, "4C 89 4C 24 ? 48 89 4C 24 ? 55 53 56 57 41 54", __fastcall, ENGINE_DLL, int64_t, 
    void* pVTable, DrawModelState_t* modelState, ModelRenderInfo_t* renderInfo, matrix3x4_t* boneMatrix)
{
    // Don't do material altering at unstable states, else causes crahes.
    if(directX::UI::UI_has_been_shutdown == true)
        return Hook::DrawModelExecute::O_DrawModelExecute(pVTable, modelState, renderInfo, boneMatrix);


    // No model entity yet.
    BaseEntity* pModelEnt = F::modelPreview.GetModelEntity();
    if(pModelEnt == nullptr)
        return Hook::DrawModelExecute::O_DrawModelExecute(pVTable, modelState, renderInfo, boneMatrix);


    if (pModelEnt == modelState->m_pRenderable)
    {
        std::vector<Material_t*>* pMaterials = F::materialGen.GetModelMaterials();
        
        // Early exit if no material selected.
        if(pMaterials == nullptr)
            return Hook::DrawModelExecute::O_DrawModelExecute(pVTable, modelState, renderInfo, boneMatrix);

        // Draw once as a fail safe. ( we also need this result so we can return if after wards. )
        auto result = Hook::DrawModelExecute::O_DrawModelExecute(pVTable, modelState, renderInfo, boneMatrix);

        // Now draw one material at a time from our MaterialBundle.
        for(int iMatIndex = 0; iMatIndex < pMaterials->size(); iMatIndex++)
        {
            IMaterial* pMat = (*pMaterials)[iMatIndex]->m_pMaterial;
            if (pMat == nullptr)
                continue;

            // Override materials.
            I::iStudioRender->ForcedMaterialOverride(pMat, OverrideType_t::OVERRIDE_NORMAL);

            // Draw model once with this material.
            Hook::DrawModelExecute::O_DrawModelExecute(pVTable, modelState, renderInfo, boneMatrix);
        }

        // Reset material override & return result.
        I::iStudioRender->ForcedMaterialOverride(nullptr, OverrideType_t::OVERRIDE_NORMAL);
        return result;
    }


    return Hook::DrawModelExecute::O_DrawModelExecute(pVTable, modelState, renderInfo, boneMatrix);
}