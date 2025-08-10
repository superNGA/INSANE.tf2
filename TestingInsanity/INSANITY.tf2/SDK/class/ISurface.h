#pragma once

#include "../../Utility/Interface Handler/Interface.h"
#include "../../Utility/Signature Handler/signatures.h"

#include "IAppSystem.h"
#include "Basic Structures.h"


MAKE_SIG(IMatSystemSurface_Begin3DPaint, "48 89 5C 24 ? 48 89 6C 24 ? 56 57 41 54 41 56 41 57 48 83 EC ? 41 8B F1", VGUIMATSURFACE_DLL, void,
	void*, int, int, int, int, bool)
MAKE_SIG(IMatSystemSurface_End3DPaint, "40 53 55 56 57 41 54 41 55", VGUIMATSURFACE_DLL, void*, void*)


class ISurface : public IAppSystem
{
public:
	inline void Begin3DPaint_Sig(int iLeft, int iTop, int iRight, int iBottom, bool bRenderToTexture)
	{
		Sig::IMatSystemSurface_Begin3DPaint(this, iLeft, iTop, iRight, iBottom, bRenderToTexture);
	}
	
	inline void End3DPaint_Sig()
	{
		Sig::IMatSystemSurface_End3DPaint(this);
	}


	// call to Shutdown surface; surface can no longer be used after this is called
	virtual void Shutdown() = 0;

	// frame
	virtual void RunFrame() = 0;

	// hierarchy root
	virtual vgui::VPANEL GetEmbeddedPanel() = 0;
	virtual void SetEmbeddedPanel(vgui::VPANEL pPanel) = 0;

	// drawing context
	virtual void PushMakeCurrent(vgui::VPANEL panel, bool useInsets) = 0;
	virtual void PopMakeCurrent(vgui::VPANEL panel) = 0;

	// rendering functions
	virtual void DrawSetint(int r, int g, int b, int a) = 0;
	virtual void DrawSetint(int col) = 0;

	virtual void DrawFilledRect(int x0, int y0, int x1, int y1) = 0;
	virtual void DrawFilledRectArray(void* pRects, int numRects) = 0;
	virtual void DrawOutlinedRect(int x0, int y0, int x1, int y1) = 0;

	virtual void DrawLine(int x0, int y0, int x1, int y1) = 0;
	virtual void DrawPolyLine(int* px, int* py, int numPoints) = 0;

	virtual void DrawSetTextFont(int font) = 0;
	virtual void DrawSetTextint(int r, int g, int b, int a) = 0;
	virtual void DrawSetTextint(int col) = 0;
	virtual void DrawSetTextPos(int x, int y) = 0;
	virtual void DrawGetTextPos(int& x, int& y) = 0;
	virtual void DrawPrintText(const wchar_t* text, int textLen, int drawType) = 0;
	virtual void DrawUnicodeChar(wchar_t wch, int drawType) = 0;

	virtual void DrawFlushText() = 0;		// flushes any buffered text (for rendering optimizations)
	virtual void* CreateHTMLWindow(void* events, vgui::VPANEL context) = 0;
	virtual void PaintHTMLWindow(void* htmlwin) = 0;
	virtual void DeleteHTMLWindow(void* htmlwin) = 0;

	enum ETextureFormat
	{
		eTextureFormat_RGBA,
		eTextureFormat_BGRA,
		eTextureFormat_BGRA_Opaque, // bgra format but alpha is always 255, CEF does this, we can use this fact for better perf on win32 gdi
	};

	virtual int	 DrawGetTextureId(char const* filename) = 0;
	virtual bool DrawGetTextureFile(int id, char* filename, int maxlen) = 0;
	virtual void DrawSetTextureFile(int id, const char* filename, int hardwareFilter, bool forceReload) = 0;
	virtual void DrawSetTextureRGBA(int id, const unsigned char* rgba, int wide, int tall, int hardwareFilter, bool forceReload) = 0;
	virtual void DrawSetTexture(int id) = 0;
	virtual void DrawGetTextureSize(int id, int& wide, int& tall) = 0;
	virtual void DrawTexturedRect(int x0, int y0, int x1, int y1) = 0;
	virtual bool IsTextureIDValid(int id) = 0;
	virtual bool DeleteTextureByID(int id) = 0;

