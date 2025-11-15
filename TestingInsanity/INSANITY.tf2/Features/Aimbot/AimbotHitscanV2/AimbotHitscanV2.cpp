#include "AimbotHitscanV2.h"

// SDK
#include "../../../SDK/class/CUserCmd.h"
#include "../../../SDK/class/BaseEntity.h"
#include "../../../SDK/class/BaseWeapon.h"
#include "../../../SDK/class/IVEngineClient.h"
#include "../../../SDK/class/CommonFns.h"
#include "../../../SDK/class/IVModelInfo.h"
#include "../../../SDK/class/IVDebugOverlay.h"
#include "../../../SDK/class/IEngineTrace.h"
#include "../../../SDK/class/FileWeaponInfo.h"
#include "../../../SDK/TF object manager/TFOjectManager.h"
#include "../../../SDK/class/HitboxDefs.h"
#include "../../Entity Iterator/EntityIterator.h"

// UTILITY
#include "../../../Extra/math.h"
#include "../../../Utility/ConsoleLogging.h"
#include "../../../Utility/Profiler/Profiler.h"
#include "../../../Utility/Signature Handler/signatures.h"

#define HITSCANAIMBOT_DEBUG_HITBOX false

MAKE_SIG(CBaseAnimatinng_LookUpBones, "40 53 48 83 EC ? 48 8B DA E8 ? ? ? ? 48 8B C8 48 8B D3 48 83 C4 ? 5B E9 ? ? ? ? CC CC 48 89 74 24", CLIENT_DLL, int64_t, void*, const char*)

 

///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
void AimbotHitscanV2_t::Run(CUserCmd* pCmd, BaseEntity* pLocalPlayer, baseWeapon* pActiveWeapon, bool* pCreateMoveResult)
{
    PROFILER_RECORD_FUNCTION(CreateMove);

    if (Features::Aimbot::AimbotHitscanV2::AimbotHitscan_Enable.IsActive() == false)
        return;

    
    // Choose target.
    bool bInAttack  = SDK::InAttack(pLocalPlayer, pActiveWeapon);
    bool bCanAttack = SDK::CanAttack(pLocalPlayer, pActiveWeapon);
    /*if(bInAttack == false)
    {
        m_pBestTarget = _ChooseTarget(pLocalPlayer, pActiveWeapon, pCmd);
    }*/
    m_pBestTarget = _ChooseTarget(pLocalPlayer, pActiveWeapon, pCmd);


    // If no target found, then leave.
    if (m_pBestTarget == nullptr)
        return;

    
    // Zoom-in if Auto-Scope is active.
    _DoAutoScope(pLocalPlayer, pActiveWeapon, pCmd);


    // If valid target found, just shoot.
    if (_ShouldAutoFire(pLocalPlayer, pActiveWeapon) == true)
    {
        pCmd->buttons |= IN_ATTACK;
    }


    // Aim @ that nigga we just found.
    if ((pCmd->buttons & IN_ATTACK) == true && bCanAttack == true)
    {
        _ShootAtTarget(pLocalPlayer, pCmd, pCreateMoveResult);
    }
}


///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
BaseEntity* AimbotHitscanV2_t::GetTargetEntity() const
{
    return m_pLastTarget;
}


///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
void AimbotHitscanV2_t::_DoAutoScope(BaseEntity* pLocalPlayer, baseWeapon* pActiveWeapon, CUserCmd* pCmd)
{
    if (Features::Aimbot::AimbotHitscanV2::AimbotHitscan_AutoScope.IsActive() == false)
        return;

    bool bIsUsingSniperRifle = pLocalPlayer->m_iClass() == TF_SNIPER && pActiveWeapon->getSlot() == WPN_SLOT_PRIMARY;
    if (bIsUsingSniperRifle == false)
        return;

    if (pLocalPlayer->InCond(TF_COND_ZOOMED) == true)
        return;

    // Yea, there is a button enum named as IN_ZOOM, but to zoom, you have to use this.
    pCmd->buttons |= IN_SECOND_ATTACK;
}


///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
bool AimbotHitscanV2_t::_ShouldAutoFire(BaseEntity* pLocalPlayer, baseWeapon* pActiveWeapon)
{
    if (Features::Aimbot::AimbotHitscanV2::AimbotHitscan_AutoFire.IsActive() == false)
        return false;

    // causing trouble with pistols.
    /*if (SDK::CanAttack(pLocalPlayer, pActiveWeapon) == false)
        return false;*/

    // Don't Auto-Fire unscooped, if user doesn't wanna shoot unscoped.
    if (Features::Aimbot::AimbotHitscanV2::AimbotHitscan_DontShootUnscoped.IsActive() == true)
    {
        bool bIsUsingSniperRifle = pLocalPlayer->m_iClass() == TF_SNIPER && pActiveWeapon->getSlot() == WPN_SLOT_PRIMARY;
        
        bool bUnscoped = bIsUsingSniperRifle == true && pActiveWeapon->m_flChargedDamage() == 0.0f;
        if (bUnscoped == true)
            return false;
    }

    
    // Only shooting is shot is lethal
    if (Features::Aimbot::AimbotHitscanV2::AimbotHitscan_AutoFireWhenLethal.IsActive() == true)
    {
        if (pLocalPlayer->m_iClass() == TF_SNIPER && pActiveWeapon->getSlot() == WPN_SLOT_PRIMARY)
        {
            if (m_pBestTarget->m_iHealth() > static_cast<int32_t>(_EstimateSniperDamage(pLocalPlayer, pActiveWeapon)))
                return false;
        }
    }

    return true;
}


