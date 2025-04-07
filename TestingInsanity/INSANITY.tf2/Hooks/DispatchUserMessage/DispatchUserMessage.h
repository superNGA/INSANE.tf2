#pragma once

class bf_read;

namespace hook
{
    namespace DispatchUserMsg
    {
        typedef bool(__fastcall* T_DispatchUserMsg)(void*, int, bf_read*);
        extern T_DispatchUserMsg O_DispatchUserMsg;
        bool __fastcall H_DispatchUserMsg(void* pVTable, int iDataType, bf_read* msg);
    }
}