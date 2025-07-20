#include "../Utility/Hook Handler/Hook_t.h"
#include "../Utility/ConsoleLogging.h"

#include "../Features/config.h"
#include "../SDK/class/BaseEntity.h"
#include "../SDK/Entity Manager/entityManager.h"
#include "../SDK/class/viewSetup.h"

#include "../Features/Anti Aim/AntiAim.h"
#include "../Features/FeatureHandler.h"
#include "../Features/Aimbot/AimbotHelper.h"


MAKE_HOOK(OverrideView, "48 89 5C 24 ? 55 48 8D 6C 24 ? 48 81 EC ? ? ? ? 48 8B DA", __fastcall, CLIENT_DLL, int64_t, 
	void* pVTable, CViewSetup* pViewSetup)
{
	BaseEntity* pLocalPlayer = entityManager.GetLocalPlayer();

	if(pLocalPlayer == nullptr)
		return Hook::OverrideView::O_OverrideView(pVTable, pViewSetup);

	if (pLocalPlayer->InCond(TF_COND_ZOOMED) == false)
	{
		pViewSetup->fov = Features::Misc::View::FOV.GetData().m_flVal;
	}

	return Hook::OverrideView::O_OverrideView(pVTable, pViewSetup);
}