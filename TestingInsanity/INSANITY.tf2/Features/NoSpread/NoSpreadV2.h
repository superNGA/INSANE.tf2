#pragma once

#include <deque>

#include "../FeatureHandler.h"

class BaseEntity;
class baseWeapon;
class CUserCmd;

class NoSpreadV2_t
{
public:
    void Run(BaseEntity* pLocalPlayer, baseWeapon* pActiveWeapon, CUserCmd* pCmd, bool* pCreateMoveResult, bool* pSendPacket);
    void Reset();

    bool ExtractTime(std::string & szPlayerPerf);
    bool ExtractTimeV2(std::string& szPlayerPerf);

private:
    // Time syncing logic
    bool  _ShouldCreateTimeStamps() const;
    void  _CreateTimeStamps(bool* pSendPacket);
    static constexpr int MAX_TIME_STAMP_COUNT = 5;
    std::deque<double> m_qTimeStamps = {};

    void  _AskPlayerPerf();

    // Actual spread removal logic.
    void  _FixSpread(BaseEntity* pLocalPlayer, baseWeapon* pActiveWeapon, CUserCmd* pCmd, bool* pCreateMoveResult);
    bool  _IsShotPerfect(BaseEntity* pLocalPlayer, baseWeapon* pActiveWeapon);
    
    // Maths related functions.
    int   _GetSeed(CUserCmd* pCmd) const;
    float _GetMantissaStep(float x);
    float _GetLeastCount(float x);
    
    void  _Draw();

    bool   m_bPerfRequestPending = false;
    double m_dServerTimeDelta    = 0.0;
    double m_dRequestSendTime    = 0.0;
    double m_dOutgoingLatency    = 0.0;
    double m_dCorrectionOffset   = 0.0;

    bool m_bSynced = false;

    static constexpr int MAX_TIME_DELTA_SAMPLE = 32;
    std::deque<double> m_qTimeDeltaSamples  = {};
};

DECLARE_FEATURE_OBJECT(noSpreadV2, NoSpreadV2_t)

DEFINE_TAB(NoSpreadV2, 9)
DEFINE_SECTION(NoSpread, "NoSpreadV2", 1)

DEFINE_FEATURE(NoSpread, bool, NoSpread, NoSpreadV2, 1, false)


/*

I send in imfo & it takes 8 & upto 20 ticks to come back. Now what time belongs to what fucking tick?
This makes it difficult. Each message takes different times to come back & we don't 
know what time belongs to what tick. How can we even try to calculate the delta when we 
don't even have an anchor on the server time.

*/