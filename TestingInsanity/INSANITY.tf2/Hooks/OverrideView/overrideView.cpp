#include "overrideView.h"
#include "../../Features/config.h"
#include "../../SDK/class/Source Entity.h"
#include "../../SDK/Entity Manager/entityManager.h"

hook::overrideView::T_overrideView hook::overrideView::O_overrideView = nullptr;
int64_t __fastcall hook::overrideView::H_overrideView(void* VT_IClientMode, CViewSetup* pViewSetup) {

	if (entityManager.initialized.load() == false)
	{
		return O_overrideView(VT_IClientMode, pViewSetup);
	}

	if ((entityManager.getLocalPlayer()->getPlayerCond() & TF_COND_ZOOMED) == false)
	{
		pViewSetup->fov = config.viewConfig.FOV;
	}

	return O_overrideView(VT_IClientMode, pViewSetup);
}