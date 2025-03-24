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
#include "../../SDK/TF object manager/TFOjectManager.h"

//=========================================================================
//                     PUBLIC METHODS
//=========================================================================

int64_t Chams_t::Run(void* pVTable, DrawModelState_t* modelState, ModelRenderInfo_t* renderInfo, matrix3x4_t* boneMatrix)
{
    int8_t nMaterial        = modelState->m_pStudioHWData->m_pLODs->numMaterials;
    IMaterial** ppMaterial  = modelState->m_pStudioHWData->m_pLODs->ppMaterials;
    if (nMaterial == 0 || ppMaterial == nullptr)
        return hook::DME::O_DME(pVTable, modelState, renderInfo, boneMatrix);

    BaseEntity* entity      = static_cast<BaseEntity*>(renderInfo->pRenderable);
    IDclass_t entId         = IDManager.getID(entity);

    bool bIsMaterialOverridden = false;

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
        bIsMaterialOverridden = _ChamsBaseAnimating(nMaterial, ppMaterial);
        break;
    case ENT_RESOURCE_MANAGER:
        break;
    default:
        break;
    }

    auto result = hook::DME::O_DME(pVTable, modelState, renderInfo, boneMatrix);
    /*if(bIsMaterialOverridden)
        tfObject.pForcedMaterialOverride(tfObject.IVRenderModel, nullptr, OverrideType_t::OVERRIDE_NORMAL);*/
    return result;
}


//=========================================================================
//                     PRIVATE METHODS
//=========================================================================


bool Chams_t::_ChamsBaseAnimating(int8_t nMaterial, IMaterial** ppMaterial)
{
    static IMaterial* customMat = nullptr;
    if (customMat == nullptr)
    {
        printf("mat pointer : %p\n", customMat);
        customMat = tfObject.FindMaterial(tfObject.IMaterialSystem, "debug/debugambientcube", TEXTURE_GROUP_MODEL, true, NULL);
        printf("mat pointer : %p\n", customMat);
        customMat->SetMaterialVarFlag(MATERIAL_VAR_IGNOREZ, true);
        printf("mat pointer : %p\n", customMat);
    }
    tfObject.pForcedMaterialOverride(tfObject.IVRenderModel, customMat, OverrideType_t::OVERRIDE_NORMAL);

    /*for (int8_t matIndex = 0; matIndex < nMaterial && ppMaterial[matIndex] != nullptr; matIndex++)
    {
        ppMaterial[matIndex]->SetMaterialVarFlag(MATERIAL_VAR_IGNOREZ, true);
    }*/

    return true;
}