//=========================================================================
//                      CLASS ID MANAGER
//=========================================================================
// by      : INSANE
// created : 25/02/2025
// 
// purpose : cache the class IDs for entities, for fast lookup & also cause 
// they are prone to change with updates.

#include "classIDManager.h"
#include "../class/Source Entity.h"
#include "../../Libraries/Console System/Console_System.h"
extern Console_System cons;

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
		#ifdef _DEBUG
		cons.Log(FG_GREEN, "ID Manager", "Cached class ID for : %s", name.c_str());
		#endif
	}
	// engi -> sentery gun
	else if (name == "CObjectSentrygun") {
		TEMPclassID = SENTRY_GUN;
		#ifdef _DEBUG
		cons.Log(FG_GREEN, "ID Manager", "Cached class ID for : %s", name.c_str());
		#endif
	}
	// engi -> dispenser
	else if (name == "CObjectDispenser") {
		TEMPclassID = DISPENSER;
		#ifdef _DEBUG
		cons.Log(FG_GREEN, "ID Manager", "Cached class ID for : %s", name.c_str());
		#endif
	}
	// engi -> Teleported
	else if (name == "CObjectTeleporter") {
		TEMPclassID = TELEPORTER;
		#ifdef _DEBUG
		cons.Log(FG_GREEN, "ID Manager", "Cached class ID for : %s", name.c_str());
		#endif
	}
	// TODO : CHECK THIS 
	else if (name == "CTFAmmoPack") {
		TEMPclassID = AMMO_PACK;
		#ifdef _DEBUG
		cons.Log(FG_GREEN, "ID Manager", "Cached class ID for : %s", name.c_str());
		#endif
	}
	// Payload cart, if more than one in a match, that means we are not playing PAYLOAD
	else if (name == "CFuncTrackTrain") {
		TEMPclassID = PAYLOAD;
		#ifdef _DEBUG
		cons.Log(FG_GREEN, "ID Manager", "Cached class ID for : %s", name.c_str());
		#endif
	}
	// Flag in capture the Flag
	else if (name == "CCaptureFlag") {
		TEMPclassID = TF_ITEM;
		#ifdef _DEBUG
		cons.Log(FG_GREEN, "ID Manager", "Cached class ID for : %s", name.c_str());
		#endif
	}
	else if (name == "CTFPlayerResource") {
		TEMPclassID = ENT_RESOURCE_MANAGER;
		#ifdef _DEBUG
		cons.Log(FG_GREEN, "ID Manager", "Cached class ID for : %s", name.c_str());
		#endif
	}
	// WEAPONS
	else if (name == "CBaseCombatWeapon") {
		TEMPclassID = WEAPON;
		#ifdef _DEBUG
		cons.Log(FG_GREEN, "ID Manager", "Cached class ID for : %s", name.c_str());
		#endif
	}
	// ROTATING AMMO PACKS, MEDKITS & ACTIVE WEAPON
	else if (name == "CBaseAnimating") {
		TEMPclassID = CBASEANIMATING;
		#ifdef _DEBUG
		cons.Log(FG_GREEN, "ID Manager", "Cached class ID for : %s @ %d", name.c_str(), TEMPclassID);
		#endif
	}
	else if (name == "CTFProjectile_Rocket")
	{
		TEMPclassID = ROCKET;
		#ifdef _DEBUG
		cons.Log(FG_GREEN, "ID Manager", "Cached class ID for : %s @ %d", name.c_str(), TEMPclassID);
		#endif
	}
	else if (name == "CTFGrenadePipebombProjectile")
	{
		TEMPclassID = DEMO_PROJECTILES;
		#ifdef _DEBUG
		cons.Log(FG_GREEN, "ID Manager", "Cached class ID for : %s @ %d", name.c_str(), TEMPclassID);
		#endif
	}
	else if (name == "CTFDroppedWeapon")
	{
		TEMPclassID = ID_DROPPED_WEAPON;
		#ifdef _DEBUG
		cons.Log(FG_GREEN, "ID Manager", "Cached class ID for : %s @ %d", name.c_str(), TEMPclassID);
		#endif
	}

	// storing and returning ID
	if (TEMPclassID != NOT_DEFINED) CHE_mapID[name] = TEMPclassID;
	return CHE_mapID[name];
}