	virtual int CreateNewTextureID(bool procedural = false) = 0;

	virtual void GetScreenSize(int& wide, int& tall) = 0;
	virtual void SetAsTopMost(vgui::VPANEL panel, bool state) = 0;
	virtual void BringToFront(vgui::VPANEL panel) = 0;
	virtual void SetForegroundWindow(vgui::VPANEL panel) = 0;
	virtual void SetPanelVisible(vgui::VPANEL panel, bool state) = 0;
	virtual void SetMinimized(vgui::VPANEL panel, bool state) = 0;
	virtual bool IsMinimized(vgui::VPANEL panel) = 0;
	virtual void FlashWindow(vgui::VPANEL panel, bool state) = 0;
	virtual void SetTitle(vgui::VPANEL panel, const wchar_t* title) = 0;
	virtual void SetAsToolBar(vgui::VPANEL panel, bool state) = 0;		// removes the window's task bar entry (for context menu's, etc.)

	// windows stuff
	virtual void CreatePopup(vgui::VPANEL panel, bool minimised, bool showTaskbarIcon = true, bool disabled = false, bool mouseInput = true, bool kbInput = true) = 0;
	virtual void SwapBuffers(vgui::VPANEL panel) = 0;
	virtual void Invalidate(vgui::VPANEL panel) = 0;
	virtual void SetCursor(int cursor) = 0;
	virtual void SetCursorAlwaysVisible(bool visible) = 0;
	virtual bool IsCursorVisible() = 0;
	virtual void ApplyChanges() = 0;
	virtual bool IsWithin(int x, int y) = 0;
	virtual bool HasFocus() = 0;

	// returns true if the surface supports minimize & maximize capabilities
	enum SurfaceFeature_e
	{
		ANTIALIASED_FONTS = 1,
		DROPSHADOW_FONTS = 2,
		ESCAPE_KEY = 3,
		OPENING_NEW_HTML_WINDOWS = 4,
		FRAME_MINIMIZE_MAXIMIZE = 5,
		OUTLINE_FONTS = 6,
		DIRECT_HWND_RENDER = 7,
	};
	virtual bool SupportsFeature(SurfaceFeature_e feature) = 0;

	// restricts what gets drawn to one panel and it's children
	// currently only works in the game
	virtual void RestrictPaintToSinglePanel(vgui::VPANEL panel) = 0;

	// these two functions obselete, use IInput::SetAppModalSurface() instead
	virtual void SetModalPanel(vgui::VPANEL) = 0;
	virtual vgui::VPANEL GetModalPanel() = 0;

	virtual void UnlockCursor() = 0;
	virtual void LockCursor() = 0;
	virtual void SetTranslateExtendedKeys(bool state) = 0;
	virtual vgui::VPANEL GetTopmostPopup() = 0;

	// engine-only focus handling (replacing WM_FOCUS windows handling)
	virtual void SetTopLevelFocus(vgui::VPANEL panel) = 0;

	// fonts
	// creates an empty handle to a vgui font.  windows fonts can be add to this via SetFontGlyphSet().
	virtual int CreateFont() = 0;

	// adds to the font
	enum EFontFlags
	{
		FONTFLAG_NONE,
		FONTFLAG_ITALIC = 0x001,
		FONTFLAG_UNDERLINE = 0x002,
		FONTFLAG_STRIKEOUT = 0x004,
		FONTFLAG_SYMBOL = 0x008,
		FONTFLAG_ANTIALIAS = 0x010,
		FONTFLAG_GAUSSIANBLUR = 0x020,
		FONTFLAG_ROTARY = 0x040,
		FONTFLAG_DROPSHADOW = 0x080,
		FONTFLAG_ADDITIVE = 0x100,
		FONTFLAG_OUTLINE = 0x200,
		FONTFLAG_CUSTOM = 0x400,		// custom generated font - never fall back to asian compatibility mode
		FONTFLAG_BITMAP = 0x800,		// compiled bitmap font - no fallbacks
	};

	virtual bool SetFontGlyphSet(int font, const char* windowsFontName, int tall, int weight, int blur, int scanlines, int flags, int nRangeMin = 0, int nRangeMax = 0) = 0;

	// adds a custom font file (only supports true type font files (.ttf) for now)
	virtual bool AddCustomFontFile(const char* fontName, const char* fontFileName) = 0;

