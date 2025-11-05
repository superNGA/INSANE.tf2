#include "../Utility/Hook Handler/Hook_t.h"
#include "../Features/WorldColorModulation/WorldColorModulation.h"

// SDK
#include "../SDK/class/Basic Structures.h"
#include "../SDK/class/CBaseLightCache.h"
#include "../SDK/class/IStudioRender.h"
#include "../SDK/class/IMaterial.h"
#include "../SDK/class/IVModelRender.h"


///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
MAKE_HOOK(IStudioRender_DrawModelStaticProp, "48 89 5C 24 ? 48 89 6C 24 ? 48 89 74 24 ? 44 89 4C 24", __fastcall, STUDIORENDER_DLL, void*,
    void* a1, void* a2, void* a3, int a4)
{
    Vec4 clr; F::worldColorMod.GetWorldColorModulation(&clr);
    I::iStudioRender->SetColorModulation(&clr.x);

    I::iStudioRender->SetAmbientLightColors(F::worldColorMod.GetAmbientLight());

    return Hook::IStudioRender_DrawModelStaticProp::O_IStudioRender_DrawModelStaticProp(a1, a2, a3, a4);
}