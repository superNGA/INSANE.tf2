//=========================================================================
//                      CHAMS MANAGMENT SYSTEM
//=========================================================================
// by      : INSANE
// created : 24/03/2025
// 
// purpose : Handles chams for all desiered entities. :)
//-------------------------------------------------------------------------
#include <iostream>
#include "Chams.h"
Chams_t chams;
#include "../../SDK/class/Source Entity.h"
#include "../../SDK/Class ID Manager/classIDManager.h"

//=========================================================================
//                     PUBLIC METHODS
//=========================================================================

int64_t Chams_t::Run(void* pVTable, DrawModelState_t* modelState, ModelRenderInfo_t* renderInfo, matrix3x4_t* boneMatrix)
{
    BaseEntity* entity      = reinterpret_cast<BaseEntity*>(renderInfo->pRenderable);
    IDclass_t entId         = IDManager.getID(entity);
    
    int8_t nMaterial        = modelState->m_pStudioHWData->m_pLODs->numMaterials;
    IMaterial** ppMaterial  = modelState->m_pStudioHWData->m_pLODs->ppMaterials;

    if (nMaterial == 0 || ppMaterial == nullptr)
        return hook::DME::O_DME(pVTable, modelState, renderInfo, boneMatrix);

    switch (entId)
    {
    case NOT_DEFINED:
        break;
    case PLAYER:
        break;
    case AMMO_PACK:
        break;
    case DISPENSER:
        break;
    case SENTRY_GUN:
        break;
    case TELEPORTER:
        break;
    case TF_ITEM:
        break;
    case CAPTURE_POINT:
        break;
    case WEAPON:
        break;
    case PAYLOAD:
        break;
    case CBASEANIMATING:
        printf("%s\n", renderInfo->pModel->strName);
        //_ChamsBaseAnimating(nMaterial, ppMaterial);
        break;
    case ENT_RESOURCE_MANAGER:
        break;
    default:
        break;
    }

    auto result = hook::DME::O_DME(pVTable, modelState, renderInfo, boneMatrix);
    return result;
}


//=========================================================================
//                     PRIVATE METHODS
//=========================================================================


void Chams_t::_ChamsBaseAnimating(int8_t nMaterial, IMaterial** ppMaterial)
{
    for (int8_t matIndex = 0; matIndex < nMaterial && ppMaterial[matIndex] != nullptr; matIndex++)
    {
        ppMaterial[matIndex]->SetMaterialVarFlag(MATERIAL_VAR_IGNOREZ, true);
    }
}