	// returns the details about the font
	virtual int GetFontTall(int font) = 0;
	virtual int GetFontTallRequested(int font) = 0;
	virtual int GetFontAscent(int font, wchar_t wch) = 0;
	virtual bool IsFontAdditive(int font) = 0;
	virtual void GetCharABCwide(int font, int ch, int& a, int& b, int& c) = 0;
	virtual int GetCharacterWidth(int font, int ch) = 0;
	virtual void GetTextSize(int font, const wchar_t* text, int& wide, int& tall) = 0;

	// notify icons?!?
	virtual vgui::VPANEL GetNotifyPanel() = 0;
	virtual void SetNotifyIcon(vgui::VPANEL context, int icon, vgui::VPANEL panelToReceiveMessages, const char* text) = 0;

	// plays a sound
	virtual void PlaySound(const char* fileName) = 0;

	//!! these functions should not be accessed directly, but only through other vgui items
	//!! need to move these to seperate interface
	virtual int GetPopupCount() = 0;
	virtual vgui::VPANEL GetPopup(int index) = 0;
	virtual bool ShouldPaintChildPanel(vgui::VPANEL childPanel) = 0;
	virtual bool RecreateContext(vgui::VPANEL panel) = 0;
	virtual void AddPanel(vgui::VPANEL panel) = 0;
	virtual void ReleasePanel(vgui::VPANEL panel) = 0;
	virtual void MovePopupToFront(vgui::VPANEL panel) = 0;
	virtual void MovePopupToBack(vgui::VPANEL panel) = 0;

	virtual void SolveTraverse(vgui::VPANEL panel, bool forceApplySchemeSettings = false) = 0;
	virtual void PaintTraverse(vgui::VPANEL panel) = 0;

	virtual void EnableMouseCapture(vgui::VPANEL panel, bool state) = 0;

	// returns the size of the workspace
	virtual void GetWorkspaceBounds(int& x, int& y, int& wide, int& tall) = 0;

	// gets the absolute coordinates of the screen (in windows space)
	virtual void GetAbsoluteWindowBounds(int& x, int& y, int& wide, int& tall) = 0;

	// gets the base resolution used in proportional mode
	virtual void GetProportionalBase(int& width, int& height) = 0;

	virtual void CalculateMouseVisible() = 0;
	virtual bool NeedKBInput() = 0;

	virtual bool HasCursorPosFunctions() = 0;
	virtual void SurfaceGetCursorPos(int& x, int& y) = 0;
	virtual void SurfaceSetCursorPos(int x, int y) = 0;

	// SRC only functions!!!
	virtual void DrawTexturedLine(const vec& a, const vec& b) = 0;
	virtual void DrawOutlinedCircle(int x, int y, int radius, int segments) = 0;
	virtual void DrawTexturedPolyLine(const vec* p, int n) = 0; // (Note: this connects the first and last points).
	virtual void DrawTexturedSubRect(int x0, int y0, int x1, int y1, float texs0, float text0, float texs1, float text1) = 0;
	virtual void DrawTexturedPolygon(int n, vec* pVertice, bool bClipVertices = true) = 0;
	virtual const wchar_t* GetTitle(vgui::VPANEL panel) = 0;
	virtual bool IsCursorLocked(void) const = 0;
	virtual void SetWorkspaceInsets(int left, int top, int right, int bottom) = 0;

	// Lower level char drawing code, call DrawGet then pass in info to DrawRender
	virtual bool DrawGetUnicodeint(wchar_t ch, int& info) = 0;
	virtual void DrawRenderCharFromInfo(const int& info) = 0;

	// global alpha setting functions
	// affect all subsequent draw calls - shouldn't normally be used directly, only in Panel::PaintTraverse()
	virtual void DrawSetAlphaMultiplier(float alpha /* [0..1] */) = 0;
	virtual float DrawGetAlphaMultiplier() = 0;

	// web browser
	virtual void SetAllowHTMLJavaScript(bool state) = 0;

	// video mode changing
	virtual void OnScreenSizeChanged(int nOldWidth, int nOldHeight) = 0;

	virtual int CreateCursorFromFile(char const* curOrAniFile, char const* pPathID = 0) = 0;

	// create IVguiMatInfo object ( IMaterial wrapper in VguiMatSurface, NULL in CWin32Surface )
	virtual void* DrawGetTextureMatInfoFactory(int id) = 0;

