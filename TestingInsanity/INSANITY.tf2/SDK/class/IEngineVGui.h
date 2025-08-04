#pragma once

#include "vgui_panel.h"

#include "Basic Structures.h"
#include "../../Utility/Interface Handler/Interface.h"
#include "../../Utility/Signature Handler/signatures.h"


enum VGuiPanel_t
{
	PANEL_ROOT = 0,
	PANEL_GAMEUIDLL,
	PANEL_CLIENTDLL,
	PANEL_TOOLS,
	PANEL_INGAMESCREENS,
	PANEL_GAMEDLL,
	PANEL_CLIENTDLL_TOOLS,
	PANEL_COUNT
};


MAKE_SIG(CEngineVGui_GetRootPanel, "80 3D ? ? ? ? ? 4C 8B C1", ENGINE_DLL, Panel*, void*, VGuiPanel_t)

class IEngineVGui
{
public:
	virtual					~IEngineVGui(void) {}

	virtual vgui::VPANEL	GetPanel(VGuiPanel_t type) = 0;
	virtual bool			IsGameUIVisible() = 0;

	inline Panel* GetRootPanel(VGuiPanel_t type)
	{
		return Sig::CEngineVGui_GetRootPanel(this, type);
	}
};

MAKE_INTERFACE_VERSION(iEngineVGui, "VEngineVGui001", IEngineVGui, ENGINE_DLL)