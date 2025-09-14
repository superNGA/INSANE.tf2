#pragma once

#include "Basic Structures.h"
#include "viewSetup.h"
#include "../../Utility/Interface Handler/Interface.h"

class IRender
{
public:
    virtual void	FrameBegin(void) = 0;
    virtual void	FrameEnd(void) = 0;

    virtual void	ViewSetupVis(bool novis, int numorigins, const vec origin[]) = 0;

    virtual void	ViewDrawFade(byte* color, void* pFadeMaterial) = 0;

    virtual void	DrawSceneBegin(void) = 0;
    virtual void	DrawSceneEnd(void) = 0;

    virtual void*   CreateWorldList() = 0;
    virtual void	BuildWorldLists(void* pList, void* pInfo, int iForceViewLeaf, const void* pVisData, bool bShadowDepth, float* pReflectionWaterHeight) = 0;
    virtual void	DrawWorldLists(void* pList, unsigned long flags, float waterZAdjust) = 0;

    // UNDONE: these are temporary functions that will end up on the other
    // side of this interface
    // accessors
//	virtual const vec& UnreflectedViewOrigin() = 0;
    virtual const vec& ViewOrigin(void) = 0;
    virtual const qangle& ViewAngles(void) = 0;
    virtual CViewSetup const& ViewGetCurrent(void) = 0;
    virtual const void ViewMatrix(void) = 0;
    virtual const void WorldToScreenMatrix(void) = 0;
    virtual float	GetFramerate(void) = 0;
    virtual float	GetZNear(void) = 0;
    virtual float	GetZFar(void) = 0;

    // Query current fov and view model fov
    virtual float	GetFov(void) = 0;
    virtual float	GetFovY(void) = 0;
    virtual float	GetFovViewmodel(void) = 0;


    // Compute the clip-space coordinates of a point in 3D
    // Clip-space is normalized screen coordinates (-1 to 1 in x and y)
    // Returns true if the point is behind the camera
    virtual bool	ClipTransformWithProjection(const void* worldToScreen, const vec& point, vec* pClip) = 0;
    // Same, using the current engine's matrices.
    virtual bool	ClipTransform(vec const& point, vec* pClip) = 0;

    // Compute the screen-space coordinates of a point in 3D
    // This returns actual pixels
    // Returns true if the point is behind the camera
    virtual bool ScreenTransform(vec const& point, vec* pScreen) = 0;

    virtual void Push3DView(const CViewSetup& view, int nFlags, void* pRenderTarget, int frustumPlanes) = 0;
    virtual void Push3DView(const CViewSetup& view, int nFlags, void* pRenderTarget, int frustumPlanes, void* pDepthTexture) = 0;
    virtual void Push2DView(const CViewSetup& view, int nFlags, void* pRenderTarget, int frustumPlanes) = 0;
    virtual void PopView(int frustumPlanes) = 0;
    virtual void SetMainView(const vec& vecOrigin, const qangle& angles) = 0;
    virtual void ViewSetupVisEx(bool novis, int numorigins, const vec origin[], unsigned int& returnFlags) = 0;
    virtual void OverrideViewFrustum(int custom) = 0;
    virtual void UpdateBrushModelLightmap(model_t* model, int* Renderable) = 0;
    virtual void BeginUpdateLightmaps(void) = 0;
    virtual void EndUpdateLightmaps(void) = 0;
    virtual bool InLightmapUpdate(void) const = 0;

    float		m_yFOV;

    // timing
    double		m_frameStartTime;
    float		m_framerate;

    float		m_zNear;
    float		m_zFar;
};

MAKE_INTERFACE_SIGNATURE(iRender, "48 8B 0D ? ? ? ? 48 8B 01 FF 50 ? 8B 8D", IRender*, ENGINE_DLL, 3, 7)