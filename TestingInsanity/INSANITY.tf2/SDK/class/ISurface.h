#pragma once
#include "../../Utility/Interface Handler/Interface.h"
#include "IAppSystem.h"

class ISurface : public IAppSystem
{
public:
	// call to Shutdown surface; surface can no longer be used after this is called
	virtual void Shutdown() = 0;

	// frame
	virtual void RunFrame() = 0;

	// hierarchy root
	virtual int GetEmbeddedPanel() = 0;
	virtual void SetEmbeddedPanel(int pPanel) = 0;

	// drawing context
	virtual void PushMakeCurrent(int panel, bool useInsets) = 0;
	virtual void PopMakeCurrent(int panel) = 0;

	// rendering functions
	virtual void DrawSetint(int r, int g, int b, int a) = 0;
	virtual void DrawSetint(int col) = 0;

	virtual void DrawFilledRect(int x0, int y0, int x1, int y1) = 0;
	virtual void DrawFilledRectArray(int* pRects, int numRects) = 0;
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
	virtual int* CreateHTMLWindow(int* events, int context) = 0;
	virtual void PaintHTMLWindow(int* htmlwin) = 0;
	virtual void DeleteHTMLWindow(int* htmlwin) = 0;

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
	virtual void SetAsTopMost(int panel, bool state) = 0;
	virtual void BringToFront(int panel) = 0;
	virtual void SetForegroundWindow(int panel) = 0;
	virtual void SetPanelVisible(int panel, bool state) = 0;
	virtual void SetMinimized(int panel, bool state) = 0;
	virtual bool IsMinimized(int panel) = 0;
	virtual void FlashWindow(int panel, bool state) = 0;
	virtual void SetTitle(int panel, const wchar_t* title) = 0;
	virtual void SetAsToolBar(int panel, bool state) = 0;		// removes the window's task bar entry (for context menu's, etc.)

	// windows stuff
	virtual void CreatePopup(int panel, bool minimised, bool showTaskbarIcon = true, bool disabled = false, bool mouseInput = true, bool kbInput = true) = 0;
	virtual void SwapBuffers(int panel) = 0;
	virtual void Invalidate(int panel) = 0;
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
	virtual void RestrictPaintToSinglePanel(int panel) = 0;

	// these two functions obselete, use IInput::SetAppModalSurface() instead
	virtual void SetModalPanel(int) = 0;
	virtual int GetModalPanel() = 0;

	virtual void UnlockCursor() = 0;
	virtual void LockCursor() = 0;
	virtual void SetTranslateExtendedKeys(bool state) = 0;
	virtual int GetTopmostPopup() = 0;

	// engine-only focus handling (replacing WM_FOCUS windows handling)
	virtual void SetTopLevelFocus(int panel) = 0;

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
	virtual int GetNotifyPanel() = 0;
	virtual void SetNotifyIcon(int context, int icon, int panelToReceiveMessages, const char* text) = 0;

	// plays a sound
	virtual void PlaySound(const char* fileName) = 0;

	//!! these functions should not be accessed directly, but only through other vgui items
	//!! need to move these to seperate interface
	virtual int GetPopupCount() = 0;
	virtual int GetPopup(int index) = 0;
	virtual bool ShouldPaintChildPanel(int childPanel) = 0;
	virtual bool RecreateContext(int panel) = 0;
	virtual void AddPanel(int panel) = 0;
	virtual void ReleasePanel(int panel) = 0;
	virtual void MovePopupToFront(int panel) = 0;
	virtual void MovePopupToBack(int panel) = 0;

	virtual void SolveTraverse(int panel, bool forceApplySchemeSettings = false) = 0;
	virtual void PaintTraverse(int panel) = 0;

	virtual void EnableMouseCapture(int panel, bool state) = 0;

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
	virtual void DrawTexturedLine(const int& a, const int& b) = 0;
	virtual void DrawOutlinedCircle(int x, int y, int radius, int segments) = 0;
	virtual void DrawTexturedPolyLine(const int* p, int n) = 0; // (Note: this connects the first and last points).
	virtual void DrawTexturedSubRect(int x0, int y0, int x1, int y1, float texs0, float text0, float texs1, float text1) = 0;
	virtual void DrawTexturedPolygon(int n, int* pVertice, bool bClipVertices = true) = 0;
	virtual const wchar_t* GetTitle(int panel) = 0;
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

	virtual void CreateCursorFromFile(char const* curOrAniFile, char const* pPathID = 0) = 0;

	// create IVguiMatInfo object ( IMaterial wrapper in VguiMatSurface, NULL in CWin32Surface )
	virtual void* DrawGetTextureMatInfoFactory(int id) = 0;

	virtual void PaintTraverseEx(int panel, bool paintPopups = false) = 0;

	virtual float GetZPos() const = 0;

	// From the Xbox
	virtual void SetPanelForInput() = 0;
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
	virtual void DrawUnicodeString(const wchar_t* pwString, int drawType) = 0;
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
	virtual bool BHTMLWindowNeedsPaint(int* htmlwin) = 0;

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

	// aight lil bro, chill
};

MAKE_INTERFACE_VERSION(iSurface, "VGUI_Surface030", ISurface, VGUIMATSURFACE_DLL);