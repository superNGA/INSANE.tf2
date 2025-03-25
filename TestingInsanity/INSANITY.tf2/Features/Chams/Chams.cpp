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
    // Checking if cheat backEnd is initialized
    if (tfObject.bIsInitialized.load() == false || tfObject.FindMaterial == nullptr)
    {
        WAIT_MSG("TFObject Manager", "initialize");
        return hook::DME::O_DME(pVTable, modelState, renderInfo, boneMatrix);
    }

    int8_t nMaterial        = modelState->m_pStudioHWData->m_pLODs->numMaterials;
    IMaterial** ppMaterial  = modelState->m_pStudioHWData->m_pLODs->ppMaterials;
    
    // Checking if material data is valid
    if (nMaterial == 0 || ppMaterial == nullptr)
        return hook::DME::O_DME(pVTable, modelState, renderInfo, boneMatrix);

    BaseEntity* entity      = static_cast<BaseEntity*>(renderInfo->pRenderable);
    IDclass_t entId         = IDManager.getID(entity);

    bool bIsMaterialOverridden = false;

    // remove this and make mechanism
    static IMaterial* customMat = nullptr;
    if (customMat != nullptr)
    {
        customMat = tfObject.FindMaterial(tfObject.IMaterialSystem, "custom/customMat", nullptr, true, NULL);
        customMat->SetMaterialVarFlag(MATERIAL_VAR_IGNOREZ, true);

        printf("mat pointer : %p\n", customMat);
    }

    switch (entId)
    {
    case NOT_DEFINED:
        break;
    case PLAYER:
        //bIsMaterialOverridden = _ChamsBaseAnimating(nMaterial, customMat);
        break;
    case AMMO_PACK:
        bIsMaterialOverridden = _ChamsBaseAnimating(nMaterial, customMat, ppMaterial);
        break;
    case DISPENSER:
        bIsMaterialOverridden = _ChamsBaseAnimating(nMaterial, customMat, ppMaterial);
        break;
    case SENTRY_GUN:
        bIsMaterialOverridden = _ChamsBaseAnimating(nMaterial, customMat, ppMaterial);
        break;
    case TELEPORTER:
        bIsMaterialOverridden = _ChamsBaseAnimating(nMaterial, customMat, ppMaterial);
        break;
    case TF_ITEM:
        bIsMaterialOverridden = _ChamsBaseAnimating(nMaterial, customMat, ppMaterial);
        break;
    case CAPTURE_POINT:
        break;
    case WEAPON:
        break;
    case PAYLOAD:
        break;
    case CBASEANIMATING:
        bIsMaterialOverridden = _ChamsBaseAnimating(nMaterial, customMat, ppMaterial);
        break;
    case ENT_RESOURCE_MANAGER:
        break;
    default:
        break;
    }

    auto result = hook::DME::O_DME(pVTable, modelState, renderInfo, boneMatrix);
    if (bIsMaterialOverridden)
    {
        tfObject.pForcedMaterialOverride(tfObject.IStudioRender, nullptr, OverrideType_t::OVERRIDE_NORMAL);
    }

    return result;
}


//=========================================================================
//                     PRIVATE METHODS
//=========================================================================


bool Chams_t::_ChamsBaseAnimating(int8_t nMaterial, IMaterial* pMaterial, IMaterial** ppMaterial)
{    
    //tfObject.pForcedMaterialOverride(tfObject.IStudioRender, pMaterial, OverrideType_t::OVERRIDE_NORMAL);

    for (int8_t matIndex = 0; matIndex < nMaterial && ppMaterial[matIndex] != nullptr; matIndex++)
    {
        ppMaterial[matIndex]->SetMaterialVarFlag(MATERIAL_VAR_IGNOREZ, true);
    }

    return false;
}