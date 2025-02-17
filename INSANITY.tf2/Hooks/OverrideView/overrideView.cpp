#include "overrideView.h"

hook::overrideView::T_overrideView hook::overrideView::O_overrideView = nullptr;
int64_t __fastcall hook::overrideView::H_overrideView(void* VT_IClientMode, CViewSetup* pViewSetup) {

	pViewSetup->fov = 120.0f;
	return O_overrideView(VT_IClientMode, pViewSetup);
}