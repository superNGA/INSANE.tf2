#include "thread2.h"

vec proj_aimbot_calc(vec ent_pos, vec ent_vel)
{
	/* caclulating distance from local player to entity */
	float time = sqrt(pow(entities::local::eye_pos.x - ent_pos.x, 2) + pow(entities::local::eye_pos.y - ent_pos.y, 2) + pow(entities::local::eye_pos.z - ent_pos.z, 2)) / 1100.0f;

	/* return cacluated positions */
	return vec(ent_pos.x + ent_vel.x * time, ent_pos.y + ent_vel.y * time, (ent_pos.z * time) + (TF2_GRAVITY * time * time / 2));
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