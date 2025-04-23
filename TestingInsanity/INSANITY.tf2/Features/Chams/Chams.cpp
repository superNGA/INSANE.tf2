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

#include "../../Hooks/DrawModelExecutes.h"

#include "../../SDK/class/Source Entity.h"
#include "../../SDK/Class ID Manager/classIDManager.h"
#include "../../SDK/TF object manager/TFOjectManager.h"
#include "../../SDK/Entity Manager/entityManager.h"
#include "../../SDK/class/IVModelInfo.h"
#include "../../SDK/class/IVRenderView.h"
#include "../../SDK/class/IStudioRender.h"
#include "../../SDK/class/IMaterialSystem.h"

#include "../../Libraries/Utility/Utility.h"
#include "../ImGui/InfoWindow/InfoWindow_t.h"
#include "../Anti Aim/AntiAim.h"

#include "../../Utility/ConsoleLogging.h"
#include "../../Libraries/Timer.h"
#include "../../Extra/math.h"

#include "../../Utility/signatures.h"
#include "../../Utility/ConsoleLogging.h"

#define MIN_TIME 0.0001

MAKE_SIG(CreateMaterial, "48 89 5C 24 ? 57 48 83 EC ? 48 8B C2", MATERIALSYSTEM_DLL, IMaterial*, void*, const char*, KeyValues*)
MAKE_SIG(ForcedMaterialOverride, "48 89 91 ? ? ? ? 44 89 81",    STUDIORENDER_DLL, void, void*, IMaterial*, OverrideType_t)

MAKE_SIG(InitKeyValue,  "40 53 48 83 EC ? 48 8B D9 C7 01",      MATERIALSYSTEM_DLL, KeyValues*, void*, const char*)
MAKE_SIG(KVSetInt,      "40 53 48 83 EC ? 41 8B D8 41 B0",      MATERIALSYSTEM_DLL, void, KeyValues*, const char*, int64_t)
MAKE_SIG(KVSetFloat,    "48 83 EC ? 0F 29 74 24 ? 41 B0",       MATERIALSYSTEM_DLL, void, KeyValues*, const char*, float)
MAKE_SIG(KVSetString,   "48 89 5C 24 ? 55 48 83 EC ? 49 8B D8", MATERIALSYSTEM_DLL, void, KeyValues*, const char*, const char*)
MAKE_SIG(KVSetColor,    "44 89 44 24 ? 53 48 83 EC ? 41 8B D8", CLIENT_DLL,         void, KeyValues*, const char*, TFclr_t) // <- this shit in client.dll, not in MaterialSystem.dll

