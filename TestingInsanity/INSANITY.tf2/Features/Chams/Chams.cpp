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
#include "../../SDK/Entity Manager/entityManager.h"

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
    if (customMat == nullptr)
    {
        customMat = tfObject.FindMaterial(tfObject.IMaterialSystem, "chamed.vmt", nullptr, true, NULL);
        customMat->IncrementReferenceCount();
        customMat->SetMaterialVarFlag(MATERIAL_VAR_IGNOREZ, true);

        printf("mat pointer : %p\n", customMat);
    }

    switch (entId)
    {
    case NOT_DEFINED:
        break;
    case PLAYER:
        // do a little bit of error handling here or maybe in the function
        bIsMaterialOverridden = _ChamsPlayer(nMaterial, customMat, ppMaterial, entity);
        break;
    case AMMO_PACK:
        bIsMaterialOverridden = _ChamsAmmoPack(nMaterial, customMat, ppMaterial);
        break;
    case DISPENSER:
        bIsMaterialOverridden = _ChamsDispenser(nMaterial, customMat, ppMaterial);
        break;
    case SENTRY_GUN:
        bIsMaterialOverridden = _ChamsSentery(nMaterial, customMat, ppMaterial);
        break;
    case TELEPORTER:
        bIsMaterialOverridden = _ChamsTeleporter(nMaterial, customMat, ppMaterial);
        break;
    case TF_ITEM:
        bIsMaterialOverridden = _ChamsItems(nMaterial, customMat, ppMaterial);
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
    if (config.visualConfig.baseAnimating == false)
        return false;

    tfObject.pForcedMaterialOverride(tfObject.IStudioRender, pMaterial, OverrideType_t::OVERRIDE_NORMAL);

    tfObject.iVRenderView->SetColorModulation(&config.visualConfig.clrBaseAnimatingCham.r);
    tfObject.iVRenderView->SetBlend(config.visualConfig.clrBaseAnimatingCham.a);

    /*for (int8_t matIndex = 0; matIndex < nMaterial && ppMaterial[matIndex] != nullptr; matIndex++)
    {
        ppMaterial[matIndex]->SetMaterialVarFlag(MATERIAL_VAR_IGNOREZ, true);
    }*/

    return true;
}

bool Chams_t::_ChamsPlayer(int8_t nMaterial, IMaterial* pMaterial, IMaterial** ppMaterial, BaseEntity* pEntity)
{
    if (config.visualConfig.playerChams == false)
        return false;

    if (entityManager.getLocalPlayer() == nullptr)
        return false;

    tfObject.pForcedMaterialOverride(tfObject.IStudioRender, pMaterial, OverrideType_t::OVERRIDE_NORMAL);
    
    if(pEntity->getTeamNum() != entityManager.getLocalPlayer()->getTeamNum())
        tfObject.iVRenderView->SetColorModulation(&config.visualConfig.clrEnemyPlayerCham.r);
    else
        tfObject.iVRenderView->SetColorModulation(&config.visualConfig.clrSentryCham.r);
    tfObject.iVRenderView->SetBlend(config.visualConfig.clrFriendlyPlayerCham.a);
    return true;
}

bool Chams_t::_ChamsDispenser(int8_t nMaterial, IMaterial* pMaterial, IMaterial** ppMaterial)
{
    if (config.visualConfig.dispenserChams == false)
        return false;

    tfObject.pForcedMaterialOverride(tfObject.IStudioRender, pMaterial, OverrideType_t::OVERRIDE_NORMAL);

    tfObject.iVRenderView->SetColorModulation(&config.visualConfig.clrDispenserCham.r);
    tfObject.iVRenderView->SetBlend(config.visualConfig.clrDispenserCham.a);
    return true;
}

bool Chams_t::_ChamsSentery(int8_t nMaterial, IMaterial* pMaterial, IMaterial** ppMaterial)
{
    if (config.visualConfig.sentryChams == false)
        return false;

    tfObject.pForcedMaterialOverride(tfObject.IStudioRender, pMaterial, OverrideType_t::OVERRIDE_NORMAL);

    tfObject.iVRenderView->SetColorModulation(&config.visualConfig.clrSentryCham.r);
    tfObject.iVRenderView->SetBlend(config.visualConfig.clrSentryCham.a);
    return true;
}

bool Chams_t::_ChamsTeleporter(int8_t nMaterial, IMaterial* pMaterial, IMaterial** ppMaterial)
{
    if (config.visualConfig.teleporterChams == false)
        return false;

    tfObject.pForcedMaterialOverride(tfObject.IStudioRender, pMaterial, OverrideType_t::OVERRIDE_NORMAL);

    tfObject.iVRenderView->SetColorModulation(&config.visualConfig.clrTeleporterCham.r);
    tfObject.iVRenderView->SetBlend(config.visualConfig.clrTeleporterCham.a);
    return true;
}

bool Chams_t::_ChamsItems(int8_t nMaterial, IMaterial* pMaterial, IMaterial** ppMaterial)
{
    if (config.visualConfig.tfItemChams == false)
        return false;

    tfObject.pForcedMaterialOverride(tfObject.IStudioRender, pMaterial, OverrideType_t::OVERRIDE_NORMAL);

    tfObject.iVRenderView->SetColorModulation(&config.visualConfig.clrTfItemCham.r);
    tfObject.iVRenderView->SetBlend(config.visualConfig.clrTfItemCham.a);
    return true;
}

bool Chams_t::_ChamsAmmoPack(int8_t nMaterial, IMaterial* pMaterial, IMaterial** ppMaterial)
{
    if (config.visualConfig.ammoPackChams == false)
        return false;

    tfObject.pForcedMaterialOverride(tfObject.IStudioRender, pMaterial, OverrideType_t::OVERRIDE_NORMAL);

    tfObject.iVRenderView->SetColorModulation(&config.visualConfig.clrAmmoPackCham.r);
    tfObject.iVRenderView->SetBlend(config.visualConfig.clrAmmoPackCham.a);
    return true;
}
