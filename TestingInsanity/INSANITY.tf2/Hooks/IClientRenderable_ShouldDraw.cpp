#include "../Utility/Hook Handler/Hook_t.h"
#include "../Utility/ConsoleLogging.h" 
#include "../Features/Entity Iterator/EntityIterator.h"
#include "../Features/Aimbot/AimbotHelper.h"

#include "../SDK/class/BaseWeapon.h"
#include "../SDK/class/BaseEntity.h"


///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
MAKE_HOOK(C_BaseCombatWeapon_ShouldDraw, "48 89 5C 24 ? 57 48 83 EC ? 83 B9 ? ? ? ? ? 48 8B D9 74 ? 8B 81", __fastcall, CLIENT_DLL, bool, I_client_renderable* pActiveWeapon)
{
    bool bPlayerSniper = F::entityIterator.GetLocalPlayerInfo().m_iClass == TF_SNIPER;

    if (Features::Misc::View::View_DrawModelWhileScoped.IsActive() == true && bPlayerSniper == true)
    {
        if (pActiveWeapon->GetBaseEntFromRenderable()->m_hOwnerEntity() == F::entityIterator.GetLocalPlayerInfo().m_refEHandle)
        {
            return true;
        }
    }

    return Hook::C_BaseCombatWeapon_ShouldDraw::O_C_BaseCombatWeapon_ShouldDraw(pActiveWeapon);
}


///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
MAKE_HOOK(C_TFPlayer_ShouldDraw, "48 89 74 24 ? 57 48 83 EC ? 48 8D 71", __fastcall, CLIENT_DLL, bool, I_client_renderable* pEnt)
{
    bool bPlayerSniper = F::entityIterator.GetLocalPlayerInfo().m_iClass == TF_SNIPER;

    if (Features::Misc::View::View_DrawModelWhileScoped.IsActive() == true && bPlayerSniper == true)
    {
        if(pEnt->GetBaseEntFromRenderable()->GetRefEHandle() == F::entityIterator.GetLocalPlayerInfo().m_refEHandle)
        {
            return true;
        }
    }

    return Hook::C_TFPlayer_ShouldDraw::O_C_TFPlayer_ShouldDraw(pEnt);
}


///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
MAKE_HOOK(C_TFWearable_ShouldDraw, "48 89 5C 24 ? 48 89 6C 24 ? 48 89 74 24 ? 48 89 7C 24 ? 41 54 41 56 41 57 48 83 EC ? 8B 91", __fastcall, CLIENT_DLL, bool, I_client_renderable* pWearable)
{
    bool bPlayerSniper = F::entityIterator.GetLocalPlayerInfo().m_iClass == TF_SNIPER;

    if (Features::Misc::View::View_DrawModelWhileScoped.IsActive() == true && bPlayerSniper == true)
    {
        if(pWearable->GetBaseEntFromRenderable()->m_hOwnerEntity() == F::entityIterator.GetLocalPlayerInfo().m_refEHandle)
        {
            return true;
        }
    }

    return Hook::C_TFWearable_ShouldDraw::O_C_TFWearable_ShouldDraw(pWearable);
}