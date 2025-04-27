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

slot_t baseWeapon::getSlot() {
    typedef slot_t(__fastcall* O_getSlot)(void*);
    return ((O_getSlot)util.GetVirtualTable((void*)this)[331])(this); // calling 331th function. // fix this, make it dynamic
}

reload_t baseWeapon::getReloadMode() {
    return *(reload_t*)((uintptr_t)this + netvar.m_iReloadMode);
}

//================== GET TRACER HOOK =======================
/* VTable hook*/
typedef const char* (__fastcall* T_getTracer)(void* pWeapon);
T_getTracer O_getTracer = nullptr;
const char* __fastcall H_getTracer(void* pWeapon) {
   
    if (pWeapon == (void*)entityManager.getActiveWeapon()) {
        return "merasmus_zap";
    }
    
    return O_getTracer(pWeapon);
}

void baseWeapon::setCustomTracer(const char* tracerName) {
    
    // skipping melle weapon
    if (this->getSlot() == WPN_SLOT_MELLE) return;

    // hooking tracer
    if (!TracerHook) {
        uintptr_t adrs = g_FNindexManager.getFnAdrs(FN_GET_TRACER_TYPE, (void*)this);
        MH_CreateHook((LPVOID)adrs, (LPVOID)H_getTracer, (LPVOID*)&O_getTracer);
        MH_EnableHook((LPVOID)adrs);
        TracerHook = true;
    }
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

int baseWeapon::GetTotalCritsOccured()
{
    return *reinterpret_cast<int*>(reinterpret_cast<uintptr_t>(this) + 0xE9C);
}

int baseWeapon::GetTotalCritChecks()
{
    return *reinterpret_cast<int*>(reinterpret_cast<uintptr_t>(this) + 0xE98);
}