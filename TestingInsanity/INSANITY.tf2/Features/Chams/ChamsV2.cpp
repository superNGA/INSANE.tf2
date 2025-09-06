#include "ChamsV2.h"

// SDK
#include "../../SDK/class/BaseEntity.h"
#include "../../SDK/class/IVEngineClient.h"
#include "../../SDK/class/IMaterial.h"
#include "../../SDK/class/IStudioRender.h"

// Other
#include "../Entity Iterator/EntityIterator.h"
#include "../Material Gen/MaterialGen.h"


// Draw model execute's template
typedef int64_t(__fastcall* T_DME)(void*, DrawModelState_t*, ModelRenderInfo_t*, matrix3x4_t*);


///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
void ChamsV2_t::Run(void* pVTable, DrawModelState_t* modelState, ModelRenderInfo_t* renderInfo, matrix3x4_t* boneMatrix, void* pOriginalDME)
{
    _SetupMatDropDowns();

    if (F::materialGen.GetMaterialList().empty() == true)
        return;
    
    BaseEntity* pLocalPlayer = I::IClientEntityList->GetClientEntity(I::iEngine->GetLocalPlayer());
    if (pLocalPlayer == nullptr)
        return;

    int         iFriendlyTeam      = pLocalPlayer->m_iTeamNum();
    BaseEntity* pEnt               = modelState->m_pRenderable->GetBaseEntFromRenderable();
    int         iClassID           = pEnt->GetClientClass()->m_ClassID;
    const auto& mapJumpTableHelper = F::entityIterator.GetJumpTableHelper();
    int         iWishMaterialIndex = -1;

    if (mapJumpTableHelper.empty() == true)
        return;

    auto it = mapJumpTableHelper.find(iClassID);
    if (it == mapJumpTableHelper.end())
        return;

    // Choosing which material to use to draw.
    switch (it->second)
    {
    case EntityIterator_t::ClassIdIndex::CTFPlayer:
    {
        iWishMaterialIndex =
            pEnt->m_iTeamNum() == iFriendlyTeam ?
            Features::Materials::Materials::Player_TeamMates.GetData() : Features::Materials::Materials::Player_Enemy.GetData();

        if (pEnt->m_iTeamNum() != iFriendlyTeam)
            _DrawBackTrack(pVTable, modelState, renderInfo, pOriginalDME, pEnt);

        break;
    }
    case EntityIterator_t::ClassIdIndex::CObjectSentrygun:
        iWishMaterialIndex = 
            pEnt->m_iTeamNum() == iFriendlyTeam ?
            Features::Materials::Materials::Sentry_TeamMates.GetData() : Features::Materials::Materials::Sentry_Enemy.GetData();
        break;
    case EntityIterator_t::ClassIdIndex::CObjectTeleporter:
        iWishMaterialIndex = 
            pEnt->m_iTeamNum() == iFriendlyTeam ?
            Features::Materials::Materials::Teleporter_TeamMates.GetData() : Features::Materials::Materials::Teleporter_Enemy.GetData();
        break;
    case EntityIterator_t::ClassIdIndex::CObjectDispenser:
        iWishMaterialIndex = 
            pEnt->m_iTeamNum() == iFriendlyTeam ?
            Features::Materials::Materials::Dispenser_TeamMates.GetData() : Features::Materials::Materials::Dispenser_Enemy.GetData();
        break;
    
    default: break;
    }

    iWishMaterialIndex -= 1;

    // First draw.
    reinterpret_cast<T_DME>(pOriginalDME)(pVTable, modelState, renderInfo, boneMatrix);

    int iCustomMatOverride = F::entityIterator.GetEntityMaterial(pEnt);
    if (iCustomMatOverride >= 0)
        iWishMaterialIndex = iCustomMatOverride;

    if (iWishMaterialIndex < 0)
        return;

    iWishMaterialIndex = std::clamp<int>(iWishMaterialIndex, 0, F::materialGen.GetMaterialList().size() - 1);
     
    // Drawing using selected material.
    const std::vector<Material_t*>& vecMaterials = F::materialGen.GetMaterialList()[iWishMaterialIndex].m_vecMaterials;
     
    for (const Material_t* pMat : vecMaterials)
    {
        if (pMat == nullptr || pMat->m_pMaterial == nullptr)
            continue;
            
        // Override materials.
        I::iStudioRender->ForcedMaterialOverride(pMat->m_pMaterial, OverrideType_t::OVERRIDE_NORMAL);

        // Draw model once with this material.
        reinterpret_cast<T_DME>(pOriginalDME)(pVTable, modelState, renderInfo, boneMatrix);
    }
     
    // Clear material residue.
    I::iStudioRender->ForcedMaterialOverride(nullptr, OverrideType_t::OVERRIDE_NORMAL);
}


