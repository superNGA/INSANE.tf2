#include "DMEHandler.h"

// SDK
#include "../../SDK/class/BaseEntity.h"
#include "../../SDK/class/IVEngineClient.h"
#include "../../SDK/class/IMaterial.h"
#include "../../SDK/class/IStudioRender.h"
#include "../../SDK/class/CMultAnimState.h"

// Other
#include "../../Extra/math.h"
#include "../TickManip/TickManipHelper.h"
#include "../TickManip/AntiAim/AntiAimV2.h"
#include "../Entity Iterator/EntityIterator.h"
#include "../Material Gen/MaterialGen.h"
#include "../ModelPreview/ModelPreview.h"


MAKE_SIG(CBaseAnimating_InvalidateBoneCache2, "8B 05 ? ? ? ? FF C8 C7 81", CLIENT_DLL, int64_t, void*)

// Draw model execute's template
typedef int64_t(__fastcall* T_DME)(void*, DrawModelState_t*, ModelRenderInfo_t*, matrix3x4_t*);


///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
int64_t DMEHandler_t::Run(void* pVTable, DrawModelState_t* modelState, ModelRenderInfo_t* renderInfo, matrix3x4_t* boneMatrix, void* pOriginalDME)
{
    _SetupMatDropDowns();

    if (F::materialGen.GetMaterialList().empty() == true)
        return reinterpret_cast<T_DME>(pOriginalDME)(pVTable, modelState, renderInfo, boneMatrix);
    
    
    _HandleModelPreview(pVTable, modelState, renderInfo, boneMatrix, pOriginalDME);


    // No local player, no cool materials.
    BaseEntity* pLocalPlayer = I::IClientEntityList->GetClientEntity(I::iEngine->GetLocalPlayer());
    if (pLocalPlayer == nullptr)
    {
        return reinterpret_cast<T_DME>(pOriginalDME)(pVTable, modelState, renderInfo, boneMatrix);
    }


    BaseEntity* pEnt = modelState->m_pRenderable->GetBaseEntFromRenderable();
    if (pEnt == pLocalPlayer)
    {
        _HandleLocalPlayer(pVTable, modelState, renderInfo, &boneMatrix, pOriginalDME);
    }
    

    int nDrawCalls = 0; int64_t iResult = _ApplyChams(nDrawCalls, pVTable, modelState, renderInfo, boneMatrix, pOriginalDME);
    if (nDrawCalls > 0)
        return iResult;

    return reinterpret_cast<T_DME>(pOriginalDME)(pVTable, modelState, renderInfo, boneMatrix);
}


///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
int64_t DMEHandler_t::_ApplyChams(int& nDrawCalls, void* pVTable, DrawModelState_t* modelState, ModelRenderInfo_t* renderInfo, matrix3x4_t* boneMatrix, void* pOriginalDME)
{
    BaseEntity* pLocalPlayer       = I::IClientEntityList->GetClientEntity(I::iEngine->GetLocalPlayer());
    BaseEntity* pEnt               = renderInfo->pRenderable->GetBaseEntFromRenderable();
    int         iFriendlyTeam      = pLocalPlayer->m_iTeamNum();
    int         iClassID           = pEnt->GetClientClass()->m_ClassID;
    const auto& mapJumpTableHelper = F::entityIterator.GetJumpTableHelper();
    int         iWishMaterialIndex = -1;


    // We need to use recaculated bone matrix for local player while using antiaim custom real angles.
    boneMatrix = (F::tickManipHelper.UseCustomBonesLocalPlayer() == true && pEnt == pLocalPlayer) ? F::tickManipHelper.GetRealAngleBones() : boneMatrix;


    auto it = mapJumpTableHelper.find(iClassID);
    if (it == mapJumpTableHelper.end())
        return -1LL;


    // Choosing which material to use to draw.
    switch (it->second)
    {

    case EntityIterator_t::ClassIdIndex::CTFPlayer:
    {
        iWishMaterialIndex =
            pEnt->m_iTeamNum() == iFriendlyTeam ?
            Features::Materials::Player::Player_TeamMates.GetData() : Features::Materials::Player::Player_Enemy.GetData();

        if (pEnt->m_iTeamNum() != iFriendlyTeam)
            _DrawBackTrack(pVTable, modelState, renderInfo, pOriginalDME, pEnt);

        break;
    }

    case EntityIterator_t::ClassIdIndex::CObjectSentrygun:
    {
        iWishMaterialIndex =
            pEnt->m_iTeamNum() == iFriendlyTeam ?
            Features::Materials::Sentry::Sentry_TeamMates.GetData() : Features::Materials::Sentry::Sentry_Enemy.GetData();
        break;
    }

    case EntityIterator_t::ClassIdIndex::CObjectTeleporter:
    {
        iWishMaterialIndex =
            pEnt->m_iTeamNum() == iFriendlyTeam ?
            Features::Materials::Teleporter::Teleporter_TeamMates.GetData() : Features::Materials::Teleporter::Teleporter_Enemy.GetData();
        break;
    }

    case EntityIterator_t::ClassIdIndex::CObjectDispenser:
    {
        iWishMaterialIndex =
            pEnt->m_iTeamNum() == iFriendlyTeam ?
            Features::Materials::Dispenser::Dispenser_TeamMates.GetData() : Features::Materials::Dispenser::Dispenser_Enemy.GetData();
        break;
    }

    default: break;
    }


    // UI's drop down's index to actual material index.
    iWishMaterialIndex = _GetMatIndexFromUIIndex(iWishMaterialIndex);


    // Does this entity has a custom material set to it via the player list ( left panel ).
    int iCustomMatOverride = F::entityIterator.GetEntityMaterial(pEnt);
    if (iCustomMatOverride >= 0)
    {
        iWishMaterialIndex = iCustomMatOverride;
    }


    // No material for this model.
    if (iWishMaterialIndex < 0)
        return -1LL;


    // Finally, draw this model using desired material list.
    const std::vector<Material_t*>& vecMaterials = F::materialGen.GetMaterialList()[iWishMaterialIndex].m_vecMaterials;
    nDrawCalls                                   = vecMaterials.size();
    return _DrawModelWithMatList(&vecMaterials, pVTable, modelState, renderInfo, boneMatrix, pOriginalDME);
}