///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
float AimbotHitscanV2_t::_EstimateSniperDamage(BaseEntity* pLocalPlayer, baseWeapon* pActiveWeapon)
{
    float flDamage    = std::clamp<float>(pActiveWeapon->m_flChargedDamage(), TF_WEAPON_SNIPERRIFLE_DAMAGE_MIN, TF_WEAPON_SNIPERRIFLE_DAMAGE_MAX);
    float flDamageMod = 1.0f; pActiveWeapon->CALL_ATRIB_HOOK_FLOAT(flDamageMod, "mult_dmg");
    flDamage *= flDamageMod;

    // if fully charged, then fire!
    if (pActiveWeapon->m_flChargedDamage() >= TF_WEAPON_SNIPERRIFLE_DAMAGE_MAX)
        return 10000.0f;

    // Now, assuming that we hit our target, we will simply hack together a number that roughly resembles 
    // the damage we are gonna deal. This is by no means accurate at all, but my big brain is hi on cheap chocolates right now. :)
    float flDamageMult = 1.0f;
    switch (m_iTargetHitbox)
    {
    case HitboxPlayer_Head:
        flDamageMult = DMG_HITGROUP_HEAD;
        break;
    case HitboxPlayer_Hip:
    case HitboxPlayer_SpineLower:
    case HitboxPlayer_SpineMiddle:
    case HitboxPlayer_SpineUpper:
    case HitboxPlayer_SpineTop:
        flDamageMult = DMG_HITGROUP_CHEST;
        break;
    case HitboxPlayer_LeftUpperArm:
    case HitboxPlayer_LeftForearm:
    case HitboxPlayer_LeftHand:
    case HitboxPlayer_RightUpperArm:
    case HitboxPlayer_RightForearm:
    case HitboxPlayer_RightHand:
        flDamageMult = DMG_HITGROUP_RIGHTARM;
        break;
    case HitboxPlayer_LeftUpperLeg:
    case HitboxPlayer_LeftLowerLeg:
    case HitboxPlayer_LeftFoot:
    case HitboxPlayer_RightUpperLeg:
    case HitboxPlayer_RightLowerLeg:
    case HitboxPlayer_RightFoot:
        flDamageMult = DMG_HITGROUP_RIGHTLEG;
        break;
    case HitboxPlayer_Count:
    default: break;
    }

    return flDamage * flDamageMult;
}


