#pragma once
#include <deque>
#include "../FeatureHandler.h"

#include "../../SDK/class/Source Entity.h"
#include "../../SDK/class/BaseWeapon.h"

class CUserCmd;
class baseWeapon;
class IGameEvent;
struct WeaponData_t;

// Pending stuff
// TODO : Implement proper RAPID-FIRE support
// TODO : This is not working with STICKY-BOMBS for some reason, fix that too.
// TODO : Properly remove weapons who just can't crit, like DIAMONDBACK, etc..
// TODO : Add CritBucket Decay support.
// TODO : When sniper does a headshot, does it gets added to the total crit damage?
//        EDIT : ( YES IT DOES, the headshot damage gets added to the total crit damage. )
// TODO : Handle other edge cases & remove bullshit weapons like Wrangler and shit like that.

class WeaponCritData_t
{
public:
    void AddToCritBucket();
    void WithDrawlFromCritBucket();
    
    bool UpdateStats(baseWeapon* pWeapon);
    void Reset();

    void IncrementCritRequestCount();

    // Weapon stats
    baseWeapon*   m_pWeapon         = nullptr;
    float         m_flDamagePerShot = 0.0f;
    float         m_flCritCostBase  = 0.0f;
    int           m_iWeaponID       = 0;
    int           m_iWeaponEntIdx   = 0;
    bool          m_bIsRapidFire    = false;
    int           m_iSlot           = WPN_SLOT_INVALID;
    WeaponData_t* m_pWeaponInfo     = nullptr;
    float         m_flBulletsShotDuringCrit  = 0.0f;
    float         m_flLastWithdrawlTime      = 0.0f;
    float         m_flLastCritRequestIncTime = 0.0f;

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

    // Setups crit seed for game to use in "calcIsAttackCriticalHelper()"
    void CalcIsAttackCriticalHandler();
    
    BaseEntity* m_pLocalPlayer = nullptr;
    
    void Reset();

    // Crit Bucket parameters ( depends on server configuration )
    float m_flCritBucketBottom  = 0;
    float m_flCritBucketCap     = 0;
    float m_flCritBucketDefault = 0;

private:
    int   m_iWishSeed                    = 0;
    bool  m_bLastShotDeemedCrit          = false; // <-- This helps crithack to not break in case of accidental crits.
    bool  m_bIsCritBoosted               = false;
    float m_flLastFireTime               = 0.0f;
    float m_flLastRapidFireCritCheckTime = 0.0f;
    int   m_nOldCritCount                = DEFAULT_OLD_CRIT_COUNT;
    float m_flCritChance                 = 0.0f;
    static constexpr int DEFAULT_OLD_CRIT_COUNT = -1;
    uint32_t             m_iLastWeaponID        = 0;
    WeaponCritData_t*    m_pLastShotWeapon      = nullptr;
    WeaponCritData_t*    m_pLastWeapon          = nullptr;
    int                  m_iActiveWeaponSlot    = slot_t::WPN_SLOT_INVALID;

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
    bool _IsSeedCrit(int iSeed, float flCritChance, WeaponCritData_t* pWeaponCritData, bool bSafeCheck = false) const;
    bool _IsSeedNOTcrit(int iSeed, float flCritChance, WeaponCritData_t* pWeaponCritData, bool bSafeCheck = false) const;
    bool _CanThisTickPotentiallyCrit(CUserCmd* pCmd, float flCritChance, WeaponCritData_t* pWeaponCritData);
    void _AdjustWeaponsBucket(WeaponCritData_t* pWeaponData, BaseEntity* pLocalPlayer);

    void _Draw(CritBanStatus_t iBanStatus, CritHackStatus_t iCritHackStatus, 
        WeaponCritData_t* pWeaponCritData, int iPendingDamage);

    void _InitializeCVars();

    bool _IsWeaponEligibleForCritHack(BaseEntity* pLocalPlayer, WeaponCritData_t* pActiveWeapon);
    float _GetCritChance(BaseEntity* pLocalPlayer, WeaponCritData_t* pWeaponCritData);
    
    // Forcing & Avoiding Crit...
    void _ForceCritV2(int iWishSeed, CUserCmd* pCmd, CritBanStatus_t iCritBanStatus, WeaponCritData_t* pWeaponCritData, bool bTickConsideredForRapidFireCheck);
    void _AvoidCritV2(CUserCmd* pCmd, WeaponCritData_t* pWeaponCritData, float flCritChance)const;

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
    inline void _ClearCritCmdRecord() { m_qCritCommands.clear(); }
    std::deque<int> m_qCritCommands = {};
};

DECLARE_FEATURE_OBJECT(critHack, CritHack_t)

DEFINE_TAB(CritHack, 2)
DEFINE_SECTION(CritHack, "CritHack", 1)

//             Name               type  Section     Tab        order    default value
DEFINE_FEATURE(CritHack,          bool, "CritHack", "CritHack", 1,      false,              FeatureFlags::FeatureFlag_SupportKeyBind, "Forces crit seeds when shooting")
DEFINE_FEATURE(Always_Crit_Melee, bool, "CritHack", "CritHack", 2,      false,              FeatureFlag_SupportKeyBind,               "All Melee Swings will be crit")
DEFINE_FEATURE(Draw_Info,         bool, "CritHack", "CritHack", 3,      false,              FeatureFlag_None,                         "Draws Crit-Ban status, Crit-Hack Status")
DEFINE_FEATURE(Draw_Debug_Info,   bool, "CritHack", "CritHack", 4,      false,              FeatureFlag_None,                         "Draws Debug Related Info")