#pragma once

class BaseEntity;
class baseWeapon;
class CUserCmd;

namespace SDK
{
    bool InAttack(BaseEntity* pPlayer, baseWeapon* pWeapon, CUserCmd* pCmd);
    bool CanAttack(BaseEntity* pPlayer, baseWeapon* pWeapon);
}