///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
BaseEntity* AimbotHitscanV2_t::_ChooseTarget(BaseEntity* pLocalPlayer, baseWeapon* pActiveWeapon, CUserCmd* pCmd)
{
#if (HITSCANAIMBOT_DEBUG_HITBOX == true)
    I::IDebugOverlay->ClearAllOverlays();
#endif

    BaseEntity* pTarget = nullptr;

    // First, try to find a valid enemy player...
    bool bPlayerAimbotEnabled =
        Features::Aimbot::AimbotHitscan_Hitbox::Hitbox_Head.IsActive()  ||
        Features::Aimbot::AimbotHitscan_Hitbox::Hitbox_Torso.IsActive() ||
        Features::Aimbot::AimbotHitscan_Hitbox::Hitbox_Arms.IsActive()  ||
        Features::Aimbot::AimbotHitscan_Hitbox::Hitbox_Legs.IsActive();


    if(bPlayerAimbotEnabled == true)
    {
        pTarget = _ChoosePlayerTarget(pLocalPlayer, pActiveWeapon, pCmd);
        if (pTarget != nullptr)
            return pTarget;
    }


    // If can't find a valid enemy player, try to find a valid enemy building.
    
    // SENTRIES...
    if(Features::Aimbot::AimbotHitscan_Hitbox::Hitbox_Sentry.IsActive() == true)
    {
        Containers::DoubleBuffer_t<std::vector<BaseEntity*>>& pDoubleBufferEnemies = F::entityIterator.GetEnemySentry();
        std::vector<BaseEntity*>*                             pVecEnemies          = pDoubleBufferEnemies.GetReadBuffer();
        DOUBLEBUFFER_AUTO_RELEASE_READBUFFER(&pDoubleBufferEnemies, pVecEnemies);

        pTarget = _ChooseBuildingTarget(pVecEnemies, pLocalPlayer, pActiveWeapon, pCmd);
        if (pTarget != nullptr)
            return pTarget;
    }

    // TELEPORTERS...
    if(Features::Aimbot::AimbotHitscan_Hitbox::Hitbox_Teleporter.IsActive() == true)
    {
        Containers::DoubleBuffer_t<std::vector<BaseEntity*>>& pDoubleBufferEnemies = F::entityIterator.GetEnemyTeleporter();
        std::vector<BaseEntity*>*                             pVecEnemies          = pDoubleBufferEnemies.GetReadBuffer();
        DOUBLEBUFFER_AUTO_RELEASE_READBUFFER(&pDoubleBufferEnemies, pVecEnemies);

        pTarget = _ChooseBuildingTarget(pVecEnemies, pLocalPlayer, pActiveWeapon, pCmd);
        if (pTarget != nullptr)
            return pTarget;
    }

    // DISPENSERS...
    if(Features::Aimbot::AimbotHitscan_Hitbox::Hitbox_Dispenser.IsActive() == true)
    {
        Containers::DoubleBuffer_t<std::vector<BaseEntity*>>& pDoubleBufferEnemies = F::entityIterator.GetEnemyDispenser();
        std::vector<BaseEntity*>*                             pVecEnemies          = pDoubleBufferEnemies.GetReadBuffer();
        DOUBLEBUFFER_AUTO_RELEASE_READBUFFER(&pDoubleBufferEnemies, pVecEnemies);

        pTarget = _ChooseBuildingTarget(pVecEnemies, pLocalPlayer, pActiveWeapon, pCmd);
        if (pTarget != nullptr)
            return pTarget;
    }


    // STICKY BOMBS...
    if(Features::Aimbot::AimbotHitscan_Hitbox::Hitbox_StickyBombs.IsActive() == true)
    {
        Containers::DoubleBuffer_t<std::vector<BaseEntity*>>& pDoubleBufferEnemies = F::entityIterator.GetEnemyPipeBombs();
        std::vector<BaseEntity*>* pVecEnemies = pDoubleBufferEnemies.GetReadBuffer();
        DOUBLEBUFFER_AUTO_RELEASE_READBUFFER(&pDoubleBufferEnemies, pVecEnemies);

        pTarget = _ChooseStickyBombTarget(pVecEnemies, pLocalPlayer, pActiveWeapon, pCmd);
        if (pTarget != nullptr)
            return pTarget;
    }


    return pTarget;
}


