#include "BaseWeapon.h"

// Utility
#include "../../Utility/signatures.h"
#include "../../Utility/Interface.h"
#include "../../Utility/PullFromAssembly.h"
#include "../NetVars/NetVarHandler.h"

// SDK
#include "FileWeaponInfo.h"
#include "ETFWeaponType.h"

#define OFFSET_TO_INDEX(offset) ((offset) / sizeof(void*))

//=========================================================================
//                     SIGNATURES
//=========================================================================
MAKE_SIG(baseWeapon_WeaponIDToAlias,      "48 63 C1 48 83 F8 ? 73 ? 85 C9 78 ? 48 8D 0D ? ? ? ? 48 8B 04 C1 C3 33 C0 C3 48 83 E9", CLIENT_DLL, const char*, int)
MAKE_SIG(baseWeapon_LookUpWeaponInfoSlot, "48 8B D1 48 8D 0D ? ? ? ? E9 ? ? ? ? CC 48 89 5C 24 ? 48 89 6C 24", CLIENT_DLL, int16_t, const char*)
MAKE_SIG(baseWeapon_GetWeaponFileHandle,  "66 3B 0D", CLIENT_DLL, FileWeaponInfo_t*, int16_t)

MAKE_SIG(CTFWeaponBase_CalcIsAttackCritHelper,      "48 89 5C 24 ? 55 56 57 48 81 EC ? ? ? ? 0F 29 74 24", "client.dll", bool, baseWeapon*);
MAKE_SIG(CTFWeaponBaseMelee_CalcIsAttackCritHelper, "40 57 48 83 EC ? 48 8B 05 ? ? ? ? 48 8B F9 83 78 ? ? 75", "client.dll", bool, baseWeapon*);

MAKE_SIG(CTFWeaponBase_GetSlot, "48 83 EC ? 0F B7 89 ? ? ? ? E8 ? ? ? ? 8B 80 ? ? ? ? 48 83 C4 ? C3 CC CC CC CC CC 48 81 C1",
    CLIENT_DLL, int64_t, baseWeapon*);

MAKE_SIG(ATRIB_HOOK_FLOAT, "4C 8B DC 49 89 5B ? 49 89 6B ? 56 57 41 54 41 56 41 57 48 83 EC ? 48 8B 3D ? ? ? ? 4C 8D 35",
    CLIENT_DLL, float, float, const char*, void*, void*, bool);

// Since the function signature of CTFWeaponBase::GetWeaponID() is too weak and the function index keep juggling
// BTW 381 & 383, and it keeps breaking my software. We will be getting the function index from the next function which
// is CTFWeaponBase::IsWeapon() which calls the prev. mentioned function via index.
GET_ADRS_FROM_ASSEMBLY(Index_GetWpnID, int, "FF 90 ? ? ? ? 48 98 48 8D 0D ? ? ? ? 8B 04 81 48 83 C4 ? C3", 0x2, 0x4, CLIENT_DLL);


//=========================================================================
//                     FUNCTION DEFINITIONS
//=========================================================================
int baseWeapon::getSlot() 
{
    // can also get from FileWeaponInfo_t->m_iSlot;
    return Sig::CTFWeaponBase_GetSlot(this);
}

