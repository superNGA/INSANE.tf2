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
class BaseEntity;
typedef BaseEntity BaseEntity;

//=======================STRUCTS=======================

class EntityManager_t
{
public:
	EntityManager_t()
	{
		m_pLocalPlayer.store(nullptr);
	};

	void Reset()
	{
		m_pLocalPlayer.store(nullptr);
	}

	void		UpdateLocalPlayer();
	BaseEntity* GetLocalPlayer();

	std::atomic<BaseEntity*> m_pLocalPlayer;
};
inline EntityManager_t entityManager;