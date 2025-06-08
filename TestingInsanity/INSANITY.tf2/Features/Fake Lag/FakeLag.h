#pragma once
#include <cstdint>
#include "../FeatureHandler.h"

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

DECLARE_FEATURE_OBJECT(fakeLag, FakeLag_t)

DEFINE_TAB(Misc, 3)
DEFINE_SECTION(FakeLag, "Misc", 1)

DEFINE_FEATURE(fakeLag,      bool,        FakeLag, Misc,   1, false,                 FeatureFlag_None)
DEFINE_FEATURE(AutoRelease,  bool,        FakeLag, Misc,   2, false,                 FeatureFlag_None)
DEFINE_FEATURE(choked_ticks, IntSlider_t, FakeLag, Misc,   3, IntSlider_t(0, 0, 30), FeatureFlag_None)