//=========================================================================
//                      MELEE AIMBOT
//=========================================================================
// by      : INSANE
// created : 14/06/2025
// 
// purpose : Aims for you when using melee weapons, also contains backtrack auto backstab
//-------------------------------------------------------------------------

#include "AimbotMelee.h"
#include "../AimbotHelper.h"
#include "../../CritHack/CritHack.h"

#include <deque>
#include <algorithm>

// SDK
#include "../../../SDK/class/Source Entity.h"
#include "../../../SDK/class/BaseWeapon.h"
#include "../../../SDK/class/IVEngineClient.h"
#include "../../../SDK/class/IVDebugOverlay.h"
#include "../../../SDK/class/FileWeaponInfo.h"
#include "../../../SDK/class/CCollisionProperty.h"
#include "../../../SDK/class/Basic Structures.h"
#include "../../../SDK/class/CUserCmd.h"
#include "../../../SDK/class/IEngineTrace.h"
#include "../../../SDK/TF object manager/TFOjectManager.h"
#include "../../../SDK/class/CommonFns.h"
#include "../../../SDK/class/IVEngineServer.h"

// UTILITY
#include "../../Entity Iterator/EntityIterator.h"
#include "../../../Utility/CVar Handler/CVarHandler.h"
#include "../../../Utility/Hook Handler/Hook_t.h"
#include "../../../Utility/Profiler/Profiler.h"
#include "../../MovementSimulation/MovementSimulation.h"
#include "../../../Extra/math.h"
#include "../../ImGui/InfoWindowV2/InfoWindow.h"


#define DEBUG_BACKSTAB_CHECK false


///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
bool AimbotMelee_t::RunV3(BaseEntity* pLocalPlayer, baseWeapon* pActiveWeapon, CUserCmd* pCmd, bool* pCreatemoveResult)
{
    // NOTE : Output false means, no target has been found.
    //        Output true  means,  a target has been found.

    PROFILER_RECORD_FUNCTION(CreateMove);

    if (Features::Aimbot::Melee_Aimbot::MeleeAimbot.IsActive() == false)
        return false;

    // Don't do aimbot while cloaked.
    if (pLocalPlayer->m_iClass() == TF_SPY && pLocalPlayer->InCond(FLAG_playerCond::TF_COND_STEALTHED) == true)
        return false;

    // Detecting the best target.
    if (SDK::InAttack(pLocalPlayer, pActiveWeapon) == false)
    {
        m_pBestTarget = _ChooseTarget(pLocalPlayer, pActiveWeapon);
    }

    if (m_pBestTarget == nullptr)
        return false;

    // Auto firing.
    if (Features::Aimbot::Melee_Aimbot::MeleeAimbot_AutoFire.IsActive() == true && SDK::CanAttack(pLocalPlayer, pActiveWeapon) == true)
    {
        pCmd->buttons |= IN_ATTACK;
    }

    // SETTING AIMBOT ANGLES
    if(_ShouldAim(pLocalPlayer, pActiveWeapon, pCmd) == true)
    {
        // Calculating target's angles
        qangle qTargetBestAngles;
        Maths::VectorAnglesFromSDK(m_vBestTargetFuturePos - m_vAttackerFutureEyePos, qTargetBestAngles);

        LOG("Fired @ command [ %d ]. Difference %d", pCmd->tick_count, pCmd->tick_count - m_iBestTargetsTick);

        // Silent aim
        pCmd->viewangles   = qTargetBestAngles;
        pCmd->tick_count   = pLocalPlayer->m_iClass() == TF_SPY ? m_iBestTargetsTick : pCmd->tick_count;
        *pCreatemoveResult = false; // Setting silent aim

        // Reset everything once swing is done.
        Reset();
    }

    return true;
}