///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
void ChamsV2_t::_DrawBackTrack(void* pVTable, DrawModelState_t* modelState, ModelRenderInfo_t* renderInfo, void* pOriginalDME, BaseEntity* pEnt)
{
    // -1 -> Not selected | 0 -> None |  1-> Last record only | 2 -> all Records
    if (Features::BackTrack::BackTrack::BackTrack_Cham_Setting.GetData() < 1)
        return;

    // No backtrack ?
    if (F::entityIterator.GetBackTrackTime() == 0.0f)
        return;

    std::deque<BackTrackRecord_t>* records = F::entityIterator.GetBackTrackRecord(pEnt);
    if (records == nullptr)
        return;

    if (records->size() <= 0)
        return;
    
    // NOTE: A non-zero, non-nullptr backtrack record deque if promised from this point onwards.

    int iWishMaterialIndex = Features::BackTrack::BackTrack::BackTrack_Cham.GetData();
    int nTicks             = TIME_TO_TICK(F::entityIterator.GetBackTrackTime());

    if (Features::BackTrack::BackTrack::BackTrack_Cham_Setting.GetData() == 1)
    {
        int                iRecordIndex = std::clamp<int>(nTicks, 0, records->size() - 1);
        BackTrackRecord_t& record       = (*records)[iRecordIndex];

        reinterpret_cast<T_DME>(pOriginalDME)(pVTable, modelState, renderInfo, record.m_bones);

        // No material set ?
        if (iWishMaterialIndex < 0)
            return;

        iWishMaterialIndex = std::clamp<int>(iWishMaterialIndex, 0, F::materialGen.GetMaterialList().size() - 1);

        // Drawing using selected material.
        const std::vector<Material_t*>& vecMaterials = F::materialGen.GetMaterialList()[iWishMaterialIndex].m_vecMaterials;

        for (const Material_t* pMat : vecMaterials)
        {
            if (pMat == nullptr || pMat->m_pMaterial == nullptr)
                continue;

            // Override materials.
            I::iStudioRender->ForcedMaterialOverride(pMat->m_pMaterial, OverrideType_t::OVERRIDE_NORMAL);

            // Draw model once with this material.
            reinterpret_cast<T_DME>(pOriginalDME)(pVTable, modelState, renderInfo, record.m_bones);
        }

        // Clear material residue.
        I::iStudioRender->ForcedMaterialOverride(nullptr, OverrideType_t::OVERRIDE_NORMAL);
    }
    else
    {
        for (int iTick = nTicks - TIME_TO_TICK(0.2f); iTick < nTicks; iTick++)
        {
            if (iTick >= records->size())
                return;

            BackTrackRecord_t& record = (*records)[iTick];

            // Draw original
            reinterpret_cast<T_DME>(pOriginalDME)(pVTable, modelState, renderInfo, record.m_bones);

            // No material set ?
            if (iWishMaterialIndex < 0)
                return;

            iWishMaterialIndex = std::clamp<int>(iWishMaterialIndex, 0, F::materialGen.GetMaterialList().size() - 1);

            // Drawing using selected material.
            const std::vector<Material_t*>& vecMaterials = F::materialGen.GetMaterialList()[iWishMaterialIndex].m_vecMaterials;

            for (const Material_t* pMat : vecMaterials)
            {
                if (pMat == nullptr || pMat->m_pMaterial == nullptr)
                    continue;

                // Override materials.
                I::iStudioRender->ForcedMaterialOverride(pMat->m_pMaterial, OverrideType_t::OVERRIDE_NORMAL);

                // Draw model once with this material.
                reinterpret_cast<T_DME>(pOriginalDME)(pVTable, modelState, renderInfo, record.m_bones);
            }
        }

        // Clear material residue.
        I::iStudioRender->ForcedMaterialOverride(nullptr, OverrideType_t::OVERRIDE_NORMAL);
    }
}


///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
void ChamsV2_t::_SetupMatDropDowns()
{
    // TODO : Maybe when a notification UI is in place, push a notification to alert of this limit.
    constexpr int MAX_MATERIALS = 128;
    static const char* szMaterialList[MAX_MATERIALS]; // No ones making more than 128 fucking mateirals. This is more than enough.


    // Constructing Material list. ( all mateiral that we have in mat gen. )
    const std::vector<MaterialBundle_t>& vecMatList = F::materialGen.GetMaterialList();

    int nMaterials = vecMatList.size() <= MAX_MATERIALS ? vecMatList.size() : MAX_MATERIALS;
    szMaterialList[0] = "None";
    for (int iMatIndex = 1; iMatIndex <= nMaterials; iMatIndex++)
    {
        szMaterialList[iMatIndex] = vecMatList[iMatIndex - 1].m_szMatBundleName.c_str();
    }


    // Finally setting list.
    Features::Materials::Materials::Player_Enemy.SetItems(szMaterialList,         nMaterials + 1);
    Features::Materials::Materials::Player_TeamMates.SetItems(szMaterialList,     nMaterials + 1);
    Features::Materials::Materials::Sentry_Enemy.SetItems(szMaterialList,         nMaterials + 1);
    Features::Materials::Materials::Sentry_TeamMates.SetItems(szMaterialList,     nMaterials + 1);
    Features::Materials::Materials::Dispenser_Enemy.SetItems(szMaterialList,      nMaterials + 1);
    Features::Materials::Materials::Dispenser_TeamMates.SetItems(szMaterialList,  nMaterials + 1);
    Features::Materials::Materials::Teleporter_Enemy.SetItems(szMaterialList,     nMaterials + 1);
    Features::Materials::Materials::Teleporter_TeamMates.SetItems(szMaterialList, nMaterials + 1);
    Features::BackTrack::BackTrack::BackTrack_Cham.SetItems(szMaterialList,       nMaterials + 1);
}
