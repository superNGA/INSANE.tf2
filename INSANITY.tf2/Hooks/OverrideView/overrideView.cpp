#include "overrideView.h"
#include "../../Features/config.h"

hook::overrideView::T_overrideView hook::overrideView::O_overrideView = nullptr;
int64_t __fastcall hook::overrideView::H_overrideView(void* VT_IClientMode, CViewSetup* pViewSetup) {

	pViewSetup->fov = config.viewConfig.FOV;
	return O_overrideView(VT_IClientMode, pViewSetup);
}