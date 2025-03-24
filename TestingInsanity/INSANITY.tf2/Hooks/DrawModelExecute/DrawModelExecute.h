//=========================================================================
//                      DrawModelExecute Hook
//=========================================================================
// by      : INSANE
// created : 23/03/2025
// 
// purpose : Hooked drawModelExecute function
//-------------------------------------------------------------------------
#pragma once
#include <cstdint>

struct  DrawModelState_t;
struct  ModelRenderInfo_t;
class   matrix3x4_t;

namespace hook
{
    namespace DME
    {
		typedef int64_t(__fastcall* T_DME)(void*, DrawModelState_t*, ModelRenderInfo_t*, matrix3x4_t*);
		extern T_DME O_DME;
		int64_t __fastcall H_DME(void* pVTable, DrawModelState_t* modelState, ModelRenderInfo_t* renderInfo, matrix3x4_t* boneMatrix);
    }
}