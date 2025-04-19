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
constexpr int DELTA_UPDATE_FREQUENCY = 20; // we will be updating delta every 20 tickss

class NoSpread_t
{
public:
    void        Run(CUserCmd* cmd, bool& result);
    bool        _ParsePlayerPerf(std::string sMsg);

    uint32_t    GetSeed();
    std::atomic<uint32_t> m_iSeed;

private:
    bool        _ShouldRun(CUserCmd* cmd);
    void        _RequestPlayerPerf(CUserCmd* cmd);
    float       _CalcMantissaStep(float flInput);
    std::string _GetServerUpTime(float flServerEngineTime);

    bool        _FixSpread(CUserCmd* cmd, uint32_t seed, baseWeapon* pActiveWeapon);

    enum SyncState_t
    {
        SYNC_NULL=-1,
        SYNC_STARTING=0,
        SYNC_DONE
    };
    SyncState_t m_eSyncState = SYNC_NULL;
    
    float m_flRequestTime    = 0.0f;
    float m_flRequestDelay = 0.0f;
    float m_flRequestLatency = 0.0f;

    bool m_bWaitingForPlayerPerf = false;
    float m_flServerTime = 0.0f;
    float m_flDelta = 0.0f;
    float m_flOffset = 0.0f;

    uint32_t counter = 0;
    uint32_t m_iRequestTick = 0;
    
    CUserCmd* pCmd = nullptr;

    std::deque<float> m_qServerTimes;
};

ADD_FEATURE(noSpread, NoSpread_t);