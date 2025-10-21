#include "FakeLag.h"
#include "../../SDK/class/CUserCmd.h"
#include "../../Utility/Profiler/Profiler.h"


void FakeLag_t::Run(bool* bSendPacket, CUserCmd* pCmd)
{
    PROFILER_RECORD_FUNCTION(CreateMove);

    if (bSendPacket == nullptr || pCmd == nullptr)
        return;

    if (Features::Misc::FakeLag::fakeLag.IsActive() == false)
        return;

    if (m_nTicksChoked >= Features::Misc::FakeLag::choked_ticks.m_Data.m_iVal)
    {
        *bSendPacket = true;
        m_nTicksChoked = 0;
        m_iLastPacketTick = pCmd->tick_count;
    }
    else
    {
        *bSendPacket = false;
        m_nTicksChoked++;
    }

    if (Features::Misc::FakeLag::AutoRelease.IsActive() == true && pCmd->buttons & IN_ATTACK)
    {
        *bSendPacket = true;
        m_nTicksChoked = 0;
    }
}

void FakeLag_t::Reset()
{
    m_nTicksChoked = 0;
    m_iLastPacketTick = 0;
}