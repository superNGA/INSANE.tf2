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
    if (pLocalPlayer->m_lifeState() != lifeState_t::LIFE_ALIVE)
        return;

    if(pActiveWeapon->getSlot() == WPN_SLOT_MELLE)
    {
        // Smack em niggas!
        FeatureObj::aimbotMelee.RunV3(pLocalPlayer, pActiveWeapon, pCmd, pSendPackets);
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
    int nEntities         = I::IClientEntityList->NumberOfEntities(false);
    int iLocalPlayerIndex = I::iEngine->GetLocalPlayer();
    int iFriendlyTeam     = I::IClientEntityList->GetClientEntity(iLocalPlayerIndex)->m_iTeamNum();

    for (int iEntIndex = 0; iEntIndex < nEntities; iEntIndex++)
    {
        // Skip local player
        if (iEntIndex == iLocalPlayerIndex)
            continue;

        BaseEntity* pEnt = I::IClientEntityList->GetClientEntity(iEntIndex);
        
        // don't want Nullptrs
        if (pEnt == nullptr)
            continue;

        // don't want Dormants
        if (pEnt->IsDormant() == true)
            continue;

        // don't want dead entities.
        if (pEnt->m_lifeState() != lifeState_t::LIFE_ALIVE)
            continue;

        bool bEnemy = (pEnt->m_iTeamNum() != iFriendlyTeam);
        GameObjectID_t iEntID = IDManager.GetObjectID(pEnt);

        // Sorting-n-Filling entities :)
        switch (iEntID)
        {
        case GameObjectID_t::CTF_PLAYER:
            bEnemy == true ?
                m_aimbotTargetData.m_vecEnemyPlayers.push_back(pEnt) :
                m_aimbotTargetData.m_vecFriendlyPlayers.push_back(pEnt);
            break;

        case GameObjectID_t::C_OBJECT_SENTRY_GUN:
            bEnemy == true ?
                m_aimbotTargetData.m_vecEnemySentry.push_back(pEnt) :
                m_aimbotTargetData.m_vecFriendlySentry.push_back(pEnt);
            break;

        case GameObjectID_t::C_OBJECT_DISPENSER:
            bEnemy == true ?
                m_aimbotTargetData.m_vecEnemyDispensers.push_back(pEnt) :
                m_aimbotTargetData.m_vecFriendlyDispensers.push_back(pEnt);
            break;

        case GameObjectID_t::C_OBJECT_TELEPORTER:
            bEnemy == true ?
                m_aimbotTargetData.m_vecEnemyTeleporters.push_back(pEnt) :
                m_aimbotTargetData.m_vecFriendlyTeleporters.push_back(pEnt);
            break;

        case GameObjectID_t::CTF_PROJECTILE_ROCKET:
            bEnemy == true ?
                m_aimbotTargetData.m_vecEnemyRockets.push_back(pEnt) :
                m_aimbotTargetData.m_vecFriendlyRockets.push_back(pEnt);
            break;

        case GameObjectID_t::CTF_PROJECTILE_PIPEBOMB:
            bEnemy == true ?
                m_aimbotTargetData.m_vecEnemyPipeBombs.push_back(pEnt) :
                m_aimbotTargetData.m_vecFriendlyPipeBombs.push_back(pEnt);
            break;
        }

    }
    return;
}

void AimbotHelper_t::_ClearAimbotData()
{
    m_aimbotTargetData.m_vecEnemyPlayers.clear();
    m_aimbotTargetData.m_vecFriendlyPlayers.clear();

    m_aimbotTargetData.m_vecEnemySentry.clear();
    m_aimbotTargetData.m_vecFriendlySentry.clear();

    m_aimbotTargetData.m_vecEnemyDispensers.clear();
    m_aimbotTargetData.m_vecFriendlyDispensers.clear();

    m_aimbotTargetData.m_vecEnemyTeleporters.clear();
    m_aimbotTargetData.m_vecFriendlyTeleporters.clear();

    m_aimbotTargetData.m_vecEnemyRockets.clear();
    m_aimbotTargetData.m_vecFriendlyRockets.clear();

    m_aimbotTargetData.m_vecEnemyPipeBombs.clear();
    m_aimbotTargetData.m_vecFriendlyPipeBombs.clear();
}