///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
int64_t DMEHandler_t::_HandleModelPreview(void* pVTable, DrawModelState_t* modelState, ModelRenderInfo_t* renderInfo, matrix3x4_t* boneMatrix, void* pOriginalDME)
{
    BaseEntity* pEnt = renderInfo->pRenderable->GetBaseEntFromRenderable();
    if (pEnt != F::modelPreview.GetModelEntity())
        return 0LL;


    const std::vector<Material_t*>* pVecMaterials = F::materialGen.GetModelMaterials();
    if (pVecMaterials == nullptr)
        return 0LL;


    return _DrawModelWithMatList(pVecMaterials, pVTable, modelState, renderInfo, boneMatrix, pOriginalDME);
}

bool bShit = false;
///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
int64_t DMEHandler_t::_HandleLocalPlayer(void* pVTable, DrawModelState_t* modelState, ModelRenderInfo_t* renderInfo, matrix3x4_t** ppBoneMatrix, void* pOriginalDME)
{
    assert(ppBoneMatrix != nullptr && *ppBoneMatrix != nullptr && "Bad bone matrix for local player  ->  " __FUNCTION__);
    if (ppBoneMatrix == nullptr || *ppBoneMatrix == nullptr)
        return 0LL;


    // In case using legit antiaim, use manually created bones.
    if (F::tickManipHelper.UseCustomBonesLocalPlayer() == true)
    {
        //*ppBoneMatrix = F::tickManipHelper.GetRealAngleBones();
    }


    // User doesn't want to draw second model?
    if (F::tickManipHelper.ShouldDrawSecondModel() == false)
        return 1LL;


    // material index.
    int iMatIndex = _GetMatIndexFromUIIndex(Features::Materials::Player::Player_AntiAim.GetData());
    if (iMatIndex < 0)
    {
        // if no mat still draw once.
        return reinterpret_cast<T_DME>(pOriginalDME)(pVTable, modelState, renderInfo, F::tickManipHelper.GetFakeAngleBones());
    }

    
    // draw using materials of choice.
    const std::vector<Material_t*>& vecMaterials = F::materialGen.GetMaterialList()[iMatIndex].m_vecMaterials;
    return _DrawModelWithMatList(&vecMaterials, pVTable, modelState, renderInfo, F::tickManipHelper.GetFakeAngleBones(), pOriginalDME);
}


