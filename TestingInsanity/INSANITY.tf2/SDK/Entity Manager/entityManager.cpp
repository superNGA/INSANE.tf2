//=========================================================================
//								ENTITY MANAGER
//=========================================================================
// by      : INSANE
// created : 25/02/2025
// 
// purpose : Process / condense entity information so it can be used by software easily
//           and safely.

#include "entityManager.h"
#include "../Aimbot/aimbot_t.h"
#include <cstdint>
#include "../../GlobalVars.h"
#include "../TF object manager/TFOjectManager.h"
#include "../Class ID Manager/classIDManager.h"
#include "../class/BaseWeapon.h"
#include "../../Libraries/Console System/Console_System.h"

#include "../class/IVEngineClient.h"
#include "../class/Source Entity.h"

extern Console_System cons;

EntityManager_t entityManager;


//=========================================================================
//                     CONSTRUCTOR
//=========================================================================
EntityManager_t::EntityManager_t()
{
    _pLocalPlayer.store(nullptr);
    _class.store(player_class::INVALID);
    _teamNum.store(0);
    _pActiveWeapon.store(nullptr);
    _activeWeaponId.store(0);

    initialized.store(false);
}


//=========================================================================
//                     PUBLIC METHODS
//=========================================================================

//=========================================================================
// void EntityManger_t::processEntities()
//=========================================================================
/**
* process all entites including aimbot, esp and other data.
*/
void EntityManager_t::processEntities()
{
    _swapEspBuffer();
    _internalEspdata.clear();

	if (_processLocalPlayer() == false)
	{
		return;
	}

	_processActiveWeapon();
    //_processEntityList();

    initialized.store(true);
}


//=========================================================================
// bool EntityManager_t::isActiveWeaponProjectile()
//=========================================================================
/**
* return true if local player's active weapon is a projectile weapon
* else returns FALSE for hit-scan weapons
*/
bool EntityManager_t::isActiveWeaponProjectile()
{
    if (initialized.load() == false)
    {
        return false;
    }

    switch (_activeWeaponId.load()) {
        // Soldier
    case 18:  // Rocket Launcher
    case 127: // Black Box
    case 228: // Direct Hit
    case 414: // Liberty Launcher
    case 441: // Beggar's Bazooka
    case 513: // Air Strike

        // Demoman
    case 19:  // Grenade Launcher
    case 206: // Loch-n-Load
    case 308: // Loose Cannon
    case 996: // Iron Bomber
    case 20:  // Stickybomb Launcher
    case 207: // Scottish Resistance
    case 661: // Quickiebomb Launcher

        // Pyro
    case 40:  // Flare Gun
    case 351: // Detonator
    case 595: // Manmelter
    case 740: // Scorch Shot
        return true;

    default:
        return false;
    }
}


//=========================================================================
// vecEspData EntityManager_t::getEspData()
//=========================================================================
/**
* gets the esp data vector safely
*/
vecEspData EntityManager_t::getEspData()
{
    std::lock_guard<std::mutex> lock(_mtxEspData);
    return _externalEspData;
}


//=========================================================================
// BaseEntity* EntityManager_t::getLocalPlayer()
//=========================================================================
/**
* give local player pointer safely
*/
//-------------------------------------------------------------------------
BaseEntity* EntityManager_t::getLocalPlayer()
{
    if (initialized.load() == false)
    {
        ERROR("EntityManager", "not initialized yet");
        return nullptr;
    }

    return _pLocalPlayer.load();
}


//=========================================================================
// baseWeapon* EntityManager_t::getActiveWeapon()
//=========================================================================
/**
* gets active weapon poitner for local player safely
*/
//-------------------------------------------------------------------------
baseWeapon* EntityManager_t::getActiveWeapon()
{
    if (initialized.load() == false)
    {
        ERROR("EntityManager", "not initialized yet");
        return nullptr;
    }

    return _pActiveWeapon.load();
}

//=========================================================================
//                     PRIVATE METHODS
//=========================================================================


