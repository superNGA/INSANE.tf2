#include "SendStringCommand.h"
#include <iostream>
#include "../../SDK/TF object manager/TFOjectManager.h"

hook::SendStringCommand::T_SendStringCommand hook::SendStringCommand::O_SendStringCommand = nullptr;
void __fastcall hook::SendStringCommand::H_SendStringCommand(void* pVTable, const char* cmd)
{
    printf("STORING [ %p ]\n", pVTable);
    tfObject.pCBaseClientState.store(pVTable);

    O_SendStringCommand(pVTable, cmd);
}