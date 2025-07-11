#include "../Utility/Hook Handler/Hook_t.h"
#include "../Utility/ConsoleLogging.h"
#include "../Features/Chams/Chams.h"

MAKE_HOOK(DrawModelExecute, "4C 89 4C 24 ? 48 89 4C 24 ? 55 53 56 57 41 54", __fastcall, ENGINE_DLL, int64_t, 
    void* pVTable, DrawModelState_t* modelState, ModelRenderInfo_t* renderInfo, matrix3x4_t* boneMatrix)
{
    return Hook::DrawModelExecute::O_DrawModelExecute(pVTable, modelState, renderInfo, boneMatrix);//chams.Run(pVTable, modelState, renderInfo, boneMatrix);
}