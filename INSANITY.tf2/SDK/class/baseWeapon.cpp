#include "BaseWeapon.h"

slot_t baseWeapon::getSlot() {
    typedef slot_t(__fastcall* O_getSlot)(void*);
    return ((O_getSlot)util.GetVirtualTable((void*)this)[330])(this); // calling 330th function.
}

reload_t baseWeapon::getReloadMode() {
    return *(reload_t*)((uintptr_t)this + netvar.m_iReloadMode);
}
