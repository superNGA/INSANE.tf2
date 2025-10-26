#include "../Utility/Hook Handler/Hook_t.h"

// SDK
#include "../SDK/class/INetChannelInfo.h"
#include "../SDK/class/IVEngineClient.h"
#include "../SDK/class/I_EngineClientReplay.h"
#include "../SDK/class/CVar.h"
#include "../SDK/TF object manager/TFOjectManager.h"
#include "../Utility/CVar Handler/CVarHandler.h"

#include "../Features/Entity Iterator/EntityIterator.h"


///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
MAKE_HOOK(CNetChan_SendDatagram, "40 55 57 41 56 48 8D AC 24", __fastcall, ENGINE_DLL, void*, INetChannel* pThis, void* pDatagram)
{
    if (I::iEngine->IsInGame() == false)
        return Hook::CNetChan_SendDatagram::O_CNetChan_SendDatagram(pThis, pDatagram);

    if (I::iEngineClientReplay->GetNetChannel()->IsLoopback() == true)
    {
        ConVar* pCVar = I::iCvar->FindVar("net_fakelag");
        
        if(pCVar != nullptr)
        {
            float flFakeLag = std::clamp<float>(F::entityIterator.GetBackTrackTimeInSec() - 0.2f, 0.0f, 1.0f);
            pCVar->m_fValue = (flFakeLag * 1000.0f) / 2.0f;
        }

        return Hook::CNetChan_SendDatagram::O_CNetChan_SendDatagram(pThis, pDatagram);
    }

    std::deque<EntityIterator_t::DatagramStat_t>& qSequences = F::entityIterator.GetDatagramSequences();

    // add sequence to list.
    int& iLastAddedSequence = F::entityIterator.m_iLastAddedSequence;
    if (pThis->m_nInSequenceNr > iLastAddedSequence)
    {
        iLastAddedSequence = pThis->m_nInSequenceNr;
        qSequences.push_front({ pThis->m_nInSequenceNr, pThis->m_nInReliableState, tfObject.pGlobalVar->realtime });

        if (qSequences.size() > 256)
            qSequences.pop_back();
    }

    int iOldInSequenceNr    = pThis->m_nInSequenceNr;
    int iOldInReliableState = pThis->m_nInReliableState;

    // Find a sequence old enough from the list.
    float flFakeLatency = F::entityIterator.GetBackTrackTimeInSec() - 0.2f;
    flFakeLatency       = std::clamp<float>(flFakeLatency, 0.0f, CVars::sv_maxunlag);

    if (flFakeLatency <= 0.0f)
    {
        //LOG("No latency required!");
        return Hook::CNetChan_SendDatagram::O_CNetChan_SendDatagram(pThis, pDatagram);
    }
    //LOG("Latency : %.2f ms", flFakeLatency * 1000.0f);

    for (const auto& sequence : qSequences)
    {
        if ((tfObject.pGlobalVar->realtime - sequence.m_flTimeStamp) * CVars::host_timescale >= flFakeLatency)
        {
            pThis->m_nInSequenceNr    = sequence.m_nInSequenceNr;
            pThis->m_nInReliableState = sequence.m_nInReliableState;
            //LOG("Found spoof!");
            break;
        }
    }

    // Call with spoofed shit
    auto result = Hook::CNetChan_SendDatagram::O_CNetChan_SendDatagram(pThis, pDatagram);

    // Restore
    pThis->m_nInSequenceNr    = iOldInSequenceNr;
    pThis->m_nInReliableState = iOldInReliableState;

    return result;
}
