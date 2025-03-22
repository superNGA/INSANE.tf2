//=========================================================================
//                      AIM-BOT MANAGMENT SYSTEM
//=========================================================================
// by      : INSANE
// created : 25/02/2025
// 
// purpose : Manages all aimbot mechanims. hit-scan / projectile, or sorts 
// of shit

#include "aimbot_t.h"
#include "../class/Basic Structures.h"
#include "../class/Source Entity.h"

//=========================================================================
//                     PUBLIC METHODS
//=========================================================================


//=========================================================================
// bool Aimbot_t::processPlayerAimbot(BaseEntity* entity, 
//                                  const matrix3x4_t* boneMatrix, 
//                                  bool isWeaponProjectile)
//=========================================================================
/**
* processes aimbot logic for player entity
*
* @param entity : BaseEntity* for local player
* @param boneMatrix : bone matrix for local player
* @param isWeaponProjectile : if true, will do projectile aimbot
*/
bool Aimbot_t::processPlayerAimbot(BaseEntity* entity, const matrix3x4_t* boneMatrix, bool isWeaponProjectile)
{
    // check bones for hitscan.
    // do aimbot maths
    // compare and store angles
    
    return true;
}