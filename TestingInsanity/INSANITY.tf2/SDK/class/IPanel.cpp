#include "IPanel.h"
#include "ISurface.h"
#include <unordered_set>
#include "../../Utility/ConsoleLogging.h"
#include "../../Features/ImGui/NotificationSystem/NotificationSystem.h"


///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
vgui::VPANEL IPanel::FindChildByName(vgui::VPANEL parent, const std::string& szChildName, bool bRecurse)
{
	// Sanity checks
	if (parent == NULL)
		return NULL;

	// parent is child ? ( LMAO )
	if (std::string(I::iPanel->GetName(parent)) == szChildName)
		return parent;

	// Child count
	int nChildren = I::iPanel->GetChildCount(parent);

	// No children ?
	if (nChildren == 0)
		return NULL;

	// Check all children for a match
	for (int iChildIndex = 0; iChildIndex < nChildren; iChildIndex++)
	{
		vgui::VPANEL child = I::iPanel->GetChild(parent, iChildIndex);

		// match found
		if (std::string(I::iPanel->GetName(child)) == szChildName)
			return child;
	}


	if (bRecurse == true)
	{
		for (int iChildIndex = 0; iChildIndex < nChildren; iChildIndex++)
		{
			vgui::VPANEL child = I::iPanel->GetChild(parent, iChildIndex);

			// recurse till we found match
			vgui::VPANEL childMatch = FindChildByName(child, szChildName, bRecurse);

			// if match found
			if (childMatch != NULL)
				return childMatch;
		}
	}

	return NULL;
}


///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
void IPanel::RefreshTargetPanelId()
{
	IPanelHelper::m_mapTargetPanels.clear();

	for (std::string& szPanelName : IPanelHelper::m_vecTargetPanels)
	{
		vgui::VPANEL iPanelID = FindChildByName(I::iSurface->GetEmbeddedPanel(), szPanelName.c_str(), true);

		if (iPanelID == NULL)
		{
			Render::notificationSystem.PushBack("Failed to find target panel [ %s ]", szPanelName.c_str());
			FAIL_LOG("Failed to find target panel [ %s ]", szPanelName.c_str());
			continue;
		}

		IPanelHelper::m_mapTargetPanels.insert({ iPanelID, szPanelName });

		LOG("Refreshed panel ID for [ %s ]", szPanelName.c_str());
		Render::notificationSystem.PushBack("Refreshed panel ID for [ %s ]", szPanelName.c_str());
	}
}
