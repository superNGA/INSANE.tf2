#include "AimbotHelper.h"

// SDK
#include "../../SDK/class/BaseWeapon.h"
#include "../../SDK/class/Source Entity.h"
#include "../../SDK/Class ID Manager/classIDManager.h"

// AIMBOTS
#include "Aimbot Hitscan/AimbotHitscan.h"
#include "Aimbot Projectile/AimbotProjectile.h"
#include "Aimbot Melee/AimbotMelee.h"


//=========================================================================
//                     PUBLIC METHODS
//=========================================================================
void AimbotHelper_t::Run(BaseEntity* pLocalPlayer, baseWeapon* pActiveWeapon, CUserCmd* pCmd, bool* pSendPackets)
{
    // We alive?
    if (pLocalPlayer->getLifeState() != lifeState_t::LIFE_ALIVE)
        return;

    if(pActiveWeapon->getSlot() == WPN_SLOT_MELLE)
    {
        // Smack em niggas!
        FeatureObj::aimbotMelee.Run(pLocalPlayer, pActiveWeapon, pCmd, pSendPackets);
    }
    else if (pActiveWeapon->IsProjectile() == true)
    {
        // Run Projectile Aimbot here
    }
    else
    {
        // Hitscan aimbot
    }
}

//=========================================================================
//                     PRIVATE METHODS
//=========================================================================
void AimbotHelper_t::_ConstructAimbotTargetData()
{
    int nEntities = I::IClientEntityList->NumberOfEntities(false);
    
    for (int iEntIndex = 0; iEntIndex < nEntities; iEntIndex++)
    {
        BaseEntity* pEnt = I::IClientEntityList->GetClientEntity(iEntIndex);
        
        // don't want Nullptrs
        if (pEnt == nullptr)
            continue;

        // don't want Dormants
        if (pEnt->IsDormant() == true)
            continue;

        // don't want dead entities.
        if (pEnt->getLifeState() != lifeState_t::LIFE_ALIVE)
            continue;

        IDclass_t iEntID = IDManager.getID(pEnt);
        switch (iEntID)
        {
        case PLAYER:     // Store Enemy Players
        {
            if (pEnt->isEnemy() == true)
                m_AllTargets.m_vecEnemyPlayers.push_back(pEnt);
            break;
        }
        case DISPENSER:  // Store Enemy No-Harm Buildings
        case TELEPORTER:
        {
            if (pEnt->isEnemy() == true)
                m_AllTargets.m_vecEnemyBuildings.push_back(pEnt);
            break;
        }
        case SENTRY_GUN: // Store sentries
        {
            if (pEnt->isEnemy() == true)
                m_AllTargets.m_vecEnemySentry.push_back(pEnt);
            break;
        }
        case ROCKET:     // Store projectiles
        case DEMO_PROJECTILES:
        {
            if (pEnt->isEnemy() == true)
                m_AllTargets.m_vecEnemyProjectiles.push_back(pEnt);
            break;
        }
        default:
            break;
        }
    }
}

void AimbotHelper_t::_ClearAimbotData()
{
    m_AllTargets.m_vecEnemyPlayers.clear();
    m_AllTargets.m_vecEnemyBuildings.clear();
    m_AllTargets.m_vecEnemySentry.clear();
    m_AllTargets.m_vecEnemyProjectiles.clear();
}