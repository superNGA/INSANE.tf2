#include <iostream>
#include <chrono>

#include "DrawModelExecute.h"
#include "../../SDK/Entity Manager/entityManager.h"
#include "../../SDK/Class ID Manager/classIDManager.h"
#include "../../SDK/entInfo_t.h"
#include "../../SDK/class/IMaterial.h"
#include "../../SDK/class/Source Entity.h"
#include "../../SDK/class/BaseWeapon.h"

/*
* AMMO PACKS : single texture
* PLAYER     : from 21 upto 31 textures
*/

/*
    -> Some of the ammo packs, large ones specifically ain't rendering like I want them to.
    -> active weapon is also turned to shit, fix that too.
*/

inline void processAmmoPackModel(int nMaterial, IMaterial** ppMaterial)
{
    ppMaterial[0]->SetMaterialVarFlag(MATERIAL_VAR_IGNOREZ, true);
}

inline void processBaseAnimatingModel(int nMaterial, IMaterial** ppMaterial)
{
    ppMaterial[0]->SetMaterialVarFlag(MATERIAL_VAR_IGNOREZ, true);
}

// Brief case Model has 6 textures, not modifiying all of them, can cause
// bullshit & in-proper rendering.
inline void processBriefCaseModel(int nMaterial, IMaterial** ppMaterial)
{
    ppMaterial[0]->SetMaterialVarFlag(MATERIAL_VAR_IGNOREZ, true);
}

hook::DME::T_DME hook::DME::O_DME = nullptr;
int64_t __fastcall hook::DME::H_DME(void* pVTable, DrawModelState_t* modelState, ModelRenderInfo_t* renderInfo, matrix3x4_t* boneMatrix)
{
    BaseEntity* ent  = (BaseEntity*)renderInfo->pRenderable;
    auto id          = IDManager.getID((BaseEntity*)renderInfo->pRenderable);
    
    // Material Count
    int8_t nMaterial = modelState->m_pStudioHWData->m_pLODs->numMaterials;
    if (nMaterial == 0)
        return O_DME(pVTable, modelState, renderInfo, boneMatrix);

    // Material array
    IMaterial** ppMaterial = modelState->m_pStudioHWData->m_pLODs->ppMaterials;

    // Processing models differently according to class ID.
    switch (id)
    {
    case NOT_DEFINED:
        break;
    case PLAYER:
        break;
    case AMMO_PACK:
        processAmmoPackModel(nMaterial, ppMaterial);
        break;

    case DISPENSER:
        processAmmoPackModel(nMaterial, ppMaterial);
        break;
    case SENTRY_GUN:
        processAmmoPackModel(nMaterial, ppMaterial);
        break;
    case TELEPORTER:
        processAmmoPackModel(nMaterial, ppMaterial);
        break;

    // Literally just has the BRIEF-CASE and nothing else.
    case TF_ITEM:
        processBriefCaseModel(nMaterial, ppMaterial);
        break;

    case CAPTURE_POINT:
        break;
    case WEAPON:
        break;
    case PAYLOAD:
        break;
    case CBASEANIMATING:
        processBaseAnimatingModel(nMaterial, ppMaterial);
        break;

    case ENT_RESOURCE_MANAGER:
        break;
    default:
        break;
    }

    return O_DME(pVTable, modelState, renderInfo, boneMatrix);
}