///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
bool AimbotMelee_t::_ShouldAim(BaseEntity* pAttacker, baseWeapon* pActiveWeapon, CUserCmd* pCmd)
{
    if (m_pBestTarget == nullptr)
        return false;

    float flCurTime = TICK_TO_TIME(pAttacker->m_nTickBase());
    
    // If not spy, we can just use the netvar :)
    if (pAttacker->m_iClass() != TF_SPY)
        return SDK::InAttack(pAttacker, pActiveWeapon) == true && flCurTime >= pActiveWeapon->m_flSmackTime();

    // If we are spy & trying to backstab, Aiming & Firing must be done on the same tick.
    // You will get a backstab as long as the angles are right.
    return SDK::CanAttack(pAttacker, pActiveWeapon);
}


///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
void AimbotMelee_t::Reset()
{
    m_vAttackerFutureEyePos.Init();
    m_vBestTargetFuturePos.Init();
    m_vBestTargetPosAtLock.Init();

    m_pBestTarget      = nullptr;
    m_iBestTargetsTick = 0;
}


///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
BaseEntity* AimbotMelee_t::_ChooseTarget(BaseEntity* pLocalPlayer, baseWeapon* pActiveWeapon)
{
    BaseEntity* pBestTarget  = nullptr;

    // First we will arrange the target list in a ascending order of distance from local player using quick sort. 
    // This theoratically should find us a target faster.
    auto* vecEnemyReadOnlyList = F::entityIterator.GetEnemyPlayers().GetReadBuffer();
    DOUBLEBUFFER_AUTO_RELEASE_READBUFFER(&F::entityIterator.GetEnemyPlayers(), vecEnemyReadOnlyList);

    if (vecEnemyReadOnlyList == nullptr)
        return nullptr;

    std::vector<BaseEntity*> vecTargets = *vecEnemyReadOnlyList;
    std::sort(vecTargets.begin(), vecTargets.end(), [&](BaseEntity* pEnt1, BaseEntity* pEnt2)->bool
        {
            return (pEnt1->GetAbsOrigin() - pLocalPlayer->GetAbsOrigin()).LengthSqrt() < (pEnt2->GetAbsOrigin() - pLocalPlayer->GetAbsOrigin()).LengthSqrt();
        });


    // Now we find a record / entity we can hit.
    pBestTarget = _ChooseTargetFromList(pLocalPlayer, pActiveWeapon, vecTargets);
    if (pBestTarget != nullptr)
        return pBestTarget;


    return nullptr;
}


