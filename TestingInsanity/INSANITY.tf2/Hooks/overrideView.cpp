#include "../Utility/Hook Handler/Hook_t.h"
#include "../Utility/ConsoleLogging.h"

#include "../Features/config.h"
#include "../SDK/class/BaseEntity.h"
#include "../SDK/class/viewSetup.h"

#include "../Features/Anti Aim/AntiAim.h"
#include "../Features/FeatureHandler.h"
#include "../Features/Aimbot/AimbotHelper.h"
#include "../Features/Entity Iterator/EntityIterator.h"


MAKE_HOOK(OverrideView, "48 89 5C 24 ? 55 48 8D 6C 24 ? 48 81 EC ? ? ? ? 48 8B DA", __fastcall, CLIENT_DLL, int64_t, 
	void* pVTable, CViewSetup* pViewSetup)
{
	const LocalPlayerInfo_t& localPlayerInfo = F::entityIterator.GetLocalPlayerInfo();

	if (localPlayerInfo.m_iClass == TF_SNIPER && (localPlayerInfo.m_iCond & TF_COND_ZOOMED) == true)
	{
		pViewSetup->fov = Features::Misc::View::View_ScopedInFOV.GetData().m_flVal;
	}
	else
	{
		pViewSetup->fov = Features::Misc::View::View_FOV.GetData().m_flVal;
	}

	F::aimbotHelper.NotifyGameFOV(pViewSetup->fov);

	return Hook::OverrideView::O_OverrideView(pVTable, pViewSetup);
}