#include <Windows.h>
#include <string>

#include "../../Utility/Hook_t.h"
#include "../../Utility/ConsoleLogging.h"

#include "../../SDK/class/bf_buf.h"
#include "../../Features/NoSpread/NoSpread.h"

inline const char* GetMsgName(void* pCUserMsg, int iMsgType)
{
    if (pCUserMsg == nullptr)
        return nullptr;

    int64_t dictMsg = *((int64_t*)pCUserMsg + 1);
    int64_t offset = 32LL * (unsigned int)iMsgType;
    return *(const char**)(dictMsg + offset + 16);
}


MAKE_HOOK(DispatchUserMsg, "40 56 48 83 EC ? 49 8B F0", __fastcall, CLIENT_DLL, bool, void* pVTable, int iDataType, bf_read* msg)
{
    bool result = Hook::DispatchUserMsg::O_DispatchUserMsg(pVTable, iDataType, msg);

    if (iDataType == 5)
    {
        //printf("Recieved textMsg from server\n");

        char rawMsg[256]; msg->ReadString(rawMsg, sizeof(rawMsg), true);
        msg->Seek(0);
        std::string sMsg = rawMsg;
        //std::cout << sMsg << '\n';
        
        //Features::noSpread.ParsePlayerPerf(sMsg);
        //Features::noSpread.ParsePlayerPerfExperimental(sMsg);
        Features::noSpread._ParsePlayerPerf(sMsg);
    }
    //printf("data type recieved : %d -> [ %s ]\n", iDataType, GetMsgName(pVTable, iDataType));

    return result;
}