///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
BaseEntity* AimbotMelee_t::_ChooseTargetFromList(BaseEntity* pLocalPlayer, baseWeapon* pActiveWeapon, const std::vector<BaseEntity*>& vecTargets)
{
    float flSmackDelay = pActiveWeapon->GetTFWeaponInfo()->GetWeaponData(0)->m_flSmackDelay;
    float flSwingRange = _GetSwingHullRange(pLocalPlayer, pActiveWeapon);

    if (pLocalPlayer->m_iClass() != TF_SPY)
    {
        float       flBestDistance = std::numeric_limits<float>::infinity();
        BaseEntity* pBestTarget    = nullptr;

        int nTicksToSimulate = 0;
        //nTicksToSimulate += TIME_TO_TICK(Maths::MAX<float>(CVars::cl_interp, CVars::cl_interp_ratio / static_cast<float>(CVars::cl_updaterate))); // lerp.
        nTicksToSimulate += TIME_TO_TICK(flSmackDelay); // Smack delay.

        // Getting local player's future eye pos.
        vec vFutureEyePos;
        F::movementSimulation.Initialize(pLocalPlayer, false); // strafe prediction on attacker or target seems to work worse when simulating such small durations.
        for (int iTick = 0; iTick < nTicksToSimulate; iTick++)
        {
            F::movementSimulation.RunTick();
        }
        vFutureEyePos = F::movementSimulation.GetSimulationPos() + pLocalPlayer->m_vecViewOffset();
        F::movementSimulation.Restore();

        // Finding most smackable target.
        for (BaseEntity* pTarget : vecTargets)
        {
            // Get target's future pos.
            vec vTargetFuturePos;
            F::movementSimulation.Initialize(pTarget, false);
            for (int iTick = 0; iTick < nTicksToSimulate; iTick++)
            {
                F::movementSimulation.RunTick();
            }
            vTargetFuturePos = F::movementSimulation.GetSimulationPos();
            F::movementSimulation.Restore();

            
            // Too far away ?
            const vec vBestPosOnTarget = _GetClosestPointOnEntity(pLocalPlayer, vFutureEyePos, pTarget, vTargetFuturePos);
            
            float flDistance = vFutureEyePos.DistTo(vBestPosOnTarget);
            if (flDistance >= flSwingRange)
                continue;

            // FOV check.
            if (_IsInFOV(pLocalPlayer, vFutureEyePos, vTargetFuturePos, Features::Aimbot::Melee_Aimbot::MeleeAimbot_FOV.GetData().m_flVal) == false)
                continue;

            // Is target visible.
            trace_t trace;
            ITraceFilter_IgnoreSpawnVisualizer filter(pLocalPlayer);
            I::EngineTrace->UTIL_TraceRay(vFutureEyePos, vTargetFuturePos, MASK_SHOT, &filter, &trace);
            if (trace.m_fraction < 1.0f && trace.m_entity != pTarget)
                continue;

            // Store best target.
            if(flDistance < flBestDistance)
            {
                flBestDistance          = flDistance;

                pBestTarget             = pTarget;
                m_vBestTargetFuturePos  = vTargetFuturePos;
                m_vAttackerFutureEyePos = vFutureEyePos;
            }
        }

        return pBestTarget;
    }
    else // SPY
    {
        vec   vAttackerEyePos = pLocalPlayer->GetEyePos();
        float flBackTrackTime = F::entityIterator.GetBackTrackTimeInSec();
        for (BaseEntity* pTarget : vecTargets)
        {
            std::deque<BackTrackRecord_t>* pRecords = F::entityIterator.GetBackTrackRecord(pTarget);
            if (pRecords == nullptr)
                continue;

            if (pRecords->size() <= 0)
                continue;

            int iStartRecordIndex = std::clamp<int>(TIME_TO_TICK(flBackTrackTime - MAX_BACKTRACK_TIME), 0, pRecords->size() - 1);
            int iFinalRecordIndex = std::clamp<int>(TIME_TO_TICK(flBackTrackTime),                      0, pRecords->size() - 1);

            for (int iRecordIndex = iStartRecordIndex; iRecordIndex <= iFinalRecordIndex; iRecordIndex++)
            {
                BackTrackRecord_t& record = (*pRecords)[iRecordIndex];
                
                vec vBestPointOnTarget = _GetClosestPointOnEntity(pLocalPlayer, vAttackerEyePos, pTarget, record.m_vOrigin);
                if (vAttackerEyePos.DistTo(vBestPointOnTarget) >= flSwingRange)
                    continue;

                // if we are standing inside the backtrack's Bounding Box, its sometimes doesn't register backstab.
                // So we are doing a little collision check kinda thing here. This should tell us if we inside target's BB.
                if (vBestPointOnTarget == vAttackerEyePos)
                    continue;

                if (_CanBackStab(pLocalPlayer, pTarget, record.m_vOrigin, record.m_qViewAngles) == false)
                    continue;

                LOG("Found target backtrack @ IDX : %d ( Total record : %d ) | Angles : %.2f %.2f", 
                    iRecordIndex, pRecords->size(), record.m_qViewAngles.pitch, record.m_qViewAngles.yaw);

                m_vAttackerFutureEyePos = vAttackerEyePos;
                m_vBestTargetFuturePos  = vBestPointOnTarget;
                m_pBestTarget           = pTarget;
                m_iBestTargetsTick      = record.m_iTick;
                return pTarget;
            }
        }
    }
    

    return nullptr;
}