///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
BaseEntity* AimbotHitscanV2_t::_ChoosePlayerTarget(BaseEntity* pLocalPlayer, baseWeapon* pActiveWeapon, CUserCmd* pCmd)
{
    // Get the enemy list. Very important step.
    Containers::DoubleBuffer_t<std::vector<BaseEntity*>>& pDoubleBufferEnemies = F::entityIterator.GetEnemyPlayers();
    std::vector<BaseEntity*>*                             pVecEnemies          = pDoubleBufferEnemies.GetReadBuffer();
    DOUBLEBUFFER_AUTO_RELEASE_READBUFFER(&pDoubleBufferEnemies, pVecEnemies);


    // Now we sort the enemy list according to user's prefrences.
    std::vector<BaseEntity*> vecSortedTargets; vecSortedTargets.clear();
    qangle qEngineAngles; I::iEngine->GetViewAngles(qEngineAngles);
    _SortTargetList(pVecEnemies, vecSortedTargets, pLocalPlayer, qEngineAngles, true);
    if (vecSortedTargets.size() == 0LLU) // User doesn't wanna shoot at any enemy players it seems...
        return nullptr;


    // Some localplayer stuff...
    vec vEyePos         = pLocalPlayer->GetEyePos();
    vec vAttackerOrigin = pLocalPlayer->GetAbsOrigin();
    float flMaxDistance = Features::Aimbot::AimbotHitscanV2::AimbotHitscan_MaxDistance.GetData().m_flVal;


    // This holds the hitbox indexes in decreasing order or priority.
    // we itearate all hitboxes in this list and shoot at the first one we can hit.
    // Priorty depends on our active weapon & damage multiplier of that hitbox n stuff like that.
    std::vector<int> vecHitboxPriorityList;
    _ConstructHitboxPriorityList(&vecHitboxPriorityList);



    // Now iterate all enemies & find a good one to kill :)
    for (BaseEntity* pEnt : vecSortedTargets)
    {
        // Max distance check ( for skipping unwanted entities early )...
        if (flMaxDistance > 0.0f)
        {
            if (pEnt->GetAbsOrigin().DistTo(vAttackerOrigin) > flMaxDistance)
                continue;
        }

        // Model
        const model_t* pModel = pEnt->GetModel();
        if (pModel == nullptr)
            continue;

        // Studio model for this entities model
        const StudioHdr_t* pStudioModel = I::iVModelInfo->GetStudiomodel(pModel);
        if (pStudioModel == nullptr)
            continue;

        // Hit boxes for this studio model.
        const auto* pHitBoxSet = pStudioModel->pHitboxSet(pEnt->m_nHitboxSet());
        if (pHitBoxSet == nullptr)
            continue;


        // Just in case, we don't happen to have any backtrack records for this entity.
        std::deque<BackTrackRecord_t>* pQBackTrackRecords = F::entityIterator.GetBackTrackRecord(pEnt);
        if (pQBackTrackRecords == nullptr)
            continue;

        if (pQBackTrackRecords->size() == 0LLU)
            continue;


        // NOTE : In case no choice is selected, the first record / the latest record will simply be targated...
        enum BackTrackChoice_t { BackTrackChoice_FirstOnly = 0, BackTrackChoice_LastOnly, BackTrackChoice_All };
        bool bIterateBackwards = (Features::Aimbot::AimbotHitscanV2::AimbotHitscan_BackTrackChoice.GetData() == BackTrackChoice_LastOnly);
        bool bSingleIteration  = (Features::Aimbot::AimbotHitscanV2::AimbotHitscan_BackTrackChoice.GetData() != BackTrackChoice_All);
        
        // BackTrack min & max...
        float flBackTrackTimeInSec = F::entityIterator.GetBackTrackTimeInSec();
        int iBackTrackTickMax = std::clamp<int>(TIME_TO_TICK(flBackTrackTimeInSec),                      0, pQBackTrackRecords->size() - 1LLU);
        int iBackTrackTickMin = std::clamp<int>(TIME_TO_TICK(flBackTrackTimeInSec - MAX_BACKTRACK_TIME), 0, pQBackTrackRecords->size() - 1LLU);

        // First index for iteration & number of records to iterate... 
        int iFirstRecordIndex = bIterateBackwards == true ? iBackTrackTickMax : iBackTrackTickMin;
        int nRecords          = bSingleIteration  == true ? 1 : TIME_TO_TICK(MAX_BACKTRACK_TIME);

        for (int i = 0, iRecordIndex = iFirstRecordIndex; i < nRecords && iRecordIndex >= iBackTrackTickMin && iRecordIndex <= iBackTrackTickMax; i++)
        {
            BackTrackRecord_t& record = (*pQBackTrackRecords)[iRecordIndex];

            // Update record index here, cause we don't know if user wanted the first or last records.
            // That means, we might need to iterate forward or backwards, hence +1 or -1 is unknown.
            iFirstRecordIndex = bIterateBackwards == true ? iRecordIndex - 1 : iRecordIndex + 1;

            // Itearte over all hitboxes for this entity, to find one we can hit.
            // NOTE : the hitbox indexes are arranged in decreasing order of priority.
            for (int iHitboxIndex : vecHitboxPriorityList)
            {
                // Target hitbox & bone for that hitbox.
                mstudiobbox_t* pHitbox    = pHitBoxSet->pHitbox(iHitboxIndex);
                matrix3x4_t*   targetBone = &record.m_bones[pHitbox->bone];

                // Is this hitbox in FOV ?
                if (_IsInFOV(targetBone, pHitbox, vEyePos) == false)
                    continue;


                // Most visible point on target.
                vec vBestTargetPos;
                if (_IsVisible(targetBone, pHitbox, vEyePos, vBestTargetPos, pLocalPlayer, pEnt, qEngineAngles) == false)
                    continue;


#if (HITSCANAIMBOT_DEBUG_HITBOX == true)

                qangle qBoneAngle; Maths::MatrixAngles(*targetBone, qBoneAngle);
                static vec s_vMarkerBoxSize(2.0f, 2.0f, 2.0f); 
                vec vMaxRotated; Maths::VectorTransform(pHitbox->bbmax, *targetBone, vMaxRotated);
                vec vMinRotated; Maths::VectorTransform(pHitbox->bbmin, *targetBone, vMinRotated);
                vec vBoneCenter = vMinRotated + ((vMaxRotated - vMinRotated) / 2.0f);
                vec vBoneOrigin = targetBone->GetWorldPos();

                // Hitbox...
                I::IDebugOverlay->AddBoxOverlay(targetBone->GetWorldPos(), pHitbox->bbmin, pHitbox->bbmax, qBoneAngle, 255, 255, 255, 0, 5.0f);
                
                // Hitpoint ( where we shot )
                I::IDebugOverlay->AddBoxOverlay(vBestTargetPos, s_vMarkerBoxSize * -0.5f, s_vMarkerBoxSize * 0.5f, qBoneAngle,   0, 255, 255, 100, 5.0f);
                I::IDebugOverlay->AddBoxOverlay(vBoneCenter,    s_vMarkerBoxSize * -0.5f, s_vMarkerBoxSize * 0.5f, qBoneAngle, 255,   0,   0, 100, 5.0f);

                // Min & Max on the hitbox...
                I::IDebugOverlay->AddBoxOverlay(/*vBoneOrigin + */vMinRotated, s_vMarkerBoxSize * -1.0f, s_vMarkerBoxSize, qBoneAngle, 255, 0, 0, 100, 5.0f);
                I::IDebugOverlay->AddBoxOverlay(/*vBoneOrigin + */vMaxRotated, s_vMarkerBoxSize * -1.0f, s_vMarkerBoxSize, qBoneAngle, 255, 0, 0, 100, 5.0f);

                // Additional information...
                I::IDebugOverlay->AddLineOverlay(vEyePos, vBestTargetPos, 255, 255, 255, false, 5.0f);
                I::IDebugOverlay->AddTextOverlayRGB(vBestTargetPos, 0, 5.0f, 255, 255, 255, 255, "bip_head  : %d", Sig::CBaseAnimatinng_LookUpBones(pEnt, "bip_head"));
                I::IDebugOverlay->AddTextOverlayRGB(vBestTargetPos, 1, 5.0f, 255, 255, 255, 255, "Hitbox    : %d", iHitboxIndex);
                I::IDebugOverlay->AddTextOverlayRGB(vBestTargetPos, 2, 5.0f, 255, 255, 255, 255, "TagetBone : %d", pHitbox->bone);

#endif


                // Store angles if we can shoot his nigga & leave. ( no more scanning and cause it is alread quite expensive to run this much )
                m_iTargetHitbox  = iHitboxIndex;
                m_vBestTargetPos = vBestTargetPos;
                m_iTickCount     = record.m_iTick;
                return pEnt;
            }
        }

    }

    return nullptr;
}


