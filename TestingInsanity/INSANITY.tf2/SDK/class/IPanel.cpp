#include "IPanel.h"


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
