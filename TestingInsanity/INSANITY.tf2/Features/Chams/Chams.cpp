//=========================================================================
//                      CHAMS MANAGMENT SYSTEM
//=========================================================================
// by      : INSANE
// created : 24/03/2025
// 
// purpose : Handles chams for all desiered entities. :)
//-------------------------------------------------------------------------
#define _CRT_SECURE_NO_WARNINGS
#include <iostream>
#include "Chams.h"
Chams_t chams;

#include "../../SDK/class/Source Entity.h"
#include "../../SDK/Class ID Manager/classIDManager.h"
#include "../../SDK/TF object manager/TFOjectManager.h"
#include "../../SDK/Entity Manager/entityManager.h"
#include "../../SDK/class/IVModelInfo.h"
#include "../../Libraries/Utility/Utility.h"

#include "../../Libraries/Timer.h"

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
    /*if (customMat == nullptr)
    {
        KeyValues* kasutaMat = new KeyValues;
        KV_Initialize(kasutaMat);
        customMat = tfObject.pCreateMaterial(tfObject.IMaterialSystem, "KasutaMat", kasutaMat);
        printf("mat pointer : %p\n", customMat);
    }*/
    if (customMat == nullptr)
    {
        if (_CreateMaterial("UnlitGeneric", "FlatMat"))
        {
            std::cout << "Created new mat successfully\n";
            customMat = UM_materials["FlatMat"]->pMaterial;
            printf("%p\n", customMat);
        }
    }
    
    StartTimer();

    switch (entId)
    {
    case PLAYER:
        pEntity->isEnemy() ? // Fix this shit nigga, and fuck you bastard. I want this done by today or you dead >:( !!!!
            bIsMaterialOverridden = _ApplyChams(modelState, customMat, config.visualConfig.ignorezEnemyPlayer, config.visualConfig.bPlayerChamsEnemy, config.visualConfig.clrEnemyPlayerCham) :
            bIsMaterialOverridden = _ChamsPlayerFriendly(nMaterial, customMat, ppMaterial, pEntity);
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
        else
        {
            hook::DME::O_DME(pVTable, modelState, renderInfo, boneMatrix);
            customMat->SetMaterialVarFlag(MATERIAL_VAR_WIREFRAME, true);
            _ChamsViewModel(nMaterial, customMat, ppMaterial);
            auto result = hook::DME::O_DME(pVTable, modelState, renderInfo, boneMatrix);
            customMat->SetMaterialVarFlag(MATERIAL_VAR_WIREFRAME, false);
            tfObject.pForcedMaterialOverride(tfObject.IStudioRender, nullptr, OverrideType_t::OVERRIDE_NORMAL);
            return result;
        }
        break;    
    case ROCKET:
    case DEMO_PROJECTILES:
        pEntity->isEnemy() ?
            bIsMaterialOverridden = _ChamsProjectilesEnemy(nMaterial, customMat, ppMaterial) :
            bIsMaterialOverridden = _ChamsProjectilesFriendly(nMaterial, customMat, ppMaterial);
        break;
    
    case ID_DROPPED_WEAPON:
        bIsMaterialOverridden = _ChamsAnimAmmoPack(nMaterial, customMat, ppMaterial);
        break;
    default:
        break;
    }

    static int nTick = 0;
    static float nTotalTime = 0.0f;
    nTotalTime += EndTimer();
    if(nTick >= 64)
    {
        Timer::flDMETimeInMs.store(nTotalTime/64.0f);
        nTotalTime = 0.0f;
        nTick = 0;
    }
    nTick++;

    auto result = hook::DME::O_DME(pVTable, modelState, renderInfo, boneMatrix);
    if (bIsMaterialOverridden)
    {
        tfObject.pForcedMaterialOverride(tfObject.IStudioRender, nullptr, OverrideType_t::OVERRIDE_NORMAL);
    }

    return result;
}

bool Chams_t::FreeAllMaterial()
{
    for (auto& iterator : UM_materials)
    {
        delete iterator.second->pKV;
        #ifdef _DEBUG
        cons.Log(FG_GREEN, "DME", "successfully free'ed material [ %s ]", iterator.second->szMatName);
        #endif
        delete iterator.second;
    }
    cons.Log(FG_GREEN, "DME", "FREE'ED ALL MATERIALS");
    return true;
}


//=========================================================================
//                     PRIVATE METHODS
//=========================================================================


bool Chams_t::_ApplyChams(DrawModelState_t* pModelState, IMaterial* pChamMaterial, bool bIgnoreZConfig, bool bChamToggleConfig, clr_t& clrCham)
{
    int32_t nMaterials     = pModelState->m_pStudioHWData->m_pLODs->numMaterials;
    IMaterial** ppMaterial = pModelState->m_pStudioHWData->m_pLODs->ppMaterials;

    switch (bIgnoreZConfig + bChamToggleConfig * 2)
    {
    case 0:
        return false;
    case 1:
        for (int8_t matIndex = 0; matIndex < nMaterials && ppMaterial[matIndex] != nullptr; matIndex++)
        {
            ppMaterial[matIndex]->SetMaterialVarFlag(MATERIAL_VAR_IGNOREZ, true);
        }
        return false;
    case 2 :
    case 3 : 
        tfObject.pForcedMaterialOverride(tfObject.IStudioRender, pChamMaterial, OverrideType_t::OVERRIDE_NORMAL);
        tfObject.iVRenderView->SetColorModulation(&clrCham.r);
        tfObject.iVRenderView->SetBlend(clrCham.a);
        return true;

    default:
        return false;
    }
}

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