bool baseWeapon::IsProjectile()
{
    if (getSlot() == WPN_SLOT_MELLE)
        return false;

    // TODO : ChatGPT told to use Unordered_set cause switch statements can 
    // sometimes be converted to if-else OR Binary searchs instead of jump tables. 
    // And using a unordered_set would give O(1) time in average case.
    switch (GetWeaponDefinitionID())
    {
    case TF_WEAPON_ROCKETLAUNCHER:
    case TF_WEAPON_ROCKETLAUNCHER_DIRECTHIT:
    case TF_WEAPON_GRENADELAUNCHER:
    case TF_WEAPON_PIPEBOMBLAUNCHER:
    case TF_WEAPON_CANNON:
    case TF_WEAPON_FLAREGUN:
    case TF_WEAPON_FLAREGUN_REVENGE:
    case TF_WEAPON_COMPOUND_BOW:
    case TF_WEAPON_CROSSBOW:
    case TF_WEAPON_PARTICLE_CANNON:
    case TF_WEAPON_DRG_POMSON:
    case TF_WEAPON_CLEAVER:
    case TF_WEAPON_STICKY_BALL_LAUNCHER:
    case TF_WEAPON_THROWABLE:
    case TF_WEAPON_GRENADE_THROWABLE:
    case TF_WEAPON_GRENADE_JAR:
    case TF_WEAPON_GRENADE_JAR_MILK:
    case TF_WEAPON_GRENADE_CLEAVER:
    case TF_WEAPON_GRENADE_STICKY_BALL:
    case TF_WEAPON_SPELLBOOK_PROJECTILE:
    case TF_WEAPON_SENTRY_ROCKET:
    case TF_WEAPON_FLAMETHROWER_ROCKET:  // rarely used, but included for completeness
    case TF_WEAPON_GRENADE_WATERBALLOON:
    case TF_WEAPON_GRENADE_MIRVBOMB:
    case TF_WEAPON_GRENADE_PIPEBOMB:
    case TF_WEAPON_GRENADE_NORMAL:
    case TF_WEAPON_GRENADE_CONCUSSION:
    case TF_WEAPON_GRENADE_NAPALM:
    case TF_WEAPON_GRENADE_MIRV:
    case TF_WEAPON_GRENADE_EMP:
    case TF_WEAPON_GRENADE_ORNAMENT_BALL:
    case TF_WEAPON_GRENADE_STUNBALL:
    case TF_WEAPON_GRENADE_HEAL:
    case TF_WEAPON_GRENADE_GAS:
        return true;

    default:
        return false;
    }

    return false;
}


CTFWeaponInfo* baseWeapon::GetTFWeaponInfo()
{
    // TODO : Make it proper with some segfault checks
    auto output = Sig::baseWeapon_GetWeaponFileHandle(
        Sig::baseWeapon_LookUpWeaponInfoSlot(
            Sig::baseWeapon_WeaponIDToAlias(this->GetWeaponTypeID())));
    return static_cast<CTFWeaponInfo*>(output);
}

int baseWeapon::GetWeaponTypeID()
{
    typedef int(__fastcall* T_getID)(void*);
    return reinterpret_cast<T_getID>(util.GetVirtualTable(this)[OFFSET_TO_INDEX(*ASM::Index_GetWpnID)])(this);
}

int baseWeapon::GetWeaponDefinitionID()
{
    return *reinterpret_cast<int32_t*>(
        reinterpret_cast<uintptr_t>(this) + 
/*        Netvars::DT_EconEntity::m_AttributeManager + 
        Netvars::DT_AttributeContainer::m_Item +*/ 
        Netvars::DT_ScriptCreatedItem::m_iItemDefinitionIndex);
}

float baseWeapon::GetDamagePerShot()
{
    // weapon base stats
    const WeaponData_t* pWeaponData = this->GetTFWeaponInfo()->GetWeaponData(0);
    
    // damage per shot
    float flDamage = pWeaponData->m_nDamage;
    
    // adjusting for weapon's buffs & nerfs.
    flDamage       = Sig::ATRIB_HOOK_FLOAT(flDamage, "mult_dmg", static_cast<BaseEntity*>(this), 0, true);
    
    // compensating for multiple bullets per shot weapons like shotguns
    if(pWeaponData->m_nBulletsPerShot > 1)
        flDamage *= Sig::ATRIB_HOOK_FLOAT(pWeaponData->m_nBulletsPerShot, "mult_bullets_per_shot", static_cast<BaseEntity*>(this), 0, true);

    return flDamage;
}
