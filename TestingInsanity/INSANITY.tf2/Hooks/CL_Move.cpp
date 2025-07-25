#include "../Utility/Hook Handler/Hook_t.h"
#include "../Utility/Signature Handler/signatures.h"
#include "../Utility/ConsoleLogging.h"

#include "../Extra/math.h"
#include "../Features/Tick Shifting/TickShifting.h"
#include "../SDK/class/IVEngineClient.h"
#include "../SDK/TF object manager/TFOjectManager.h"


MAKE_HOOK(CL_Move, "40 55 53 48 8D AC 24 ? ? ? ? B8 ? ? ? ? E8 ? ? ? ? 48 2B E0 83 3D", __fastcall, ENGINE_DLL, void,
    float flAccumulatedExtraSample, bool bFinalTick)
{
    if(I::iEngine->IsInGame() == false)
    {
        Hook::CL_Move::O_CL_Move(flAccumulatedExtraSample, bFinalTick);
        return;
    }
    
    F::tickShifter.HandleTick(Hook::CL_Move::O_CL_Move, flAccumulatedExtraSample, bFinalTick);
}