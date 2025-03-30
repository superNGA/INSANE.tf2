//=========================================================================
//                      CHAMS MANAGMENT SYSTEM
//=========================================================================
// by      : INSANE
// created : 24/03/2025
// 
// purpose : Handles chams for all desiered entities. :)
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
*/

/*
* TODO : 
* -> get & Add some cool material options.
* -> Create custom Material at run time and preview ?? How does that sound.
* 
* -> software menu show mouse without using escape menu.
* 
* -> soetimes translucent ignoreZ ? fix that 
* -> team and enemy diff in all chams
* -> document content well / cleanUp this bullshit and make it " propa' "
* -> custom mats for Arms and Active weapon
* -> make a proper material creation logic.
* 
* -> Look into world material modulation, and put that rijin mateiral if you can.
*/

class Chams_t
{
public:
    int64_t Run(void* pVTable, DrawModelState_t* modelState, ModelRenderInfo_t* renderInfo, matrix3x4_t* boneMatrix);

private:
    bool _ChamsAnimAmmoPack(int8_t nMaterial, IMaterial* pMaterial, IMaterial** ppMaterial);
    bool _ChamsMedKit(int8_t nMaterial, IMaterial* pMaterial, IMaterial** ppMaterial);
    
    bool _ChamsPlayer(int8_t nMaterial, IMaterial* pMaterial, IMaterial** ppMaterial, BaseEntity * pEntity);
    
    bool _ChamsDispenserEnemy(int8_t nMaterial, IMaterial* pMaterial, IMaterial** ppMaterial);
    bool _ChamsDispenserFriendly(int8_t nMaterial, IMaterial* pMaterial, IMaterial** ppMaterial);
    
    bool _ChamsSenteryEnemy(int8_t nMaterial, IMaterial* pMaterial, IMaterial** ppMaterial, BaseEntity* pEntity);
    bool _ChamsSenteryFriendly(int8_t nMaterial, IMaterial* pMaterial, IMaterial** ppMaterial, BaseEntity* pEntity);
    
    bool _ChamsTeleporterEnemy(int8_t nMaterial, IMaterial* pMaterial, IMaterial** ppMaterial);
    bool _ChamsTeleporterFriendly(int8_t nMaterial, IMaterial* pMaterial, IMaterial** ppMaterial);

    bool _ChamsItems(int8_t nMaterial, IMaterial* pMaterial, IMaterial** ppMaterial);
    bool _ChamsAmmoPack(int8_t nMaterial, IMaterial* pMaterial, IMaterial** ppMaterial);
    bool _ChamsProjectiles(int8_t nMaterial, IMaterial* pMaterial, IMaterial** ppMaterial);

    bool _IsAmmoPack(uint32_t iHash);
    bool _IsMedKit(uint32_t iHash);
};
extern Chams_t chams;