bool Chams_t::_ChamsPlayerEnemy(int8_t nMaterial, IMaterial* pMaterial, IMaterial** ppMaterial, BaseEntity* pEntity)
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


bool Chams_t::_ChamsPlayerFriendly(int8_t nMaterial, IMaterial* pMaterial, IMaterial** ppMaterial, BaseEntity* pEntity)
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

bool Chams_t::_ChamsViewModel(int8_t nMaterial, IMaterial* pMaterial, IMaterial** ppMaterial)
{
    switch ((config.visualConfig.ignorezViewModel == true) + (config.visualConfig.bViewModelChams == true) * 2)
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
        tfObject.iVRenderView->SetColorModulation(&config.visualConfig.clrViewModelChams.r);
        tfObject.iVRenderView->SetBlend(config.visualConfig.clrViewModelChams.a);
        return true;

    default:
        return false;
    }
}

bool Chams_t::_ChamsProjectilesEnemy(int8_t nMaterial, IMaterial* pMaterial, IMaterial** ppMaterial)
{
    switch (config.visualConfig.ignorezProjectilesEnemy + config.visualConfig.bProjectileEnemy * 2)
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
        tfObject.iVRenderView->SetColorModulation(&config.visualConfig.clrProjectileEnemy.r);
        tfObject.iVRenderView->SetBlend(config.visualConfig.clrProjectileEnemy.a);
        return true;

    default:
        return false;
    }
}

bool Chams_t::_ChamsProjectilesFriendly(int8_t nMaterial, IMaterial* pMaterial, IMaterial** ppMaterial)
{
    switch (config.visualConfig.ignorezProjectileFriendly + config.visualConfig.bProjectileFriendly * 2)
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
        tfObject.iVRenderView->SetColorModulation(&config.visualConfig.clrProjectileFirendly.r);
        tfObject.iVRenderView->SetBlend(config.visualConfig.clrProjectileFirendly.a);
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

bool Chams_t::_CreateMaterial(const char* pBaseMaterialType, std::string szMatName)
{
    if (pBaseMaterialType == nullptr || szMatName == "")
    {
        #ifdef _DEBUG
        cons.Log(FG_RED, "DME", "Bad material name given [ %s ] & [ %s ]", pBaseMaterialType, szMatName.c_str());
        #endif 
        return false;
    }

    // returning if already created.
    auto it = UM_materials.find(szMatName);
    if (it != UM_materials.end())
    {
        #ifdef _DEBUG
        cons.Log(FG_RED, "DME", "Material with name [ %s ] already exists @ [ %p ]", szMatName.c_str(), it->second->pMaterial);
        #endif
        return false;
    }

    // Creating Material
    Material_t* newMat = new Material_t;
    newMat->pKV        = new KeyValues;
    static TFclr_t red = { 255, 0, 0, 255 };
    static TFclr_t white = { 255, 255, 255, 255 };
    static TFclr_t cyan = { 0, 255, 255, 255 };
    KeyValues* pInitializedKV = tfObject.pInitKeyValue(newMat->pKV, pBaseMaterialType);
    tfObject.pKVSetInt(pInitializedKV, "$ignorez", 1);

    // Metallic Chams
    //tfObject.pKVSetString(pInitializedKV, "$envmap", "env_cubemap");
    //tfObject.pKVSetColor(pInitializedKV, "$envmaptint", red);
    //tfObject.pKVSetInt(pInitializedKV, "$envmapfresnel", 1);
    //tfObject.pKVSetInt(pInitializedKV, "$phong", 1);
    //tfObject.pKVSetInt(pInitializedKV, "phongexponent", 20);
    //tfObject.pKVSetInt(pInitializedKV, "phongboost", 2);
    //tfObject.pKVSetInt(pInitializedKV, "$ignorez", 1);
    //tfObject.pKVSetInt(pInitializedKV, "$wireframe", 0);
    
    // filling up material object
    newMat->pMaterial = tfObject.pCreateMaterial(tfObject.IMaterialSystem, szMatName.c_str(), newMat->pKV);
    strncpy(newMat->szMatName, szMatName.c_str(), MAX_MATERIAL_NAME_SIZE-1);
    newMat->szMatName[MAX_MATERIAL_NAME_SIZE - 1] = '\0';
    UM_materials[szMatName] = newMat;
    #ifdef _DEBUG
    cons.Log(FG_GREEN, "DME", "Successfully Created Material [ %s ]", newMat->szMatName);
    #endif 
    return true;
}

bool Chams_t::_DeleteMaterial(std::string szMatName)
{
    auto it = UM_materials.find(szMatName);
    if (it == UM_materials.end())
    {
        #ifdef _DEBUG
        cons.Log(FG_RED, "DME", "Material [ %s ] doesn't exist", szMatName.c_str());
        #endif // _DEBUG
        return false;
    }

    delete it->second->pKV;
    delete it->second;
    #ifdef _DEBUG
    cons.Log(FG_GREEN, "DME", "successfully free'ed material [ %s ]", szMatName);
    #endif
    return true;
}