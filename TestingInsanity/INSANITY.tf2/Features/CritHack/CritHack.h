#pragma once
#include <deque>
#include "../features.h"

#include "../../SDK/class/Source Entity.h"
#include "../../SDK/class/BaseWeapon.h"

class CUserCmd;
class baseWeapon;
class IGameEvent;

/*
DONE :
-> Stop recoding damage when crit boosted.
-> Properly dropped crit bucket upon weapon change.
*/

class WeaponCritData_t
{
public:
    void AddToCritBucket();
    void WithDrawlFromCritBucket();
    
    bool UpdateStats(baseWeapon* pWeapon);
    void Reset();

    void IncrementCritRequestCount();

    // Weapon stats
    baseWeapon* m_pWeapon         = nullptr;
    float       m_flDamagePerShot = 0.0f;
    float       m_flCritCostBase  = 0.0f;
    int         m_iWeaponID       = 0;
    int         m_iWeaponEntIdx   = 0;
    bool        m_bIsRapidFire    = false;
    slot_t      m_iSlot           = WPN_SLOT_INVALID;
    float       m_flBulletsShotDuringCrit  = 0.0f;
    float       m_flLastWithdrawlTime      = 0.0f;
    float       m_flLastCritRequestIncTime = 0.0f;

    // Bucket stats
    float       m_flCritBucket  = 0.0f;
    uint32_t    m_nCritRequests = 0;
};

class CritHack_t
{
public:
    void RunV2(CUserCmd* pCmd, BaseEntity* pLocalPlayer, baseWeapon* pActiveWeapon);

    // Used in CTFWeaponBase::CalcIsAttackCritical() hook to add up normal shots to bucket
    void AddToWeaponsBucket(baseWeapon* pActiveWeapon);
    WeaponCritData_t* GetWeaponCritData(baseWeapon* pActiveWeapon);
    
    // Recording all damage dealt by us.
    void RecordDamageEvent(IGameEvent* pEvent);
    void ResetDamageRecords();
    
    BaseEntity* m_pLocalPlayer = nullptr;
    
    void Reset();

    // Crit Bucket parameters ( depends on server configuration )
    float m_flCritBucketBottom  = 0;
    float m_flCritBucketCap     = 0;
    float m_flCritBucketDefault = 0;

private:
    bool  m_bLastShotDeemedCrit = false; // <-- This helps crithack to not break in case of accidental crits.
    bool  m_bIsCritBoosted      = false;
    float m_flLastCritHackTime  = 0.0f;
    float m_flLastFireTime      = 0.0f;
    int   m_iLastCheckSeed      = 0;
    int   m_nOldCritCount       = DEFAULT_OLD_CRIT_COUNT;
    int   m_iLastUsedCritSeed   = 0;
    int   m_nLastCritRequests   = 0;
    float m_flLastCritMult      = 0.0f;
    float m_flCritChance        = 0.0f;
    float m_flLastRapidFireCritTime             = -10.0f; // When did we detect the last rapid fire crit.
    static constexpr int DEFAULT_OLD_CRIT_COUNT = -1;
    uint32_t             m_iLastWeaponID        = 0;
    WeaponCritData_t*    m_pLastShotWeapon      = nullptr;
    WeaponCritData_t*    m_pLastWeapon          = nullptr;
    slot_t               m_iActiveWeaponSlot    = slot_t::WPN_SLOT_INVALID;

    // Determines (current) Crit restriction...
    enum CritBanStatus_t
    {
        CRIT_ALLOWED = 0,      // We can do crit whenever we want
        CRIT_BANNED,           // We CAN'T do crit, and crit request count won't change
        CRIT_TOO_EXPENSIVE     // We CAN   do crit, and crit request cound WILL  change.
    };
    CritBanStatus_t _GetCritBanStatus(BaseEntity* pLocalPlayer, WeaponCritData_t* pActiveWeaponData,
        int* p_iDamagePending = nullptr);

    // (current) Crit-Hack Status, i.e. On or Off ...
    enum CritHackStatus_t
    {
        CRITHACK_WPN_NOT_ELLIGIBLE = -1, // This weapon's not elligible for CritHack
        CRITHACK_DISABLED = 0, // Turned off
        CRITHACK_INACTIVE,     // Turned on, but user DOESN'T want to crit
        CRITHACK_ACTIVE        // Turned on, and user WANTS to crit
    };
    CritHackStatus_t _GetCritHackStatus(BaseEntity* pLocalPlayer, WeaponCritData_t* pWeaponCritData, byte iKey);
    
    // Crit seed search & confirmation...
    int  _GetCritSeed(CUserCmd* pCmd, WeaponCritData_t* pWeaponCritData, BaseEntity* pLocalPlayer);
    bool _IsSeedCrit(int iSeed, float flCritChance, WeaponCritData_t* pWeaponCritData) const;
    bool _CanThisTickPotentiallyCrit(CUserCmd* pCmd, float flCritChance, WeaponCritData_t* pWeaponCritData);
    void _AdjustWeaponsBucket(WeaponCritData_t* pWeaponData, BaseEntity* pLocalPlayer);

    void _Draw(CritBanStatus_t iBanStatus, CritHackStatus_t iCritHackStatus, 
        WeaponCritData_t* pWeaponCritData, int iPendingDamage);

    void _InitializeCVars();

    bool _IsWeaponEligibleForCritHack(BaseEntity* pLocalPlayer, baseWeapon* pActiveWeapon);
    float _GetCritChance(BaseEntity* pLocalPlayer, WeaponCritData_t* pWeaponCritData);
    
    // Forcing & Avoiding Crit...
    void _ForceCritV2(int iWishSeed, CUserCmd* pCmd, CritBanStatus_t iCritBanStatus, WeaponCritData_t* pWeaponCritData);
    void _AvoidCritV2(CUserCmd* pCmd, WeaponCritData_t* pWeaponCritData, float flCritChance);

    // Crit restrictions realated...
    bool _AreWeCritBanned(BaseEntity* pLocalPlayer, WeaponCritData_t* pActiveWeapon, int* iPendingDamage = nullptr);
    bool _CanWithdrawlCritV3(BaseEntity* pLocalPlayer, WeaponCritData_t* pActiveWeapon, int* iPendingDamage = nullptr);

    // Records any changes in Enemy health
    void _StoreHealthChanges();
    void RecordHealth(BaseEntity* pEnt);
    struct HealthRecord_t { int iOldHealth = -1, iHealth = -1; };
    std::unordered_map<BaseEntity*, HealthRecord_t> m_mapHealthRecords = {};

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