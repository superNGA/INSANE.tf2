#include "IVDebugOverlay.h"
#include "Basic Structures.h"

void IVDebugOverlay::AddCircleOverlay(const vec& origin, const vec& normal, const float flRadius, const int nSegments, int r, int g, int b, int a, bool noDepthTest, float duration, float flInitialAngleInRad = 0.0f)
{
    // Creating a dummy vector that needs to not in the direction of Normal vector.
    vec vDummy = normal;
    vDummy.x   += 10.0f;

    // vTangent & vBiTangent now serve as our new Axis.
    vec vTangent = normal.CrossProduct(vDummy);
    vTangent.NormalizeInPlace();

    vec vBiTangent = normal.CrossProduct(vTangent);
    vBiTangent.NormalizeInPlace();

    // Calculating angle per segment
    float flSegmentAngleInRad = 2.0f * M_PI / static_cast<float>(nSegments);
    
    // Creating each line segment, to contruct the circle.
    for (int iSegment = 0; iSegment < nSegments; iSegment++)
    {
        // segment START & END angles
        float flSegmentStartAngleInRad = static_cast<float>(iSegment) * flSegmentAngleInRad + flInitialAngleInRad;
        float flSegmentEndAngleInRad   = flSegmentStartAngleInRad     + flSegmentAngleInRad;

        vec vSegmentStart(
            origin.x + (cos(flSegmentStartAngleInRad) * vTangent.x * flRadius) + (sin(flSegmentStartAngleInRad) * vBiTangent.x * flRadius),
            origin.y + (cos(flSegmentStartAngleInRad) * vTangent.y * flRadius) + (sin(flSegmentStartAngleInRad) * vBiTangent.y * flRadius),
            origin.z + (cos(flSegmentStartAngleInRad) * vTangent.z * flRadius) + (sin(flSegmentStartAngleInRad) * vBiTangent.z * flRadius)
            );

        vec vSegmentEnd(
            origin.x + (cos(flSegmentEndAngleInRad) * vTangent.x * flRadius) + (sin(flSegmentEndAngleInRad) * vBiTangent.x * flRadius),
            origin.y + (cos(flSegmentEndAngleInRad) * vTangent.y * flRadius) + (sin(flSegmentEndAngleInRad) * vBiTangent.y * flRadius),
            origin.z + (cos(flSegmentEndAngleInRad) * vTangent.z * flRadius) + (sin(flSegmentEndAngleInRad) * vBiTangent.z * flRadius)
        );

        // Finally, drawing the line segment.
        I::IDebugOverlay->AddLineOverlayAlpha(vSegmentStart, vSegmentEnd, r, g, b, a, noDepthTest, duration);
    }
}