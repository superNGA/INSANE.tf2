#pragma once

#include "../../Utility/Interface Handler/Interface.h"

class Panel;

class IViewPort
{
public:
	virtual void UpdateAllPanels(void) = 0;
	virtual void ShowPanel(const char* pName, bool state) = 0;
	virtual void ShowPanel(Panel* pPanel, bool state) = 0;
	virtual void ShowBackGround(bool bShow) = 0;
	virtual Panel* FindPanelByName(const char* szPanelName) = 0;
	virtual Panel* GetActivePanel(void) = 0;
	virtual void PostMessageToPanel(const char* pName, void* pKeyValues) = 0;
};

MAKE_INTERFACE_SIGNATURE(iViewPort, "48 8B 0D ? ? ? ? 48 8D 15 ? ? ? ? 48 8B 01 FF 50 ? 48 85 C0 74 ? 48 8B 10 48 8B C8 FF 52 ? 84 C0 75 ? E8", IViewPort, CLIENT_DLL, 3, 7)