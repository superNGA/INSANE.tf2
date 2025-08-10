#pragma once

#include "ISurface.h"
#include "../../Utility/Signature Handler/signatures.h"


class IMatSystemSurface : public ISurface
{
public:
	// Hook needed to get input to work.
	// If the app drives the input (like the engine needs to do for VCR mode), 
	// it can set bLetAppDriveInput to true and call HandleInputEvent for the input events.
	virtual void AttachToWindow(void* hwnd, bool bLetAppDriveInput = false) = 0;

	// Tells the surface to ignore windows messages
	virtual void EnableWindowsMessages(bool bEnable) = 0;

	// Starts, ends 3D painting
	// NOTE: These methods should only be called from within the paint()
	// method of a panel.
	virtual void Begin3DPaint(int iLeft, int iTop, int iRight, int iBottom, bool bRenderToTexture = true) = 0;
	virtual void End3DPaint() = 0;

	// NOTE: This also should only be called from within the paint()
	// method of a panel. Use it to disable clipping for the rendering
	// of this panel.
	virtual void DisableClipping(bool bDisable) = 0;
	virtual void GetClippingRect(int& left, int& top, int& right, int& bottom, bool& bClippingDisabled) = 0;  // <<<<< NOTE: output flag is *disabled* state, not enabled, to match the rest of the interface
	virtual void SetClippingRect(int left, int top, int right, int bottom) = 0;

	// Prevents vgui from changing the cursor
	virtual bool IsCursorLocked() const = 0;

	// Sets the mouse get + set callbacks
	virtual void SetMouseCallbacks(int getFunc, int setFunc) = 0;

	// Installs a function to play sounds
	virtual void InstallPlaySoundFunc(int soundFunc) = 0;

	// Some drawing methods that cannot be accomplished under Win32
	virtual void DrawColoredCircle(int centerx, int centery, float radius, int r, int g, int b, int a) = 0;
	virtual int DrawColoredText(int font, int x, int y, int r, int g, int b, int a,   const char* fmt, ...) = 0;

	// Draws text with current font at position and wordwrapped to the rect using color values specified
	virtual void DrawColoredTextRect(int font, int x, int y, int w, int h, int r, int g, int b, int a,   const char* fmt, ...) = 0;
	virtual void DrawTextHeight(int font, int w, int& h,   const char* fmt, ...) = 0;

	// Returns the length of the text string in pixels
	virtual int	DrawTextLen(int font,   const char* fmt, ...) = 0;

	// Draws a panel in 3D space. Assumes view + projection are already set up
	// Also assumes the (x,y) coordinates of the panels are defined in 640xN coords
	// (N isn't necessary 480 because the panel may not be 4x3)
	// The width + height specified are the size of the panel in world coordinates
	virtual void DrawPanelIn3DSpace(vgui::VPANEL pRootPanel, const int& panelCenterToWorld, int nPixelWidth, int nPixelHeight, float flWorldWidth, float flWorldHeight) = 0;

	// Binds a material to a surface texture ID
	virtual void DrawSetTextureMaterial(int id, IMaterial* pMaterial) = 0;

	// Handles an input event, returns true if the event should be filtered from the rest of the game
	virtual bool HandleInputEvent(const int& event) = 0;

	virtual void Set3DPaintTempRenderTarget(const char* pRenderTargetName) = 0;
	virtual void Reset3DPaintTempRenderTarget(void) = 0;

	// Gets a material bound to a surface texture ID
	virtual IMaterial* DrawGetTextureMaterial(int id) = 0;

	virtual void GetFullscreenViewportAndRenderTarget(int& x, int& y, int& w, int& h, void** ppRenderTarget) = 0;
	virtual void SetFullscreenViewportAndRenderTarget(int x, int y, int w, int h, void* pRenderTarget) = 0;

	// get texture id for a texture
	virtual int  DrawGetTextureId(void* pTexture) = 0;

	// begin and end skin composition painting
	virtual void BeginSkinCompositionPainting() = 0;
	virtual void EndSkinCompositionPainting() = 0;
};