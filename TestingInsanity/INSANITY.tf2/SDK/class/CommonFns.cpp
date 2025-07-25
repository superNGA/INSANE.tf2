#include "CommonFns.h"

// SDK
#include "BaseEntity.h"
#include "baseWeapon.h"
#include "CUserCmd.h"
#include "../TF object manager/TFOjectManager.h"

bool SDK::InAttack(BaseEntity* pPlayer, baseWeapon* pWeapon)
{
    float flCurTime = TICK_TO_TIME(pPlayer->m_nTickBase());
    
    return flCurTime < pWeapon->m_flNextPrimaryAttack() && pWeapon->m_iReloadMode() == 0;
}


bool SDK::CanAttack(BaseEntity* pPlayer, baseWeapon* pWeapon)
{    
    // We got ammo ?
    if (pWeapon->getSlot() != WPN_SLOT_MELLE && pWeapon->m_iClip1() == 0)
        return false;

    // We can fire when reloading
    if (pWeapon->m_iReloadMode() != 0)
        return true;

    float flCurTime = TICK_TO_TIME(pPlayer->m_nTickBase());
    return (flCurTime >= pWeapon->m_flNextPrimaryAttack()) && (flCurTime >= pPlayer->m_flNextAttack());
}