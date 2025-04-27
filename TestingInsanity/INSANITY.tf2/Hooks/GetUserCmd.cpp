#include "../Utility/Hook_t.h"
#include "../SDK/class/CUserCmd.h"

MAKE_HOOK(CInput_GetUserCmd, "44 8B CA 4C 8B C1 B8", __fastcall, CLIENT_DLL, CUserCmd*, uintptr_t pCInput, int iSequenceNumber)
{
    __int64 result; // rax
    result = *(__int64*)(pCInput + 264) + 72LL * (iSequenceNumber % 90);

    return reinterpret_cast<CUserCmd*>(result);
}