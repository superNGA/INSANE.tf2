#include "IGameEventManager.h"
#include <iostream>
#include <vector>

#include "../../Utility/ConsoleLogging.h"
#include "../../Utility/signatures.h"

MAKE_SIG(AddListener, "48 89 6C 24 ? 48 89 7C 24 ? 41 56 48 83 EC ? 41 0F B6 E9", ENGINE_DLL, bool,
    IGameEventManager2*, IGameEventListener2*, const char*, bool);

IGameEventListener2 iEventListener;

bool IGameEventListener2::Initialize()
{
    // All the listeners that will be added by our "software"
    std::vector<const char*> vEventNames = {
        "player_hurt",
        "round_end", "round_start", "stats_resetround"
    };

    // looping through all of the listerners
    bool bSuccess = true;
    for (const char* szEventName : vEventNames)
    {
        // trying to create this listener
        // NOTE : This will always return false. To check if the listener was created we must
        //        try to find it.
        I::iGameEventManager->AddListener(this, szEventName, false);
        
        // checking if we found that listener
        if (I::iGameEventManager->FindListener(this, szEventName) == false)
        {
            FAIL_LOG("Failed to find listener [ %s ]", szEventName);
            bSuccess = false;
        }
        else
        {
            WIN_LOG("Successfully initialized listener [ %s ]", szEventName);
        }

    }

    return bSuccess;
}

void IGameEventListener2::FireGameEvent(IGameEvent* event)
{
    // Hashing event name, so we can easily campare it with others
    uint32_t iHash = FNV1A32(event->GetName());

    switch (iHash)
    {
    case FNV1A32("player_hurt"):
        F::critHack.RecordDamageEvent(event);
        break;

    //case FNV1A32("round_end"):
    //case FNV1A32("round_start"):
    case FNV1A32("stats_resetround"):
        F::critHack.ResetDamageRecords();
        break;

    default: break;
    }
}