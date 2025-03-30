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
#include "../../SDK/class/IVModelInfo.h"
#include "../../Libraries/Utility/Utility.h"

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

    BaseEntity* pEntity      = static_cast<BaseEntity*>(renderInfo->pRenderable);
    IDclass_t entId          = IDManager.getID(pEntity);

    bool bIsMaterialOverridden = false;

    // remove this and make mechanism
    static IMaterial* customMat = nullptr;
    if (customMat == nullptr)
    {
        KeyValues* kasutaMat = new KeyValues;
        KV_Initialize(kasutaMat);
        customMat = tfObject.pCreateMaterial(tfObject.IMaterialSystem, "KasutaMat", kasutaMat);
        printf("mat pointer : %p\n", customMat);
    }
    
    switch (entId)
    {
    case PLAYER:
        bIsMaterialOverridden = _ChamsPlayer(nMaterial, customMat, ppMaterial, pEntity);
        break;
    case AMMO_PACK:
        bIsMaterialOverridden = _ChamsAmmoPack(nMaterial, customMat, ppMaterial);
        break;
    case DISPENSER:
        pEntity->isEnemy() ?
            bIsMaterialOverridden = _ChamsDispenserEnemy(nMaterial, customMat, ppMaterial) :
            bIsMaterialOverridden = _ChamsDispenserFriendly(nMaterial, customMat, ppMaterial);
        break;
    case SENTRY_GUN:
        pEntity->isEnemy() ?
            bIsMaterialOverridden = _ChamsSenteryEnemy(nMaterial, customMat, ppMaterial, pEntity) :
            bIsMaterialOverridden = _ChamsSenteryFriendly(nMaterial, customMat, ppMaterial, pEntity);
        break;
    case TELEPORTER:
        pEntity->isEnemy() ?
            bIsMaterialOverridden = _ChamsTeleporterEnemy(nMaterial, customMat, ppMaterial) :
            bIsMaterialOverridden = _ChamsTeleporterFriendly(nMaterial, customMat, ppMaterial);
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
        //printf("%d @ %s\n", tfObject.iVModelInfo->GetModelIndex(renderInfo->pModel->strName), tfObject.iVModelInfo->GetModelName(renderInfo->pModel));
        if(_IsAmmoPack(FNV1A32(renderInfo->pModel->strName)))
        {
            bIsMaterialOverridden = _ChamsAnimAmmoPack(nMaterial, customMat, ppMaterial);
        }
        else if (_IsMedKit(FNV1A32(renderInfo->pModel->strName)))
        {
            bIsMaterialOverridden = _ChamsMedKit(nMaterial, customMat, ppMaterial);
        }
        break;    
    case ROCKET:
    case DEMO_PROJECTILES:
        bIsMaterialOverridden = _ChamsProjectiles(nMaterial, customMat, ppMaterial);
        break;
    
    case ID_DROPPED_WEAPON:
        bIsMaterialOverridden = _ChamsAnimAmmoPack(nMaterial, customMat, ppMaterial);
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


bool Chams_t::_ChamsAnimAmmoPack(int8_t nMaterial, IMaterial* pMaterial, IMaterial** ppMaterial)
{   
    switch (config.visualConfig.ignorezAnimAmmoPack + config.visualConfig.bAnimAmmoPack * 2)
    {
    case 0:
        return false;

    case 1:
        for (int8_t matIndex = 0; matIndex < nMaterial && ppMaterial[matIndex] != nullptr; matIndex++)
        {
            ppMaterial[matIndex]->SetMaterialVarFlag(MATERIAL_VAR_IGNOREZ, true);
        }
        return false;

    case 2:
    case 3:
        tfObject.pForcedMaterialOverride(tfObject.IStudioRender, pMaterial, OverrideType_t::OVERRIDE_NORMAL);
        tfObject.iVRenderView->SetColorModulation(&config.visualConfig.clrAnimAmmoPackChams.r);
        tfObject.iVRenderView->SetBlend(config.visualConfig.clrAnimAmmoPackChams.a);
        return true;

    default:
        return false;
    }
}

bool Chams_t::_ChamsMedKit(int8_t nMaterial, IMaterial* pMaterial, IMaterial** ppMaterial)
{
    switch (config.visualConfig.ignorezMedkit + config.visualConfig.bMedkit * 2)
    {
    case 0:
        return false;

    case 1:
        for (int8_t matIndex = 0; matIndex < nMaterial && ppMaterial[matIndex] != nullptr; matIndex++)
        {
            ppMaterial[matIndex]->SetMaterialVarFlag(MATERIAL_VAR_IGNOREZ, true);
        }
        return false;

    case 2:
    case 3:
        tfObject.pForcedMaterialOverride(tfObject.IStudioRender, pMaterial, OverrideType_t::OVERRIDE_NORMAL);
        tfObject.iVRenderView->SetColorModulation(&config.visualConfig.clrMedkit.r);
        tfObject.iVRenderView->SetBlend(config.visualConfig.clrMedkit.a);
        return true;

    default:
        return false;
    }
}

bool Chams_t::_ChamsPlayer(int8_t nMaterial, IMaterial* pMaterial, IMaterial** ppMaterial, BaseEntity* pEntity)
{
    // FRIENDLY PLAYERS CHAMS
    if (entityManager.getLocalPlayer()->getTeamNum() == pEntity->getTeamNum())
    {
        switch (config.visualConfig.ignorezFriendlyPlayer + config.visualConfig.bPlayerChamsFriendly * 2)
        {
        case 0:
            return false;

        case 1:
            for (int8_t matIndex = 0; matIndex < nMaterial && ppMaterial[matIndex] != nullptr; matIndex++)
            {
                ppMaterial[matIndex]->SetMaterialVarFlag(MATERIAL_VAR_IGNOREZ, true);
            }
            return false;

        case 2:
        case 3:
            tfObject.pForcedMaterialOverride(tfObject.IStudioRender, pMaterial, OverrideType_t::OVERRIDE_NORMAL);
            tfObject.iVRenderView->SetColorModulation(&config.visualConfig.clrFriendlyPlayerChams.r);
            tfObject.iVRenderView->SetBlend(config.visualConfig.clrFriendlyPlayerChams.a);
            return true;

        default:
            return false;
        }
    }
    
    // ENEMY CHAMS
    else
    {
        switch (config.visualConfig.ignorezEnemyPlayer + config.visualConfig.bPlayerChamsEnemy * 2)
        {
        case 0:
            return false;

        case 1:
            for (int8_t matIndex = 0; matIndex < nMaterial && ppMaterial[matIndex] != nullptr; matIndex++)
            {
                ppMaterial[matIndex]->SetMaterialVarFlag(MATERIAL_VAR_IGNOREZ, true);
            }
            return false;

        case 2:
        case 3:
            tfObject.pForcedMaterialOverride(tfObject.IStudioRender, pMaterial, OverrideType_t::OVERRIDE_NORMAL);
            tfObject.iVRenderView->SetColorModulation(&config.visualConfig.clrEnemyPlayerCham.r);
            tfObject.iVRenderView->SetBlend(config.visualConfig.clrEnemyPlayerCham.a);
            return true;

        default:
            return false;
        }
    }
}

bool Chams_t::_ChamsDispenserEnemy(int8_t nMaterial, IMaterial* pMaterial, IMaterial** ppMaterial)
{
    switch (config.visualConfig.ignorezDispenserEnemy + config.visualConfig.bDispenserEnemy * 2)
    {
    case 0:
        return false;

    case 1:
        for (int8_t matIndex = 0; matIndex < nMaterial && ppMaterial[matIndex] != nullptr; matIndex++)
        {
            ppMaterial[matIndex]->SetMaterialVarFlag(MATERIAL_VAR_IGNOREZ, true);
        }
        return false;

    case 2:
    case 3:
        tfObject.pForcedMaterialOverride(tfObject.IStudioRender, pMaterial, OverrideType_t::OVERRIDE_NORMAL);
        tfObject.iVRenderView->SetColorModulation(&config.visualConfig.clrDispenserEnemy.r);
        tfObject.iVRenderView->SetBlend(config.visualConfig.clrDispenserEnemy.a);
        return true;

    default:
        return false;
    }
}


bool Chams_t::_ChamsDispenserFriendly(int8_t nMaterial, IMaterial* pMaterial, IMaterial** ppMaterial)
{
    switch (config.visualConfig.ignorezDispenserFirendly + config.visualConfig.bDispenserFirendly * 2)
    {
    case 0:
        return false;

    case 1:
        for (int8_t matIndex = 0; matIndex < nMaterial && ppMaterial[matIndex] != nullptr; matIndex++)
        {
            ppMaterial[matIndex]->SetMaterialVarFlag(MATERIAL_VAR_IGNOREZ, true);
        }
        return false;

    case 2:
    case 3:
        tfObject.pForcedMaterialOverride(tfObject.IStudioRender, pMaterial, OverrideType_t::OVERRIDE_NORMAL);
        tfObject.iVRenderView->SetColorModulation(&config.visualConfig.clrDispenserFriendly.r);
        tfObject.iVRenderView->SetBlend(config.visualConfig.clrDispenserFriendly.a);
        return true;

    default:
        return false;
    }
}


bool Chams_t::_ChamsSenteryEnemy(int8_t nMaterial, IMaterial* pMaterial, IMaterial** ppMaterial, BaseEntity* pEntity)
{
    switch (config.visualConfig.ignorezEnemySentry + config.visualConfig.bSentryEnemy * 2)
    {
    case 0:
        return false;

    case 1:
        for (int8_t matIndex = 0; matIndex < nMaterial && ppMaterial[matIndex] != nullptr; matIndex++)
        {
            ppMaterial[matIndex]->SetMaterialVarFlag(MATERIAL_VAR_IGNOREZ, true);
        }
        return false;

    case 2:
    case 3:
        tfObject.pForcedMaterialOverride(tfObject.IStudioRender, pMaterial, OverrideType_t::OVERRIDE_NORMAL);
        tfObject.iVRenderView->SetColorModulation(&config.visualConfig.clrSentryEnemy.r);
        tfObject.iVRenderView->SetBlend(config.visualConfig.clrSentryEnemy.a);
        return true;

    default:
        return false;
    }
}

bool Chams_t::_ChamsSenteryFriendly(int8_t nMaterial, IMaterial* pMaterial, IMaterial** ppMaterial, BaseEntity* pEntity)
{
    switch (config.visualConfig.ignorezFriendlySentry + config.visualConfig.bSentryFriendly * 2)
    {
    case 0:
        return false;

    case 1:
        for (int8_t matIndex = 0; matIndex < nMaterial && ppMaterial[matIndex] != nullptr; matIndex++)
        {
            ppMaterial[matIndex]->SetMaterialVarFlag(MATERIAL_VAR_IGNOREZ, true);
        }
        return false;

    case 2:
    case 3:
        tfObject.pForcedMaterialOverride(tfObject.IStudioRender, pMaterial, OverrideType_t::OVERRIDE_NORMAL);
        tfObject.iVRenderView->SetColorModulation(&config.visualConfig.clrSentryFriendly.r);
        tfObject.iVRenderView->SetBlend(config.visualConfig.clrSentryFriendly.a);
        return true;

    default:
        return false;
    }
}

bool Chams_t::_ChamsTeleporterEnemy(int8_t nMaterial, IMaterial* pMaterial, IMaterial** ppMaterial)
{
    switch (config.visualConfig.ignorezTeleporterEnemy + config.visualConfig.bTeleporterEnemy *2)
    {
    case 0:
        return false;

    case 1:
        for (int8_t matIndex = 0; matIndex < nMaterial && ppMaterial[matIndex] != nullptr; matIndex++)
        {
            ppMaterial[matIndex]->SetMaterialVarFlag(MATERIAL_VAR_IGNOREZ, true);
        }
        return false;

    case 2:
    case 3:
        tfObject.pForcedMaterialOverride(tfObject.IStudioRender, pMaterial, OverrideType_t::OVERRIDE_NORMAL);
        tfObject.iVRenderView->SetColorModulation(&config.visualConfig.clrTeleporterEnemy.r);
        tfObject.iVRenderView->SetBlend(config.visualConfig.clrTeleporterEnemy.a);
        return true;

    default:
        return false;
    }
}

bool Chams_t::_ChamsTeleporterFriendly(int8_t nMaterial, IMaterial* pMaterial, IMaterial** ppMaterial)
{
    switch (config.visualConfig.ignorezTeleporterFriendly + config.visualConfig.bTeleporterFriendly * 2)
    {
    case 0:
        return false;

    case 1:
        for (int8_t matIndex = 0; matIndex < nMaterial && ppMaterial[matIndex] != nullptr; matIndex++)
        {
            ppMaterial[matIndex]->SetMaterialVarFlag(MATERIAL_VAR_IGNOREZ, true);
        }
        return false;

    case 2:
    case 3:
        tfObject.pForcedMaterialOverride(tfObject.IStudioRender, pMaterial, OverrideType_t::OVERRIDE_NORMAL);
        tfObject.iVRenderView->SetColorModulation(&config.visualConfig.clrTeleporterFriendly.r);
        tfObject.iVRenderView->SetBlend(config.visualConfig.clrTeleporterFriendly.a);
        return true;

    default:
        return false;
    }
}

bool Chams_t::_ChamsItems(int8_t nMaterial, IMaterial* pMaterial, IMaterial** ppMaterial)
{
    switch (config.visualConfig.ignorezTfItem + config.visualConfig.bTfItemChams * 2)
    {
    case 0:
        return false;

    case 1:
        for (int8_t matIndex = 0; matIndex < nMaterial && ppMaterial[matIndex] != nullptr; matIndex++)
        {
            ppMaterial[matIndex]->SetMaterialVarFlag(MATERIAL_VAR_IGNOREZ, true);
        }
        return false;

    case 2:
    case 3:
        tfObject.pForcedMaterialOverride(tfObject.IStudioRender, pMaterial, OverrideType_t::OVERRIDE_NORMAL);
        tfObject.iVRenderView->SetColorModulation(&config.visualConfig.clrTfItemCham.r);
        tfObject.iVRenderView->SetBlend(config.visualConfig.clrTfItemCham.a);
        return true;

    default:
        return false;
    }
}

bool Chams_t::_ChamsAmmoPack(int8_t nMaterial, IMaterial* pMaterial, IMaterial** ppMaterial)
{
    switch (config.visualConfig.ignorezDropAmmoPack + config.visualConfig.bDropAmmoPackChams * 2)
    {
    case 0:
        return false;

    case 1:
        for (int8_t matIndex = 0; matIndex < nMaterial && ppMaterial[matIndex] != nullptr; matIndex++)
        {
            ppMaterial[matIndex]->SetMaterialVarFlag(MATERIAL_VAR_IGNOREZ, true);
        }
        return false;

    case 2:
    case 3:
        tfObject.pForcedMaterialOverride(tfObject.IStudioRender, pMaterial, OverrideType_t::OVERRIDE_NORMAL);
        tfObject.iVRenderView->SetColorModulation(&config.visualConfig.clrDropAmmoPackChams.r);
        tfObject.iVRenderView->SetBlend(config.visualConfig.clrAnimAmmoPackChams.a);
        return true;

    default:
        return false;
    }
}

bool Chams_t::_ChamsProjectiles(int8_t nMaterial, IMaterial* pMaterial, IMaterial** ppMaterial)
{
    switch (config.visualConfig.ignorezProjectiles + config.visualConfig.bProjectileChams * 2)
    {
    case 0:
        return false;

    case 1:
        for (int8_t matIndex = 0; matIndex < nMaterial && ppMaterial[matIndex] != nullptr; matIndex++)
        {
            ppMaterial[matIndex]->SetMaterialVarFlag(MATERIAL_VAR_IGNOREZ, true);
        }
        return false;

    case 2:
    case 3:
        tfObject.pForcedMaterialOverride(tfObject.IStudioRender, pMaterial, OverrideType_t::OVERRIDE_NORMAL);
        tfObject.iVRenderView->SetColorModulation(&config.visualConfig.clrProjectilesChams.r);
        tfObject.iVRenderView->SetBlend(config.visualConfig.clrProjectilesChams.a);
        return true;

    default:
        return false;
    }
}

bool Chams_t::_IsAmmoPack(uint32_t iHash)
{
    switch (iHash)
    {
    case FNV1A32("models/items/ammopack_large.mdl"):
    case FNV1A32("models/items/ammopack_medium.mdl"):
    case FNV1A32("models/items/ammopack_small.mdl"):
        return true;
    default: 
        return false;
    }
}

bool Chams_t::_IsMedKit(uint32_t iHash)
{
    switch (iHash)
    {
    case FNV1A32("models/items/medkit_small.mdl"):
    case FNV1A32("models/items/medkit_medium.mdl"):
    case FNV1A32("models/items/medkit_large.mdl"):
    case FNV1A32("models/props_halloween/halloween_medkit_medium.mdl"):
    case FNV1A32("models/props_halloween/halloween_medkit_small.mdl"):
    case FNV1A32("models/props_halloween/halloween_medkit_large.mdl"):
        return true;
    default:
        return false;
    }
}
