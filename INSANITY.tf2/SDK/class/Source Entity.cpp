#include "Source Entity.h"
#include "../offsets/offsets.h"

extern local_netvars netvar;

int I_client_entity::get_active_weapon_handle()
{
	return (*(int32_t*)((uintptr_t)this + netvar.m_hActiveWeapon)) & 0xFFF; // 0xFFF -> 1111 1111 1111 gets 12 least significant bits, which are the active weapon entity index.
}

// NOTE : this FN has hardcoded offsets, may break with an update. be careful :)
vec I_client_entity::getEntVelocity() {
	return *(vec*)((uintptr_t)this + 0x1D8);
}

// Returns life state, anything other than 0 means dead
lifeState_t I_client_entity::getLifeState() {
	return (lifeState_t)(*(int16_t*)((uintptr_t)this + netvar.m_lifeState));
}

// What character is this player playing?
player_class I_client_entity::getCharacterChoice() {
	return (player_class)(*(int32_t*)((uintptr_t)this + netvar.m_PlayerClass + netvar.m_iClass));
}

// returns the team num for this entity
int16_t I_client_entity::getTeamNum() {
	return *(int16_t*)((uintptr_t)this + netvar.m_iTeamNum);
}