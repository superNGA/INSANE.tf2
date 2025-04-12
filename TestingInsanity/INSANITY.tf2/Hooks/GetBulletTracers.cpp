#include "../Utility/Hook_t.h"
#include "../Utility/signatures.h"
#include "../Utility/ConsoleLogging.h"
#include "../Features/BulletTracers/BulletTarcers_t.h"

MAKE_SIG(GetTracer, "48 89 5C 24 ? 48 89 6C 24 ? 48 89 74 24 ? 57 48 83 EC ? 48 8B D9 E8 ? ? ? ? 4C 8D 0D", "client.dll")
MAKE_HOOK(BaseWeapon_GetBulletTracers, "48 89 5C 24 ? 48 89 6C 24 ? 48 89 74 24 ? 57 48 83 EC ? 48 8B D9 E8 ? ? ? ? 4C 8D 0D", __fastcall, "client.dll", const char*,
    void* pWeapon)
{
    //return Features::bulletTracers.Run(pWeapon);
    auto output = Hook::BaseWeapon_GetBulletTracers::O_BaseWeapon_GetBulletTracers(pWeapon);
    //printf("%s\n", output);
    //return output;
    return "merasmus_zap_beam01";
}