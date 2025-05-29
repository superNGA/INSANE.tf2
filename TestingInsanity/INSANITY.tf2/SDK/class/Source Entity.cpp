#include "Source Entity.h"
#include "../offsets/offsets.h"
#include "../TF object manager/TFOjectManager.h"
#include "../Entity Manager/entityManager.h"
#include "../../Extra/math.h"
#include "../../Utility/signatures.h"
#include "../../SDK/class/IVEngineClient.h"

MAKE_SIG(mShared_IsCritBoosted, "48 89 5C 24 ? 57 48 83 EC ? 48 8B D9 0F 29 7C 24", CLIENT_DLL, bool, uintptr_t);

MAKE_SIG(CALL_ATRIB_HOOK_INT, "4C 8B DC 49 89 5B ? 49 89 6B ? 49 89 73 ? 57 41 54 41 55 41 56 41 57 48 83 EC ? 48 8B 3D ? ? ? ? 4C 8D 35", CLIENT_DLL, 
	int, int, const char*, BaseEntity*, int, bool)
MAKE_SIG(CALL_ATRIB_HOOK_FLOAT, "4C 8B DC 49 89 5B ? 49 89 6B ? 56 57 41 54 41 56 41 57 48 83 EC ? 48 8B 3D ? ? ? ? 4C 8D 35", CLIENT_DLL, 
	float, float, const char*, void*, void*, bool);

extern local_netvars netvar;

// 0x11D8 
baseWeapon* BaseEntity::getActiveWeapon()
{
	return reinterpret_cast<baseWeapon*>(I::IClientEntityList->GetClientEntity((*(int32_t*)((uintptr_t)this + netvar.m_hActiveWeapon)) & 0xFFF)); // 0xFFF -> 1111 1111 1111 gets 12 least significant bits, which are the active weapon entity index.
}

// NOTE : this FN has hardcoded offsets, may break with an update. be careful :)
vec BaseEntity::getEntVelocity() {
	return *(vec*)((uintptr_t)this + 0x1D8);
}

// E1
// Returns life state, anything other than 0 means dead
lifeState_t BaseEntity::getLifeState() {
	return (lifeState_t)(*(int16_t*)((uintptr_t)this + netvar.m_lifeState));
}

// 0x1BA8 + 0x8
// What character is this player playing?
player_class BaseEntity::getCharacterChoice() {
	return (player_class)(*(int32_t*)((uintptr_t)this + netvar.m_PlayerClass + netvar.m_iClass));
}

const char* BaseEntity::GetPlayerClassName()
{
	switch (getCharacterChoice())
	{
	case TF_SCOUT:   return "Scout";
	case TF_SNIPER:  return "Sniper";
	case TF_SOLDIER: return "Soldier";
	case TF_DEMOMAN: return "Demoman";
	case TF_MEDIC:	 return "Medic";
	case TF_HEAVY:	 return "Heavy";
	case TF_PYRO:	 return "Pyro";
	case TF_SPY:	 return "Spy";
	case TF_ENGINEER:return "Engineer";
	default:		 return "BullShit";
	}

	return "UnicornShit";
}

// 0xEC
// returns the team num for this entity
int32_t BaseEntity::getTeamNum()
{
	return *reinterpret_cast<int32_t*>(reinterpret_cast<uintptr_t>(this) + netvar.m_iTeamNum);
}

bool BaseEntity::isEnemy()
{
	if (entityManager.GetLocalPlayer() == nullptr)
		return false;

	return entityManager.GetLocalPlayer()->getTeamNum() != this->getTeamNum();
}

// 0x11DD
// setting glow
void BaseEntity::setGlow(bool b_glowStatus) {
	*(bool*)((uintptr_t)this + netvar.m_bGlowEnabled) = b_glowStatus;
}

// 0xC38 + 0x90 + 0x48 = 0xD10
// return the weapon index
int32_t BaseEntity::getWeaponIndex() {
	
	// causing crashes so had to put this check
	if (this != nullptr) {
		return *(int32_t*)((uintptr_t)this + netvar.m_AttributeManager + netvar.m_Item + netvar.m_iItemDefinitionIndex);
	}
	return 0;
}


uint32_t BaseEntity::getEntHealth() const
{
	return *(int16_t*)((uintptr_t)this + netvar.m_iHealth);
}


