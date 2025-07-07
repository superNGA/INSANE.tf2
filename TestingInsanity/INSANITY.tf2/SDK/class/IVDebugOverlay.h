#pragma once
#include "Basic Structures.h"
#include "../../Utility/Interface Handler/Interface.h"

class IVDebugOverlay
{
public:
	virtual void AddEntityTextOverlay(int ent_index, int line_offset, float duration, int r, int g, int b, int a,   const char* format, ...) = 0;
	virtual void AddBoxOverlay(const vec& origin, const vec& mins, const vec& max, qangle const& orientation, int r, int g, int b, int a, float duration) = 0;
	virtual void AddTriangleOverlay(const vec& p1, const vec& p2, const vec& p3, int r, int g, int b, int a, bool noDepthTest, float duration) = 0;
	virtual void AddLineOverlay(const vec& origin, const vec& dest, int r, int g, int b,bool noDepthTest, float duration) = 0;
	virtual void AddTextOverlay(const vec& origin, float duration,   const char* format, ...) = 0;
	virtual void AddTextOverlay(const vec& origin, int line_offset, float duration,   const char* format, ...) = 0;
	virtual void AddScreenTextOverlay(float flXPos, float flYPos,float flDuration, int r, int g, int b, int a, const char* text) = 0;
	virtual void AddSweptBoxOverlay(const vec& start, const vec& end, const vec& mins, const vec& max, const qangle& angles, int r, int g, int b, int a, float flDuration) = 0;
	virtual void AddGridOverlay(const vec& origin) = 0;
	virtual int ScreenPosition(const vec& point, vec& screen) = 0;
	virtual int ScreenPosition(float flXPos, float flYPos, vec& screen) = 0;

	virtual void* GetFirst(void) = 0;
	virtual void* GetNext(void* current) = 0;
	virtual void ClearDeadOverlays(void) = 0;
	virtual void ClearAllOverlays() = 0;

	virtual void AddTextOverlayRGB(const vec& origin, int line_offset, float duration, float r, float g, float b, float alpha,   const char* format, ...) = 0;
	virtual void AddTextOverlayRGB(const vec& origin, int line_offset, float duration, int r, int g, int b, int a,   const char* format, ...) = 0;

	virtual void AddLineOverlayAlpha(const vec& origin, const vec& dest, int r, int g, int b, int a, bool noDepthTest, float duration) = 0;

	void AddCircleOverlay(const vec& origin, const vec& normal, const float	flRadius, const int nSegments, int r, int g, int b, int a, bool noDepthTest, float duration, float flInitialAngleInRad);
	void AddAngleOverlay(const qangle& qAngles, const vec& vOrigin, const float flLength, int r, int g, int b, int a, float flDuration);
};

MAKE_INTERFACE_VERSION(IDebugOverlay, "VDebugOverlay003", IVDebugOverlay, ENGINE_DLL)