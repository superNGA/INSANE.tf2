#include "../Utility/Hook Handler/Hook_t.h"
#include "../Utility/CVar Handler/CVarHandler.h"
#include "../Utility/ClassIDHandler/ClassIDHandler.h"

#include "../Features/Entity Iterator/EntityIterator.h"
#include "../Features/ModelPreview/ModelPreview.h"
#include "../Features/Tick Shifting/TickShifting.h"
#include "../Features/NoSpread/NoSpreadV2.h"
#include "../Features/ESP/ESPV2.h"
#include "../Features/BulletTracers/BulletTarcers_t.h"

#include "../SDK/class/IPanel.h"


MAKE_HOOK(ClientModeShared_LevelInit, "48 89 5C 24 ? 57 48 83 EC ? 48 8B D9 48 8B FA 48 8B 49 ? 48 8B 01", __fastcall, CLIENT_DLL, int64_t,
    void* pVTable, const char* szMapName)
{
    auto result = Hook::ClientModeShared_LevelInit::O_ClientModeShared_LevelInit(pVTable, szMapName);

    F::modelPreview.CaptureAllEngineModels();

    F::cVarHandler.InvalidateCVars();
    F::cVarHandler.InitializeAllCVars();

    F::classIDHandler.Initialize();

    F::tickShifter.Reset();
    F::noSpreadV2.Reset();
    
    F::entityIterator.ClearBackTrackData(); F::entityIterator.ClearLists();
    F::entityIterator.ClearEntityMaterialOverrides();
    F::entityIterator.ClearSequenceData();

    I::iPanel->RefreshTargetPanelId();

    F::tracerHandler.InvalidateTracerCount();

    return result;
}