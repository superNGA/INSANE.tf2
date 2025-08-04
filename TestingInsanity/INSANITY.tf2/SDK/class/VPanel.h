#pragma once

#include "Basic Structures.h"
class Panel;

class VPanel
{
public:
	VPanel();
	virtual ~VPanel();

	virtual void Init(Panel* attachedClientPanel);

	virtual void* Plat();
	virtual void SetPlat(void* pl);

	virtual int GetHPanel() { return _hPanel; } // safe pointer handling
	virtual void SetHPanel(int hPanel) { _hPanel = hPanel; }

	virtual bool IsPopup();
	virtual void SetPopup(bool state);
	virtual bool IsFullyVisible();

	virtual void SetPos(int x, int y);
	virtual void GetPos(int& x, int& y);
	virtual void SetSize(int wide, int tall);
	virtual void GetSize(int& wide, int& tall);
	virtual void SetMinimumSize(int wide, int tall);
	virtual void GetMinimumSize(int& wide, int& tall);
	virtual void SetZPos(int z);
	virtual int  GetZPos();

	virtual void GetAbsPos(int& x, int& y);
	virtual void GetClipRect(int& x0, int& y0, int& x1, int& y1);
	virtual void SetInset(int left, int top, int right, int bottom);
	virtual void GetInset(int& left, int& top, int& right, int& bottom);

	virtual void Solve();

	virtual void SetVisible(bool state);
	virtual void SetEnabled(bool state);
	virtual bool IsVisible();
	virtual bool IsEnabled();
	virtual void SetParent(VPanel* newParent);
	virtual int GetChildCount();
	virtual VPanel* GetChild(int index);
	virtual VPanel* GetParent();
	virtual void MoveToFront();
	virtual void MoveToBack();
	virtual bool HasParent(VPanel* potentialParent);

	virtual void* GetChildren();

	// gets names of the object (for debugging purposes)
	virtual const char* GetName();
	virtual const char* GetClassName();

	virtual int GetScheme();

	// handles a message
	virtual void SendMessage(void* params, vgui::VPANEL ifromPanel);

	// wrapper to get Client panel interface
	virtual Panel* Client() const = 0;

	// input interest
	virtual void SetKeyBoardInputEnabled(bool state);
	virtual void SetMouseInputEnabled(bool state);
	virtual bool IsKeyBoardInputEnabled();
	virtual bool IsMouseInputEnabled();

	virtual bool IsTopmostPopup() const;
	virtual void SetTopmostPopup(bool bEnable);

	// sibling pins
	virtual void SetSiblingPin(VPanel* newSibling, byte iMyCornerToPin = 0, byte iSiblingCornerToPinTo = 0);

public:
	virtual void GetInternalAbsPos(int& x, int& y);
	virtual void TraverseLevel(int val);

	/*Dar<VPanel*> _childDar;*/ char m_padding[20];
	VPanel* _parent;
	void* _plat;	// platform-specific data
	vgui::HPANEL _hPanel;

	// our companion Client panel
	Panel* _clientPanel;

	short _pos[2];
	short _size[2];
	short _minimumSize[2];

	short _inset[4];
	short _clipRect[4];
	short _absPos[2];

	short _zpos;	// z-order position

	bool _visible : 1;
	bool _enabled : 1;
	bool _popup : 1;
	bool _mouseInput : 1; // used for popups
	bool _kbInput : 1;
	bool _isTopmostPopup : 1;

	VPanel* _pinsibling;
	byte	_pinsibling_my_corner;
	byte	_pinsibling_their_corner;

	int		m_nMessageContextId;
	int		m_nThinkTraverseLevel;
	int		_clientPanelHandle; // Temp to check if _clientPanel is valid.
};
