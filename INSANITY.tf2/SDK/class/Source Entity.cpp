#include "Source Entity.h"
#include "../offsets/offsets.h"

extern local_netvars netvar;

// 0x11D8 
baseWeapon* I_client_entity::getActiveWeapon()
{
	return reinterpret_cast<baseWeapon*>(interface_tf2::entity_list->GetClientEntity((*(int32_t*)((uintptr_t)this + netvar.m_hActiveWeapon)) & 0xFFF)); // 0xFFF -> 1111 1111 1111 gets 12 least significant bits, which are the active weapon entity index.
}

// NOTE : this FN has hardcoded offsets, may break with an update. be careful :)
vec I_client_entity::getEntVelocity() {
	return *(vec*)((uintptr_t)this + 0x1D8);
}

// E1
// Returns life state, anything other than 0 means dead
lifeState_t I_client_entity::getLifeState() {
	return (lifeState_t)(*(int16_t*)((uintptr_t)this + netvar.m_lifeState));
}

// 0x1BA8 + 0x8
// What character is this player playing?
player_class I_client_entity::getCharacterChoice() {
	return (player_class)(*(int32_t*)((uintptr_t)this + netvar.m_PlayerClass + netvar.m_iClass));
}

// 0xEC
// returns the team num for this entity
int16_t I_client_entity::getTeamNum() {
	return *(int16_t*)((uintptr_t)this + netvar.m_iTeamNum);
}

// 0x11DD
// setting glow
void I_client_entity::setGlow(bool b_glowStatus) {
	*(bool*)((uintptr_t)this + netvar.m_bGlowEnabled) = b_glowStatus;
}

// 0xC38 + 0x90 + 0x48 = 0xD10
// return the weapon index
int32_t I_client_entity::getWeaponIndex() {
	
	// causing crashes so had to put this check
	if (this != nullptr) {
		return *(int32_t*)((uintptr_t)this + netvar.m_AttributeManager + netvar.m_Item + netvar.m_iItemDefinitionIndex);
	}
	return 0;
}

int16_t I_client_entity::getEntHealth() {
	return *(int16_t*)((uintptr_t)this + netvar.m_iHealth);
}


bool I_client_entity::isDisguised() {
	
	// is player a spy ?
	if (getCharacterChoice() != TF_SPY)
		return false;

	// checking if disguised or not
	int32_t playerCond = *(int32_t*)((uintptr_t)this + netvar.m_Shared + netvar.m_nPlayerCond);
	
	if(playerCond & (1 << TF_COND_DISGUISED))
		return true;

	return false;
}


bool I_client_entity::isCloaked() {
	
	// is player a spy ?
	if (getCharacterChoice() != TF_SPY)
		return false;

	// checking if disguised or not
	int32_t playerCond = *(int32_t*)((uintptr_t)this + netvar.m_Shared + netvar.m_nPlayerCond);

	if (playerCond & (1 << TF_COND_STEALTHED))
		return true;

	return false;
}


void I_client_entity::changeThirdPersonVisibility(renderGroup_t renderGroup) {

	// skip if not in thirdpeson
	if (!config.miscConfig.third_person) {
		return;
	}

	// skip if not selected
	if (!config.viewConfig.alwaysRenderInThirdPerson) {
		return;
	}

	TF2_functions::FN_addToLeafSystem(this, renderGroup);
}


vec I_client_entity::getLocalEyePos() {

	// adding eye offset in abs origin for local player
	return (this->GetAbsOrigin() + vec(0.0f, 0.0f, *(float*)((uintptr_t)this + netvar.m_vecViewOffset)));
}


int32_t I_client_entity::getPlayerCond() {

	return *(int32_t*)((uintptr_t)this + netvar.m_Shared + netvar.m_nPlayerCond);
}