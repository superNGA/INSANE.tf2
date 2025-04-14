#pragma once
#include <math.h>
#include "../SDK/class/Basic Structures.h"

const inline float MAX_PITCH = 89.0f;
const inline float MIN_PITCH = -89.0f;
const inline float MAX_YAW = 180.0f;
const inline float MIN_YAW = -180.0f;

namespace Maths
{
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
        SinCos(DEG2RAD * vAngles.pitch , &sp, &cp);
        SinCos(DEG2RAD * vAngles.yaw , &sy, &cy);

        if (pForward)
        {
            pForward->x = cp * cy;
            pForward->y = cp * sy;
            pForward->z = -sp;
        }

        if (pRight || pUp)
        {
            SinCos(DEG2RAD * vAngles.roll, &sr, &cr);

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

        yaw       = (atan2(forward.y, forward.x) * RAD2DEG);
        float tmp = sqrtf(forward.x * forward.x + forward.y * forward.y);
        pitch     = (atan2(-forward.z, tmp) * RAD2DEG);

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
            yaw = RAD2DEG * atan2f(vForward.y, vForward.x);
            if (yaw < 0)
                yaw += 360;

            pitch = RAD2DEG * atan2f(-vForward.z, sqrtf(vForward.x * vForward.x + vForward.y * vForward.y));
            if (pitch < 0)
                pitch += 360;
        }

        vAngles.pitch = pitch;
        vAngles.yaw = yaw;
        vAngles.roll = 0;
    }

}