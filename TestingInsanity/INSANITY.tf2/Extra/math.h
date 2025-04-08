#pragma once
#include <math.h>
#include "../SDK/class/Basic Structures.h"

const inline float MAX_PITCH = 89.0f;
const inline float MIN_PITCH = -89.0f;
const inline float MAX_YAW = 180.0f;
const inline float MIN_YAW = -180.0f;

namespace Maths
{
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
    inline void AngleVectors(const qangle& angles, vec* forward, vec* right = nullptr, vec* up = nullptr)
    {
        float sp, sy, sr;
        float cp, cy, cr;

        float pitch = DEG2RAD * angles.pitch;
        float yaw = DEG2RAD * angles.yaw;
        float roll = DEG2RAD * angles.roll;

        sp = sinf(pitch);
        cp = cosf(pitch);
        sy = sinf(yaw);
        cy = cosf(yaw);
        sr = sinf(roll);
        cr = cosf(roll);

        if (forward)
        {
            forward->x = cp * cy;
            forward->y = cp * sy;
            forward->z = -sp;
        }

        if (right)
        {
            right->x = -1 * sr * sp * cy + -1 * cr * -sy;
            right->y = -1 * sr * sp * sy + -1 * cr * cy;
            right->z = -1 * sr * cp;
        }

        if (up)
        {
            up->x = cr * sp * cy + -sr * -sy;
            up->y = cr * sp * sy + -sr * cy;
            up->z = cr * cp;
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

}