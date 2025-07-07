#include "../../Utility/Hook Handler/Hook_t.h"
#include "../../Utility/ConsoleLogging.h"

MAKE_HOOK(CheckForWhiteListServer, "40 56 48 83 EC ? 83 3D ? ? ? ? ? 48 8B F1 0F 8E", __fastcall, ENGINE_DLL, void, void* pFile)
{
    WIN_LOG("SKIPPED SV-PURE CHECK!");
    return;
}