#include "../Utility/Hook Handler/Hook_t.h"
#include "../Features/WorldColorModulation/WorldColorModulation.h"
#include "../SDK/class/Basic Structures.h"



///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
MAKE_HOOK(GetFogColor, "40 55 56 48 81 EC ? ? ? ? F7 05", __fastcall, CLIENT_DLL, void*,
    void* eax, vec* pClrOut)
{
    void* result = Hook::GetFogColor::O_GetFogColor(eax, pClrOut);

    Vec4 clr; F::worldColorMod.GetWorldColorModulation(&clr);
    *pClrOut = clr.XYZ();

    return result;
}