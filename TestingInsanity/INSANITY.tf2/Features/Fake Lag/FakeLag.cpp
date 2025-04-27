#include "FakeLag.h"
#include "../../SDK/class/CUserCmd.h"

void FakeLag_t::Run(bool* bSendPacket, CUserCmd* pCmd)
{
    if (bSendPacket == nullptr || pCmd == nullptr)
        return;

    if (Feature::fakeLag == false)
        return;

    if (m_nTicksChoked >= Feature::nTicksChocked)
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

    if (Feature::autoRelease == true && pCmd->buttons & IN_ATTACK)
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