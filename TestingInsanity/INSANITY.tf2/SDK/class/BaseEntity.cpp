#include "BaseEntity.h"

// UTILITY
#include "../TF object manager/TFOjectManager.h"
#include "../Entity Manager/entityManager.h"
#include "../../Extra/math.h"
#include "../../Utility/Signature Handler/signatures.h"

// SDK
#include "../../SDK/class/IVEngineClient.h"

MAKE_SIG(mShared_IsCritBoosted, "48 89 5C 24 ? 57 48 83 EC ? 48 8B D9 0F 29 7C 24", CLIENT_DLL, bool, uintptr_t);

MAKE_SIG(CALL_ATRIB_HOOK_INT, "4C 8B DC 49 89 5B ? 49 89 6B ? 49 89 73 ? 57 41 54 41 55 41 56 41 57 48 83 EC ? 48 8B 3D ? ? ? ? 4C 8D 35", CLIENT_DLL,
	int, int, const char*, const BaseEntity*, int, bool)
MAKE_SIG(CALL_ATRIB_HOOK_FLOAT, "4C 8B DC 49 89 5B ? 49 89 6B ? 56 57 41 54 41 56 41 57 48 83 EC ? 48 8B 3D ? ? ? ? 4C 8D 35", CLIENT_DLL,
		float, float, const char*, const BaseEntity*, void*, bool);


// 0x11D8 
baseWeapon* BaseEntity::getActiveWeapon()
{
	return reinterpret_cast<baseWeapon*>(I::IClientEntityList->GetClientEntity((*(int32_t*)((uintptr_t)this + Netvars::DT_BaseCombatCharacter::m_hActiveWeapon)) & 0xFFF)); // 0xFFF -> 1111 1111 1111 gets 12 least significant bits, which are the active weapon entity index.
}

const char* BaseEntity::GetPlayerClassName()
{
	switch (this->m_iClass())
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

bool BaseEntity::IsEnemy()
{
	if (entityManager.GetLocalPlayer() == nullptr)
		return false;

	return entityManager.GetLocalPlayer()->m_iTeamNum() != this->m_iTeamNum();
}

// 0xC38 + 0x90 + 0x48 = 0xD10
// return the weapon index
int32_t BaseEntity::GetWeaponIndex() 
{
	return *reinterpret_cast<int32_t*>(
		reinterpret_cast<uintptr_t>(this) + 
		Netvars::DT_EconEntity::m_AttributeManager + 
		Netvars::DT_AttributeContainer::m_Item + 
		Netvars::DT_ScriptCreatedItem::m_iItemDefinitionIndex
	);
}

void BaseEntity::changeThirdPersonVisibility(renderGroup_t renderGroup) {

	// skip if not in thirdpeson
	//if (!config.miscConfig.third_person) {
	//	return;
	//}

	//// skip if not selected
	//if (!config.viewConfig.alwaysRenderInThirdPerson) {
	//	return;
	//}

	//tfObject.addToLeafSystem(this, renderGroup);

	return;
}


vec BaseEntity::GetEyePos() const
{
	return (GetAbsOrigin() + *reinterpret_cast<vec*>(reinterpret_cast<uintptr_t>(this) + Netvars::DT_LocalPlayerExclusive::m_vecViewOffset));
}


uint32_t BaseEntity::GetPlayerCond() 
{
	return *reinterpret_cast<uint32_t*>(reinterpret_cast<uintptr_t>(this) + Netvars::DT_TFPlayerShared::m_nPlayerCond);
}

bool BaseEntity::InCond(FLAG_playerCond eCond)
{
	// Getting Player Condition array
	uint32_t* iPlayerCondArray = reinterpret_cast<uint32_t*>(reinterpret_cast<uintptr_t>(this) + Netvars::DT_TFPlayerShared::m_nPlayerCond);

	// Getting target player condition & bit
	int iCondIndex = eCond / (sizeof(uint32_t) * 8);
	int iTargetBit = eCond % (sizeof(uint32_t) * 8);
	uint32_t iPlayerCond = iPlayerCondArray[iCondIndex];

	// returning target bit
	return iPlayerCond & (1 << iTargetBit);
}

bool BaseEntity::isOnGround()
{
	return this->m_fFlags() & (1 << 0);
}

float BaseEntity::GetCritMult()
{
	int flCritMult = *reinterpret_cast<int*>(reinterpret_cast<uintptr_t>(this) + Netvars::DT_TFPlayerShared::m_iCritMult);
	return Maths::RemapValClamped(static_cast<float>(flCritMult), 0.0f, 255.0f, 1.0, 4.0);
}

RoundStats_t* BaseEntity::GetPlayerRoundData()
{
	return reinterpret_cast<RoundStats_t*>(reinterpret_cast<uintptr_t>(this) + Netvars::DT_TFPlayerSharedLocal::m_RoundScoreData);
}

bool BaseEntity::IsCritBoosted()
{
	return Sig::mShared_IsCritBoosted(reinterpret_cast<uintptr_t>(this) + Netvars::DT_TFPlayer::m_Shared);
}

bool BaseEntity::IsFeignDeathReady()
{
	return *reinterpret_cast<bool*>(reinterpret_cast<uintptr_t>(this) + Netvars::DT_TFPlayerShared::m_bFeignDeathReady);
}

void BaseEntity::CALL_ATRIB_HOOK_INT(int& iAtributeOut, const char* szAtribute) const
{
	iAtributeOut = Sig::CALL_ATRIB_HOOK_INT(iAtributeOut, szAtribute, this, 0, true);
}

void BaseEntity::CALL_ATRIB_HOOK_FLOAT(float& flAtributeOut, const char* szAtribute) const
{
	flAtributeOut = Sig::CALL_ATRIB_HOOK_FLOAT(flAtributeOut, szAtribute, this, 0, true);
}

BaseEntity* I_client_entity_list::GetClientEntityFromUserID(int userID)
{
	int iEntIndex = I::iEngine->GetPlayerForUserID(userID);
	return GetClientEntity(iEntIndex);
}

int32_t BaseEntity::GetAirDash()
{
	return *reinterpret_cast<int32_t*>(reinterpret_cast<uintptr_t>(this) + Netvars::DT_TFPlayerShared::m_iAirDash);
}