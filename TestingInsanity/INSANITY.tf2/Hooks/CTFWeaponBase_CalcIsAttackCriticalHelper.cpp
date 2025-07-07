// UTILITY
#include "../Utility/ConsoleLogging.h"
#include "../Utility/Hook Handler/Hook_t.h"
#include "../Utility/Signature Handler/signatures.h"
#include "../Features/CritHack/CritHack.h"

class baseWeapon;

// isAllowedToWithdrawlfrom : 40 53 48 83 EC ? FF 81
// caclIsAttackCritcalHelperMelee : 40 57 48 83 EC ? 48 8B 05 ? ? ? ? 48 8B F9 83 78 ? ? 75
// CalcIsAttackCritcalhelper BaseWeapon : 48 89 5C 24 ? 55 56 57 48 81 EC ? ? ? ? 0F 29 74 24
// CalcIsAttck : BaseWeapon : 48 89 74 24 ? 57 48 83 EC ? 48 8B F9 E8 ? ? ? ? 48 8B C8 C7 44 24 ? ? ? ? ? 4C 8D 0D ? ? ? ? 33 D2 4C 8D 05 ? ? ? ? E8 ? ? ? ? 48 8B F0 48 85 C0 0F 84 ? ? ? ? 48 8B 10


//MAKE_HOOK(CTFWeaponBase_CalcIsAttackCriticalHelper, "48 89 5C 24 ? 55 56 57 48 81 EC ? ? ? ? 0F 29 74 24", __fastcall, CLIENT_DLL, bool,
//    baseWeapon* pActiveWeapon)
//{
//    WeaponCritData_t* pActiveWeaponCritData = Features::critHack.GetWeaponCritData(pActiveWeapon);
//    pActiveWeaponCritData->AddToCritBucket();
//    printf("--> called! BASE WEAPON <-- ( make sure it ain't been called for other players on the server, which it shouldn't (cause common sense :| ))\n");
//    return Hook::CTFWeaponBase_CalcIsAttackCriticalHelper::O_CTFWeaponBase_CalcIsAttackCriticalHelper(pActiveWeapon);
//}