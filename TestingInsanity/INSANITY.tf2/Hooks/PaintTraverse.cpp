#include "../Utility/Hook Handler/Hook_t.h"
#include "../Utility/Signature Handler/signatures.h"
#include "../Utility/ConsoleLogging.h"

//#include "../GlobalVars.h"
#include "../SDK/Entity Manager/entityManager.h"
#include "../SDK/class/BaseEntity.h"
#include "../Features/config.h"
//#include "../SDK/TF object manager/TFOjectManager.h"

MAKE_SIG(GetPanelName, "48 8B 02 48 8B CA 48 FF A0 ? ? ? ? CC CC CC 48 8B 02 48 8B CA 48 FF A0 ? ? ? ? CC CC CC 48 8B 02 48 8B CA 48 FF A0 ? ? ? ? CC CC CC 48 83 EC", VGUI2_DLL, 
	char*, void*, int64_t);

struct panelIndex_t {
	int64_t scopeOverlay = 0;
	int64_t chargeOverlay = 0;
};
panelIndex_t panelIndex;

// using restrict tells the compiler that this is the only pointer who is pointing to this memory, so 
// compilter optimizes it more. :) -Mr. smart pants
int64_t removePanel(void* pVTable, int64_t PANEL, const char* __restrict panelName, int64_t& panelIndex) {

	// CACHING index
	if (!panelIndex && !strcmp(Sig::GetPanelName(pVTable, PANEL), panelName)) {
		panelIndex = PANEL;
		LOG("Cached[% s] panel index : % lld", panelName ,PANEL);
	}

	// if Cached and matches desired index, return 0
	if (panelIndex && PANEL == panelIndex) {
		return 0;
	}

	// if no match found and 
	return PANEL;
}

MAKE_HOOK(PaintTraverse, "48 89 5C 24 ? 57 48 83 EC ? 48 8B 01 41 0F B6 D9", __stdcall, VGUI2_DLL, int64_t,
	void* pVTable, int64_t PANEL, uint8_t idk1, uint8_t idk2)
{
	// REMOVING SCOPE OVERLAY
	if (config.viewConfig.RemoveSniperScopeOverlay && entityManager.GetLocalPlayer()->m_iClass() == TF_SNIPER) {
		PANEL = removePanel(pVTable, PANEL, "HudScope", panelIndex.scopeOverlay);
	}

	// REMOVING CHARGE OVERLAY
	if (config.viewConfig.RemoveSniperChargeOverlay && entityManager.GetLocalPlayer()->m_iClass() == TF_SNIPER) {
		PANEL = removePanel(pVTable, PANEL, "HudScopeCharge", panelIndex.chargeOverlay);
	}

	
	if (PANEL) {
		return Hook::PaintTraverse::O_PaintTraverse(pVTable, PANEL, idk1, idk2);
	}
	return PANEL;
}