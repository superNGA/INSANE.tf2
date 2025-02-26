//=========================================================================
//                      CLASS ID MANAGER
//=========================================================================
// by      : INSANE
// created : 25/02/2025
// 
// purpose : cache the class IDs for entities, for fast lookup & also cause 
// they are prone to change with updates.

#pragma once
#include <unordered_map>
#include "../entInfo_t.h"

//=======================FORWARD DECLERATIONS=======================
class I_client_entity;

class IDManager_t {

public:
	IDclass_t getID(I_client_entity* ent);

private:
	std::unordered_map<std::string, IDclass_t> CHE_mapID;
};
inline IDManager_t IDManager;