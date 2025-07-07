#pragma once

#include "../../Utility/Interface Handler/Interface.h";

class IInput
{
public:
    
    virtual ~IInput() = 0;// <-- This actually accounts for IBaseInterface's virtual destructor.

	virtual void SetMouseFocus(int newMouseFocus) = 0;
	virtual void SetMouseCapture(int panel) = 0;

	// returns the string name of a scan code
	virtual void GetintText(int code, int buflen) = 0;

	// focus
	virtual int GetFocus() = 0;
	virtual int GetCalculatedFocus() = 0;// to handle cases where the focus changes inside a frame.
	virtual int GetMouseOver() = 0;		// returns the panel the mouse is currently over, ignoring mouse capture

	// mouse state
	virtual void SetCursorPos(int x, int y) = 0;
	virtual void GetCursorPos(int& x, int& y) = 0;
	virtual bool WasMousePressed(int code) = 0;
	virtual bool WasMouseDoublePressed(int code) = 0;
	virtual bool IsMouseDown(int code) = 0;

	// cursor override
	virtual void SetCursorOveride(int cursor) = 0;
	virtual int GetCursorOveride() = 0;

	// key state
	virtual bool WasMouseReleased(int code) = 0;
	virtual bool WasKeyPressed(int code) = 0;
	virtual bool IsKeyDown(int code) = 0;
	virtual bool WasKeyTyped(int code) = 0;
	virtual bool WasKeyReleased(int code) = 0;

	virtual int GetAppModalSurface() = 0;
	// set the modal dialog panel.
	// all events will go only to this panel and its children.
	virtual void SetAppModalSurface(int panel) = 0;
	// release the modal dialog panel
	// do this when your modal dialog finishes.
	virtual void ReleaseAppModalSurface() = 0;

	virtual void GetCursorPosition(int& x, int& y) = 0;

	virtual void SetIMEWindow(void* hwnd) = 0;
	virtual void* GetIMEWindow() = 0;

	virtual void OnChangeIME(bool forward) = 0;
	virtual int  GetCurrentIMEHandle() = 0;
	virtual int  GetEnglishIMEHandle() = 0;

	// Returns the Language Bar label (Chinese, Korean, Japanese, Russion, Thai, etc.)
	virtual void GetIMELanguageName() = 0;
	// Returns the short code for the language (EN, CH, KO, JP, RU, TH, etc. ).
	virtual void GetIMELanguageShortCode() = 0;

	struct LanguageItem
	{
		wchar_t		shortname[4];
		wchar_t		menuname[128];
		int			handleValue;
		bool		active; // true if this is the active language
	};

	struct ConversionModeItem
	{
		wchar_t		menuname[128];
		int			handleValue;
		bool		active; // true if this is the active conversion mode
	};

	struct SentenceModeItem
	{
		wchar_t		menuname[128];
		int			handleValue;
		bool		active; // true if this is the active sentence mode
	};

	// Call with NULL dest to get item count
	virtual int	 GetIMELanguageList(LanguageItem* dest, int destcount) = 0;
	virtual int	 GetIMEConversionModes(ConversionModeItem* dest, int destcount) = 0;
	virtual int	 GetIMESentenceModes(SentenceModeItem* dest, int destcount) = 0;

	virtual void OnChangeIMEByHandle(int handleValue) = 0;
	virtual void OnChangeIMEConversionModeByHandle(int handleValue) = 0;
	virtual void OnChangeIMESentenceModeByHandle(int handleValue) = 0;

	virtual void OnInputLanguageChanged() = 0;
	virtual void OnIMEStartComposition() = 0;
	virtual void OnIMEComposition(int flags) = 0;
	virtual void OnIMEEndComposition() = 0;

	virtual void OnIMEShowCandidates() = 0;
	virtual void OnIMEChangeCandidates() = 0;
	virtual void OnIMECloseCandidates() = 0;
	virtual void OnIMERecomputeModes() = 0;

	virtual int  GetCandidateListCount() = 0;
	virtual void GetCandidate(int num) = 0;
	virtual int  GetCandidateListSelectedItem() = 0;
	virtual int  GetCandidateListPageSize() = 0;
	virtual int  GetCandidateListPageStart() = 0;

	//NOTE:  We render our own candidate lists most of the time...
	virtual void SetCandidateWindowPos(int x, int y) = 0;

	virtual bool GetShouldInvertCompositionString() = 0;
	virtual bool CandidateListStartsAtOne() = 0;

	virtual void SetCandidateListPageStart(int start) = 0;

	// Passes in a int which allows hitting other mouse buttons w/o cancelling capture mode
	virtual void SetMouseCaptureEx(int panel) = 0;

	virtual void RegisterintUnhandledListener(int panel) = 0;
	virtual void UnregisterintUnhandledListener(int panel) = 0;

	// Posts unhandled message to all interested panels
	virtual void OnintUnhandled() = 0;

	virtual void SetModalSubTree(int subTree, int unhandledMouseClickListener, bool restrictMessagesToSubTree = true) = 0;
	virtual void ReleaseModalSubTree() = 0;
	virtual int	 GetModalSubTree() = 0;

	// These toggle whether the modal subtree is exclusively receiving messages or conversely whether it's being excluded from receiving messages
	// Sends a "ModalSubTree", state message
	virtual void SetModalSubTreeReceiveMessages(bool state) = 0;
	virtual bool ShouldModalSubTreeReceiveMessages() const = 0;
				 
	virtual int  GetMouseCapture() = 0; // 62th form 0
};

MAKE_INTERFACE_VERSION(iInput, "VGUI_Input005", IInput, VGUI2_DLL)