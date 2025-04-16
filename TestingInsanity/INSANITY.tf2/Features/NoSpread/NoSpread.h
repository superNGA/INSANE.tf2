//=========================================================================
//                      No Spread
//=========================================================================
// by      : INSANE
// created : 05/04/2025
// 
// purpose : Removes spread (random weapon inaccuracy ) from weapon
//-------------------------------------------------------------------------
#pragma once
#include <cstdint>
#include <string>
#include <atomic>
#include <deque>
#include "../features.h"
class CUserCmd;

/* Conclusions : 
* -> weaponFileInfo_t seems to be something of very constant nature and looping through bullets
*		from weaponFIleINfo ain't necessary. avoid it.
* -> the bullet tracers tell me that I have absolute 0 spread and the bullet holes in the
*       walls and the damage output tells me that I have all the fucking spread. even in local
*       servers. now what the fuck is that?
*/

/* DONE :
* -> Aquire Seed
* -> hook the fire bullet Fn & compare your seed against it and see what's up
* -> GetRandomFloat seems to be working very nicely
* -> find a absolute no-spread removal logic, cause even if the seed was wrong,
*       view angles should be a little off and not just aiming at the fucking ground &
*       in the fucking air and shit.
* -> underStand what the AngleVectors fn is doing
* -> clamp the values
* -> look the cStd clam and gMod and make a proper efficient clamping logic.
* -> underStand what the VectorAngles function is doing
* -> for Heavy the Base spread seems to have some random-ness. get that after some progress.
* -> device a method to send in commands on the in-game console.
* -> send in the playerperf command & store / read it.
* -> fix the no-spread logic for minigun & make no spread work user cmd seed enabled.
* -> make it the calculated server seed match the client seed on local server.
* -> Mantissa calc?
* -> add time for 1 tick?
* -> get the seed and learn what the syncing logic even does in bigger softwares.
* -> maybe even aquire the in-game ping
* -> make a imformation window ( translucent ) and display some good imformation on it.
* -> completely understand how game is calculating spread.
* check where spread fix is being called.
* aquire bullets per shot
* make a spread fix, with the bullets per shots. iterate over all bullets and remove spread
*   by incrementin seed for each bullet.
*/

/* TODO :
* Fix no spread on loop back server first. Then onto server time syncing.
* keep updating records, and don't just store specific number of records in teh begninig.
*/

/*
* SUSPECTS : 
* -> on our local servers, the servers engine time seems to be a little less than client 
*       engine time ( cause servers engine maybe starts later , IDK won't they both be running
*       on the same engine or something like that ). so that can be the issue for the no spread?
*       I mean server will be using its engine time and it must be correct to the last 
*       fucking decimal init?
*   Amalgum just used the client's engine time for local servers ( i.e. delta = 0.0f ). and IDk 
*   how well it works.
*/

#define DEBUG_NOSPREAD

#define MAX_TIME_RECORDS 20

class NoSpread_t
{
public:
    void        Run(CUserCmd* cmd, bool& result);
    bool        ParsePlayerPerf(std::string strPlayerperf);
    bool        ParsePlayerPerfExperimental(std::string szMsg);
    bool        ParsePlayerPerfV3(std::string sMsg);
    float adjustServerTime(int offset, float flServerTime);
    uint32_t GetSeed();
    std::atomic<uint32_t> m_iSeed;

private:
    bool        _ShouldRun(CUserCmd* cmd);
    bool        _FixSpread(CUserCmd* cmd, uint32_t seed, baseWeapon* pActiveWeapon);
    bool        _FixSpreadSingleBullet(CUserCmd* cmd, uint32_t seed, baseWeapon* pActiveWeapon);
    void        _DemandPlayerPerf(CUserCmd* cmd);
    float       _CalcMantissaStep(float flInput);
    std::string _GetServerUpTime(float flServerEngineTime);

    bool        m_bIsSynced = false;
    float       m_flRequestTime = 0.0f;
    bool        m_bWaitingForPlayerPerf = false;
    float       m_flDeltaClientServer = 0.0f;
    float       m_flServerEngineTime = 0.0f;
    bool        m_bLoopBack = false;
    float       m_flMantissaStep = 0.0f;
    float       m_flAquisitionDelay = 0.0f;
    std::deque<float> m_vecTimeDeltas = {};

    // new implementation shit
    float m_flServertime = 0.0f;
    float m_flPrevServertime = 0.0f;
    float m_flEstimatedServerTime = 0.0f;
    float m_flEstimatedServerTimeDelta = 0.0f;
    float m_flResponseTime = 0.0f;
    float m_flSyncOffset = 0.0f;

    // 3rd Implementation. 3rd time's the charm, init?
    float m_flFineTuneOffset = 0.0f;
    float m_flIsThisServerTime = 0.0f;

    enum {
        SYNC_INPROGRESS=0,
        SYNC_DONE,
        SYNC_LOST
    } m_syncStatus = SYNC_INPROGRESS;
};

ADD_FEATURE(noSpread, NoSpread_t);
