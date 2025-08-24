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

// SDK
#include "../../SDK/class/BaseEntity.h"
#include "../../Utility/ClassIDHandler/ClassIDHandler.h"
#include "../../SDK/TF object manager/TFOjectManager.h"
#include "../../SDK/Entity Manager/entityManager.h"
#include "../../SDK/class/IVModelInfo.h"
#include "../../SDK/class/IVRenderView.h"
#include "../../SDK/class/IStudioRender.h"
#include "../../SDK/class/IMaterialSystem.h"
#include "../../SDK/class/IVEngineClient.h"

#include "../ImGui/InfoWindow/InfoWindow_t.h"
#include "../Anti Aim/AntiAim.h"

#include "../../Libraries/Timer.h"
#include "../../Extra/math.h"

#include "../../Utility/Signature Handler/signatures.h"
#include "../../Utility/Hook Handler/Hook_t.h"

#define MIN_TIME 0.0001

MAKE_SIG(CreateMaterial, "48 89 5C 24 ? 57 48 83 EC ? 48 8B C2", MATERIALSYSTEM_DLL, IMaterial*, IMaterialSystem*, const char*, KeyValues*)
MAKE_SIG(ForcedMaterialOverride, "48 89 91 ? ? ? ? 44 89 81",    STUDIORENDER_DLL, void, void*, IMaterial*, OverrideType_t)
MAKE_SIG(CMaterialSystem_CacheUsedMateiral, "48 83 EC ? 48 89 5C 24 ? 48 8B D9", MATERIALSYSTEM_DLL, void, IMaterialSystem*)
MAKE_SIG(CMaterialSystem_FindMateiral, "48 83 EC ? 48 8B 44 24 ? 4C 8B 11", MATERIALSYSTEM_DLL, IMaterial*, 
    void*, const char* , const char*, bool , const char*)

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
    // we in-game?
    if (I::iEngine->IsInGame() == false)
        return Hook::DrawModelExecute::O_DrawModelExecute(pVTable, modelState, renderInfo, boneMatrix);
    
    int8_t nMaterial        = modelState->m_pStudioHWData->m_pLODs->numMaterials;
    IMaterial** ppMaterial  = modelState->m_pStudioHWData->m_pLODs->ppMaterials;
    
    // Checking if material data is valid
    if (nMaterial == 0 || ppMaterial == nullptr)
        return Hook::DrawModelExecute::O_DrawModelExecute(pVTable, modelState, renderInfo, boneMatrix);

    BaseEntity* pEntity      = static_cast<BaseEntity*>(renderInfo->pRenderable);
    int         entId        = pEntity->GetClientClass()->m_ClassID;

    // TODO : remove this and make mechanism
    static IMaterial* FlatMat  = nullptr;
    static IMaterial* ShinyMat = nullptr;
    _RefreshMaterials(&FlatMat, &ShinyMat);

    bool bIsMaterialOverridden = false;
    StartTimer();

    switch (entId)
    {
    case /*PLAYER*/1:
        pEntity->IsEnemy() ?
            bIsMaterialOverridden = _ApplyChams(modelState, FlatMat, Features::Chams::Player::Enemy_IgnoreZ.IsActive(), Features::Chams::Player::Enemy_Chams.IsActive(), &Features::Chams::Player::EnemyChams_Color.GetData().r, Features::Chams::Player::EnemyChams_Color.GetData().a) :
            bIsMaterialOverridden = _ApplyChams(modelState, FlatMat, Features::Chams::Player::Friendly_IgnoreZ.IsActive(), Features::Chams::Player::Friendly_Chams.IsActive(), &Features::Chams::Player::FriendlyChams_Color.GetData().r, Features::Chams::Player::FriendlyChams_Color.GetData().a);
        break;
    case /*AMMO_PACK*/2:
        bIsMaterialOverridden     = _ApplyChams(modelState, FlatMat, Features::Chams::Misc::DroppedAmmoPack_IgnoreZ.IsActive(), Features::Chams::Misc::DroppedAmmoPack_Chams.IsActive(), &Features::Chams::Misc::DroppedAmmoPackChams_Color.GetData().r, Features::Chams::Misc::DroppedAmmoPackChams_Color.GetData().a);
        break;
    case /*DISPENSER*/3:
        pEntity->IsEnemy() ?
            bIsMaterialOverridden = _ApplyChams(modelState, FlatMat, Features::Chams::Building::EnemyDispenser_IgnoreZ.IsActive(), Features::Chams::Building::EnemyDispenser_Chams.IsActive(), &Features::Chams::Building::EnemyDispenserChams_Color.GetData().r, Features::Chams::Building::EnemyDispenserChams_Color.GetData().a) :
            bIsMaterialOverridden = _ApplyChams(modelState, FlatMat, Features::Chams::Building::FriendlyDispenser_IgnoreZ.IsActive(), Features::Chams::Building::FriendlyDispenser_Chams.IsActive(), &Features::Chams::Building::FriendlyDispenserChams_Color.GetData().r, Features::Chams::Building::FriendlyDispenserChams_Color.GetData().a);
        break;
    case /*SENTRY_GUN*/4:
        pEntity->IsEnemy() ?
            bIsMaterialOverridden = _ApplyChams(modelState, FlatMat, Features::Chams::Building::EnemySentry_IgnoreZ.IsActive(), Features::Chams::Building::EnemySentry_Chams.IsActive(), &Features::Chams::Building::EnemySentryChams_Color.GetData().r, Features::Chams::Building::EnemySentryChams_Color.GetData().a) :
            bIsMaterialOverridden = _ApplyChams(modelState, FlatMat, Features::Chams::Building::FriendlySentry_IgnoreZ.IsActive(), Features::Chams::Building::FriendlySentry_Chams.IsActive(), &Features::Chams::Building::FriendlySentryChams_Color.GetData().r, Features::Chams::Building::FriendlySentryChams_Color.GetData().a);
        break;
    case /*TELEPORTER*/5:
        pEntity->IsEnemy() ?
            bIsMaterialOverridden = _ApplyChams(modelState, FlatMat, Features::Chams::Building::EnemyTeleporter_IgnoreZ.IsActive(), Features::Chams::Building::EnemyTeleporter_Chams.IsActive(), &Features::Chams::Building::EnemyTeleporterChams_Color.GetData().r, Features::Chams::Building::EnemyTeleporterChams_Color.GetData().a) :
            bIsMaterialOverridden = _ApplyChams(modelState, FlatMat, Features::Chams::Building::FriendlyTeleporter_IgnoreZ.IsActive(), Features::Chams::Building::FriendlyTeleporter_Chams.IsActive(), &Features::Chams::Building::FriendlyTeleporterChams_Color.GetData().r, Features::Chams::Building::FriendlyTeleporterChams_Color.GetData().a);
        break;
    case /*TF_ITEM*/6:
        bIsMaterialOverridden = _ApplyChams(modelState, FlatMat, 
            Features::Chams::Misc:: TFItem_IgnoreZ.IsActive(), 
            Features::Chams::Misc:: TFItem_Chams.IsActive(),
            &Features::Chams::Misc::TFItemChams_Color.GetData().r,
            Features::Chams::Misc:: TFItemChams_Color.GetData().a);
        break;
    //case /*CAPTURE_POINT*/:
    //    break;
    //case WEAPON:
    //    break;
    //case PAYLOAD:
    //    break;
    case /*CBASEANIMATING*/7:
        if(_IsAmmoPack(FNV1A32(renderInfo->pModel->strName)))
        {
            bIsMaterialOverridden = _ApplyChams(modelState, FlatMat, 
                Features::Chams::Misc::AmmoPack_IgnoreZ.IsActive(),
                Features::Chams::Misc::AmmoPack_Chams.IsActive(),
                &Features::Chams::Misc::AmmoPackChams_Color.GetData().r,
                Features::Chams::Misc::AmmoPackChams_Color.GetData().a);
        }
        else if (_IsMedKit(FNV1A32(renderInfo->pModel->strName)))
        {
            bIsMaterialOverridden = _ApplyChams(modelState, FlatMat, 
                Features::Chams::Misc::Medkit_IgnoreZ.IsActive(),
                Features::Chams::Misc::Medkit_Chams.IsActive(),
                &Features::Chams::Misc::MedkitChams_Color.GetData().r,
                Features::Chams::Misc::MedkitChams_Color.GetData().a);
        }
        else
        {
            Hook::DrawModelExecute::O_DrawModelExecute(pVTable, modelState, renderInfo, boneMatrix);
            
            ShinyMat->SetMaterialVarFlag(MATERIAL_VAR_WIREFRAME, true);
            bIsMaterialOverridden = _ApplyChams(modelState, ShinyMat, 
                Features::Chams::Misc:: ViewModel_IgnoreZ.IsActive(),
                Features::Chams::Misc:: ViewModel_Chams.IsActive(),
                &Features::Chams::Misc::ViewModelChams_Color.GetData().r,
                Features::Chams::Misc:: ViewModelChams_Color.GetData().a);
            auto result = Hook::DrawModelExecute::O_DrawModelExecute(pVTable, modelState, renderInfo, boneMatrix);
            ShinyMat->SetMaterialVarFlag(MATERIAL_VAR_WIREFRAME, false);

            Sig::ForcedMaterialOverride(I::iStudioRender, nullptr, OverrideType_t::OVERRIDE_NORMAL);
            return result;
        }
        break;    
    case 8:
    case 9:
        pEntity->IsEnemy() ?
            bIsMaterialOverridden = _ApplyChams(modelState, FlatMat, 
                Features::Chams::Projectile:: EnemyProjectile_IgnoreZ.IsActive(),
                Features::Chams::Projectile:: EnemyProjectile_Chams.IsActive(),
                &Features::Chams::Projectile::EnemyProjectileChams_Color.GetData().r,
                Features::Chams::Projectile:: EnemyProjectileChams_Color.GetData().a) :
            bIsMaterialOverridden = _ApplyChams(modelState, ShinyMat, 
                Features::Chams::Projectile:: FriendlyProjectile_IgnoreZ.IsActive(),
                Features::Chams::Projectile:: FriendlyProjectile_Chams.IsActive(),
                &Features::Chams::Projectile::FriendlyProjectileChams_Color.GetData().r,
                Features::Chams::Projectile:: FriendlyProjectileChams_Color.GetData().a);
        break;
    
    case 89:
        bIsMaterialOverridden = _ApplyChams(modelState, FlatMat,
            Features::Chams::Misc:: DroppedWeapon_IgnoreZ.IsActive(),
            Features::Chams::Misc:: DroppedWeapon_Chams.IsActive(),
            &Features::Chams::Misc::DroppedWeaponChams_Color.GetData().r,
            Features::Chams::Misc:: DroppedWeaponChams_Color.GetData().a);
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
        Render::InfoWindow.AddToInfoWindow("Chams",     std::format("DME exec. time : {:.6f} ms", flAvgTime < MIN_TIME ? 0.0f : flAvgTime));
        Render::InfoWindow.AddToInfoWindow("refCount",  std::format("Refrence count [ FlatMat ] : {}", FlatMat->GetRefrenceCount()));
        nTotalTime = 0.0f;
        nTick = 0;
    }
    nTick++;

    int64_t result = 0;

    // anti aim cham rendering logic, sketchy one, will make proper one, once I see that it is working.
    auto* me = entityManager.GetLocalPlayer();
    if(me != nullptr && pEntity == me->GetClientRenderable())
    {
        if (Features::AntiAim::AntiAim::Cham.IsActive() == true)
        {
            Hook::DrawModelExecute::O_DrawModelExecute(pVTable, modelState, renderInfo, F::antiAim.pBone); // <- fake me
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


void Chams_t::_RefreshMaterials(IMaterial** pMat1, IMaterial** pMat2)
{
    if (m_bMateiralUncached == false)
        return;

    //FreeAllMaterial();
    static int nRefreshCount = 0;

    _CreateMaterial(std::format("FlatMat_{}", nRefreshCount).c_str(), szMat03);
    _CreateMaterial(std::format("ShinyMat_{}", nRefreshCount).c_str(), szMat02);

    *pMat1 = UM_materials.find(std::format("FlatMat_{}", nRefreshCount).c_str())->second->pMaterial;
    (*pMat1)->IncrementReferenceCount(); 
    (*pMat1)->IncrementReferenceCount(); 
    *pMat2 = UM_materials.find(std::format("ShinyMat_{}", nRefreshCount).c_str())->second->pMaterial;
    (*pMat2)->IncrementReferenceCount();
    (*pMat2)->IncrementReferenceCount();

    ++nRefreshCount;

    WIN_LOG("Recreated all materials [ %d ]", nRefreshCount);

    m_bMateiralUncached = false;
}

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
bool Chams_t::_ApplyChams(DrawModelState_t* pModelState, IMaterial* pChamMaterial, bool bIgnoreZ, bool bChams, const float* pChamClrs, const float flAlpha)
{
    int32_t nMaterials = pModelState->m_pStudioHWData->m_pLODs->numMaterials;
    IMaterial** ppMaterial = pModelState->m_pStudioHWData->m_pLODs->ppMaterials;

    switch (bIgnoreZ + bChams * 2)
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
        //I::iVRenderView->SetColorModulation(pChamClrs);
        I::iVRenderView->SetBlend(flAlpha);
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
        FAIL_LOG("DME", "Failed to find material type");
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
        FAIL_LOG("DME", "Bad material name given [ %s ] & [ %s ]", szMatName.c_str());
        #endif 
        return false;
    }

    // returning if already created.
    /*auto it = UM_materials.find(szMatName);
    if (it != UM_materials.end())
    {
        #ifdef _DEBUG
        FAIL_LOG("DME", "Material with name [ %s ] already exists @ [ %p ]", szMatName.c_str(), it->second->pMaterial);
        #endif
        return false;
    }*/

    // Getting Material properity vector
    std::vector<MatProp_t> vecMaterialVMT;
    vecMaterialVMT.clear();
    _GetMaterialPropVector(vecMaterialVMT, szMaterialVMT);
    if (vecMaterialVMT.empty() == true || vecMaterialVMT[0].dataType != TYPE_MATERIAL_TYPE)
    {
        #ifdef _DEBUG
        FAIL_LOG("DME", "Failed to convert material VMT string to vector or Couldn't identify material type");
        #endif
        return false;
    }
    #ifdef _DEBUG
    else
        WIN_LOG("DME", "Successfully conveted Mateiral string to vector for [ %s ]. Detected mat type [ %s ]", szMatName.c_str(), vecMaterialVMT[0].szPropName);
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
    UM_materials.insert({ szMatName, newMat });

    WIN_LOG("DME", "Successfully Created Material [ %s ]", newMat->szMatName);
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
        FAIL_LOG("DME", "Material [ %s ] doesn't exist", szMatName.c_str());
        #endif // _DEBUG
        return false;
    }

    it->second->pMaterial->DecrementReferenceCount();

    //delete it->second->pKV;
    delete it->second;
    #ifdef _DEBUG
    WIN_LOG("DME", "successfully free'ed material [ %s ]", szMatName);
    #endif
    return true;
}

//=========================================================================
//                     DEBUG HOOKS
//=========================================================================
MAKE_HOOK(CMateiralSystem_UncacheAllMateirals, "48 89 5C 24 ? 48 89 74 24 ? 57 48 83 EC ? 48 8B 01 48 8B D9", __fastcall, MATERIALSYSTEM_DLL,
    void*, void* pIMaterialSystem)
{
    FAIL_LOG("Game UnCached all materials :(");

    auto result = Hook::CMateiralSystem_UncacheAllMateirals::O_CMateiralSystem_UncacheAllMateirals(pIMaterialSystem);
    
    chams.m_bMateiralUncached = true;
    
    return result;
}

MAKE_HOOK(CMateiralSystem_UncacheUsedMateirals, "4C 8B DC 41 54 41 57 48 83 EC ? 49 89 5B", __fastcall, MATERIALSYSTEM_DLL,
    void*, void* pIMaterialSystem, char idk1)
{
    FAIL_LOG("Game UnCached \"USED\" materials :(");
    auto result = Hook::CMateiralSystem_UncacheUsedMateirals::O_CMateiralSystem_UncacheUsedMateirals(pIMaterialSystem, idk1);

    chams.m_bMateiralUncached = true;

    return result;
}