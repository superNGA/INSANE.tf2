#include "IGameEventManager.h"
#include <iostream>

#include "../../Utility/ConsoleLogging.h"
#include "../../Utility/signatures.h"

MAKE_SIG(AddListener, "48 89 6C 24 ? 48 89 7C 24 ? 41 56 48 83 EC ? 41 0F B6 E9", ENGINE_DLL, bool,
    IGameEventManager2*, IGameEventListener2*, const char*, bool);

IGameEventListener2 iEventListener;

bool IGameEventListener2::Initialize()
{
    I::iGameEventManager->AddListener(this, "player_hurt", false);
    //Sig::AddListener(I::iGameEventManager, this, "damageamount", true);
    bool result = I::iGameEventManager->FindListener(this, "player_hurt");

    if (result == false)
    {
        FAIL_LOG("Failed to initialize event listener damageammount");
    }
    else
    {
        WIN_LOG("created listener with adrs : [ %p ]", Sig::AddListener.m_ullAdrs);
    }

    return result;
}