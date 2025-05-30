//=========================================================================
//                      CLASS ID MANAGER
//=========================================================================
// by      : INSANE
// created : 25/02/2025
// 
// purpose : cache the class IDs for entities, for fast lookup & also cause 
// they are prone to change with updates.

#include "classIDManager.h"
#include "../class/BaseEntity.h"
#include "../../Utility/ConsoleLogging.h"

//=========================================================================
//                     PUBLIC METHODS
//=========================================================================


//=========================================================================
// IDclass_t getID(I_client_entity* ent)
//=========================================================================
/**
* cached and return the class ID for given entity
*
* @param ent : pointer to entity
*/
IDclass_t IDManager_t::getID(BaseEntity* ent)
{

	// entity clas name
	std::string name = std::string(ent->GetClientNetworkable()->GetClientClass()->m_pNetworkName);

	// class ID
	int ID = ent->GetClientNetworkable()->GetClientClass()->m_ClassID;

	auto iterator = CHE_mapID.find(name);
	if (iterator != CHE_mapID.end()) { // if it is stored in the map
		return iterator->second; // return class ID
	}

	// if not stored then store it
	IDclass_t TEMPclassID = NOT_DEFINED;
	// any PLAYER
	if (name == "CTFPlayer") {
		TEMPclassID = PLAYER;
		
		WIN_LOG("ID Manager", "Cached class ID for : %s", name.c_str());
		
	}
	// engi -> sentery gun
	else if (name == "CObjectSentrygun") {
		TEMPclassID = SENTRY_GUN;
		WIN_LOG("ID Manager", "Cached class ID for : %s", name.c_str());
	}
	// engi -> dispenser
	else if (name == "CObjectDispenser") {
		TEMPclassID = DISPENSER;
		WIN_LOG("ID Manager", "Cached class ID for : %s", name.c_str());
	}
	// engi -> Teleported
	else if (name == "CObjectTeleporter") {
		TEMPclassID = TELEPORTER;
		
		WIN_LOG("ID Manager", "Cached class ID for : %s", name.c_str());
		
	}
	// TODO : CHECK THIS 
	else if (name == "CTFAmmoPack") {
		TEMPclassID = AMMO_PACK;
		
		WIN_LOG("ID Manager", "Cached class ID for : %s", name.c_str());
		
	}
	// Payload cart, if more than one in a match, that means we are not playing PAYLOAD
	else if (name == "CFuncTrackTrain") {
		TEMPclassID = PAYLOAD;
		
		WIN_LOG("ID Manager", "Cached class ID for : %s", name.c_str());
		
	}
	// Flag in capture the Flag
	else if (name == "CCaptureFlag") {
		TEMPclassID = TF_ITEM;
		WIN_LOG("ID Manager", "Cached class ID for : %s", name.c_str());
	}
	else if (name == "CTFPlayerResource") {
		TEMPclassID = ENT_RESOURCE_MANAGER;
		WIN_LOG("ID Manager", "Cached class ID for : %s", name.c_str());
	}
	// WEAPONS
	else if (name == "CBaseCombatWeapon") {
		TEMPclassID = WEAPON;
		
		WIN_LOG("ID Manager", "Cached class ID for : %s", name.c_str());
		
	}
	// ROTATING AMMO PACKS, MEDKITS & ACTIVE WEAPON
	else if (name == "CBaseAnimating") {
		TEMPclassID = CBASEANIMATING;
		
		WIN_LOG("ID Manager", "Cached class ID for : %s @ %d", name.c_str(), TEMPclassID);
		
	}
	else if (name == "CTFProjectile_Rocket")
	{
		TEMPclassID = ROCKET;
		
		WIN_LOG("ID Manager", "Cached class ID for : %s @ %d", name.c_str(), TEMPclassID);
		
	}
	else if (name == "CTFGrenadePipebombProjectile")
	{
		TEMPclassID = DEMO_PROJECTILES;
		
		WIN_LOG("ID Manager", "Cached class ID for : %s @ %d", name.c_str(), TEMPclassID);
		
	}
	else if (name == "CTFDroppedWeapon")
	{
		TEMPclassID = ID_DROPPED_WEAPON;
		
		WIN_LOG("ID Manager", "Cached class ID for : %s @ %d", name.c_str(), TEMPclassID);
		
	}

	// storing and returning ID
	if (TEMPclassID != NOT_DEFINED) CHE_mapID[name] = TEMPclassID;
	return CHE_mapID[name];
}