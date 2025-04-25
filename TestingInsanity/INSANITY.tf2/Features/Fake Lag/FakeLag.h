#pragma once
#include <cstdint>
#include "../features.h"

class CUserCmd;

class FakeLag_t
{
public:
    void Run(bool* bSendPacket, CUserCmd* pCmd);
    void Reset();

private:
    uint32_t m_nTicksChoked = 0;
    uint32_t m_iLastPacketTick = 0;

};

MAKE_FEATURE_BOOL(fakeLag, "FakeLag->global_FakeLag", 1);
MAKE_FEATURE_BOOL(autoRelease, "FakeLag->global_FakeLag->auto_release_on_shoot", 1);
MAKE_FEATURE_INTEGER(nTicksChocked, "FakeLag->global_FakeLag->nTicksChoked", 1, 0, 30);

ADD_FEATURE(fakeLag, FakeLag_t);