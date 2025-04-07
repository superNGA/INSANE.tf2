#pragma once
#include <math.h>
#include "../SDK/class/Basic Structures.h"

// working now
inline void AngleVectors(const qangle& angles, vec* forward, vec* right = nullptr, vec* up = nullptr)
{
    float sp, sy, sr;
    float cp, cy, cr;

    float pitch = DEG2RAD * angles.pitch;
    float yaw   = DEG2RAD * angles.yaw;
    float roll  = DEG2RAD * angles.roll;

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


inline void VectorAngles(const vec& forward, qangle& angles)
{
	float	tmp, yaw, pitch;

	if (forward.y == 0 && forward.x == 0)
	{
		yaw = 0;
		if (forward.z > 0)
			pitch = 270;
		else
			pitch = 90;
	}
	else
	{
		yaw = (atan2(forward.y, forward.x) * 180 / M_PI);
		if (yaw < 0)
			yaw += 360;

		tmp = sqrtf(forward.x * forward.x + forward.y * forward.y);
		pitch = (atan2(-forward.z, tmp) * 180 / M_PI);
		if (pitch < 0)
			pitch += 360;
	}

	angles.pitch = pitch;
	angles.yaw = yaw;
	angles.roll = 0;
}