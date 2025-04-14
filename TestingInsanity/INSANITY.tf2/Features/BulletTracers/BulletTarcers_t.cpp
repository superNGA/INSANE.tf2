#include "BulletTarcers_t.h"
#include "../../Utility/signatures.h"

MAKE_SIG(BaseWeapon_GetTracers, "48 89 5C 24 ? 48 89 6C 24 ? 48 89 74 24 ? 57 48 83 EC ? 48 8B D9 E8 ? ? ? ? 4C 8D 0D", "client.dll")

const char* BulletTarcers_t::Run(void* pWeapon, const char* szOriginalOutput)
{
    // if don't wanna then ain't gonna :)
    if (_ShouldRun(pWeapon) == false)
        return szOriginalOutput;

    //return "dxhr_lightningball_hit_zap_red";
    //return "bullet_tracer_raygun_red";
    //return "bullet_bignasty_tracer01_blue";
    //return "tfc_sniper_distortion_trail";
    return "merasmus_zap_beam02";
}

bool BulletTarcers_t::_ShouldRun(void* pWeapon)
{
    // can't take some more scalable code here...
    auto pActiveWeapon = entityManager.getActiveWeapon();
    if (pActiveWeapon == nullptr || pWeapon != pActiveWeapon)
        return false;

    return true;
}