#pragma once
#include <math.h>
#include <algorithm>
#include "../SDK/class/Basic Structures.h"

const inline float MAX_PITCH = 89.0f;
const inline float MIN_PITCH = -89.0f;
const inline float MAX_YAW = 180.0f;
const inline float MIN_YAW = -180.0f;

namespace Maths
{
    // I could have used something like a static_assert to std::concept to
    // restrict this to its basic datatypes, but I didn't :) and IDK why :)
    template < typename T>
    __forceinline T MIN(const T& val1, const T& val2)
    {
        return (val1 < val2 ? val1 : val2);
    }

    template < typename T>
    __forceinline T MAX(const T& val1, const T& val2)
    {
        return (val1 > val2 ? val1 : val2);
    }


    inline void SinCos(float flRadians, float* pSin, float* pCos)
    {
        *pSin = std::sin(flRadians);
        *pCos = std::cos(flRadians);
    }

//=========================================================================
// inline void AngleVectors(const qangle& angles, vec* forward, vec* right = nullptr, vec* up = nullptr)
//=========================================================================
/**
* converts a qangle to eular angles, each of the output vector is optional
*
* @param angles : input qangle
* @param forward : forward output vector
* @param right : right output vector
* @param up : up output vector
**************************************************************************/
    inline void AngleVectors(const qangle& vAngles, vec* pForward = nullptr, vec* pRight = nullptr, vec* pUp = nullptr)
    {
        float sp, sy, sr, cp, cy, cr;
        SinCos(DEG2RAD(vAngles.pitch), &sp, &cp);
        SinCos(DEG2RAD(vAngles.yaw) , &sy, &cy);

        if (pForward)
        {
            pForward->x = cp * cy;
            pForward->y = cp * sy;
            pForward->z = -sp;
        }

        if (pRight || pUp)
        {
            SinCos(DEG2RAD(vAngles.roll), &sr, &cr);

            if (pRight)
            {
                pRight->x = (-1 * sr * sp * cy + -1 * cr * -sy);
                pRight->y = (-1 * sr * sp * sy + -1 * cr * cy);
                pRight->z = -1 * sr * cp;
            }

            if (pUp)
            {
                pUp->x = (cr * sp * cy + -sr * -sy);
                pUp->y = (cr * sp * sy + -sr * cy);
                pUp->z = cr * cp;
            }
        }
    }


//=========================================================================
// inline void ClampQAngle(qangle& anglesIn)
//=========================================================================
/**
* clampes qangles to fixed values
*
* @param anglesIn : input angles, to be clamped
**************************************************************************/
    inline void ClampQAngle(qangle& anglesIn)
    {
        // Fixxing PITCH
        anglesIn.pitch < MIN_PITCH ?
            anglesIn.pitch = MIN_PITCH :
            (anglesIn.pitch > MAX_PITCH ? MAX_PITCH : anglesIn.pitch);

        // Fixxing YAW
        anglesIn.yaw > MAX_YAW ? // if yaw > max ( 180 )
            anglesIn.yaw = MIN_YAW + std::fmod(abs(anglesIn.yaw), MAX_YAW) : // add ammount more than 180 to -180.0f
            (anglesIn.yaw < MIN_YAW ? anglesIn.yaw = MAX_YAW - std::fmod(abs(anglesIn.yaw), MAX_YAW) : anglesIn.yaw); // else do the opposite.

        // Fixxing ROLL
        anglesIn.roll = 0.0f;
    }

    // Converts angle from [ 0 , 360 ] TO [ -180 , 180 ]
    inline float NormalizeAngle(float flAngle, float flRange = 360.f)
    {
        return std::isfinite(flAngle) ? std::remainder(flAngle, flRange) : 0.f;
    }

    inline void ClampAngles(qangle& angles)
    {
        angles.pitch = std::clamp(NormalizeAngle(angles.pitch), -89.f, 89.f);
        angles.yaw = NormalizeAngle(angles.yaw);
        angles.roll = 0.f;
    }


//=========================================================================
// inline void VectorAngles(const vec& forward, qangle& angles)
//=========================================================================
/**
* Converts Vector to Qangles
*
* @param forward : input vector
* @param angels : output qangle
**************************************************************************/
    inline void VectorAngles(const vec& forward, qangle& angles)
    {
        // NOTE : PITCH IS INVERTED IN TF2. CAUSE VALVE IS A FUCKING NIGGER.
        float yaw, pitch;

        if (forward.y == 0 && forward.x == 0)
        {
            yaw = 0;
            if (forward.z > 0)
                angles.pitch = MIN_YAW; // looking staright up
            else
                angles.pitch = MAX_YAW; // looking staright down
            
            return;
        }

        yaw       = (RAD2DEG(atan2(forward.y, forward.x)));
        float tmp = sqrtf(forward.x * forward.x + forward.y * forward.y);
        pitch     = (RAD2DEG(atan2(-forward.z, tmp)));

        angles.pitch = pitch;
        angles.yaw   = yaw;
        angles.roll  = 0.0f;
    }

    inline void VectorAnglesFromSDK(const vec& vForward, qangle& vAngles)
    {
        float yaw, pitch;

        if (vForward.y == 0 && vForward.x == 0)
        {
            yaw = 0;
            pitch = vForward.z > 0 ? 270 : 90;
        }
        else
        {
            yaw = RAD2DEG(atan2f(vForward.y, vForward.x));
            if (yaw < 0)
                yaw += 360;

            pitch = RAD2DEG(atan2f(-vForward.z, sqrtf(vForward.x * vForward.x + vForward.y * vForward.y)));
            if (pitch < 0)
                pitch += 360;
        }

        vAngles.pitch = pitch;
        vAngles.yaw = yaw;
        vAngles.roll = 0;
    }

    inline void MatrixSetColumn(const vec& in, int column, matrix3x4_t& out)
    {
        out.m[0][column] = in.x;
        out.m[1][column] = in.y;
        out.m[2][column] = in.z;
    }

    inline void AngleMatrix(const qangle& angles, matrix3x4_t& matrix)
    {
        float sr, sp, sy, cr, cp, cy;

        SinCos(DEG2RAD(angles.yaw), &sy, &cy);
        SinCos(DEG2RAD(angles.pitch), &sp, &cp);
        SinCos(DEG2RAD(angles.roll), &sr, &cr);

        // matrix = (YAW * PITCH) * ROLL
        matrix.m[0][0] = cp * cy;
        matrix.m[1][0] = cp * sy;
        matrix.m[2][0] = -sp;

        float crcy = cr * cy;
        float crsy = cr * sy;
        float srcy = sr * cy;
        float srsy = sr * sy;
        matrix.m[0][1] = sp * srcy - crsy;
        matrix.m[1][1] = sp * srsy + crcy;
        matrix.m[2][1] = sr * cp;

        matrix.m[0][2] = (sp * crcy + srsy);
        matrix.m[1][2] = (sp * crsy - srcy);
        matrix.m[2][2] = cr * cp;

        matrix.m[0][3] = 0.0f;
        matrix.m[1][3] = 0.0f;
        matrix.m[2][3] = 0.0f;
    }

    inline void AngleMatrix(const qangle& angles, const vec& position, matrix3x4_t& matrix)
    {
        AngleMatrix(angles, matrix);
        MatrixSetColumn(position, 3, matrix);
    }

    inline float RemapValClamped(float val, float A, float B, float C, float D)
    {
        if (A == B)
            return val >= B ? D : C;
        float cVal = (val - A) / (B - A);
        cVal = std::clamp(cVal, 0.0f, 1.0f);

        return C + (D - C) * cVal;
    }
}