///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
BaseEntity* AimbotHitscanV2_t::_ChooseBuildingTarget(const std::vector<BaseEntity*>* pVecTargets, BaseEntity* pLocalPlayer, baseWeapon* pActiveWeapon, CUserCmd* pCmd)
{
    // Now we sort the enemy list according to user's prefrences.
    std::vector<BaseEntity*> vecSortedTargets; vecSortedTargets.clear();
    qangle qEngineAngles; I::iEngine->GetViewAngles(qEngineAngles);
    _SortTargetList(pVecTargets, vecSortedTargets, pLocalPlayer, qEngineAngles, true);

    // Some localplayer stuff...
    vec vEyePos         = pLocalPlayer->GetEyePos();
    vec vAttackerOrigin = pLocalPlayer->GetAbsOrigin();
    float flMaxDistance = Features::Aimbot::AimbotHitscanV2::AimbotHitscan_MaxDistance.GetData().m_flVal;



    // Now iterate all enemies & find a good one to kill :)
    for (BaseEntity* pEnt : vecSortedTargets)
    {
        // Max distance check ( for skipping unwanted entities early )...
        if (flMaxDistance > 0.0f)
        {
            if (pEnt->GetAbsOrigin().DistTo(vAttackerOrigin) > flMaxDistance)
                continue;
        }

        ICollideable_t* pCollidable = pEnt->GetCollideable();
        if (pCollidable == nullptr)
            continue;


        // Min, Max & Origin for building...
        vec vEntOrigin = pCollidable->GetCollisionOrigin();
        vec vObbMin    = pCollidable->OBBMinsPreScaled();
        vec vObbMax    = pCollidable->OBBMaxsPreScaled();


        // Is this building in FOV ?
        {
            float flFOV = Features::Aimbot::AimbotHitscanV2::AimbotHitscan_FOV.GetData().m_flVal;
            float flAngleMin = _GetAngleFromCrosshair(vEntOrigin + vObbMin, vEyePos, qEngineAngles);
            float flAngleMax = _GetAngleFromCrosshair(vEntOrigin + vObbMax, vEyePos, qEngineAngles);

            // if both of the corners are out of FOV-circle, then building ain't in the FOV-circle.
            // Atleast one of the corners ( min or max ) should be in FOV.
            if (flAngleMin > flFOV && flAngleMax > flFOV)
                continue;
        }

        // Visibile or not?
        {
            vec   vVisibleTargetPos;
            vec   vEntCenter     = vEntOrigin + (vObbMax + vObbMin) / 2.0f;
            float flBloomRadius  = Features::Aimbot::AimbotHitscanV2::AimbotHitscan_BuildingBloom.GetData().m_flVal / 2.0f;
            bool  bTargetVisible = _MultipointVisibilityCheck(vEntCenter, vObbMin, vObbMax, qEngineAngles, vVisibleTargetPos, pLocalPlayer, pEnt, flBloomRadius);

            if (bTargetVisible == false)
                continue;

            m_vBestTargetPos = vVisibleTargetPos;
            return pEnt;
        }
    }

    return nullptr;
}


