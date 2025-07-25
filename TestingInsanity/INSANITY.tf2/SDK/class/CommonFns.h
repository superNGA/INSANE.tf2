#pragma once

class BaseEntity;
class baseWeapon;
class CUserCmd;

namespace SDK
{
    bool InAttack(BaseEntity* pPlayer, baseWeapon* pWeapon);
    bool CanAttack(BaseEntity* pPlayer, baseWeapon* pWeapon);
}