	virtual void PaintTraverseEx(vgui::VPANEL panel, bool paintPopups = false) = 0;

	virtual float GetZPos() const = 0;

	// From the Xbox
	virtual void SetPanelForInput(vgui::VPANEL a) = 0;
	virtual void DrawFilledRectFastFade(int x0, int y0, int x1, int y1, int fadeStartPt, int fadeEndPt, unsigned int alpha0, unsigned int alpha1, bool bHorizontal) = 0;
	virtual void DrawFilledRectFade(int x0, int y0, int x1, int y1, unsigned int alpha0, unsigned int alpha1, bool bHorizontal) = 0;
	virtual void DrawSetTextureRGBAEx(int id, const unsigned char* rgba, int wide, int tall, int imageFormat) = 0;
	virtual void DrawSetTextScale(float sx, float sy) = 0;
	virtual bool SetBitmapFontGlyphSet(int font, const char* windowsFontName, float scalex, float scaley, int flags) = 0;
	// adds a bitmap font file
	virtual bool AddBitmapFontFile(const char* fontFileName) = 0;
	// sets a symbol for the bitmap font
	virtual void SetBitmapFontName(const char* pName, const char* pFontFilename) = 0;
	// gets the bitmap font filename
	virtual const char* GetBitmapFontName(const char* pName) = 0;
	virtual void ClearTemporaryFontCache(void) = 0;

	virtual void* GetIconImageForFullPath(char const* pFullPath) = 0;
	virtual void DrawUnicodeString(const wchar_t* pwString, int drawType = 0) = 0;
	virtual void PrecacheFontCharacters(int font, const wchar_t* pCharacters) = 0;
	// Console-only.  Get the string to use for the current video mode for layout files.
	virtual const char* GetResolutionKey(void) const = 0;

	virtual const char* GetFontName(int font) = 0;
	virtual const char* GetFontFamilyName(int font) = 0;
	virtual void GetKernedCharWidth(int font, wchar_t ch, wchar_t chBefore, wchar_t chAfter, float& wide, float& abcA) = 0;

	virtual bool ForceScreenSizeOverride(bool bState, int wide, int tall) = 0;
	// LocalToScreen, ParentLocalToScreen fixups for explicit PaintTraverse calls on Panels not at 0, 0 position
	virtual bool ForceScreenPosOffset(bool bState, int x, int y) = 0;
	virtual void OffsetAbsPos(int& x, int& y) = 0;


	// Causes fonts to get reloaded, etc.
	virtual void ResetFontCaches() = 0;

	virtual int GetTextureNumFrames(int id) = 0;
	virtual void DrawSetTextureFrame(int id, int nFrame, unsigned int* pFrameCache) = 0;
	virtual bool IsScreenSizeOverrideActive(void) = 0;
	virtual bool IsScreenPosOverrideActive(void) = 0;

	virtual void DestroyTextureID(int id) = 0;

	virtual void DrawUpdateRegionTextureRGBA(int nTextureID, int x, int y, const unsigned char* pchData, int wide, int tall, int imageFormat) = 0;
	virtual bool BHTMLWindowNeedsPaint(void* htmlwin) = 0;

	virtual const char* GetWebkitHTMLUserAgentString() = 0;

	virtual void* Deprecated_AccessChromeHTMLController() = 0;

	// the origin of the viewport on the framebuffer (Which might not be 0,0 for stereo)
	virtual void SetFullscreenViewport(int x, int y, int w, int h) = 0; // this uses NULL for the render target.
	virtual void GetFullscreenViewport(int& x, int& y, int& w, int& h) = 0;
	virtual void PushFullscreenViewport() = 0;
	virtual void PopFullscreenViewport() = 0;

	// handles support for software cursors
	virtual void SetSoftwareCursor(bool bUseSoftwareCursor) = 0;
	virtual void PaintSoftwareCursor() = 0;


	// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
	// !! WARNING! YOU MUST NOT ADD YOUR NEW METHOD HERE OR YOU WILL BREAK MODS !!
	// !! Add your new stuff to the bottom of IMatSystemSurface instead.        !!
	// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
};

MAKE_INTERFACE_VERSION(iSurface, "VGUI_Surface030", ISurface, VGUIMATSURFACE_DLL);