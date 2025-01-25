#include "thread2.h"

vec proj_aimbot_calc(vec ent_pos, vec ent_vel, bool on_ground, qangle viewangles)
{
    // Convert viewangles (degrees) to radians
    viewangles.pitch = -viewangles.pitch * M_PI / 180; // Pitch is inverted in-game
    viewangles.yaw = viewangles.yaw * M_PI / 180;

    // Calculate direction vector for the projectile
    vec direction_vec;
    direction_vec.z = 0; // No vertical component
    direction_vec.x = cos(viewangles.pitch) * cos(viewangles.yaw);
    direction_vec.y = cos(viewangles.pitch) * sin(viewangles.yaw);

    // Projectile speed (hardcoded for simplicity)
    float projectile_speed = DIRECT_HIT_ROCKET_LAUNCHER;

    // Calculate relative velocity
    vec relative_vel;
    relative_vel.x = projectile_speed * direction_vec.x - ent_vel.x;
    relative_vel.y = projectile_speed * direction_vec.y - ent_vel.y;

    // Time to intercept
    vec displacement = ent_pos - entities::local::eye_pos; // Vector from you to the enemy
    float time = (displacement.x * relative_vel.x + displacement.y * relative_vel.y) /
        (relative_vel.x * relative_vel.x + relative_vel.y * relative_vel.y);

    // Ensure time is positive (projectile cannot hit in the past)
    if (time < 0) {
        time = 0;
    }

    // Predict enemy's future position
    vec predicted_pos;
    predicted_pos.x = ent_pos.x + ent_vel.x * time;
    predicted_pos.y = ent_pos.y + ent_vel.y * time;
    predicted_pos.z = ent_pos.z; // No change in Z (enemy always on the ground)

    return predicted_pos;
}

void decide_bone_id()
{
	/* Getting proper bone */
	switch (entities::local::localplayer_class)
	{
	case TF_SNIPER:
		entities::target::target_bone = BONE_HEAD;
		break;
	default:
		entities::target::target_bone = BONE_CHEST;
		break;
	}
}