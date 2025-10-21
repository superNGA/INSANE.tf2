#include "AimbotHitscan.h"

// SDK
#include "../../../SDK/class/BaseEntity.h"
#include "../../../SDK/class/BaseWeapon.h"
#include "../../../SDK/class/CommonFns.h"
#include "../../../SDK/class/CUserCmd.h"
#include "../../../SDK/TF object manager/TFOjectManager.h"
#include "../../../SDK/class/IVModelInfo.h"
#include "../../../SDK/class/IVDebugOverlay.h"
#include "../../../SDK/class/IEngineTrace.h"

// UTILITY
#include "../../../Extra/math.h"
#include "../../../Utility/Signature Handler/signatures.h"
#include "../../../Utility/Profiler/Profiler.h"

#include "../AimbotHelper.h"


MAKE_SIG(CBaseAnimatinng_LookUpBones, "40 53 48 83 EC ? 48 8B DA E8 ? ? ? ? 48 8B C8 48 8B D3 48 83 C4 ? 5B E9 ? ? ? ? CC CC 48 89 74 24", CLIENT_DLL, int64_t, void*, const char*)


bool AimbotHitscan_t::Run(BaseEntity* pLocalPlayer, baseWeapon* pActiveWeapon, CUserCmd* pCmd, bool* bCreateMoveResult)
{
    PROFILER_RECORD_FUNCTION(CreateMove);

    if (Features::Aimbot::HitscanAimbot::Enable.IsActive() == false)
        return false;

    // Getting Target.
    bool bInAttack = SDK::InAttack(pLocalPlayer, pActiveWeapon);
    if (bInAttack == false)
    {
        m_pBestTarget = _GetBestTarget(pLocalPlayer, pCmd);
    }

    // Ain't got no target to attack
    if (m_pBestTarget == nullptr)
        return false;

    // Wanna Auto-Fire ?
    if (SDK::CanAttack(pLocalPlayer, pActiveWeapon) == true && Features::Aimbot::HitscanAimbot::AutoFire.IsActive() == true)
    {
        pCmd->buttons |= IN_ATTACK;
    }

    // Aimbotting
    bool bShooting = (pCmd->buttons & IN_ATTACK) && SDK::CanAttack(pLocalPlayer, pActiveWeapon);
    if (bShooting == true)
    {
        // Constructing aimbot angles
        qangle qAimbotAngles; Maths::VectorAnglesFromSDK(m_vBestPos - pLocalPlayer->GetEyePos(), qAimbotAngles);
        Maths::WrapYaw(qAimbotAngles);

        // Aimbotting
        pCmd->viewangles = qAimbotAngles;
        *bCreateMoveResult = false; // Sileting angle change.

        // resetting target data.
        m_pBestTarget = nullptr; m_vBestPos.Init();

        // We should miss all the shots right?
        if(Features::Aimbot::HitscanAimbot::ForceFail_BackTrack.IsActive() == true)
        {
            pCmd->tick_count -= TIME_TO_TICK(0.190f);
        }
    }

    return true;
}


BaseEntity* AimbotHitscan_t::_GetBestTarget(BaseEntity* pLocalPlayer, CUserCmd* pCmd)
{
    const std::vector<BaseEntity*>& vecTargets = F::aimbotHelper.GetAimbotTargetData().m_vecEnemyPlayers;

    vec         vLocalPlayerEyePos = pLocalPlayer->GetEyePos();

    BaseEntity* pBestTarget     = nullptr;
    float       flAngBestDist   = std::numeric_limits<float>::infinity();
    matrix3x4_t pBones[MAX_STUDIO_BONES]{ 0 };

    for (BaseEntity* pTarget : vecTargets)
    {
        // Model
        const model_t* pModel = pTarget->GetModel(); 
        if (pModel == nullptr)
            continue;

        // Studio model for this entities model
        const StudioHdr_t* pStudioModel = I::iVModelInfo->GetStudiomodel(pModel);
        if (pStudioModel == nullptr)
            continue;

        // Hit boxes for this studio model.
        const auto* pHitBoxSet = pStudioModel->pHitboxSet(pTarget->m_nHitboxSet());
        if (pHitBoxSet == nullptr)
            continue;

        // Getting bones for this entity.
        if (pTarget->SetupBones(pBones, MAX_STUDIO_BONES, HITBOX_BONES, CUR_TIME) == false)
            continue;

        int64_t iHeadBoneIndex = Sig::CBaseAnimatinng_LookUpBones(pTarget, "bip_head");
        vec     vTargetBonePos = pBones[iHeadBoneIndex].GetWorldPos(); 

        // Out of FOV
        float flAngDist = _GetAngleFromCrosshair(vTargetBonePos, vLocalPlayerEyePos, pCmd->viewangles);
        if (flAngDist > Features::Aimbot::HitscanAimbot::FOV.GetData().m_flVal)
            continue;

        // Tracing to target
        ITraceFilter_IgnoreSpawnVisualizer filter(pLocalPlayer); trace_t trace;
        I::EngineTrace->UTIL_TraceRay(vLocalPlayerEyePos, vTargetBonePos, MASK_SHOT, &filter, &trace);

        // Can't hit this target.
        if (trace.m_fraction < 0.99f && trace.m_entity != pTarget)
            continue;

        // Comparing against our best found target.
        if (flAngDist < flAngBestDist)
        {
            flAngBestDist = flAngDist;
            pBestTarget   = pTarget;
            m_vBestPos    = vTargetBonePos;
        }
    }

    return pBestTarget;
}


const float AimbotHitscan_t::_GetAngleFromCrosshair(const vec& vTargetPos, const vec& vAttackerEyePos, const qangle& qViewAngles) const
{
    vec vViewAngles;
    Maths::AngleVectors(qViewAngles, &vViewAngles);
    vViewAngles.NormalizeInPlace();

    vec vAttackerToTarget = vTargetPos - vAttackerEyePos;
    vAttackerToTarget.NormalizeInPlace();

    float flDot = vViewAngles.Dot(vAttackerToTarget);
    return RAD2DEG(acosf(flDot));
}