///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
BaseEntity* AimbotHitscanV2_t::_ChooseStickyBombTarget(const std::vector<BaseEntity*>* pVecTargets, BaseEntity* pLocalPlayer, baseWeapon* pActiveWeapon, CUserCmd* pCmd)
{
    // Now we sort the enemy list according to user's prefrences.
    std::vector<BaseEntity*> vecSortedTargets; vecSortedTargets.clear();
    qangle qEngineAngles; I::iEngine->GetViewAngles(qEngineAngles);
    _SortTargetList(pVecTargets, vecSortedTargets, pLocalPlayer, qEngineAngles, false);


    // Some localplayer stuff...
    vec vEyePos         = pLocalPlayer->GetEyePos();
    vec vAttackerOrigin = pLocalPlayer->GetAbsOrigin();
    float flMaxDistance = Features::Aimbot::AimbotHitscanV2::AimbotHitscan_MaxDistance.GetData().m_flVal;



    // Now iterate all enemies & find a good one to kill :)
    for (BaseEntity* pEnt : vecSortedTargets)
    {
        // Max distance check ( for skipping unwanted entities early )...
        if (flMaxDistance > 0.0f)
        {
            if (pEnt->GetAbsOrigin().DistTo(vAttackerOrigin) > flMaxDistance)
                continue;
        }

        ICollideable_t* pCollidable = pEnt->GetCollideable();
        if (pCollidable == nullptr)
            continue;


        // Min, Max & Origin for building...
        vec vEntOrigin = pCollidable->GetCollisionOrigin();
        vec vObbMin = pCollidable->OBBMinsPreScaled();
        vec vObbMax = pCollidable->OBBMaxsPreScaled();


        // Is this building in FOV ?
        {
            float flFOV = Features::Aimbot::AimbotHitscanV2::AimbotHitscan_FOV.GetData().m_flVal;
            float flAngle = _GetAngleFromCrosshair(vEntOrigin, vEyePos, qEngineAngles);

            // if both of the corners are out of FOV-circle, then building ain't in the FOV-circle.
            if (flAngle > flFOV)
                continue;
        }

        // Visibile or not?
        {
            vec vEntCenter = vEntOrigin + (vObbMin + vObbMax) / 2.0f;
            ITraceFilter_IgnoreSpawnVisualizer filter(pLocalPlayer); trace_t trace;
            I::EngineTrace->UTIL_TraceRay(vEyePos, vEntCenter, MASK_SHOT, &filter, &trace);

            // Can't hit this target.
            bool bTargetVisible = trace.m_fraction >= 0.99f || trace.m_entity == pEnt;

            if (bTargetVisible == false)
                continue;


            m_vBestTargetPos = vEntCenter;
            return pEnt;
        }
    }

    return nullptr;
}


///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
void AimbotHitscanV2_t::_ShootAtTarget(BaseEntity* pLocalPlayer, CUserCmd* pCmd, bool* pCreateMoveResult)
{
    vec vAttackerToTarget = m_vBestTargetPos - pLocalPlayer->GetEyePos();
    vAttackerToTarget.NormalizeInPlace();

    qangle qAimbotAngles; Maths::VectorAnglesFromSDK(vAttackerToTarget, qAimbotAngles);
    Maths::WrapYaw(qAimbotAngles);

    pCmd->viewangles = qAimbotAngles;
    pCmd->tick_count = m_iTickCount > 0 ? m_iTickCount : pCmd->tick_count;


    // Silenting view angle changes for client.
    if (Features::Aimbot::AimbotHitscanV2::AimbotHitscan_SilentAim.IsActive() == true)
    {
        *pCreateMoveResult = false;
    }

    // Resetting once aimbot angles and stuff are setup
    _ResetAimbotData();
}


///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
void AimbotHitscanV2_t::_SortTargetList(const std::vector<BaseEntity*>* vecSource, std::vector<BaseEntity*>& vecDestination, BaseEntity* pLocalPlayer, const qangle& qViewAngles, bool bIsPlayer)
{
    // Copy into destination.
    vecDestination = *vecSource;

    // Just for the sake of redability.
    enum TargetPriorityType_t
    {
        TargetPriorityType_Invalid      = -1,
        TargetPriorityType_ByDistance   = 0,
        TargetPriorityType_ClosestToCrosshair
    };

    int iSortOrder = Features::Aimbot::AimbotHitscanV2::AimbotHitscan_TargetPriority.GetData();
    iSortOrder     = std::clamp<int>(iSortOrder, TargetPriorityType_ByDistance, TargetPriorityType_ClosestToCrosshair);

    vec vAttackerPos = pLocalPlayer->GetAbsOrigin();
    vec vEyePos      = pLocalPlayer->GetEyePos();

    if (iSortOrder == TargetPriorityType_ByDistance)
    {
        std::sort(vecDestination.begin(), vecDestination.end(),
            [&](BaseEntity* pEnt1, BaseEntity* pEnt2) -> bool
            {
                return pEnt1->GetAbsOrigin().DistTo(vAttackerPos) < pEnt2->GetAbsOrigin().DistTo(vAttackerPos);
            });
    }
    else
    {
        std::sort(vecDestination.begin(), vecDestination.end(),
            [&](BaseEntity* pEnt1, BaseEntity* pEnt2) -> bool
            {
                vec vEntCenter1 = pEnt1->GetAbsOrigin(); if(bIsPlayer == true ) vEntCenter1.z += pEnt1->m_vecViewOffset().z / 2.0f;
                vec vEntCenter2 = pEnt2->GetAbsOrigin(); if(bIsPlayer == true ) vEntCenter2.z += pEnt2->m_vecViewOffset().z / 2.0f;

                return _GetAngleFromCrosshair(vEntCenter1, vEyePos, qViewAngles) < _GetAngleFromCrosshair(vEntCenter2, vEyePos, qViewAngles);
            });
    }
}


