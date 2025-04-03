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


hook::DME::T_DME hook::DME::O_DME = nullptr;
int64_t __fastcall hook::DME::H_DME(void* pVTable, DrawModelState_t* modelState, ModelRenderInfo_t* renderInfo, matrix3x4_t* boneMatrix)
{
    //return O_DME(pVTable, modelState, renderInfo, boneMatrix);
    return chams.Run(pVTable, modelState, renderInfo, boneMatrix);
}