///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
const vec AimbotMelee_t::_GetClosestPointOnEntity(BaseEntity* pLocalPlayer, const BaseEntity* pEnt) const
{
    auto* pCollidable  = pEnt->GetCollideable();

    // Hull Min & Max in World-Space
    const vec vHullMin = pCollidable->GetCollisionOrigin() + pCollidable->OBBMins();
    const vec vHullMax = pCollidable->GetCollisionOrigin() + pCollidable->OBBMaxs();

    // Shooting origin
    const vec vEyePos  = pLocalPlayer->GetEyePos();

    vec vClosestPoint;
    vClosestPoint.x = std::clamp(vEyePos.x, vHullMin.x, vHullMax.x);
    vClosestPoint.y = std::clamp(vEyePos.y, vHullMin.y, vHullMax.y);
    vClosestPoint.z = std::clamp(vEyePos.z, vHullMin.z, vHullMax.z);

    return vClosestPoint;
}


///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
const vec AimbotMelee_t::_GetClosestPointOnEntity(BaseEntity* pAttacker, const vec& vAttackerEyePos, const BaseEntity* pTarget, const vec& vTargetOrigin) const
{
    auto* pCollidable = pTarget->GetCollideable();

    // Hull Min & Max in World-Space
    const vec vHullMin = vTargetOrigin + pCollidable->OBBMins();
    const vec vHullMax = vTargetOrigin + pCollidable->OBBMaxs();

    // Shooting origin
    const vec vEyePos = vAttackerEyePos;

    vec vClosestPoint;
    vClosestPoint.x = std::clamp(vEyePos.x, vHullMin.x, vHullMax.x);
    vClosestPoint.y = std::clamp(vEyePos.y, vHullMin.y, vHullMax.y);
    vClosestPoint.z = std::clamp(vEyePos.z, vHullMin.z, vHullMax.z);

    return vClosestPoint;
}


///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
float AimbotMelee_t::_GetLooseSwingRange(BaseEntity* pLocalPlayer, baseWeapon* pActiveWeapon)
{
    float flSwingRange = 0.0f;

    if (pLocalPlayer->InCond(TF_COND_SHIELD_CHARGE) == true)
    {
        flSwingRange = 128.0f;
    }
    else
    {
        int iIsSword = 0;
        pActiveWeapon->CALL_ATRIB_HOOK_INT(iIsSword, "is_a_sword");
        
        if (iIsSword == 0)
            flSwingRange = 48.0f;
        else
            flSwingRange = 72.0f;
    }

    return flSwingRange;
}


///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
float AimbotMelee_t::_GetSwingHullRange(BaseEntity* pLocalPlayer, baseWeapon* pActiveWeapon)
{
    constexpr float flSwingMaxBase = 18.0f;
    float flBoundScale = 1.0f;
    pActiveWeapon->CALL_ATRIB_HOOK_FLOAT(flBoundScale, "melee_bounds_multiplier");
    float flSwingMax = flSwingMaxBase * flBoundScale;

    // Compensating for Model Scale
    float flSwingRange = _GetLooseSwingRange(pLocalPlayer, pActiveWeapon);
    float flModelScale = pLocalPlayer->m_flModelScale();
    if (flModelScale > 1.0f)
        flSwingRange *= flModelScale;

    // Adding Atribute Melee range
    pActiveWeapon->CALL_ATRIB_HOOK_FLOAT(flSwingRange, "melee_range_multiplier");
    
    // Since melee swing is traced using a hull, we gonna have to add this too.
    // Imagine, flSwingRange as distance centers of 2 boxes, ( 2 boxes means the min & max of the hull )
    // and flSwingMax is the distance between the center of the box to the surface.
    flSwingRange += flSwingMax;
    return flSwingRange;
}


