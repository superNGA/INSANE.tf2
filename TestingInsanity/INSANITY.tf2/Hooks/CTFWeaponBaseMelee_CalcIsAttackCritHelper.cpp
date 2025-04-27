#include "../Utility/Hook_t.h"

MAKE_HOOK(CTFWeaponBaseMelee_CalcIsAttackCriticalHelper, "40 53 48 83 EC ? FF 81", __fastcall, CLIENT_DLL,
    bool, void* pBaseWeapon, float flDamage)
{
    bool result = Hook::CTFWeaponBaseMelee_CalcIsAttackCriticalHelper::O_CTFWeaponBaseMelee_CalcIsAttackCriticalHelper(pBaseWeapon, flDamage);
    printf("%s | damage : %.2f", result ? "TRUE" : "FALSE", flDamage);
    return result;
}