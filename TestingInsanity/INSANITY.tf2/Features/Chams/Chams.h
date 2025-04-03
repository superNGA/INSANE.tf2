//=========================================================================
//                      CHAMS MANAGMENT SYSTEM
//=========================================================================
// by      : INSANE
// created : 24/03/2025
// 
// purpose : Handles chams :)
//-------------------------------------------------------------------------
#pragma once
#include <unordered_map>
#include <string>
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
* ( this feature doesn't seem to cause much performance issue. just using 0.5 ms approax)
* -> Getting some CRITICAL performance issues. Fix & get to the cause!
* -> make a proper material creation logic.
* -> FREE all materials when terminating.
*/

/*
* TODO : 
* FINAL TASKS : ( move to no-spread or something like that after this )
* -> make a argument structure for _CreateMaterial Fn & Find a method to showCase / preview 
*    a Material.
* 
* -> Imcorporate the ConsoleSystem V2 in software.
* -> deal with dropped weapon chams, maybe remove or give user control. likely causing
*    performance complications.
* -> Make something that could take in properties and create a trully custom material
*    accoring to that. Don't hardCode specs in the _CreateMaterial Fn.
* -> Make some logic to let user create and save materials with a very wide variety
*    of options. ( or maybe we can do that later when more important features are done ?)
* -> get & Add some cool material options.
* -> Create custom Material at run time and preview ?? How does that sound.
* 
* -> soetimes translucent ignoreZ ? fix that 
* -> document content well / cleanUp this bullshit and make it " propa' "
* 
* -> software menu show mouse without using escape menu.
* -> Look into world material modulation, and put that rijin mateiral if you can.
*/

#define MAX_MATERIAL_NAME_SIZE 32
struct Material_t
{
    IMaterial* pMaterial                    = nullptr;
    KeyValues* pKV                          = nullptr;
    char szMatName[MAX_MATERIAL_NAME_SIZE]  = "NOT DEFINED";
};

class Chams_t
{
public:
    int64_t Run(void* pVTable, DrawModelState_t* modelState, ModelRenderInfo_t* renderInfo, matrix3x4_t* boneMatrix);
    bool FreeAllMaterial();

private:
    // All of these fucking functions do the same thing, and it could have
    // been done with a single fucking function. This shows how smart I am
    // and I truly am just a dumb fuck. FIX THIS SHIT!

    bool _ApplyChams(DrawModelState_t* pModelState, IMaterial* pChamMaterial, 
        bool bIgnoreZConfig, bool bChamToggleConfig, clr_t& clrCham);

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

    // can use unordered map for material storing logic
    bool _CreateMaterial(const char* pBaseMaterialType, std::string szMatName);
    bool _DeleteMaterial(std::string szMatName);
    
    // this stores all custom made materials, reason for using a map is so I can 
    // scale is easily in future.
    std::unordered_map<std::string, Material_t*> UM_materials; 
};
extern Chams_t chams;