///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
void AimbotHitscanV2_t::_ConstructHitboxPriorityList(std::vector<int>* vecHitboxes)
{
    vecHitboxes->clear();


    // Head.
    if(Features::Aimbot::AimbotHitscan_Hitbox::Hitbox_Head.IsActive() == true)
    {
        vecHitboxes->push_back(HitboxPlayer_Head);
    }


    // Torso hitboxes.
    if(Features::Aimbot::AimbotHitscan_Hitbox::Hitbox_Torso.IsActive() == true)
    {
        // NOTE : we are giving middle torso more important here... thats intentional.
        vecHitboxes->push_back(HitboxPlayer_SpineMiddle);
        vecHitboxes->push_back(HitboxPlayer_SpineUpper);
        vecHitboxes->push_back(HitboxPlayer_SpineLower);
        vecHitboxes->push_back(HitboxPlayer_SpineTop);
        vecHitboxes->push_back(HitboxPlayer_Hip);
    }


    // Arms...
    if (Features::Aimbot::AimbotHitscan_Hitbox::Hitbox_Arms.IsActive() == true)
    {
        vecHitboxes->push_back(HitboxPlayer_LeftUpperArm);
        vecHitboxes->push_back(HitboxPlayer_RightUpperArm);
        
        vecHitboxes->push_back(HitboxPlayer_LeftForearm);
        vecHitboxes->push_back(HitboxPlayer_RightForearm);
        
        vecHitboxes->push_back(HitboxPlayer_LeftHand);
        vecHitboxes->push_back(HitboxPlayer_RightHand);
    }


    // Legs...
    if (Features::Aimbot::AimbotHitscan_Hitbox::Hitbox_Arms.IsActive() == true)
    {
        vecHitboxes->push_back(HitboxPlayer_LeftUpperLeg);
        vecHitboxes->push_back(HitboxPlayer_RightUpperLeg);

        vecHitboxes->push_back(HitboxPlayer_LeftLowerLeg);
        vecHitboxes->push_back(HitboxPlayer_RightLowerLeg);

        vecHitboxes->push_back(HitboxPlayer_LeftFoot);
        vecHitboxes->push_back(HitboxPlayer_RightFoot);
    }
}


///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
bool AimbotHitscanV2_t::_IsInFOV(const matrix3x4_t* bone, mstudiobbox_t* pHitbox, const vec& vAttackerEyePos) const
{
    float flFOV = Features::Aimbot::AimbotHitscanV2::AimbotHitscan_FOV.GetData().m_flVal;

    vec vBoneOrigin = bone->GetWorldPos();
    vec vBoneMin    = vBoneOrigin - pHitbox->bbmin;
    vec vBoneMax    = vBoneOrigin - pHitbox->bbmax;

    qangle qAttackerViewAngles; I::iEngine->GetViewAngles(qAttackerViewAngles);
    float flAngleToMin = _GetAngleFromCrosshair(vBoneMin, vAttackerEyePos, qAttackerViewAngles);
    float flAngleToMax = _GetAngleFromCrosshair(vBoneMax, vAttackerEyePos, qAttackerViewAngles);

    // If atleast one corner of the hitbox is within the fov circle, then we can shoot it.
    return flAngleToMin <= flFOV || flAngleToMax <= flFOV;
}


///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
float AimbotHitscanV2_t::_GetAngleFromCrosshair(const vec& vTargetPos, const vec& vAttackerEyePos, const qangle& qViewAngles) const
{
    vec vViewAngles;
    Maths::AngleVectors(qViewAngles, &vViewAngles);
    vViewAngles.NormalizeInPlace();

    vec vAttackerToTarget = vTargetPos - vAttackerEyePos;
    vAttackerToTarget.NormalizeInPlace();

    float flDot = vViewAngles.Dot(vAttackerToTarget);
    return RAD2DEG(acosf(flDot));
}