//=========================================================================
//                     PUBLIC METHODS
//=========================================================================
int64_t Chams_t::Run(void* pVTable, DrawModelState_t* modelState, ModelRenderInfo_t* renderInfo, matrix3x4_t* boneMatrix)
{
    // Checking if cheat's backEnd is initialized
    if(I::iEngine->IsInGame() == false)
        Hook::DrawModelExecute::O_DrawModelExecute(pVTable, modelState, renderInfo, boneMatrix);
    /*if (tfObject.bIsInitialized.load() == false)
    {
        WAIT_MSG("TFObject Manager", "initialize");
        return Hook::DrawModelExecute::O_DrawModelExecute(pVTable, modelState, renderInfo, boneMatrix);
    }*/

    int8_t nMaterial        = modelState->m_pStudioHWData->m_pLODs->numMaterials;
    IMaterial** ppMaterial  = modelState->m_pStudioHWData->m_pLODs->ppMaterials;
    
    // Checking if material data is valid
    if (nMaterial == 0 || ppMaterial == nullptr)
        return Hook::DrawModelExecute::O_DrawModelExecute(pVTable, modelState, renderInfo, boneMatrix);

    BaseEntity* pEntity      = static_cast<BaseEntity*>(renderInfo->pRenderable);
    IDclass_t entId          = IDManager.getID(pEntity);

    bool bIsMaterialOverridden = false;

    // remove this and make mechanism
    static IMaterial* FlatMat = nullptr;
    static IMaterial* ShinyMat = nullptr;
    if (FlatMat == nullptr)
    {
        if (_CreateMaterial("FlatMat", szMat01))
        {
            FlatMat = UM_materials["FlatMat"]->pMaterial;
        }
        else
        {
            printf("Failed mat creatoin\n");
        }
    }
    if (ShinyMat == nullptr)
    {
        if (_CreateMaterial("ShinyMat", szMat02))
        {
            ShinyMat = UM_materials["ShinyMat"]->pMaterial;
        }
        else
        {
            printf("Failed mat creatoin\n");
        }
    }
    
    StartTimer();

    switch (entId)
    {
    case PLAYER:
        pEntity->isEnemy() ?
            bIsMaterialOverridden = _ApplyChams(modelState, FlatMat, config.visualConfig.ChamEnemyPlayer) :
            bIsMaterialOverridden = _ApplyChams(modelState, FlatMat, config.visualConfig.ChamFriendlyPlayer);
        break;
    case AMMO_PACK:
        bIsMaterialOverridden     = _ApplyChams(modelState, FlatMat, config.visualConfig.ChamDroppedAmmoPack);
        break;
    case DISPENSER:
        pEntity->isEnemy() ?
            bIsMaterialOverridden = _ApplyChams(modelState, FlatMat, config.visualConfig.ChamEnemyDispenser) :
            bIsMaterialOverridden = _ApplyChams(modelState, FlatMat, config.visualConfig.ChamFriendlyDispenser);
        break;
    case SENTRY_GUN:
        pEntity->isEnemy() ?
            bIsMaterialOverridden = _ApplyChams(modelState, FlatMat, config.visualConfig.ChamEnemySentry) :
            bIsMaterialOverridden = _ApplyChams(modelState, FlatMat, config.visualConfig.ChamFriendlySentry);
        break;
    case TELEPORTER:
        pEntity->isEnemy() ?
            bIsMaterialOverridden = _ApplyChams(modelState, FlatMat, config.visualConfig.ChamEnemyTeleporter) :
            bIsMaterialOverridden = _ApplyChams(modelState, FlatMat, config.visualConfig.ChamFriendlyTeleporter);
        break;
    case TF_ITEM:
        bIsMaterialOverridden = _ApplyChams(modelState, FlatMat, config.visualConfig.ChamTFItem);
        break;
    case CAPTURE_POINT:
        break;
    case WEAPON:
        break;
    case PAYLOAD:
        break;
    case CBASEANIMATING:
        if(_IsAmmoPack(FNV1A32(renderInfo->pModel->strName)))
        {
            bIsMaterialOverridden = _ApplyChams(modelState, FlatMat, config.visualConfig.ChamAnimAmmoPack);
        }
        else if (_IsMedKit(FNV1A32(renderInfo->pModel->strName)))
        {
            bIsMaterialOverridden = _ApplyChams(modelState, FlatMat, config.visualConfig.ChamMedkit);
        }
        else
        {
            Hook::DrawModelExecute::O_DrawModelExecute(pVTable, modelState, renderInfo, boneMatrix);
            
            ShinyMat->SetMaterialVarFlag(MATERIAL_VAR_WIREFRAME, true);
            bIsMaterialOverridden = _ApplyChams(modelState, ShinyMat, config.visualConfig.ChamViewModel);
            auto result = Hook::DrawModelExecute::O_DrawModelExecute(pVTable, modelState, renderInfo, boneMatrix);
            ShinyMat->SetMaterialVarFlag(MATERIAL_VAR_WIREFRAME, false);

            Sig::ForcedMaterialOverride(I::iStudioRender, nullptr, OverrideType_t::OVERRIDE_NORMAL);
            return result;
        }
        break;    
    case ROCKET:
    case DEMO_PROJECTILES:
        pEntity->isEnemy() ?
            bIsMaterialOverridden = _ApplyChams(modelState, FlatMat, config.visualConfig.ChamEnemyProjectile) :
            bIsMaterialOverridden = _ApplyChams(modelState, ShinyMat, config.visualConfig.ChamFriendlyProjectile);
        break;
    
    case ID_DROPPED_WEAPON:
        bIsMaterialOverridden = _ApplyChams(modelState, FlatMat, config.visualConfig.ChamDroppedWeapon);
        break;
    default:
        break;
    }

    static int nTick = 0;
    static float nTotalTime = 0.0f;
    nTotalTime += EndTimer();
    if(nTick >= 64)
    {
        float flAvgTime = nTotalTime / 64.0f;
        Render::InfoWindow.AddToInfoWindow("Chams", std::format("DME exec. time : {:.6f} ms", flAvgTime < MIN_TIME ? 0.0f : flAvgTime));
        nTotalTime = 0.0f;
        nTick = 0;
    }
    nTick++;

    int64_t result = 0;

    // anti aim cham rendering logic, sketchy one, will make proper one, once I see that it is working.
    auto* me = entityManager.getLocalPlayer();
    if(me != nullptr && pEntity == me->GetClientRenderable())
    {
        if (Feature::AA_chams == true)
        {
            Hook::DrawModelExecute::O_DrawModelExecute(pVTable, modelState, renderInfo, Features::antiAim.pBone); // <- fake me
        }
        Sig::ForcedMaterialOverride(I::iStudioRender, nullptr, OverrideType_t::OVERRIDE_NORMAL);
        result = Hook::DrawModelExecute::O_DrawModelExecute(pVTable, modelState, renderInfo, boneMatrix); // <- real me

        return result;
    }
    else
    {
        result = Hook::DrawModelExecute::O_DrawModelExecute(pVTable, modelState, renderInfo, boneMatrix);
    }

    if (bIsMaterialOverridden)
    {
        Sig::ForcedMaterialOverride(I::iStudioRender, nullptr, OverrideType_t::OVERRIDE_NORMAL);
    }

    return result;
}


