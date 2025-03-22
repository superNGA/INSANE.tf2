#include "BaseWeapon.h"
#include "../FN index Manager/FN index manager.h"
#include "../Entity Manager/entityManager.h"
#include "../../External Libraries/MinHook/MinHook.h"

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