///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
int DMEHandler_t::_GetMatIndexFromUIIndex(int iMatUIIndex)
{
    // NOTE : the index in the drop down widget has "off" matrial for 0th index,
    // and 1st real material from the matrial gen as the index 1st. So we do a 
    // -= 1 and do some sanity / safety checks n shit so we don't crash.

    const std::vector<MaterialBundle_t>& vecMaterialBundles = F::materialGen.GetMaterialList();


    // No Material in material gen. Should never happen, we have 2 default materials.
    assert(vecMaterialBundles.size() > 0LLU && "Material bundle is 0 fucking materials.  ->  " __FUNCTION__);
    if (vecMaterialBundles.size() == 0LLU)
        return -1;


    // To compensate for the 0th "off" material.
    iMatUIIndex -= 1;


    // Out of index material?
    int iMatIndexClamped = std::clamp<int>(iMatUIIndex, 0LLU, vecMaterialBundles.size());
    if (iMatIndexClamped != iMatUIIndex)
        return -1;


    return iMatIndexClamped;
}


///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
int64_t DMEHandler_t::_DrawModelWithMatList(const std::vector<Material_t*>* pVecMaterials, void* pVTable, DrawModelState_t* modelState, ModelRenderInfo_t* renderInfo, matrix3x4_t* boneMatrix, void* pOriginalDME)
{
    int64_t iResult = 0LL;

    assert(pVecMaterials != nullptr && "nullptr mateiral vector.  ->  " __FUNCTION__);
    if (pVecMaterials == nullptr)
        return iResult;


    for (const Material_t* pMat : *pVecMaterials)
    {
        assert(pMat != nullptr && pMat->m_pMaterial != nullptr && "Bad material in material list.  ->  " __FUNCTION__);

        if (pMat == nullptr || pMat->m_pMaterial == nullptr)
            break;

        // Override materials.
        I::iStudioRender->ForcedMaterialOverride(pMat->m_pMaterial, OverrideType_t::OVERRIDE_NORMAL);

        // DrawModelExecute.
        iResult = reinterpret_cast<T_DME>(pOriginalDME)(pVTable, modelState, renderInfo, boneMatrix);
    }


    // Clear residue, else material will leak.
    I::iStudioRender->ForcedMaterialOverride(nullptr, OverrideType_t::OVERRIDE_NORMAL);

    return iResult;
}


///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
void DMEHandler_t::_DrawBackTrack(void* pVTable, DrawModelState_t* modelState, ModelRenderInfo_t* renderInfo, void* pOriginalDME, BaseEntity* pEnt)
{
    // -1 -> Not selected | 0 -> None |  1-> Last record only | 2 -> all Records
    if (Features::BackTrack::BackTrack::BackTrack_Cham_Setting.GetData() < 1)
        return;

    // No backtrack ?
    if (F::entityIterator.GetBackTrackTimeInSec() == 0.0f)
        return;

    std::deque<BackTrackRecord_t>* records = F::entityIterator.GetBackTrackRecord(pEnt);
    if (records == nullptr)
        return;

    if (records->size() <= 0)
        return;
    
    // NOTE: A valid backtrack record deque with atleast one entry is promised from this point onwards.


    int iWishMaterialIndex = _GetMatIndexFromUIIndex(Features::BackTrack::BackTrack::BackTrack_Cham.GetData());
    if (iWishMaterialIndex < 0) // no material set ?
        return;

    iWishMaterialIndex = std::clamp<int>(iWishMaterialIndex, 0, F::materialGen.GetMaterialList().size() - 1);
    int nTicks         = TIME_TO_TICK(F::entityIterator.GetBackTrackTimeInSec());

    if (Features::BackTrack::BackTrack::BackTrack_Cham_Setting.GetData() == 1)
    {
        int                iRecordIndex = std::clamp<int>(nTicks, 0, records->size() - 1);
        BackTrackRecord_t& record       = (*records)[iRecordIndex];

        const std::vector<Material_t*>& vecMaterials = F::materialGen.GetMaterialList()[iWishMaterialIndex].m_vecMaterials;

        if(vecMaterials.size() > 0LL)
        {
            _DrawModelWithMatList(&vecMaterials, pVTable, modelState, renderInfo, record.m_bones, pOriginalDME);
        }
        else
        {
            reinterpret_cast<T_DME>(pOriginalDME)(pVTable, modelState, renderInfo, record.m_bones);
        }
    }
    else
    {
        int iStartTick = std::clamp<int>(nTicks - TIME_TO_TICK(MAX_BACKTRACK_TIME), 0, records->size() - 1);
        for (int iTick = iStartTick; iTick < nTicks; iTick++)
        {
            if (iTick >= records->size() || iTick < 0)
                return;

            BackTrackRecord_t& record = (*records)[iTick];

            const std::vector<Material_t*>& vecMaterials = F::materialGen.GetMaterialList()[iWishMaterialIndex].m_vecMaterials;

            if (vecMaterials.size() > 0LL)
            {
                _DrawModelWithMatList(&vecMaterials, pVTable, modelState, renderInfo, record.m_bones, pOriginalDME);
            }
            else
            {
                reinterpret_cast<T_DME>(pOriginalDME)(pVTable, modelState, renderInfo, record.m_bones);
            }
        }
    }
}


