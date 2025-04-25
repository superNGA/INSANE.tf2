#include "../Utility/Hook_t.h"
#include "../Utility/ConsoleLogging.h"

#include "../Features/config.h"
#include "../SDK/class/Source Entity.h"
#include "../SDK/Entity Manager/entityManager.h"
#include "../SDK/class/viewSetup.h"

#include "../Features/Anti Aim/AntiAim.h"

//int64_t __fastcall hook::overrideView::H_overrideView(void* VT_IClientMode, CViewSetup* pViewSetup) 
MAKE_HOOK(OverrideView, "48 89 5C 24 ? 55 48 8D 6C 24 ? 48 81 EC ? ? ? ? 48 8B DA", __fastcall, CLIENT_DLL, int64_t, 
	void* pVTable, CViewSetup* pViewSetup)
{
	if (entityManager.initialized.load() == false)
		return Hook::OverrideView::O_OverrideView(pVTable, pViewSetup);

	if ((entityManager.getLocalPlayer()->getPlayerCond() & TF_COND_ZOOMED) == false)
	{
		pViewSetup->fov = config.viewConfig.FOV;
	}

	return Hook::OverrideView::O_OverrideView(pVTable, pViewSetup);
}