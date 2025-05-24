#pragma once
#include "../../Utility/Interface.h"
#include "IBaseInterface.h"

class IPanel : public IBaseInterface
{
public:
	virtual void Init(int vguiPanel, void* panel) = 0;

	// methods
	virtual void SetPos(int vguiPanel, int x, int y) = 0;
	virtual void GetPos(int vguiPanel, int& x, int& y) = 0;
	virtual void SetSize(int vguiPanel, int wide, int tall) = 0;
	virtual void GetSize(int vguiPanel, int& wide, int& tall) = 0;
	virtual void SetMinimumSize(int vguiPanel, int wide, int tall) = 0;
	virtual void GetMinimumSize(int vguiPanel, int& wide, int& tall) = 0;
	virtual void SetZPos(int vguiPanel, int z) = 0;
	virtual int  GetZPos(int vguiPanel) = 0;

	virtual void GetAbsPos(int vguiPanel, int& x, int& y) = 0;
	virtual void GetClipRect(int vguiPanel, int& x0, int& y0, int& x1, int& y1) = 0;
	virtual void SetInset(int vguiPanel, int left, int top, int right, int bottom) = 0;
	virtual void GetInset(int vguiPanel, int& left, int& top, int& right, int& bottom) = 0;

	virtual void SetVisible(int vguiPanel, bool state) = 0;
	virtual bool IsVisible(int vguiPanel) = 0;
	virtual void SetParent(int vguiPanel, int newParent) = 0;
	virtual int GetChildCount(int vguiPanel) = 0;
	virtual int GetChild(int vguiPanel, int index) = 0;
	virtual void GetChildren(int vguiPanel) = 0;
	virtual int GetParent(int vguiPanel) = 0;
	virtual void MoveToFront(int vguiPanel) = 0;
	virtual void MoveToBack(int vguiPanel) = 0;
	virtual bool HasParent(int vguiPanel, int potentialParent) = 0;
	virtual bool IsPopup(int vguiPanel) = 0;
	virtual void SetPopup(int vguiPanel, bool state) = 0;
	virtual bool IsFullyVisible(int vguiPanel) = 0;

	// gets the scheme this panel uses
	virtual void GetScheme(int vguiPanel) = 0;
	// gets whether or not this panel should scale with screen resolution
	virtual bool IsProportional(int vguiPanel) = 0;
	// returns true if auto-deletion flag is set
	virtual bool IsAutoDeleteSet(int vguiPanel) = 0;
	// deletes the Panel * associated with the int
	virtual void DeletePanel(int vguiPanel) = 0;

	// input interest
	virtual void SetKeyBoardInputEnabled(int vguiPanel, bool state) = 0;
	virtual void SetMouseInputEnabled(int vguiPanel, bool state) = 0;
	virtual bool IsKeyBoardInputEnabled(int vguiPanel) = 0;
	virtual bool IsMouseInputEnabled(int vguiPanel) = 0;

	// calculates the panels current position within the hierarchy
	virtual void Solve(int vguiPanel) = 0;

	// gets names of the object (for debugging purposes)
	virtual const char* GetName(int vguiPanel) = 0;
	virtual const char* GetClassName(int vguiPanel) = 0;

	// delivers a message to the panel
	virtual void SendMessage(int vguiPanel, void* params, int ifromPanel) = 0;

	// these pass through to the IClientPanel
	virtual void Think(int vguiPanel) = 0;
	virtual void PerformApplySchemeSettings(int vguiPanel) = 0;
	virtual void PaintTraverse(int vguiPanel, bool forceRepaint, bool allowForce = true) = 0;
	virtual void Repaint(int vguiPanel) = 0;
	virtual int IsWithinTraverse(int vguiPanel, int x, int y, bool traversePopups) = 0;
	virtual void OnChildAdded(int vguiPanel, int child) = 0;
	virtual void OnSizeChanged(int vguiPanel, int newWide, int newTall) = 0;

	virtual void InternalFocusChanged(int vguiPanel, bool lost) = 0;
	virtual bool RequestInfo(int vguiPanel, void* outputData) = 0;
	virtual void RequestFocus(int vguiPanel, int direction = 0) = 0;
	virtual bool RequestFocusPrev(int vguiPanel, int existingPanel) = 0;
	virtual bool RequestFocusNext(int vguiPanel, int existingPanel) = 0;
	virtual int GetCurrentKeyFocus(int vguiPanel) = 0;
	virtual int GetTabPosition(int vguiPanel) = 0;

	// used by ISurface to store platform-specific data
	virtual void* Plat(int vguiPanel) = 0;
	virtual void SetPlat(int vguiPanel, void* Plat) = 0;

	// returns a pointer to the vgui controls baseclass Panel *
	// destinationModule needs to be passed in to verify that the returned Panel * is from the same module
	// it must be from the same module since Panel * vtbl may be different in each module
	virtual void* GetPanel(int vguiPanel, const char* destinationModule) = 0;

	virtual bool IsEnabled(int vguiPanel) = 0;
	virtual void SetEnabled(int vguiPanel, bool state) = 0;

	// Used by the drag/drop manager to always draw on top
	virtual bool IsTopmostPopup(int vguiPanel) = 0;
	virtual void SetTopmostPopup(int vguiPanel, bool state) = 0;

	// sibling pins
	virtual void SetSiblingPin(int vguiPanel, int newSibling, byte iMyCornerToPin = 0, byte iSiblingCornerToPinTo = 0) = 0;
};

MAKE_INTERFACE_VERSION(iPanel, "VGUI_Panel009", IPanel, VGUI2_DLL)