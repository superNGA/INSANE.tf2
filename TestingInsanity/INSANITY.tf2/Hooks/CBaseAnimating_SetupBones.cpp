#include "../Utility/Hook Handler/Hook_t.h"
#include "../SDK/class/BaseEntity.h"
#include "../SDK/class/CMultAnimState.h"

#include "../Features/Entity Iterator/EntityIterator.h"
#include "../Features/TickManip/AntiAim/AntiAimV2.h"
#include "../Features/TickManip/TickManipHelper.h"



///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
MAKE_HOOK(SetupBones, "48 8B C4 44 89 40 ? 48 89 50 ? 55 53", __fastcall, CLIENT_DLL, void*,
    I_client_renderable* pRenderable, void* pBonesOut, int nMaxBones, int iMask, int iCurTime)
{
    // OBSERVATIONS : Setup bones is only calculated for player entities. Setup bones is only called for
    // weapon entites when we shoot, and its called about 7 to 8 times. Changing IClientRenderable::GetRenderAngles during
    // those setupBone calls will rotate our weapon in desired direction but for only those couple of frames, when 
    // weapon is drawn using those bones, else is uses its owner's bone matrix maybe.
    //

    // Check if we need to bone matrix for custom real angles?
    if (F::tickManipHelper.UseCustomBonesLocalPlayer() == false)
        return Hook::SetupBones::O_SetupBones(pRenderable, pBonesOut, nMaxBones, iMask, iCurTime);


    // if we calculating bones ourselves, don't shit on it.
    if (F::tickManipHelper.CalculatingBones() == true)
        return Hook::SetupBones::O_SetupBones(pRenderable, pBonesOut, nMaxBones, iMask, iCurTime);


    BaseEntity* pEnt = pRenderable->GetBaseEntFromRenderable();
    bool        bLocalPlayer = pEnt->GetRefEHandle() == F::entityIterator.GetLocalPlayerInfo().m_refEHandle;

    if (bLocalPlayer == true && pBonesOut != nullptr)
    {
        // Make ourselves look up.
        CMultiPlayerAnimState* pAnimState = *reinterpret_cast<CMultiPlayerAnimState**>((uintptr_t)pEnt + Netvars::DT_TFPlayer::m_hItem - 88);
        qangle                 qAngles = F::antiAimV2.GetRealAngles();
        pAnimState->Update(qAngles.yaw, qAngles.pitch);

        // Handle pose parameter index 1
        pEnt->GetPoseParameter()[1] = 0.5f;
    }

    return Hook::SetupBones::O_SetupBones(pRenderable, pBonesOut, nMaxBones, iMask, iCurTime);
}