///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
void DMEHandler_t::_DrawAAModel(void* pVTable, DrawModelState_t* modelState, ModelRenderInfo_t* renderInfo, void* pOriginalDME)
{
    if(F::tickManipHelper.ShouldDrawSecondModel() == false)
        return;

    BaseEntity* pLocalPlayer = I::IClientEntityList->GetClientEntity(I::iEngine->GetLocalPlayer());
    BaseEntity* pEnt         = renderInfo->pRenderable->GetBaseEntFromRenderable();

    if (pEnt != pLocalPlayer)
        return;


    // Get user's mateiral choice.
    int iMatIndex = Features::Materials::Player::Player_AntiAim.GetData() - 1;
    if (iMatIndex < 0) // if don't want a material override, then draw once & leave.
    {
        reinterpret_cast<T_DME>(pOriginalDME)(pVTable, modelState, renderInfo, F::tickManipHelper.GetFakeAngleBones());
        return;
    }
    

    iMatIndex = std::clamp<int>(iMatIndex, 0, F::materialGen.GetMaterialList().size() - 1);
    const std::vector<Material_t*>& vecMaterials = F::materialGen.GetMaterialList()[iMatIndex].m_vecMaterials;


    for(Material_t* pMat : vecMaterials)
    {
        if (pMat == nullptr || pMat->m_pMaterial == nullptr)
            continue;


        I::iStudioRender->ForcedMaterialOverride(pMat->m_pMaterial, OverrideType_t::OVERRIDE_NORMAL);
        reinterpret_cast<T_DME>(pOriginalDME)(pVTable, modelState, renderInfo, F::tickManipHelper.GetFakeAngleBones());
    }


    I::iStudioRender->ForcedMaterialOverride(nullptr, OverrideType_t::OVERRIDE_NORMAL);
}


///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
void DMEHandler_t::_SetupMatDropDowns()
{
    // TODO : Maybe when a notification UI is in place, push a notification to alert of this limit.
    constexpr int MAX_MATERIALS = 128;
    static const char* szMaterialList[MAX_MATERIALS]; // No ones making more than 128 fucking mateirals. This is more than enough.


    // Constructing Material list. ( all mateiral that we have in mat gen. )
    const std::vector<MaterialBundle_t>& vecMatList = F::materialGen.GetMaterialList();

    int nMaterials = Maths::MIN<int>(vecMatList.size(), MAX_MATERIALS);
    szMaterialList[0] = "None";
    for (int iMatIndex = 1; iMatIndex <= nMaterials; iMatIndex++)
    {
        szMaterialList[iMatIndex] = vecMatList[iMatIndex - 1].m_szMatBundleName.c_str();
    }


    // Finally setting list.
    Features::Materials::Player::Player_Enemy.SetItems            (szMaterialList, nMaterials + 1);
    Features::Materials::Player::Player_TeamMates.SetItems        (szMaterialList, nMaterials + 1);
    Features::Materials::Player::Player_AntiAim.SetItems          (szMaterialList, nMaterials + 1);
    Features::Materials::Sentry::Sentry_Enemy.SetItems            (szMaterialList, nMaterials + 1);
    Features::Materials::Sentry::Sentry_TeamMates.SetItems        (szMaterialList, nMaterials + 1);
    Features::Materials::Dispenser::Dispenser_Enemy.SetItems      (szMaterialList, nMaterials + 1);
    Features::Materials::Dispenser::Dispenser_TeamMates.SetItems  (szMaterialList, nMaterials + 1);
    Features::Materials::Teleporter::Teleporter_Enemy.SetItems    (szMaterialList, nMaterials + 1);
    Features::Materials::Teleporter::Teleporter_TeamMates.SetItems(szMaterialList, nMaterials + 1);
    Features::BackTrack::BackTrack::BackTrack_Cham.SetItems       (szMaterialList, nMaterials + 1);
}
