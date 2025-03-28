#include "sv_pure.h"
#include <iostream>
#include "../../SDK/TF object manager/TFOjectManager.h"

hook::sv_pure::T_svPure hook::sv_pure::O_svPure = nullptr;
void __fastcall hook::sv_pure::H_svPure(void* pFile)
{
#ifdef _DEBUG
    cons.Log(FG_GREEN, "sv_pure", "bypassed sv_pure :)");
#endif
    return;
}