#include "../Utility/Hook Handler/Hook_t.h"
#include "../Utility/PullFromAssembly.h"
#include "../Utility/Interface Handler/Interface.h"
#include "../Utility/ConsoleLogging.h"

#include "../Features/BulletTracers/BulletTarcers_t.h"
#include "../Features/Entity Iterator/EntityIterator.h"
#include "../SDK/class/INetworkStringTable.h"


//MAKE_HOOK(BaseWeapon_GetBulletTracers, "48 89 5C 24 ? 48 89 6C 24 ? 48 89 74 24 ? 57 48 83 EC ? 48 8B D9 E8 ? ? ? ? 4C 8D 0D", __fastcall, "client.dll", const char*,
//    void* pWeapon)
//{
//    //return Features::bulletTracers.Run(pWeapon, 
//        //Hook::BaseWeapon_GetBulletTracers::O_BaseWeapon_GetBulletTracers(pWeapon)); // <- original function
//
//    return Hook::BaseWeapon_GetBulletTracers::O_BaseWeapon_GetBulletTracers(pWeapon);
//}

//MAKE_HOOK(DispatchEffect, "48 89 5C 24 ? 57 48 83 EC ? 48 8B D9 48 8B FA 48 8D 4C 24", __fastcall, CLIENT_DLL, void*, const char* a1, void* a2)
//{
//    LOG("Effect : %s", a1);
//    return Hook::DispatchEffect::O_DispatchEffect(a1, a2);
//}