///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
bool AimbotMelee_t::_CanBackStab(BaseEntity* pAttacker, BaseEntity* pTarget, const vec& vTargetOrigin, const qangle& qTargetAngles)
{
    auto* pAttackerCollidable = pAttacker->GetCollideable();
    auto* pTargetCollidable = pTarget->GetCollideable();
    
    vec vAttackerOrigin = pAttackerCollidable->GetCollisionOrigin();

    // Vector connecting Attacker to Taret
    vec vAttackerToTarget = vTargetOrigin - vAttackerOrigin;
    vAttackerToTarget.z = 0.0f;
    vAttackerToTarget.NormalizeInPlace();

    // Target's view angle vector
    vec vTargetViewAngles;
    Maths::AngleVectors(qTargetAngles, &vTargetViewAngles);
    vTargetViewAngles.z = 0.0f;
    vTargetViewAngles.NormalizeInPlace();
    
#if (DEBUG_BACKSTAB_CHECK == true)
    I::IDebugOverlay->AddLineOverlay(vAttackerOrigin, vAttackerOrigin + (vAttackerToTarget * 100.0f), 255, 0, 0, false, 3.0f);
    I::IDebugOverlay->AddLineOverlay(vTargetOrigin, vTargetOrigin + (vTargetViewAngles * 100.0f), 0, 255, 0, false, 3.0f);
#endif

    float flDotProduct = vAttackerToTarget.x * vTargetViewAngles.x + vAttackerToTarget.y * vTargetViewAngles.y;

    // return flDotProduct > 0.0f; // Ideally it should be just greater than 0.0 for a backstab, but to be on the safer side, it shall be more than 0.05f
    return flDotProduct > 0.05f;
}


///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
bool AimbotMelee_t::_IsInFOV(BaseEntity* pAttacker, const vec& vAttackerPos, const vec& vTargetPos, float FOV)
{
    vec vAttackerToTarget = vTargetPos - vAttackerPos;
    vAttackerToTarget.NormalizeInPlace();

    vec vAttackerAngles;
    qangle qAttackerAngles = pAttacker->m_angEyeAngles();
    Maths::AngleVectors(qAttackerAngles, &vAttackerAngles);
    vAttackerAngles.NormalizeInPlace();

    float flDot       = vAttackerAngles.Dot(vAttackerToTarget);
    float flTargetFOV = RAD2DEG(acosf(flDot));

    return FOV > flTargetFOV;
}


//=========================================================================
//                     DEBUG HOOKS
//=========================================================================
MAKE_HOOK(StartLagComp, "40 55 57 41 55 48 81 EC", __fastcall, SERVER_DLL, void*, void* pThis, BaseEntity* pEnt, CUserCmd* pCmd)
{
    if (pEnt == nullptr || pCmd == nullptr)
    {
        FAIL_LOG("Bad inputs!!");
        return Hook::StartLagComp::O_StartLagComp(pThis, pEnt, pCmd);
    }

    int64_t iEntIndex = *(int64_t*)(reinterpret_cast<uintptr_t>(pEnt) + 48);
    int iPlayerIndex;
    if (iEntIndex)
    {
        iPlayerIndex = (unsigned int)*(__int16*)(iEntIndex + 6);// IDK What the fuck is happening here
    }
    else
    {
        iPlayerIndex = 0LL;
    }

    LOG("PlayerIndex : %d", iPlayerIndex);
    
    auto* pNetInfo = I::iVEngineServer->GetPlayerNetInfo(iPlayerIndex);
    if (pNetInfo == nullptr)
    {
        FAIL_LOG("Bad net info. fuck you tf2");
        return Hook::StartLagComp::O_StartLagComp(pThis, pEnt, pCmd);
    }

    float flPing = pNetInfo->GetLatency(FLOW_OUTGOING);

    LOG("Player Ping : %.2f ms. Player tick count : %d. Player Attack status : [ %s ]", 
        flPing * 1000, pCmd->tick_count, pCmd->buttons & IN_ATTACK ? "ATTACKING" : "not attacking");

    return Hook::StartLagComp::O_StartLagComp(pThis, pEnt, pCmd);
}