///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
bool AimbotHitscanV2_t::_IsVisible(const matrix3x4_t* bone, mstudiobbox_t* pHitbox, const vec& vAttackerEyePos, vec& vBestTargetPosOut, BaseEntity* pLocalPlayer, BaseEntity* pTarget, const qangle& qViewAngles) const
{
    vec vMinRotated; Maths::VectorTransform(pHitbox->bbmin, *bone, vMinRotated);
    vec vMaxRotated; Maths::VectorTransform(pHitbox->bbmax, *bone, vMaxRotated);
    vec vBoneOrigin = vMinRotated + ((vMaxRotated - vMinRotated) / 2.0f);

    float flBloomRadius = Features::Aimbot::AimbotHitscanV2::AimbotHitscan_PlayerBloom.GetData().m_flVal / 2.0f;
    return _MultipointVisibilityCheck(vBoneOrigin, pHitbox->bbmin, pHitbox->bbmax, qViewAngles, vBestTargetPosOut, pLocalPlayer, pTarget, flBloomRadius);
}


///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
bool AimbotHitscanV2_t::_MultipointVisibilityCheck(vec& vOrigin, vec& vMin, vec& vMax, const qangle& qViewAngles, vec& vTargetPosOut, BaseEntity* pLocalPlayer, BaseEntity* pTarget, float flBloomRadiusPerc) const
{
    // First we check if the origin is visible or not. If origin is not visible, then we go onto multipoints...
    vec vEyePos = pLocalPlayer->GetEyePos();
    ITraceFilter_IgnoreSpawnVisualizer filter(pLocalPlayer); trace_t trace;
    I::EngineTrace->UTIL_TraceRay(vEyePos, vOrigin, MASK_SHOT, &filter, &trace);
    // In case we are shooting @ backtracks, the entity pointer check won't do. 
    // Cause there is no entity there & our data is from stored backtrack information.
    if(trace.m_entity == pTarget || trace.m_fraction >= 0.99f) 
    {
        vTargetPosOut = vOrigin;
        return true;
    }


    // Split view angles into its components...
    vec vForward, vRight, vUp; Maths::AngleVectors(qViewAngles, &vForward, &vRight, &vUp);

    // vector in direction of localplayer's velocity
    vec vVelocity = pLocalPlayer->m_vecAbsVelocity();

    // Constucting a vector in direction of player's movement. I will offset hitpoint in this direction ( direction of player'd movement )
    // Cause the first hitpoint get is going to be visible will be in direction of player's movement.
    float flHorizontalVel = vVelocity.Dot(vRight);
    float flVerticalVel   = vVelocity.Dot(vUp);
    // NOTE : This vector below contains points relevant to 2d calculations, and not 3d calculations. ( if you know what I mean )
    vec vVelRelative(flHorizontalVel, flVerticalVel, 0.0f); vVelRelative.NormalizeInPlace();
    
    // If velocity is zero, then just set a default point to start checking...
    if (flHorizontalVel < 2.0f && flVerticalVel < 2.0f)
    {
        vVelRelative.x = 1.0f; vVelRelative.y = 0.0f;
    }


    int   nHitPoints = Features::Aimbot::AimbotHitscanV2::AimbotHitscan_Multipoint.GetData().m_iVal;
    float flGapAngle = (2.0f * M_PI) / static_cast<float>(nHitPoints);
    float flRadius   = vMax.DistTo(vMin) * flBloomRadiusPerc;

    for (int iHitpointIndex = 0; iHitpointIndex < nHitPoints; iHitpointIndex++)
    {
        float x = vVelRelative.x; float y = vVelRelative.y;

        // Construct hitpoint
        vec vHitpoint = vOrigin + vRight * (x * flRadius) + vUp * (y * flRadius);

#if (HITSCANAIMBOT_DEBUG_HITBOX == true)
        I::IDebugOverlay->AddBoxOverlay(vHitpoint, vec(1.0f), vec(-1.0f), qangle(0.0f), 0, 255, 0, 255, 5.0f);
#endif

        // visibility test this hitpoint...
        I::EngineTrace->UTIL_TraceRay(vEyePos, vHitpoint, MASK_SHOT, &filter, &trace);
        // In case we are shooting @ backtracks, the entity pointer check won't do. 
        // Cause there is no entity there & our data is from stored backtrack information.
        if (trace.m_entity == pTarget || trace.m_fraction >= 0.99f)
        {
            vTargetPosOut = vOrigin;
            return true;
        }

        // Modify hitpoint for next iteration... ( i.e. rotate it )
        float flCurrentAngle = atan2f(y, x);
        float flNewAngle     = flCurrentAngle + flGapAngle;
        vVelRelative.x = cosf(flNewAngle); vVelRelative.y = sinf(flNewAngle);
    }


    return false;
}


///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
void AimbotHitscanV2_t::_ResetAimbotData()
{
    m_pLastTarget   = m_pBestTarget;
    m_pBestTarget   = nullptr;
    m_vBestTargetPos.Init();
    m_iTickCount    = -1;
    m_iTargetHitbox = -1;
}
