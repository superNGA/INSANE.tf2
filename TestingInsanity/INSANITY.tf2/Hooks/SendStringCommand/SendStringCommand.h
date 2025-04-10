#pragma once

namespace hook
{
    namespace SendStringCommand
    {
        typedef void(__fastcall* T_SendStringCommand)(void*, const char*);
        extern T_SendStringCommand O_SendStringCommand;
        void __fastcall H_SendStringCommand(void* pVTable, const char* cmd);
    }
}