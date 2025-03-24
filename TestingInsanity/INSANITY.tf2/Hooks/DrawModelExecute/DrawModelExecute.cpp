//=========================================================================
//                      DrawModelExecute Hook
//=========================================================================
// by      : INSANE
// created : 23/03/2025
// 
// purpose : Hooked drawModelExecute function
//-------------------------------------------------------------------------
#include <iostream>
#include "DrawModelExecute.h"
#include "../../Features/Chams/Chams.h"

/*
* AMMO PACKS : single texture
* PLAYER     : from 21 upto 31 textures
*/

/*
    -> Some of the ammo packs, large ones specifically ain't rendering like I want them to.
    -> active weapon is also turned to shit, fix that too.
*/


hook::DME::T_DME hook::DME::O_DME = nullptr;
int64_t __fastcall hook::DME::H_DME(void* pVTable, DrawModelState_t* modelState, ModelRenderInfo_t* renderInfo, matrix3x4_t* boneMatrix)
{
    //return O_DME(pVTable, modelState, renderInfo, boneMatrix);
    return chams.Run(pVTable, modelState, renderInfo, boneMatrix);
}