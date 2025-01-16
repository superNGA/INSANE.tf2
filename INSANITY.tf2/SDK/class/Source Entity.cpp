#include "Source Entity.h"
#include "../offsets/offsets.h"

extern local_netvars netvar;

int I_client_entity::get_active_weapon_handle()
{
	return (*(int32_t*)((uintptr_t)this + netvar.m_hActiveWeapon)) & 0xFFF; // 0xFFF -> 1111 1111 1111 gets 12 least significant bits, which are the active weapon entity index.
}