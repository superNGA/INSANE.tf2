#include "../Utility/Hook_t.h"

// isAllowedToWithdrawlfrom : 40 53 48 83 EC ? FF 81
// caclIsAttackCritcalHelperMelee : 40 57 48 83 EC ? 48 8B 05 ? ? ? ? 48 8B F9 83 78 ? ? 75

MAKE_HOOK(CTFWeaponBaseMelee_CalcIsAttackCriticalHelper, "40 53 48 83 EC ? FF 81", __fastcall, CLIENT_DLL,
    bool, void* pBaseWeapon)
{
    bool result = Hook::CTFWeaponBaseMelee_CalcIsAttackCriticalHelper::O_CTFWeaponBaseMelee_CalcIsAttackCriticalHelper(pBaseWeapon);
    printf("%s\n", result ? "TRUE" : "FALSE");
    return result;
}