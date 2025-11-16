#include "../Utility/Hook Handler/Hook_t.h"
#include "../Utility/ConsoleLogging.h"

#include "../SDK/class/Basic Structures.h"
#include "../Features/config.h"
#include "../Features/BulletTracers/BulletTarcers_t.h"
#include "../Features/Entity Iterator/EntityIterator.h"


///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
MAKE_HOOK(FX_FireBullet, "48 89 5C 24 ? 48 89 74 24 ? 4C 89 4C 24 ? 55", __fastcall, CLIENT_DLL, int64_t, 
    baseWeapon* pWpn, int iPlayerIndex, vec* vOrigin, qangle* qAngles, int iWeapon, int iMode, int iSeed, float flSpread, float flDamage, bool bCritical)
{
    F::tracerHandler.BeginTracer(iPlayerIndex, pWpn);

    int64_t iResult = Hook::FX_FireBullet::O_FX_FireBullet(pWpn, iPlayerIndex, vOrigin, qAngles, iWeapon, iMode, iSeed, flSpread, flDamage, bCritical);
    
    F::tracerHandler.EndTracer();

    return iResult;
}