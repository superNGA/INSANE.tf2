#include "../Utility/Hook Handler/Hook_t.h"
#include "../Utility/ConsoleLogging.h"

#include "../Features/ModelPreview/ModelPreview.h"
#include "../Features/Entity Iterator/EntityIterator.h"
#include "../SDK/class/IVEngineClient.h"


///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
// NOTE : Called twice when leaving any match. Called once when joining a server & called twice when hosting & joining a server.
MAKE_HOOK(CClientState_Disconnect, "48 89 5C 24 ? 48 89 74 24 ? 48 89 7C 24 ? 41 56 48 83 EC ? 48 8B D9 41 0F B6 F8", __fastcall, ENGINE_DLL, void*,
    void* pThis, const char* szReason, bool bShowMainMenu)
{
    // Invalidate sorted entity lists, so we don't accidently use the previous match's entities and crash the game.
    F::entityIterator.InvalidateLists();


    FAIL_LOG("Disconnecting client. Reason [ %s ]. showMainMenu [ %s ]", 
        szReason[0]   == '\0' ? "null" : szReason,
        bShowMainMenu == true ? "True" : "False"); // True : When leaving a match. False : When joining.

    if(bShowMainMenu == false && szReason[0] == '\0')
    {
        // Only discard tables when we joining match from lobby, not when we are switching maps.
        // Cause engine will do it for us.
        if(I::iEngine->IsConnected() == false)
        {
            F::modelPreview.DiscardStringTables();
        }
        else
        {
            FAIL_LOG("Disconnected cause MAP_SWITCH. Not discarding table");
        }

        F::modelPreview.JoiningMatch(true);
    }
    else if (bShowMainMenu == true)
    {
        F::modelPreview.JoiningMatch(false);
    }

    F::modelPreview.InvalidateModelPrecache();

    return Hook::CClientState_Disconnect::O_CClientState_Disconnect(pThis, szReason, bShowMainMenu);
}