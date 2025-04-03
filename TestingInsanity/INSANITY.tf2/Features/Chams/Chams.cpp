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
        if (_CreateMaterial("FlatMat", szMat01))
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
            bIsMaterialOverridden = _ChamsPlayerEnemy(nMaterial, customMat, ppMaterial, pEntity) :
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

TFclr_t Chams_t::_GetClrFromString(std::string input)
{
    TFclr_t output = { 0, 0, 0, 0 };
    int index = 0;
    int temp = 0;
    bool firstNumOccured = false;
    for (const char x : input)
    {
        if (index >= 4)
            return output;

        if (x - '0' < 0 || x - '0' > 9)
        {
            if (firstNumOccured)
            {
                output.clr[index] = temp;
                printf("inserting %d @ %d | terminating char : %c\n", temp, index, x);
                index++;
                temp = 0;
                firstNumOccured = false;
            }
        }
        else
        {
            firstNumOccured = true;
            temp *= 10;
            temp += x - '0';
        }
    }
    return output;
}

std::string Chams_t::_GetMaterialType(const char* szMaterialVMT)
{
    int32_t len = strlen(szMaterialVMT);
    if (len == 0)
        return "";

    bool bStringStarted = false;
    int iStartIndex = 0;
    int iIndex = 0;
    for (int i = 0; i < len; i++)
    {
        if (szMaterialVMT[i] == ' ')
            continue;

        if (szMaterialVMT[i] == '{' || (bStringStarted == true && (szMaterialVMT[i] == ' ' || szMaterialVMT[i] == '\n')))
        {
            char szOutput[MAX_PROP_NAME];
            strncpy(szOutput, &szMaterialVMT[iStartIndex], iIndex);
            szOutput[iIndex] = '\0';
            return std::string(szOutput);
            break;
        }
        else
        {
            if (bStringStarted == false)
                iStartIndex = i;

            iIndex++;
            bStringStarted = true;
        }
    }

    return "";
}


types_t Chams_t::_GetMatPropDataType(data_t& data, std::string input)
{
    printf("getting input for [ %s ]\n", input.c_str());

    bool hasDot = false;
    float tempData = 0;
    types_t dataDetermined = TYPE_NONE;
    int indexAfterDotCounter = 0;
    for (const char x : input)
    {
        // if string then return as it is.
        if ((x - '0' < 0 || x - '0' > 9) && x != '.')
        {
            if (x == '[' || x == ']' || x == ',')
            {
                data.clrData = _GetClrFromString(input);
                return types_t::TYPE_COLOR;
            }
            return types_t::TYPE_STRING;
        }
        else if (x == '.')
        {
            std::cout << "FLOAT -> [ " << input << " ]\n";
            hasDot = true;
        }
        else
        {
            if (hasDot == true)
            {
                std::cout << "FLOAT -> [ " << input << " ]\n";
                dataDetermined = types_t::TYPE_FLOAT;
                indexAfterDotCounter++;
                tempData += (float)(x - '0') / (float)pow(10, indexAfterDotCounter);
            }
            else
            {
                dataDetermined = types_t::TYPE_INT;
                tempData *= 10.0f;
                tempData += x - '0';
            }
        }
    }

    switch (dataDetermined)
    {
    case TYPE_INT:
        data.iData = tempData;
        return dataDetermined;
    default:
        data.flData = tempData;
        return dataDetermined;
    }
}


