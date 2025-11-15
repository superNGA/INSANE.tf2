// Utility
#include "../Utility/Hook Handler/Hook_t.h"
#include "../Utility/ConsoleLogging.h"
// #include "../Utility/ClassIDHandler/ClassIDHandler.h"

// SDK
#include "../SDK/class/IMaterialSystem.h"
#include "../SDK/class/IStudioRender.h"
#include "../SDK/class/IMaterial.h"
#include "../SDK/class/BaseEntity.h"
#include "../SDK/class/CBaseLightCache.h"
#include "../SDK/class/IVEngineClient.h"
#include "../SDK/class/CMultAnimState.h"


#include "../Features/TickManip/TickManipHelper.h"
#include "../Features/Aimbot/AimbotHitscanV2/AimbotHitscanV2.h"
#include "../Features/DMEHandler/DMEHandler.h"
#include "../Features/ModelPreview/ModelPreview.h"
#include "../Features/Material Gen/MaterialGen.h"
#include "../Features/Entity Iterator/EntityIterator.h"
#include "../Features/Aimbot/Aimbot Melee/AimbotMelee.h"
#include "../Features/WorldColorModulation/WorldColorModulation.h"
#include "../Features/TickManip/AntiAim/AntiAimV2.h"
#include "EndScene/EndScene.h"



///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
MAKE_HOOK(DrawModelExecute, "4C 89 4C 24 ? 48 89 4C 24 ? 55 53 56 57 41 54", __fastcall, ENGINE_DLL, int64_t, 
    void* pVTable, DrawModelState_t* modelState, ModelRenderInfo_t* renderInfo, matrix3x4_t* boneMatrix)
{
    // Don't do material altering at unstable states, else causes crahes.
    if (directX::UI::UI_has_been_shutdown == true)
    {
        return Hook::DrawModelExecute::O_DrawModelExecute(pVTable, modelState, renderInfo, boneMatrix);
    }


    // Color/Alpha Modulate world materials & props n shit.
    F::worldColorMod.Run();


    // Handle this model.
    return F::DMEHandler.Run(pVTable, modelState, renderInfo, boneMatrix, Hook::DrawModelExecute::O_DrawModelExecute);
}