//=========================================================================
// bool Chams_t::FreeAllMaterial()
//=========================================================================
/**
* Deletes all stored mateirals
**************************************************************************/
bool Chams_t::FreeAllMaterial()
{
    for (auto& iterator : UM_materials)
    {
        if (_DeleteMaterial(iterator.first) == false)
            FAIL_LOG("FAILED TO DELETE MATERIAL [ %s ]", iterator.first.c_str());
        else
            WIN_LOG("successfully free'ed material [ %s ]", iterator.second->szMatName);
    }
    WIN_LOG("FREE'ED ALL MATERIALS");
    return true;
}


//=========================================================================
//                     PRIVATE METHODS
//=========================================================================


//=========================================================================
// bool Chams_t::_ApplyChams(DrawModelState_t* pModelState, IMaterial* pChamMaterial, ChamSetting_t& pChamConfig)
//=========================================================================
/**
* applies cham to current model using given Cham setings
*
* @param pModelState : poitner to model state object, to get number of mateiral & array of mat.
* @param pChamMaterial : pointer to a properly initialized IMateral.
* @param pChamConfig : reference to cham settings
**************************************************************************/
bool Chams_t::_ApplyChams(DrawModelState_t* pModelState, IMaterial* pChamMaterial, ChamSetting_t& pChamConfig)
{
    int32_t nMaterials = pModelState->m_pStudioHWData->m_pLODs->numMaterials;
    IMaterial** ppMaterial = pModelState->m_pStudioHWData->m_pLODs->ppMaterials;

    switch (pChamConfig.bIgnorez + pChamConfig.bChams * 2)
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
        Sig::ForcedMaterialOverride(I::iStudioRender, pChamMaterial, OverrideType_t::OVERRIDE_NORMAL);
        I::iVRenderView->SetColorModulation(&pChamConfig.clrChams.r);
        I::iVRenderView->SetBlend(pChamConfig.clrChams.a);
        return true;

    default:
        return false;
    }
}


//=========================================================================
// bool Chams_t::_IsAmmoPack(uint32_t iHash)
//=========================================================================
/**
* is this model a animating ammo pack?
*
* @param iHash : FNV1A32 hash output of model name
**************************************************************************/
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


//=========================================================================
// bool Chams_t::_IsMedKit(uint32_t iHash)
//=========================================================================
/**
* is this model a medkit?
*
* @param iHash : FNV1A32 hash output of model name
**************************************************************************/
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


