#include "../Utility/Hook Handler/Hook_t.h"
#include "../SDK/class/CUserCmd.h"

#define MULTIPLAYER_BACKUP 90
MAKE_HOOK(CInput_GetUserCmd, "44 8B CA 4C 8B C1 B8", __fastcall, CLIENT_DLL, CUserCmd*, uintptr_t pCInput, int iSequenceNumber)
{
    //__int64 result; // rax
    //result = *(__int64*)(pCInput + 0x108) + 72LL * (iSequenceNumber % MULTIPLAYER_BACKUP);

    //return reinterpret_cast<CUserCmd*>(result);

    return &(*reinterpret_cast<CUserCmd**>(pCInput + 0x108))[iSequenceNumber % MULTIPLAYER_BACKUP];
}