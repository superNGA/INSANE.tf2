#include "../Utility/Hook Handler/Hook_t.h"
#include "../SDK/class/Basic Structures.h"
#include "../SDK/class/CMultAnimState.h"
#include "../SDK/class/BaseEntity.h"

#include "../Features/TickManip/TickManipHelper.h"
#include "../Features/TickManip/AntiAim/AntiAimV2.h"
#include "../Features/Entity Iterator/EntityIterator.h"


///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
MAKE_HOOK(C_TFPlayer_GetRenderAngles, "40 53 48 83 EC ? 48 8B D9 48 83 C1 ? E8 ? ? ? ? 84 C0 74 ? 48 8D 05", __fastcall, CLIENT_DLL, qangle*,
    I_client_renderable* pEnt)
{
    if (pEnt->GetBaseEntFromRenderable()->GetRefEHandle() == F::entityIterator.GetLocalPlayerInfo().m_refEHandle && // we only want to mess with local player's shit.
        F::tickManipHelper.CalculatingBones()             == false && // Don't alter anim state while we are calculating our antiaim bones.
        F::tickManipHelper.UseCustomBonesLocalPlayer()    == true)    // If we aren't even using custom real angles, then no messing with aniation state..
    {
        CMultiPlayerAnimState* pAnimState = *reinterpret_cast<CMultiPlayerAnimState**>((uintptr_t)pEnt->GetBaseEntFromRenderable() + Netvars::DT_TFPlayer::m_hItem - 88);
        qangle                 qAngles = F::antiAimV2.GetRealAngles();
        pAnimState->m_flCurrentFeetYaw = qAngles.yaw;
        pAnimState->Update(qAngles.yaw, qAngles.pitch);
    }


    return Hook::C_TFPlayer_GetRenderAngles::O_C_TFPlayer_GetRenderAngles(pEnt);
}