#include "../Utility/Hook Handler/Hook_t.h"
#include "../Features/WorldColorModulation/WorldColorModulation.h"
#include "../SDK/class/Basic Structures.h"



///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
MAKE_HOOK(GetSkyBoxFogColor, "48 89 5C 24 ? 57 48 81 EC ? ? ? ? 48 8B F9 E8 ? ? ? ? 48 8B D8 48 85 C0 0F 84 ? ? ? ? F7 05", __fastcall, CLIENT_DLL, void*,
    vec* pClrOut)
{
    void* result = Hook::GetSkyBoxFogColor::O_GetSkyBoxFogColor(pClrOut);

    Vec4 clr; F::worldColorMod.GetWorldColorModulation(&clr);
    *pClrOut = clr.XYZ();

    return result;
}