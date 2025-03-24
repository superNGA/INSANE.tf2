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

// debug/debugambientcube

class Chams_t
{
public:
    int64_t Run(void* pVTable, DrawModelState_t* modelState, ModelRenderInfo_t* renderInfo, matrix3x4_t* boneMatrix);

private:
    bool _ChamsBaseAnimating(int8_t nMaterial, IMaterial** ppMaterial);
};
extern Chams_t chams;