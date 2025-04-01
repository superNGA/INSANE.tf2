//=========================================================================
//                      CHAMS MANAGMENT SYSTEM
//=========================================================================
// by      : INSANE
// created : 24/03/2025
// 
// purpose : Handles chams :)
//-------------------------------------------------------------------------
#pragma once
#include "../../SDK/class/Basic Structures.h"
#include "../../SDK/class/IMaterial.h"
#include "../../Hooks/DrawModelExecute/DrawModelExecute.h"

/*
* DONE :
* -> use CreateMaterial logic instead of yanking material out of a file
* -> put chams on the player's items like guns and secondaries.
* -> FIX MENU !!! , so we can actually use the fucking features!
* -> allow different color / materials chams on active weapons and on-ground weapons
* -> allow different color / materials chams on Medkits and Ammo boxes
* -> allow chams with and without materials
* -> team and enemy diff in all chams
* -> custom mats for Arms and Active weapon
*/

/*
* TODO : 
* -> make a proper material creation logic.
* -> FREE all materials when terminating.
* -> get & Add some cool material options.
* -> Create custom Material at run time and preview ?? How does that sound.
* 
* -> soetimes translucent ignoreZ ? fix that 
* -> document content well / cleanUp this bullshit and make it " propa' "
* 
* -> software menu show mouse without using escape menu.
* -> Look into world material modulation, and put that rijin mateiral if you can.
*/

class Chams_t
{
public:
    int64_t Run(void* pVTable, DrawModelState_t* modelState, ModelRenderInfo_t* renderInfo, matrix3x4_t* boneMatrix);

private:
    // All of these fucking functions do the same thing, and it could have
    // been done with a single fucking function. This shows how smart I am
    // and I truly am just a dumb fuck. FIX THIS SHIT!

    // fix it plz
    bool _ChamsAnimAmmoPack(int8_t nMaterial, IMaterial* pMaterial, IMaterial** ppMaterial);
    bool _ChamsMedKit(int8_t nMaterial, IMaterial* pMaterial, IMaterial** ppMaterial);
    
    bool _ChamsPlayerEnemy(int8_t nMaterial, IMaterial* pMaterial, IMaterial** ppMaterial, BaseEntity * pEntity);
    bool _ChamsPlayerFriendly(int8_t nMaterial, IMaterial* pMaterial, IMaterial** ppMaterial, BaseEntity * pEntity);
    
    bool _ChamsDispenserEnemy(int8_t nMaterial, IMaterial* pMaterial, IMaterial** ppMaterial);
    bool _ChamsDispenserFriendly(int8_t nMaterial, IMaterial* pMaterial, IMaterial** ppMaterial);
    
    bool _ChamsSenteryEnemy(int8_t nMaterial, IMaterial* pMaterial, IMaterial** ppMaterial, BaseEntity* pEntity);
    bool _ChamsSenteryFriendly(int8_t nMaterial, IMaterial* pMaterial, IMaterial** ppMaterial, BaseEntity* pEntity);
    
    bool _ChamsTeleporterEnemy(int8_t nMaterial, IMaterial* pMaterial, IMaterial** ppMaterial);
    bool _ChamsTeleporterFriendly(int8_t nMaterial, IMaterial* pMaterial, IMaterial** ppMaterial);

    bool _ChamsItems(int8_t nMaterial, IMaterial* pMaterial, IMaterial** ppMaterial);
    bool _ChamsAmmoPack(int8_t nMaterial, IMaterial* pMaterial, IMaterial** ppMaterial);
    bool _ChamsViewModel(int8_t nMaterial, IMaterial* pMaterial, IMaterial** ppMaterial);

    bool _ChamsProjectilesEnemy(int8_t nMaterial, IMaterial* pMaterial, IMaterial** ppMaterial);
    bool _ChamsProjectilesFriendly(int8_t nMaterial, IMaterial* pMaterial, IMaterial** ppMaterial);

    bool _IsAmmoPack(uint32_t iHash);
    bool _IsMedKit(uint32_t iHash);

    // to create & free material, use unordered map, and store it with ID and
    // pointer to material. when creating material, store pointer in map and 
    // return the ID, and when terminating the cheat, iterater over the map and
    // free all materials.
};
extern Chams_t chams;