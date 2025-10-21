#include "../Utility/Hook Handler/Hook_t.h"
#include "../Utility/Signature Handler/signatures.h"
#include "../Utility/ConsoleLogging.h"

#include "../SDK/class/BaseEntity.h"
#include "../SDK/class/IPanel.h"
#include "../SDK/class/ISurface.h"
#include "../SDK/class/IVEngineClient.h"

#include "../Features/FeatureHandler.h"


DEFINE_SECTION(Panels, "Misc", 3)
DEFINE_FEATURE(Panels_RemoveSniperScopeOverlay,  "Remove Scope Overlay",  bool, Panels, Misc, 1, false)
DEFINE_FEATURE(Panels_RemoveSniperChargeOverlay, "Remove Charge Overlay", bool, Panels, Misc, 2, false)


MAKE_HOOK(PaintTraverse, "48 89 5C 24 ? 57 48 83 EC ? 48 8B 01 41 0F B6 D9", __stdcall, VGUI2_DLL, int64_t,
	void* pVTable, vgui::VPANEL iPanel, uint8_t idk1, uint8_t idk2)
{
	// Don't fuck with it if not in-game.
	if(I::iEngine->IsInGame() == false)
	{
		return Hook::PaintTraverse::O_PaintTraverse(pVTable, iPanel, idk1, idk2);
	}
	
	auto it = IPanelHelper::m_mapTargetPanels.find(iPanel);

	// if not found in this map, then we don't need to do anything with this panel.
	if(it == IPanelHelper::m_mapTargetPanels.end())
	{
		return Hook::PaintTraverse::O_PaintTraverse(pVTable, iPanel, idk1, idk2);
	}
		

	// Remove tha scope overlay.
	if(Features::Misc::Panels::Panels_RemoveSniperScopeOverlay.IsActive() == true)
	{
		if (it->second == "HudScope")
		{
			I::iPanel->SetVisible(iPanel, false);
		}
	}


	// Remove tha sniper's charge overlay.
	if(Features::Misc::Panels::Panels_RemoveSniperChargeOverlay.IsActive() == true)
	{
		if (it->second == "HudScopeCharge")
		{
			I::iPanel->SetVisible(iPanel, false);
		}
	}

	return Hook::PaintTraverse::O_PaintTraverse(pVTable, iPanel, idk1, idk2);
}