#pragma once
#include <deque>
#include "../features.h"

#include "../../SDK/class/Source Entity.h"
#include "../../SDK/class/BaseWeapon.h"

class CUserCmd;
class baseWeapon;
class IGameEvent;

/* 
* OBSERVATIONS : 
* -> the game does seem to manage the crit bucket when we "force" crits, all it does it mantain
*   the crit Checks, So I need to detect cirts and mantain the crit bucket myself.
*/

// TODO : Fix the Crit Cost for rapid fire
// TODO : Check if amalgum is messing with crit multiplier n shit
// TODO : This iCritMult thing seems to be pretty important, I don't think I should be altering any of these shared offsets.
//          So, check out where amalgun is getting that shit,
// TODO : Also Check how much it changes from weapon-to-weapon.

#define MAX_CRIT_COMMANDS 128

class WeaponCritData_t
{
public:
    void AddToCritBucket();
    void WithDrawlFromCritBucket();
    
    void UpdateStats(baseWeapon* pWeapon);
    void Reset();

    // Weapon stats
    baseWeapon* m_pWeapon       = nullptr;
    float       m_flDamage      = 0.0f;
    int         m_iWeaponID     = 0;
    bool        m_bIsRapidFire  = false;
    slot_t      m_iSlot         = WPN_SLOT_INVALID;
    
    // Bucket stats
    float       m_flCritBucket  = 0.0f;
    uint32_t    m_nCritRequests = 0;
};

class CritHack_t
{
public:
    void Run(CUserCmd* pCmd, baseWeapon* pActiveWeapon, BaseEntity* pLocalPlayer);
    
    void AddToWeaponsBucket(baseWeapon* pActiveWeapon);
    WeaponCritData_t* GetWeaponCritData(baseWeapon* pActiveWeapon);
    
    void HandleEvent(IGameEvent* pEvent);

    void Reset();

    // Crit Bucket parameters ( depends on server configuration )
    float m_flCritBucketBottom  = 0;
    float m_flCritBucketCap     = 0;
    float m_flCritBucketDefault = 0;

private:
    float m_flLastCritHackTime  = 0.0f;
    int   m_iLastCheckSeed      = 0;
    static constexpr int DEFAULT_OLD_CRIT_COUNT = -1;
    int   m_nOldCritCount       = -1;
    int   m_iLastUsedCritSeed   = 0;
    uint32_t    m_iLastWeaponID = 0;
    WeaponCritData_t* m_pLastCritWeapon = nullptr;
    BaseEntity* m_pLocalPlayer  = nullptr;

    void _InitializeCVars();
    bool _IsWeaponEligibleForCritHack(BaseEntity* pLocalPlayer, baseWeapon* pActiveWeapon);

    // Crit seed search & confirmation
    void _ScanForCritCommandsV2(CUserCmd* pCmd, baseWeapon* pActiveWeapon, BaseEntity* pLocalPlayer);
    int  _GetBestCritCommand(CUserCmd* pCmd);
    bool _IsCommandCritRapidFire(int iSeed, baseWeapon* pActiveWeapon, BaseEntity* pLocalPlayer);
    
    // CUserCmd altering
    void _AvoidCrit(CUserCmd* pCmd, baseWeapon* pActiveWeapon, BaseEntity* pLocalPlayer);
    void _ForceCrit(int iCritCommand, CUserCmd* pCmd, baseWeapon* pActiveWeapon, BaseEntity* pLocalPlayer, WeaponCritData_t* pWeaponCritData);

    // Crit ban ?
    bool _IsCritShotPossible(BaseEntity* pLocalPlayer, baseWeapon* pActiveWeapon, WeaponCritData_t* pWeaponCritData);
    bool _CanFireCriticalShot(BaseEntity* pLocalPlayer, baseWeapon* pActiveWeapon);
    bool _CanWithdrawlCrit(baseWeapon* pActiveWeapon, WeaponCritData_t* pWeaponCritData) const;
    bool _CanWithdrawlCritV2(WeaponCritData_t* pWeaponCritData);

    // Crit bucket parameters
    bool     m_bCVarsInitialized    = false;
    uint32_t m_iLocalPlayerEntIndex = 0;
    uint32_t m_iTotalDamage         = 0;
    uint32_t m_iRangedCritDamage    = 0;
    
    // Weapon's crit buckets.
    WeaponCritData_t m_PrimaryCritData;
    WeaponCritData_t m_SecondaryCritData;
    WeaponCritData_t m_MeleeCritData;

    // Crit cmd records
    inline void _ClearCritCmdRecord() { m_qCritCommands.clear(); m_iLastCheckSeed = 0; }
    std::deque<int> m_qCritCommands = {};

};
ADD_FEATURE(critHack, CritHack_t);

MAKE_FEATURE_BOOL(CritHack, "CritHack->toggleCritHack", 1);


/*
-> If Enabled

-> Should Crit 
    -> not in demo
    -> not crit boosted

-> can Crit
    -> Player's observed crit chances is good
    -> Crit bucket has enough damage in it.

-> Scan for crit ticks
    -> just remake that shit, it ain't that hard.
    -> maintain the games bucket and crit checks too.
*/

/*
When to do crits ? :
    -> if no IN_ATTACK, return
    -> if curTime > NextFireTime, fire
        -> don't do shit for next 0.5 seconds
    -> 
*/