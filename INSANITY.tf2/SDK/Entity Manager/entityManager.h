//=========================================================================
//								ENTITY MANAGER
//=========================================================================
// by      : INSANE
// created : 25/02/2025
// 
// purpose : Process / condense entity information so it can be used by software easily
//           and safely.

#pragma once
#include <string>
#include <atomic>
#include <vector>
#include <mutex>
#include "../class/Basic Structures.h"

//=======================FORWARD DECLERATIONS=======================
class baseWeapon;
class I_client_entity;
typedef I_client_entity BaseEntity;

//=======================STRUCTS=======================
struct ESPData_t
{
	std::string entityName;

	// bone coordinates
};

typedef std::vector<ESPData_t> vecEspData;

class EntityManager_t
{
public:
	EntityManager_t();
	void		processEntities();
	bool		isActiveWeaponProjectile();
	vecEspData	getEspData();
	BaseEntity* getLocalPlayer();
	baseWeapon* getActiveWeapon();

	std::atomic<bool> initialized;
private:
	bool _processLocalPlayer();
	bool _processActiveWeapon();
	bool _processEntityList();
	bool _processEntityPlayer(BaseEntity* entity);
	void _processEspPlayer(const matrix3x4_t* playerBoneMatrix);
	void _swapEspBuffer();
	
//=========================================================================
//                     LOCAL PLAYER DATA
//=========================================================================
	std::atomic<BaseEntity*>	_pLocalPlayer;
	uint32_t					_indexLocalPlayer  = 0;
	std::atomic<player_class>	_class;
	std::atomic<uint16_t>		_teamNum;
	
	std::atomic<baseWeapon*>	_pActiveWeapon;
	std::atomic<int32_t>		_activeWeaponId;

//=========================================================================
//                     ESP DATA
//=========================================================================
	vecEspData					_externalEspData; // read buffer
	vecEspData					_internalEspdata; // write buffer
	std::mutex					_mtxEspData;
};
extern EntityManager_t entityManager;