bool BaseEntity::isDisguised() {
	
	// is player a spy ?
	if (getCharacterChoice() != TF_SPY)
		return false;

	// checking if disguised or not
	int32_t playerCond = *(int32_t*)((uintptr_t)this + netvar.m_Shared + netvar.m_nPlayerCond);
	if(playerCond & (1 << TF_COND_DISGUISED))
		return true;

	return false;
}


bool BaseEntity::isCloaked() {
	
	// is player a spy ?
	if (getCharacterChoice() != TF_SPY)
		return false;

	// checking if disguised or not
	int32_t playerCond = *(int32_t*)((uintptr_t)this + netvar.m_Shared + netvar.m_nPlayerCond);

	if (playerCond & (1 << TF_COND_STEALTHED))
		return true;

	return false;
}


void BaseEntity::changeThirdPersonVisibility(renderGroup_t renderGroup) {

	// skip if not in thirdpeson
	if (!config.miscConfig.third_person) {
		return;
	}

	// skip if not selected
	if (!config.viewConfig.alwaysRenderInThirdPerson) {
		return;
	}

	tfObject.addToLeafSystem(this, renderGroup);
}


vec BaseEntity::getLocalEyePos() const
{
	// adding eye offset in abs origin for local player
	return (GetAbsOrigin() + *reinterpret_cast<vec*>(reinterpret_cast<uintptr_t>(this) + netvar.m_vecViewOffset));
}


int32_t BaseEntity::getPlayerCond() {

	return *(int32_t*)((uintptr_t)this + netvar.m_Shared + netvar.m_nPlayerCond);
}

bool BaseEntity::InCond(FLAG_playerCond eCond)
{
	// Getting Player Condition array
	uint32_t* iPlayerCondArray = reinterpret_cast<uint32_t*>(reinterpret_cast<uintptr_t>(this) + netvar.m_Shared + netvar.m_nPlayerCond);
	
	// Getting target player condition & bit
	int iCondIndex		 = eCond / (sizeof(uint32_t) * 8);
	int iTargetBit		 = eCond % (sizeof(uint32_t) * 8);
	uint32_t iPlayerCond = iPlayerCondArray[iCondIndex];

	// returning target bit
	return iPlayerCond & (1 << iTargetBit);
}

bool BaseEntity::isOnGround()
{
	return (*(int32_t*)((uintptr_t)this + netvar.m_fFlags) & (1 << 0));
}

uint32_t BaseEntity::GetTickBase()
{
	return *reinterpret_cast<uint32_t*>(reinterpret_cast<uintptr_t>(this) + netvar.m_nTickBase);
}

float BaseEntity::GetCritMult()
{
	int flCritMult = *reinterpret_cast<int*>(reinterpret_cast<uintptr_t>(this) + netvar.m_Shared + netvar.m_iCritMult);
	return Maths::RemapValClamped(static_cast<float>(flCritMult), 0.0f, 255.0f, 1.0, 4.0);
}

RoundStats_t* BaseEntity::GetPlayerRoundData()
{
	return reinterpret_cast<RoundStats_t*>(reinterpret_cast<uintptr_t>(this) + netvar.m_Shared + netvar.m_RoundScoreData);
}

bool BaseEntity::IsCritBoosted()
{
	return Sig::mShared_IsCritBoosted(reinterpret_cast<uintptr_t>(this) + netvar.m_Shared);
}

bool BaseEntity::IsFeignDeathReady()
{
	return *reinterpret_cast<bool*>(reinterpret_cast<uintptr_t>(this) + netvar.m_Shared + netvar.m_bFeignDeathReady);
}

float BaseEntity::GetModelScale()
{
	return *reinterpret_cast<float*>(reinterpret_cast<uintptr_t>(this) + netvar.m_flModelScale);
}

void BaseEntity::CALL_ATRIB_HOOK_INT(int& iAtributeOut, const char* szAtribute)
{
	iAtributeOut = Sig::CALL_ATRIB_HOOK_INT(iAtributeOut, szAtribute, this, 0, true);
}

void BaseEntity::CALL_ATRIB_HOOK_FLOAT(float& flAtributeOut, const char* szAtribute)
{
	flAtributeOut = Sig::CALL_ATRIB_HOOK_FLOAT(flAtributeOut, szAtribute, this, 0, true);
}

BaseEntity* I_client_entity_list::GetClientEntityFromUserID(int userID)
{
	int iEntIndex = I::iEngine->GetPlayerForUserID(userID);
	return GetClientEntity(iEntIndex);
}