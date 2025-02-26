#include "PaintTraverse.h"
#include "../../GlobalVars.h"
#include "../../Features/config.h"
#include "../../SDK/TF object manager/TFOjectManager.h"

#ifdef _DEBUG
extern Console_System cons;
#endif // _DEBUG

struct panelIndex_t {
	int64_t scopeOverlay = 0;
	int64_t chargeOverlay = 0;
};
panelIndex_t panelIndex;

// using restrict tells the compiler that this is the only pointer who is pointing to this memory, so 
// compilter optimizes it more. :) -Mr. smart pants
int64_t removePanel(void* pVTable, int64_t PANEL, const char* __restrict panelName, int64_t& panelIndex) {

	// CACHING index
	if (!panelIndex && !strcmp(tfObject.getName(pVTable, PANEL), panelName)) {
		panelIndex = PANEL;
		#ifdef _DEBUG
		cons.Log(FG_GREEN, "PAINT TRAVERSE", "Cached [%s] panel index : %lld", panelName ,PANEL);
		#endif // _DEBUG
	}

	// if Cached and matches desired index, return 0
	if (panelIndex && PANEL == panelIndex) {
		return 0;
	}

	// if no match found and 
	return PANEL;
}

int64_t hook::paintTraverse::H_paintTraverse(void* pVTable, int64_t PANEL, uint8_t idk1, uint8_t idk2) {

	// REMOVING SCOPE OVERLAY
	if (config.viewConfig.RemoveSniperScopeOverlay && entities::local::localplayer_class.load() == TF_SNIPER) {
		PANEL = removePanel(pVTable, PANEL, "HudScope", panelIndex.scopeOverlay);
	}

	// REMOVING CHARGE OVERLAY
	if (config.viewConfig.RemoveSniperChargeOverlay && entities::local::localplayer_class.load() == TF_SNIPER) {
		PANEL = removePanel(pVTable, PANEL, "HudScopeCharge", panelIndex.chargeOverlay);
	}

	
	if (PANEL) {
		return hook::paintTraverse::O_paintTraverse(pVTable, PANEL, idk1, idk2);
	}
	return PANEL;
}