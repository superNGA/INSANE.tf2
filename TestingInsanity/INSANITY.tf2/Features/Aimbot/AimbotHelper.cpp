#include "AimbotHelper.h"

// SDK
#include "../../SDK/class/BaseWeapon.h"
#include "../../SDK/class/Source Entity.h"
#include "../../SDK/class/IVEngineClient.h"
#include "../../SDK/class/IVDebugOverlay.h"
#include "../../Utility/ClassIDHandler/ClassIDHandler.h"

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

    auto  iProjectileType = pActiveWeapon->GetTFWeaponInfo()->GetWeaponData(0)->m_iProjectile;
    float flAimbotFOV     = 0.0f;
    if(pActiveWeapon->getSlot() == WPN_SLOT_MELLE)
    {
        // Smack em niggas!
        F::aimbotMelee.RunV3(pLocalPlayer, pActiveWeapon, pCmd, pSendPackets);
        flAimbotFOV = Features::Aimbot::Melee_Aimbot::MeleeAimbot_FOV.GetData().m_flVal;
    }
    else if (iProjectileType != TF_PROJECTILE_BULLET && iProjectileType != TF_PROJECTILE_NONE)
    {
        // surface-to-air freedom delivery system :)
        F::aimbotProjectile.Run(pLocalPlayer, pActiveWeapon, pCmd, pSendPackets);
        flAimbotFOV = Features::Aimbot::Aimbot_Projectile::ProjAimbot_FOV.GetData().m_flVal;
    }
    else
    {
        F::aimbotHitscan.Run(pLocalPlayer, pActiveWeapon, pCmd, pSendPackets);
    }

    // Whenever you decide to make a FOV circle use this formula
    // (tan( aimbot_fov / 2.0f ) / tan( game_fov / 2.0f )) * (screen_width / 2.0f)
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
        int  iEntID = pEnt->GetClientClass()->m_ClassID;

        // TODO : Make jumptable mechanism for this so it is fast. This is ass.
        // 
        //I am retrieving the Class ID at runtime to avoid breaking in future, so can't use switch.
        if (iEntID == ClassID::CTFPlayer)
        {
            bEnemy == true ?
                m_aimbotTargetData.m_vecEnemyPlayers.push_back(pEnt) :
                m_aimbotTargetData.m_vecFriendlyPlayers.push_back(pEnt);
        }
        else if (iEntID == ClassID::CObjectSentrygun)
        {
            bEnemy == true ?
                m_aimbotTargetData.m_vecEnemySentry.push_back(pEnt) :
                m_aimbotTargetData.m_vecFriendlySentry.push_back(pEnt);
        }
        else if (iEntID == ClassID::CObjectDispenser)
        {
            bEnemy == true ?
                m_aimbotTargetData.m_vecEnemyDispensers.push_back(pEnt) :
                m_aimbotTargetData.m_vecFriendlyDispensers.push_back(pEnt);
        }
        else if (iEntID == ClassID::CObjectTeleporter)
        {
            bEnemy == true ?
                m_aimbotTargetData.m_vecEnemyTeleporters.push_back(pEnt) :
                m_aimbotTargetData.m_vecFriendlyTeleporters.push_back(pEnt);
        }
        else if (iEntID == ClassID::CTFProjectile_Rocket)
        {
            bEnemy == true ?
                m_aimbotTargetData.m_vecEnemyRockets.push_back(pEnt) :
                m_aimbotTargetData.m_vecFriendlyRockets.push_back(pEnt);
        }
        else if (iEntID == ClassID::CTFGrenadePipebombProjectile)
        {
            bEnemy == true ?
                m_aimbotTargetData.m_vecEnemyPipeBombs.push_back(pEnt) :
                m_aimbotTargetData.m_vecFriendlyPipeBombs.push_back(pEnt);
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