//=========================================================================
// bool EntityManager_t::_processLocalPlayer()
//=========================================================================
/**
* processes local player.
* 
* stores important imformation about local player and skips the rest of 
* "processEntities" function if local player is invalid or dead.
*
* @return returns false if dead or invalid, don't bother doing anything 
* else if returns false
*/
bool EntityManager_t::_processLocalPlayer()
{
	_indexLocalPlayer = I::iEngine->GetLocalPlayer();
	BaseEntity* pLocalPlayer_ = I::IClientEntityList->GetClientEntity(_indexLocalPlayer);
	if (pLocalPlayer_ == nullptr || pLocalPlayer_->getLifeState() != LIFE_ALIVE)
	{
		return false;
	}
	_pLocalPlayer.store(pLocalPlayer_);

	_class.store(pLocalPlayer_->getCharacterChoice());
	_teamNum.store(pLocalPlayer_->getTeamNum());

	/* skipped 
		view angles, positions, eye position */

    return true;
}


//=========================================================================
// bool EntityManager_t::_processActiveWeapon()
//=========================================================================
/**
* stores imformation about current / active weapon for local player
*/
bool EntityManager_t::_processActiveWeapon()
{
	baseWeapon* pWeapon_ = _pLocalPlayer.load()->getActiveWeapon();
	if (pWeapon_ == nullptr)
	{
		return false;
	}
	_pActiveWeapon.store(pWeapon_);
	//pWeapon_->setCustomTracer("merasmus_zap"); // fix this, make it only for local player
	_activeWeaponId.store(pWeapon_->getWeaponIndex());

    return true;
}


//=========================================================================
// bool EntityManager_t::_processEntityList()
//=========================================================================
/**
* iterates through the entity list and stores all the imformation we need
* to do our dirty work :)
*/
bool EntityManager_t::_processEntityList()
{
    uint32_t entityCount_ = I::IClientEntityList->NumberOfEntities(true);
    if (entityCount_ <= 0)
    {
        ERROR("ENTITY MANAGER", "entity list size was <= 0, skipping entity list filtering");
        return false;
    }

    for (int entIndex_ = 0; entIndex_ < entityCount_; entIndex_++)
    {
        // skipping local player
        if (entIndex_ == _indexLocalPlayer)
        {
            continue;
        }

        BaseEntity* entity = I::IClientEntityList->GetClientEntity(entIndex_);
        if (entity == nullptr || entity->IsDormant())
        {
            continue;
        }

        IDclass_t entityID_ = IDManager.getID(entity);
        switch (entityID_)
        {
        case ENT_RESOURCE_MANAGER:
            break;
        case PLAYER:
            _processEntityPlayer(entity);
            break;
        // skipping : ammo pack, buildings, tf_item ( intelligence case ) weapon, payload.
        default:
            continue;
        }
    }

    return true;
}


//=========================================================================
// bool _processEntityPlayer(BaseEntity* ent)
//=========================================================================
/**
* process entity i.e. other than local player. Entity passed here has to 
* already DORMANT & VALID pointer checked
*
* @param ent : pointer to I_client_entity object of this entity
*/
bool EntityManager_t::_processEntityPlayer(BaseEntity* entity)
{
    if (entity->getLifeState() != LIFE_ALIVE)
    {
        return false;
    }

    // todo : remove this and make something proper
    entity->setGlow(true);

    // getting player bone matrix...
    matrix3x4_t playerBoneMatrix_[MAX_STUDIO_BONES];
    entity->SetupBones(playerBoneMatrix_, MAX_STUDIO_BONES, HITBOX_BONES, tfObject.pGlobalVar->curtime);

    ABMS.processPlayerAimbot(entity, playerBoneMatrix_, isActiveWeaponProjectile());
    
    _processEspPlayer(playerBoneMatrix_);

    return true;
}


//=========================================================================
// void EntityManager_t::_processEspPlayer(const matrix3x4_t* playerBoneMatrix)
//=========================================================================
/**
* stores esp imformation about this entity
*/
void EntityManager_t::_processEspPlayer(const matrix3x4_t* playerBoneMatrix)
{
    ESPData_t espData;

    // do maths here

    _internalEspdata.push_back(espData);
}


//=========================================================================
// void _swapEspBuffer();
//=========================================================================
/**
* puts writting buffer into reading buffer safely (without race conditions)
*/
void EntityManager_t::_swapEspBuffer()
{
    std::lock_guard<std::mutex> lock(_mtxEspData);
    _externalEspData = _internalEspdata;
}