bool Chams_t::_GetMaterialPropVector(std::vector<MatProp_t>& vecMatPropOut, const char* szMaterialVMT)
{
    int32_t len = strlen(szMaterialVMT);
    if (len == 0)
        return false;

    vecMatPropOut.clear();

    MatProp_t tempMatProp;
    bool bPropStarted = false;
    bool bDataStarted = false;
    bool bThisIsFloat = false;
    bool bThisIsName = false;
    bool bVmtEnded = false;
    
    // Getting Mateiral type
    std::string szMatType = _GetMaterialType(szMaterialVMT).c_str();
    if (szMatType == "")
    {
        #ifdef _DEBUG
        cons.Log(FG_RED, "DME", "Failed to find material type");
        #endif 
        return false;
    }
    strcpy(tempMatProp.szPropName, szMatType.c_str());
    tempMatProp.dataType = TYPE_MATERIAL_TYPE;
    vecMatPropOut.push_back(tempMatProp);

    // Getting mateiral properties
    int start = 0;
    int propLen = 0;
    std::string temp;
    for (int i = 0; i < len && !bVmtEnded; i++)
    {
        switch (szMaterialVMT[i])
        {
        case '"':
            bPropStarted = !bPropStarted;
            // if new prop just started, clean / prep for data
            if (bPropStarted == true)
            {
                temp.clear();
            }
            else
            {
                // if prop ended and what we stored is a property name then store in the fucking name storage for MatProp_t
                if (bThisIsName == true)
                {
                    strcpy(tempMatProp.szPropName, temp.c_str());
                    printf("prop name added [ %s ]\n", tempMatProp.szPropName);
                }
                else
                {
                    tempMatProp.dataType = _GetMatPropDataType(tempMatProp.data, temp);
                    if (tempMatProp.dataType == types_t::TYPE_STRING)
                        strcpy(tempMatProp.data.szData, temp.c_str());

                    printf("data added of type [ %d ]\n", tempMatProp.dataType);

                    vecMatPropOut.push_back(tempMatProp);
                }
                bThisIsName = false;
                temp.clear(); // onces insearted clear this shit!
            }
            break;
        case '$':
            bThisIsName = true;
            temp.clear(); // clear and put $ sign in front
            temp.push_back('$');
            break;
        default:
            temp.push_back(szMaterialVMT[i]);
            break;
        }
    }

    return true;
}

bool Chams_t::_CreateMaterial(std::string szMatName, const char* szMaterialVMT)
{
    if (szMatName == "" || szMaterialVMT == nullptr)
    {
        #ifdef _DEBUG
        cons.Log(FG_RED, "DME", "Bad material name given [ %s ] & [ %s ]", szMatName.c_str());
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

    // Getting Material properity vector
    std::vector<MatProp_t> vecMaterialVMT;
    vecMaterialVMT.clear();
    _GetMaterialPropVector(vecMaterialVMT, szMaterialVMT);
    if (vecMaterialVMT.empty() == true || vecMaterialVMT[0].dataType != TYPE_MATERIAL_TYPE)
    {
        #ifdef _DEBUG
        cons.Log(FG_RED, "DME", "Failed to convert material VMT string to vector or Couldn't identify material type");
        #endif
        return false;
    }
#ifdef _DEBUG
    else
        cons.Log(FG_GREEN, "DME", "Successfully conveted Mateiral string to vector for [ %s ]. Detected mat type [ %s ]", szMatName.c_str(), vecMaterialVMT[0].szPropName);
#endif

    // Creating Material
    Material_t* newMat = new Material_t;
    newMat->pKV        = new KeyValues;
    KeyValues* pInitializedKV = tfObject.pInitKeyValue(newMat->pKV, vecMaterialVMT[0].szPropName); // 0th index holds the mateiral type

    int nMatPropSize = vecMaterialVMT.size();
    for (int index = 1; index < nMatPropSize; index++)
    {
        MatProp_t& matProp = vecMaterialVMT[index];
        switch (matProp.dataType)
        {
        case TYPE_INT:
            tfObject.pKVSetInt(pInitializedKV, matProp.szPropName, matProp.data.iData);
            break;
        case TYPE_FLOAT:
            tfObject.pKVSetFloat(pInitializedKV, matProp.szPropName, matProp.data.flData);
            break;
        case TYPE_STRING:
            tfObject.pKVSetString(pInitializedKV, matProp.szPropName, matProp.data.szData);
            break;
        case TYPE_COLOR:
            tfObject.pKVSetColor(pInitializedKV, matProp.szPropName, matProp.data.clrData);
            break;
        default:
            break;
        }
    }

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