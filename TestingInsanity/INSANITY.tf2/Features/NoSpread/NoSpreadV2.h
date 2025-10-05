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

    bool VerifyServerClientDelta(std::string & szPlayerPerf);
    bool ExtractTimeStamps(std::string& szPlayerPerf);

    inline bool IsSynced() const { return m_bSynced; }

private:
    
    // Time syncing logic
    void  _Sync(bool* pSendPacket);
    bool  _ShouldCreateTimeStamps() const;
    void  _CreateTimeStamps(bool* pSendPacket);
    void  _VerifySync();
    
    const int MAX_TIME_STAMP_COUNT = 5;
    const int TIME_STAMP_FREQUENCY = 2;
    const int CHOCK_SIZE           = 1;
    std::deque<double> m_qTimeStamps = {};
        
    bool m_bSendTimeStamps = false;
    bool m_bSynced         = false;
    double m_dServerClientDelta = 0.0;
    int m_iFailSyncCounter = 0;

    void  _AskPlayerPerf();
    double m_dPerfRequestTime    = 0.0;
    double m_dRequestLatency     = 0.0;
    bool   m_bPerfRequestPending = false;


    // Actual spread removal logic.
    void  _FixSpread(BaseEntity* pLocalPlayer, baseWeapon* pActiveWeapon, CUserCmd* pCmd, bool* pCreateMoveResult);
    bool  _IsShotPerfect(BaseEntity* pLocalPlayer, baseWeapon* pActiveWeapon);
    

    // Maths related functions.
    int   _GetSeed(CUserCmd* pCmd) const;
    float _GetMantissaStep(float x);
    float _GetLeastCount(float x);
    

    void  _Draw();
};

DECLARE_FEATURE_OBJECT(noSpreadV2, NoSpreadV2_t)

DEFINE_TAB(NoSpreadV2, 6)
DEFINE_SECTION(NoSpread, "NoSpreadV2", 1)

DEFINE_FEATURE(NoSpread, "Disable spread", bool, NoSpread, NoSpreadV2, 1, false)


/*

I send in imfo & it takes 8 & upto 20 ticks to come back. Now what time belongs to what fucking tick?
This makes it difficult. Each message takes different times to come back & we don't 
know what time belongs to what tick. How can we even try to calculate the delta when we 
don't even have an anchor on the server time.

*/