//=========================================================================
// TFclr_t Chams_t::_GetClrFromString(std::string input)
//=========================================================================
/**
* Extracts color from given string.
*
* @param szColorString : string contaning color
**************************************************************************/
TFclr_t Chams_t::_GetClrFromString(std::string szColorString)
{
    TFclr_t clrOutput = { 0, 0, 0, 0 };
    int iIndex = 0;
    int iTemp = 0;
    bool bNumStarted = false;
    for (const char x : szColorString)
    {
        if (iIndex >= 4)
            return clrOutput;

        if (x - '0' < 0 || x - '0' > 9)
        {
            if (bNumStarted)
            {
                clrOutput.clr[iIndex] = iTemp;
                iIndex++;
                iTemp = 0;
                bNumStarted = false;
            }
        }
        else
        {
            bNumStarted = true;
            iTemp *= 10;
            iTemp += x - '0';
        }
    }
    return clrOutput;
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


//=========================================================================
// types_t Chams_t::_GetMatPropDataType(data_t& dataOut, std::string szData)
//=========================================================================
/**
* gets material base type ( like UnlitGenerix , VertexLitGenerix )
*
* @param dataOut : data stored here after calculation
* @param szData : input
* NOTE : don't put base type in quotes (") can cause bullshit.
**************************************************************************/
types_t Chams_t::_GetMatPropDataType(data_t& dataOut, std::string szData)
{
    bool bContainsBot = false;
    float flTempData = 0;
    types_t determinedDataType = TYPE_NONE;
    int iIndexPostDecimal = 0;
    for (const char x : szData)
    {
        // if string then return as it is.
        if ((x - '0' < 0 || x - '0' > 9) && x != '.')
        {
            if (x == '[' || x == ']' || x == ',')
            {
                dataOut.clrData = _GetClrFromString(szData);
                return types_t::TYPE_COLOR;
            }
            return types_t::TYPE_STRING;
        }
        else if (x == '.')
        {
            bContainsBot = true;
        }
        else
        {
            if (bContainsBot == true)
            {
                determinedDataType = types_t::TYPE_FLOAT;
                iIndexPostDecimal++;
                flTempData += (float)(x - '0') / (float)pow(10, iIndexPostDecimal);
            }
            else
            {
                determinedDataType = types_t::TYPE_INT;
                flTempData *= 10.0f;
                flTempData += x - '0';
            }
        }
    }

    switch (determinedDataType)
    {
    case TYPE_INT:
        dataOut.iData = flTempData;
        return determinedDataType;
    default:
        dataOut.flData = flTempData;
        return determinedDataType;
    }
}


//=========================================================================
// bool Chams_t::_GetMaterialPropVector(std::vector<MatProp_t>& vecMatPropOut, const char* szMaterialVMT)
//=========================================================================
/**
* converts mateiral VMT string to vector
* First entry in output vector is base type.
*
* @param vecMatPropOut : output vector
* @param szMaterialVMT : material string in VMT format
**************************************************************************/
bool Chams_t::_GetMaterialPropVector(std::vector<MatProp_t>& vecMatPropOut, const char* szMaterialVMT)
{
    int32_t len = strlen(szMaterialVMT);
    if (len == 0)
        return false;

    vecMatPropOut.clear();

    MatProp_t tempMaterialProperty;
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
    strcpy(tempMaterialProperty.szPropName, szMatType.c_str());
    tempMaterialProperty.dataType = TYPE_MATERIAL_TYPE;
    vecMatPropOut.push_back(tempMaterialProperty);

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
                    strcpy(tempMaterialProperty.szPropName, temp.c_str());
                }
                else
                {
                    tempMaterialProperty.dataType = _GetMatPropDataType(tempMaterialProperty.data, temp);
                    if (tempMaterialProperty.dataType == types_t::TYPE_STRING)
                        strcpy(tempMaterialProperty.data.szData, temp.c_str());

                    vecMatPropOut.push_back(tempMaterialProperty);
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


//=========================================================================
// bool Chams_t::_CreateMaterial(std::string szMatName, const char* szMaterialVMT)
//=========================================================================
/**
* Creates a material from given material string ( in VMT format )
*
* @param szMatName : Name of material ( for storing )
* @param szMaterialVMT : mat string in VMT format
**************************************************************************/
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
    KeyValues* pInitializedKV = Sig::InitKeyValue(newMat->pKV, vecMaterialVMT[0].szPropName); // 0th index holds the mateiral type

    int nMatPropSize = vecMaterialVMT.size();
    for (int index = 1; index < nMatPropSize; index++)
    {
        MatProp_t& matProp = vecMaterialVMT[index];
        switch (matProp.dataType)
        {
        case TYPE_INT:
            Sig::KVSetInt(pInitializedKV, matProp.szPropName, matProp.data.iData);
            break;
        case TYPE_FLOAT:
            Sig::KVSetFloat(pInitializedKV, matProp.szPropName, matProp.data.flData);
            break;
        case TYPE_STRING:
            Sig::KVSetString(pInitializedKV, matProp.szPropName, matProp.data.szData);
            break;
        case TYPE_COLOR:
            Sig::KVSetColor(pInitializedKV, matProp.szPropName, matProp.data.clrData);
            break;
        default:
            break;
        }
    }

    // filling up material object
    newMat->pMaterial = Sig::CreateMaterial(I::iMaterialSystem, szMatName.c_str(), newMat->pKV);
    strncpy(newMat->szMatName, szMatName.c_str(), MAX_MATERIAL_NAME_SIZE-1);
    newMat->szMatName[MAX_MATERIAL_NAME_SIZE - 1] = '\0';
    UM_materials[szMatName] = newMat;
    #ifdef _DEBUG
    cons.Log(FG_GREEN, "DME", "Successfully Created Material [ %s ]", newMat->szMatName);
    #endif 
    return true;
}


//=========================================================================
// bool Chams_t::_DeleteMaterial(std::string szMatName)
//=========================================================================
/**
* Deletes stored material from given name. TRUE -> deleted successfully
* FALSE : failed to find or material doesn't exist.
*
* @param szMatName : name of material used while creating it.
**************************************************************************/
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

    it->second->pMaterial->DecrementReferenceCount();
    //it->second->pMaterial->DeleteIfUnreferenced();

    delete it->second->pKV;
    delete it->second;
    #ifdef _DEBUG
    cons.Log(FG_GREEN, "DME", "successfully free'ed material [ %s ]", szMatName);
    #endif
    return true;
}