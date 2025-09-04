// Utility
#include "../Utility/Hook Handler/Hook_t.h"
#include "../Utility/ConsoleLogging.h"
#include "../Utility/ClassIDHandler/ClassIDHandler.h"

// SDK
#include "../SDK/class/IMaterialSystem.h"
#include "../SDK/class/IStudioRender.h"
#include "../SDK/class/IMaterial.h"
#include "../SDK/class/BaseEntity.h"

#include "../Features/Chams/ChamsV2.h"
#include "../Features/ModelPreview/ModelPreview.h"
#include "../Features/Material Gen/MaterialGen.h"
#include "../Features/Entity Iterator/EntityIterator.h"
#include "EndScene/EndScene.h"


MAKE_HOOK(DrawModelExecute, "4C 89 4C 24 ? 48 89 4C 24 ? 55 53 56 57 41 54", __fastcall, ENGINE_DLL, int64_t, 
    void* pVTable, DrawModelState_t* modelState, ModelRenderInfo_t* renderInfo, matrix3x4_t* boneMatrix)
{
    // Don't do material altering at unstable states, else causes crahes.
    int64_t result = Hook::DrawModelExecute::O_DrawModelExecute(pVTable, modelState, renderInfo, boneMatrix);
    if (directX::UI::UI_has_been_shutdown == true)
        return result;

    F::chamsV2.Run(pVTable, modelState, renderInfo, boneMatrix, Hook::DrawModelExecute::O_DrawModelExecute);

    // No model entity yet.
    BaseEntity* pModelEnt = F::modelPreview.GetModelEntity();
    if(pModelEnt == nullptr)
        return result;


    if (pModelEnt->GetClientRenderable() == modelState->m_pRenderable)
    {
        std::vector<Material_t*>* pMaterials = F::materialGen.GetModelMaterials();
        
        // Early exit if no material selected.
        if(pMaterials == nullptr)
            return result;

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


    // drawing backtrack info
    auto* records = F::entityIterator.GetBackTrackRecord(modelState->m_pRenderable->GetBaseEntFromRenderable());
    if (records != nullptr && records->empty() == false)
    {
        Hook::DrawModelExecute::O_DrawModelExecute(pVTable, modelState, renderInfo, records->front().m_bones);
    }


    return result;
}