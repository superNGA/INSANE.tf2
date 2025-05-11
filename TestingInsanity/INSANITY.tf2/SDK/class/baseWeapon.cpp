#include "BaseWeapon.h"
#include "../FN index Manager/FN index manager.h"
#include "../Entity Manager/entityManager.h"
#include "../../External Libraries/MinHook/MinHook.h"
#include "../../Utility/signatures.h"
#include "FileWeaponInfo.h"

//----------------------- SIGNATURES -----------------------
MAKE_SIG(baseWeapon_WeaponIDToAlias, "48 63 C1 48 83 F8 ? 73 ? 85 C9 78 ? 48 8D 0D ? ? ? ? 48 8B 04 C1 C3 33 C0 C3 48 83 E9", CLIENT_DLL, const char*, int)
MAKE_SIG(baseWeapon_LookUpWeaponInfoSlot, "48 8B D1 48 8D 0D ? ? ? ? E9 ? ? ? ? CC 48 89 5C 24 ? 48 89 6C 24", CLIENT_DLL, int16_t, const char*)
MAKE_SIG(baseWeapon_GetWeaponFileHandle, "66 3B 0D", CLIENT_DLL, FileWeaponInfo_t*, int16_t)

MAKE_SIG(CTFWeaponBase_CalcIsAttackCritHelper, "48 89 5C 24 ? 55 56 57 48 81 EC ? ? ? ? 0F 29 74 24", "client.dll", bool, baseWeapon*);
MAKE_SIG(CTFWeaponBaseMelee_CalcIsAttackCritHelper, "40 57 48 83 EC ? 48 8B 05 ? ? ? ? 48 8B F9 83 78 ? ? 75", "client.dll", bool, baseWeapon*);

MAKE_SIG(ATRIB_HOOK_FLOAT, "4C 8B DC 49 89 5B ? 49 89 6B ? 56 57 41 54 41 56 41 57 48 83 EC ? 48 8B 3D ? ? ? ? 4C 8D 35",
    CLIENT_DLL, float, float, const char*, void*, void*, bool);

slot_t baseWeapon::getSlot() {
    typedef slot_t(__fastcall* O_getSlot)(void*);
    return ((O_getSlot)util.GetVirtualTable((void*)this)[331])(this); // calling 331th function. // fix this, make it dynamic
}

reload_t baseWeapon::getReloadMode() {
    return *(reload_t*)((uintptr_t)this + netvar.m_iReloadMode);
}


bool baseWeapon::canBackStab()
{
    if (this->getSlot() != slot_t::WPN_SLOT_MELLE)
        return false;

    return *(bool*)((uintptr_t)this + netvar.m_bReadyToBackstab);
}

CTFWeaponInfo* baseWeapon::GetTFWeaponInfo()
{
    // make something better to get weapon id
    auto output = Sig::baseWeapon_GetWeaponFileHandle(
        Sig::baseWeapon_LookUpWeaponInfoSlot(
            Sig::baseWeapon_WeaponIDToAlias(this->GetWeaponID())));
    return static_cast<CTFWeaponInfo*>(output);
}

int baseWeapon::GetWeaponID()
{
    typedef int(__fastcall* T_getID)(void*);
    return reinterpret_cast<T_getID>(util.GetVirtualTable(this)[382])(this);
}

bool baseWeapon::CanCrit()
{
    if (this->getSlot() == WPN_SLOT_MELLE)
        return Sig::CTFWeaponBaseMelee_CalcIsAttackCritHelper(this);

    return Sig::CTFWeaponBase_CalcIsAttackCritHelper(this);
}


float baseWeapon::GetCritBucket()
{
    return *reinterpret_cast<float*>(reinterpret_cast<uintptr_t>(this) + 0xE94);
}

void baseWeapon::SetCritBucket(float flCritBucket)
{
    *reinterpret_cast<float*>(reinterpret_cast<uintptr_t>(this) + 0xE94) = flCritBucket;
}

int baseWeapon::GetTotalCritsOccured()
{
    return *reinterpret_cast<int*>(reinterpret_cast<uintptr_t>(this) + 0xE9C);
}

void baseWeapon::SetTotalCritsOccured(int iCritsOccured)
{
    *reinterpret_cast<int*>(reinterpret_cast<uintptr_t>(this) + 0xE9C) = iCritsOccured;
}

int baseWeapon::GetTotalCritChecks()
{
    return *reinterpret_cast<int*>(reinterpret_cast<uintptr_t>(this) + 0xE98);
}

void baseWeapon::SetTotalCritChecks(int iCritChecks)
{
    *reinterpret_cast<int*>(reinterpret_cast<uintptr_t>(this) + 0xE98) = iCritChecks;
}

float baseWeapon::GetObservedCritChance()
{
    return *reinterpret_cast<float*>(reinterpret_cast<uintptr_t>(this) + netvar.m_flObservedCritChance);
}

float baseWeapon::GetDamagePerShot()
{
    // weapon base stats
    const WeaponData_t& pWeaponData = this->GetTFWeaponInfo()->GetWeaponData(0);
    
    // damage per shot
    float flDamage = pWeaponData.m_nDamage;
    
    // adjusting for weapon's buffs & nerfs.
    flDamage       = Sig::ATRIB_HOOK_FLOAT(flDamage, "mult_dmg", static_cast<BaseEntity*>(this), 0, true);
    
    // compensating for multiple bullets per shot weapons like shotguns
    if(pWeaponData.m_nBulletsPerShot > 1)
        flDamage *= Sig::ATRIB_HOOK_FLOAT(pWeaponData.m_nBulletsPerShot, "mult_bullets_per_shot", static_cast<BaseEntity*>(this), 0, true);

    return flDamage;
}

void baseWeapon::SetWeaponSeed(int iSeed)
{
    *reinterpret_cast<int*>(reinterpret_cast<uintptr_t>(this) + 0xFC0) = iSeed;
}

float baseWeapon::GetNextPrimaryAttackTime()
{
    return *reinterpret_cast<float*>(reinterpret_cast<uintptr_t>(this) + netvar.m_flNextPrimaryAttack);
}

float baseWeapon::GetLastRapidFireCritCheckTime()
{
    return *reinterpret_cast<float*>(reinterpret_cast<uintptr_t>(this) + netvar.m_flLastCritCheckTime);
}