#include "CritHack.h"

// UTILITY
#include "../../Utility/signatures.h"
#include "../../Utility/ExportFnHelper.h"
#include "../../Utility/ConsoleLogging.h"
#include "../../Utility/Interface.h"
#include "../../Utility/Hook_t.h"
#include "../../Utility/PullFromAssembly.h"
#include "../../Extra/math.h"
#include "../../SDK/TF object manager/TFOjectManager.h"

// SDK
#include "../../SDK/class/CUserCmd.h"
#include "../../SDK/class/BaseWeapon.h"
#include "../../SDK/class/Source Entity.h"
#include "../../SDK/class/FileWeaponInfo.h"
#include "../../SDK/class/CVar.h"
#include "../../SDK/class/LocalPlayerScoring.h"
#include "../../SDK/class/IGameEventManager.h"
#include "../../SDK/Class ID Manager/classIDManager.h"
#include "../../SDK/class/CPrediction.h"

// DEUBG related
#include "../ImGui/InfoWindow/InfoWindow_t.h"

#define TF_DAMAGE_CRIT_CHANCE_MELEE     0.15f
#define TF_DAMAGE_CRIT_CHANCE           0.02f
#define TF_DAMAGE_CRIT_MULTIPLIER       3.0f
#define TF_DAMAGE_CRIT_CHANCE_RAPID	    0.02f
#define TF_DAMAGE_CRIT_DURATION_RAPID   2.0f
#define WEAPON_RANDOM_RANGE             10000
#define WEAPON_RANDOM_RANGE_FLOAT       10000.0f

#define TF_BONUSEFFECT_NONE             4
#define TF_BONUSEFFECT_MINICRIT         1

#define MASK_SIGNED 0x7FFFFFFF

GET_EXPORT_FN(RandomSeed, VSTDLIB_DLL, void, int)
GET_EXPORT_FN(RandomInt, VSTDLIB_DLL, int, int, int)

MAKE_SIG(CBaseEntity_SetPredictionRandomSeed,       "48 85 C9 75 ? C7 05 ? ? ? ? ? ? ? ? C3", CLIENT_DLL, int64_t, CUserCmd*);
MAKE_SIG(MD5_PseudoRandom,                          "89 4C 24 ? 55 48 8B EC 48 81 EC", CLIENT_DLL, int64_t, int);
MAKE_SIG(CTFWeaponBase_CalcIsAttackCritHelper,      "48 89 5C 24 ? 55 56 57 48 81 EC ? ? ? ? 0F 29 74 24", CLIENT_DLL, bool, baseWeapon*);
MAKE_SIG(CTFWeaponBaseMelee_CalcIsAttackCritHelper, "40 57 48 83 EC ? 48 8B 05 ? ? ? ? 48 8B F9 83 78 ? ? 75", CLIENT_DLL, bool, baseWeapon*);
MAKE_SIG(IsAllowedToWithdrawFromCritBucket,         "40 53 48 83 EC ? FF 81", CLIENT_DLL, bool, baseWeapon*, float);
MAKE_SIG(mShared_IsCritBoosted,                     "48 89 5C 24 ? 57 48 83 EC ? 48 8B D9 0F 29 7C 24", CLIENT_DLL, bool, uintptr_t);
MAKE_SIG(CTFWeaponBase_CanFireRandomCrit,           "F3 0F 58 0D ? ? ? ? 0F 2F 89", CLIENT_DLL, bool, BaseEntity*, float);

MAKE_SIG(ATRIB_HOOK_FLOAT, "4C 8B DC 49 89 5B ? 49 89 6B ? 56 57 41 54 41 56 41 57 48 83 EC ? 48 8B 3D ? ? ? ? 4C 8D 35",
    CLIENT_DLL, float, float , const char* , void* , void* , bool );

MAKE_INTERFACE_SIGNATURE(p_iPredictionSeed, "89 05 ? ? ? ? C3 CC CC CC CC CC CC 8B 02", int, CLIENT_DLL, 0x2, 0x6);

//======================= Debug Macros =======================
//#define DEBUG_CRITHACK_CVAR
//#define DEBUG_CRIT_COMMAND
//#define DEGUB_CRITHACK
//#define TEST_CRITHACK_FROM_SERVER

#define GET_CRIT_SEED(command_number) (Sig::MD5_PseudoRandom(command_number) & MASK_SIGNED)

//=========================================================================
//                     PUBLIC METHODS
//=========================================================================
void CritHack_t::RunV2(CUserCmd* pCmd, BaseEntity* pLocalPlayer, baseWeapon* pActiveWeapon)
{
    // Storing some basic info.. ( nothing to look at here )
    m_pLocalPlayer         = pLocalPlayer;
    m_iLocalPlayerEntIndex = pLocalPlayer->entindex();
    m_iActiveWeaponSlot    = pActiveWeapon->getSlot();

    // Record all changes in Health for Enimies.
    _StoreHealthChanges();

    // Initialize CVars for this server (Updates each time disconnected, i.e. (iEngine->isinGame() == false))
    _InitializeCVars();

    // Updating Weapon Crit Data
    WeaponCritData_t* pWeaponCritData = GetWeaponCritData(pActiveWeapon);
    if (pWeaponCritData == nullptr)
        return;
    bool bDidWeaponChange = pWeaponCritData->UpdateStats(pActiveWeapon);

    // Skip "Bucket-Managment", "Crit-Forcing" & "Crit-Skipping" if crit Boosted
    m_bIsCritBoosted = pLocalPlayer->IsCritBoosted();
    if (m_bIsCritBoosted == true)
        return;

    // Getting CritChance for this tick. (This is used multiple times in each tick, so I am doing it here )
    m_flCritChance = _GetCritChance(pLocalPlayer, pWeaponCritData);

    // Account for Crits from our bucket.
    _AdjustWeaponsBucket(m_pLastShotWeapon, pLocalPlayer);

    // Is CritBucket still valid?
    if (bDidWeaponChange == true || pWeaponCritData != m_pLastWeapon)
    {
        FAIL_LOG("Weapon Changed, we droped the bucket :(");
        m_qCritCommands.clear();
    }
    
    // Getting Crit Ban status
    int iPendingDamage = 0;
    CritBanStatus_t iCritBanStatus = _GetCritBanStatus(pLocalPlayer, pWeaponCritData, &iPendingDamage);
    
    // Getting Crit Seed if applicable
    int iWishSeed = NULL;
    if (iCritBanStatus == CritBanStatus_t::CRIT_ALLOWED)
        iWishSeed = _GetCritSeed(pCmd, pWeaponCritData, pLocalPlayer);
    
    // Are we shooting on this crit ?
    float flNextFireTime = pWeaponCritData->m_pWeapon->GetNextPrimaryAttackTime();
    float flCurTime      = static_cast<float>(pLocalPlayer->GetTickBase()) * tfObject.pGlobalVar->interval_per_tick;
    bool  bShotFired     = (pCmd->buttons & IN_ATTACK) == true && flCurTime >= flNextFireTime/* && flNextFireTime > m_flLastFireTime*/;

    CritHackStatus_t iCritHackStatus = _GetCritHackStatus(pLocalPlayer, pWeaponCritData, VK_LSHIFT);
    if(bShotFired == true)
    {
        // Will this bullet be considered for Rapid-Fire crit check on the server.
        bool bTickConsideredForRapidFireCheck = flNextFireTime > m_flLastRapidFireCritCheckTime + 1.0f;

        // Deciding what to do. Crit yes ? Crit no ? don't bother ?
        switch (iCritHackStatus)
        {
        case CRITHACK_ACTIVE:
            _ForceCritV2(iWishSeed, pCmd, iCritBanStatus, pWeaponCritData, bTickConsideredForRapidFireCheck);
            break;
        case CRITHACK_INACTIVE:
            _AvoidCritV2(pCmd, pWeaponCritData, m_flCritChance);
            break;
        case CRITHACK_WPN_NOT_ELLIGIBLE:
        case CRITHACK_DISABLED: // Just let it go
        default:
            break;
        }

        // Don't try to predict "Crit-Request" with Rapid-fire weapons.
        if (pWeaponCritData->m_bIsRapidFire == false)
        {
            // Adjusting Crit Request count if this tick crits
            if (_CanThisTickPotentiallyCrit(pCmd, m_flCritChance, pWeaponCritData) == true)
            {
                m_bLastShotDeemedCrit = true;
                pWeaponCritData->IncrementCritRequestCount();
            }
            else
            {
                m_bLastShotDeemedCrit = false;
            }
        }

        // Updating the last weapon we used to shoot, to make sure we alter the bucket correctly.
        m_pLastShotWeapon = pWeaponCritData;
        m_flLastFireTime  = flNextFireTime;
    }
    
    m_pLastWeapon = pWeaponCritData;
    _Draw(iCritBanStatus, iCritHackStatus, pWeaponCritData, iPendingDamage);
}

void CritHack_t::_Draw(CritBanStatus_t iBanStatus, CritHackStatus_t iCritHackStatus, WeaponCritData_t* pWeaponCritData, int iPendingDamage)
{
    // CRIT BAN STATUS
    switch (iBanStatus)
    {
    case CritHack_t::CRIT_ALLOWED:
        Render::InfoWindow.AddToCenterConsole("Ban Status", "CRIT-ALLOWED", GREEN);
        break;
    case CritHack_t::CRIT_BANNED:
        Render::InfoWindow.AddToCenterConsole("Ban Status", std::format("CRIT-BANNED, {}dmg left", iPendingDamage), RED);
        break;
    case CritHack_t::CRIT_TOO_EXPENSIVE:
        Render::InfoWindow.AddToCenterConsole("Ban Status", std::format("CRIT-TOO_EXPENSIVE, {}dmg left", iPendingDamage), YELLOW);
        break;
    default:
        break;
    }

    // CRIT HACK STATUS
    switch (iCritHackStatus)
    {
    case CritHack_t::CRITHACK_WPN_NOT_ELLIGIBLE:
        Render::InfoWindow.AddToCenterConsole("CritHack Status", "NA", RED);
        break;
    case CritHack_t::CRITHACK_DISABLED:
        Render::InfoWindow.AddToCenterConsole("CritHack Status", "Disabled", RED);
        break;
    case CritHack_t::CRITHACK_INACTIVE:
        Render::InfoWindow.AddToCenterConsole("CritHack Status", "In-Active", YELLOW);
        break;
    case CritHack_t::CRITHACK_ACTIVE:
        Render::InfoWindow.AddToCenterConsole("CritHack Status", "Active", GREEN);
        break;
    default:
        break;
    }

    // Crit Reserve info...
    Render::InfoWindow.AddToCenterConsole("ReserveCritSeeds", std::format("{} Crit Seeds in reserve", m_qCritCommands.size()), m_qCritCommands.size() == 0 ? RED : GREEN);

    // Weapon's bucket's stats info...
    Render::InfoWindow.AddToInfoWindow("bucket",       std::format("Weapon Bucket : {}",   pWeaponCritData->m_flCritBucket));
    Render::InfoWindow.AddToInfoWindow("critRequests", std::format("Crit Requests : {}",   pWeaponCritData->m_nCritRequests));
    Render::InfoWindow.AddToInfoWindow("BaseCritCost", std::format("Base CritCost : {}",   pWeaponCritData->m_flCritCostBase));
    Render::InfoWindow.AddToInfoWindow("dmgPerBullet", std::format("DMG. per bullet : {}", pWeaponCritData->m_flDamagePerShot));
    Render::InfoWindow.AddToInfoWindow("WeaponID",     std::format("Cur Weapon ID : {}",   pWeaponCritData->m_iWeaponID));
    Render::InfoWindow.AddToInfoWindow("CritChecks",   std::format("Crit Checks   : {}",   pWeaponCritData->m_pWeapon->GetTotalCritChecks()));
    Render::InfoWindow.AddToInfoWindow("CritOccuered", std::format("Crit Occured accoding to game : {}", pWeaponCritData->m_pWeapon->GetTotalCritsOccured()));
    Render::InfoWindow.AddToInfoWindow("GamesBucket ", std::format("Weapon Bucket ( game's ): {}",       pWeaponCritData->m_pWeapon->GetCritBucket()));
    Render::InfoWindow.AddToInfoWindow("WeaponID",     std::format("{} <- Weapon ID",      static_cast<int>(pWeaponCritData->m_iSlot)));
}

void CritHack_t::CalcIsAttackCriticalHandler()
{
    if (I::cPrediction->m_bFirstTimePredicted == false)
        return;

    if(m_iWishSeed != 0)
    {
        ExportFn::RandomSeed(m_iWishSeed);
        *I::p_iPredictionSeed = m_iWishSeed;
        m_iWishSeed = 0;
    }
}

void CritHack_t::_AdjustWeaponsBucket(WeaponCritData_t* pWeaponData, BaseEntity* pLocalPlayer)
{
    RoundStats_t* pRoundStats = pLocalPlayer->GetPlayerRoundData();
    if (pRoundStats == nullptr)
        return;

    if (pRoundStats->m_iCrits > m_nOldCritCount && m_nOldCritCount != DEFAULT_OLD_CRIT_COUNT)
    {
        // if this crit was accidental
        if (m_bLastShotDeemedCrit == false)
        {
            m_pLastShotWeapon->IncrementCritRequestCount();
            FAIL_LOG("Detected & Accounted for an accidental Crit");
        }

        m_pLastShotWeapon->WithDrawlFromCritBucket();
    }

    m_nOldCritCount = pRoundStats->m_iCrits;
}

bool CritHack_t::_CanThisTickPotentiallyCrit(CUserCmd* pCmd, float flCritChance, WeaponCritData_t* pWeaponCritData)
{
    int iFutureSeed = GET_CRIT_SEED(pCmd->command_number);
    return _IsSeedCrit(iFutureSeed, flCritChance, pWeaponCritData, false);
}

void CritHack_t::_AvoidCritV2(CUserCmd* pCmd, WeaponCritData_t* pWeaponCritData, float flCritChance) const
{
    constexpr int CRIT_SEED_SEARCH_RANGE = 10;
    for (uint32_t iOffset = 0; iOffset < CRIT_SEED_SEARCH_RANGE; iOffset++)
    {
        int iFutureSeed = GET_CRIT_SEED(pCmd->command_number + iOffset);
        
        if (_IsSeedNOTcrit(iFutureSeed, m_flCritChance, pWeaponCritData, true) == true)
        {
            // Avoiding this crit seed
            pCmd->command_number  += iOffset;
            pCmd->random_seed     = iFutureSeed;
            *I::p_iPredictionSeed = iFutureSeed;

            if (iOffset > 0)
                WIN_LOG("This seed was a crit, but we ditched it, cause you didn't wanted to");

            // I absolutely didn't forgot about this :)
            return;
        }
    }
}

void CritHack_t::_ForceCritV2(int iWishSeed, CUserCmd* pCmd, CritBanStatus_t iCritBanStatus, WeaponCritData_t* pWeaponCritData, bool bTickConsideredForRapidFireCheck)
{
    switch (iCritBanStatus)
    {
    case CritHack_t::CRIT_BANNED: // Just let it go, if we are Crit Banned
        return;
    // Avoid any potential Crits if we can't afford them, this can mess with the CritRequest count
    case CritHack_t::CRIT_TOO_EXPENSIVE: 
        _AvoidCritV2(pCmd, pWeaponCritData, m_flCritChance);
        return;
    case CritHack_t::CRIT_ALLOWED:
    default:
        break;
    }

    // If no seed then return
    if (iWishSeed == NULL)
    {
        FAIL_LOG("No Seed to crit MF, WHO TF WROTE THIS BULLSHIT >:(");
        return;
    }

    // Will this tick even be considered for rapid fire check?
    if (pWeaponCritData->m_bIsRapidFire == true && bTickConsideredForRapidFireCheck == false)
        return;

    WIN_LOG("---> ATTEMPTING CRIT WITH [ %d ] <---", iWishSeed);

    // Setting Crit seed in
    m_iWishSeed           = iWishSeed;
    pCmd->command_number  = iWishSeed;
    pCmd->random_seed     = Sig::MD5_PseudoRandom(iWishSeed) & MASK_SIGNED;
    *I::p_iPredictionSeed = pCmd->random_seed;
}

CritHack_t::CritHackStatus_t CritHack_t::_GetCritHackStatus(BaseEntity* pLocalPlayer, WeaponCritData_t* pWeaponCritData, byte iKey)
{
    // If not turned ON
    if (TempFeatureHelper::CritHack.IsDisabled() == true)
        return CritHackStatus_t::CRITHACK_DISABLED;

    // Does this weapon support Crit-Hack
    if (_IsWeaponEligibleForCritHack(pLocalPlayer, pWeaponCritData) == false)
        return CritHackStatus_t::CRITHACK_WPN_NOT_ELLIGIBLE;

    // Always Crit Enabled ?
    if (pWeaponCritData->m_iSlot == WPN_SLOT_MELLE && TempFeatureHelper::Always_Crit_Melee.IsActive() == true)
        return CritHackStatus_t::CRITHACK_ACTIVE;

    // Is the "Magic" key down?
    if (TempFeatureHelper::CritHack.IsActive() == false)
        return CritHackStatus_t::CRITHACK_INACTIVE;

    // Just do it!
    return CritHackStatus_t::CRITHACK_ACTIVE;
}

int CritHack_t::_GetCritSeed(CUserCmd* pCmd, WeaponCritData_t* pWeaponCritData, BaseEntity* pLocalPlayer)
{
    int iWishSeed = NULL;

    // Looping over all searched crit seeds, and Discarding expired ones
    // & Getting crit seed thats still good.
    for (int iSeed : m_qCritCommands)
    {
        if (iSeed < pCmd->command_number)
        {
            m_qCritCommands.pop_back();
        }
        else
        {
            iWishSeed = iSeed;
            m_qCritCommands.pop_back();
        }
    }

    constexpr uint16_t MIN_CRIT_SEED_COUNT = 8;
    constexpr uint16_t MAX_CRIT_SEED_COUNT = 32;

    // Returning Crit seed if we have enough Crit Seeds in reserve
    if (m_qCritCommands.size() >= MIN_CRIT_SEED_COUNT)
        return iWishSeed;


    // else We will search for crit seeds to put in reserve...
    constexpr uint32_t MAX_CRIT_SEED_SEARCH_RANGE = 512;
    for (uint32_t iOffset = 0; iOffset < MAX_CRIT_SEED_SEARCH_RANGE && m_qCritCommands.size() < MAX_CRIT_SEED_COUNT; iOffset++)
    {
        // Calculating future seed
        int iFutureSeed = Sig::MD5_PseudoRandom(pCmd->command_number + iOffset) & MASK_SIGNED;
        
        // Checking & Adding future seed if it crits
        if (_IsSeedCrit(iFutureSeed, m_flCritChance, pWeaponCritData, true) == true)
            m_qCritCommands.push_front(pCmd->command_number + iOffset);
    }

    if (m_qCritCommands.empty() == false)
    {
        iWishSeed = m_qCritCommands.back();
        m_qCritCommands.pop_back();
    }

    return iWishSeed; // This can still be NULL if didn't find a seed in the search. Be aware
}


// CREDITS to Amalgum for this safe search logic.
bool CritHack_t::_IsSeedCrit(int iSeed, float flCritChance, WeaponCritData_t* pWeaponCritData, bool bSafeCheck) const
{
    // Decypting seed, just a little bit of bit shifting
    int iBitShiftOffset = pWeaponCritData->m_iSlot == WPN_SLOT_MELLE ? 8 : 0;
    int iEncryptedSeed  = iSeed ^ ((pWeaponCritData->m_iWeaponEntIdx << (8 + iBitShiftOffset)) | m_iLocalPlayerEntIndex << iBitShiftOffset);
    
    // Setting seed & Getting random int
    ExportFn::RandomSeed(iEncryptedSeed);
    float iRandom = static_cast<float>(ExportFn::RandomInt(0, WEAPON_RANDOM_RANGE - 1));

    // might give wrong result, as player's Crit-Multipier updates with a delay on official servers
    if(bSafeCheck == false)
        return flCritChance * WEAPON_RANDOM_RANGE_FLOAT > iRandom;

    // iRandom must be smaller then these, for a reliable output
    constexpr float RANDOM_INT_SAFE_UPPER_LIMIT       = 150.0f; // [ 200 , 800 ] <- Origial range
    constexpr float RANDOM_INT_SAFE_UPPER_LIMIT_MELEE = 1000.0f;// [ 1500, 6000] <- Origial range

    float iUpperLimit = pWeaponCritData->m_iSlot == WPN_SLOT_MELLE ? RANDOM_INT_SAFE_UPPER_LIMIT_MELEE : RANDOM_INT_SAFE_UPPER_LIMIT;

    return iRandom < iUpperLimit && iRandom < flCritChance * WEAPON_RANDOM_RANGE_FLOAT;
}

bool CritHack_t::_IsSeedNOTcrit(int iSeed, float flCritChance, WeaponCritData_t* pWeaponCritData, bool bSafeCheck) const
{
    // Decypting seed, just a little bit of bit shifting
    int iBitShiftOffset = pWeaponCritData->m_iSlot == WPN_SLOT_MELLE ? 8 : 0;
    int iEncryptedSeed  = iSeed ^ ((pWeaponCritData->m_iWeaponEntIdx << (8 + iBitShiftOffset)) | m_iLocalPlayerEntIndex << iBitShiftOffset);

    // Setting seed & Getting random int
    ExportFn::RandomSeed(iEncryptedSeed);
    float iRandom = static_cast<float>(ExportFn::RandomInt(0, WEAPON_RANDOM_RANGE - 1));

    // might give wrong result, as player's Crit-Multipier updates with a delay on official servers
    if (bSafeCheck == false)
        return flCritChance * WEAPON_RANDOM_RANGE_FLOAT < iRandom;

    // iRandom must be smaller then these, for a reliable output
    constexpr float RANDOM_INT_SAFE_UPPER_LIMIT       = 1000.0f; // [ 200 , 800 ] <- Origial range
    constexpr float RANDOM_INT_SAFE_UPPER_LIMIT_MELEE = 7000.0f;// [ 1500, 6000] <- Origial range

    float iLowerLimit = pWeaponCritData->m_iSlot == WPN_SLOT_MELLE ? RANDOM_INT_SAFE_UPPER_LIMIT_MELEE : RANDOM_INT_SAFE_UPPER_LIMIT;

    return iRandom > iLowerLimit && iRandom > flCritChance * WEAPON_RANDOM_RANGE_FLOAT;
}

CritHack_t::CritBanStatus_t CritHack_t::_GetCritBanStatus(BaseEntity* pLocalPlayer, WeaponCritData_t* pActiveWeaponData, int* p_iDamagePending)
{
    // Crit Ban check
    if (_AreWeCritBanned(pLocalPlayer, pActiveWeaponData, p_iDamagePending) == true)
        return CritBanStatus_t::CRIT_BANNED;

    // Can we afford this Crit
    if (_CanWithdrawlCritV3(pLocalPlayer, pActiveWeaponData, p_iDamagePending) == false)
        return CritBanStatus_t::CRIT_TOO_EXPENSIVE;

    // We are free to Crit
    return CritBanStatus_t::CRIT_ALLOWED;
}


bool CritHack_t::_CanWithdrawlCritV3(BaseEntity* pLocalPlayer, WeaponCritData_t* pActiveWeapon, int* iPendingDamage)
{
    float flMult = (pActiveWeapon->m_iSlot == WPN_SLOT_MELLE) ?
        0.5f :
        Maths::RemapValClamped(static_cast<float>(pActiveWeapon->m_nCritRequests + 1) / static_cast<float>(pActiveWeapon->m_pWeapon->GetTotalCritChecks() + 1),
            0.1f, 1.f, 1.f, 3.f);

    float flCritCost = pActiveWeapon->m_flCritCostBase * TF_DAMAGE_CRIT_MULTIPLIER * flMult;

    // if this Crit is too expensive
    if (flCritCost > pActiveWeapon->m_flCritBucket)
    {
        if (iPendingDamage != nullptr)
            *iPendingDamage = flCritCost - pActiveWeapon->m_flCritBucket;
        return false;
    }

    // If we can afford this crit
    if (iPendingDamage != nullptr)
        *iPendingDamage = 0;
    return true;
}


void CritHack_t::RecordDamageEvent(IGameEvent* pEvent)
{
    // if just spawned in the game, then it will be nullptr, then we don't save damage dealt ( sometimes it store bullshit damage as soon as we join )
    if (m_pLocalPlayer == nullptr)
        return;

    // if not ALIVE then no damage storing
    if (m_pLocalPlayer->getLifeState() != lifeState_t::LIFE_ALIVE)
        return;

    // Melee DMG isn't considered
    if (m_iActiveWeaponSlot == slot_t::WPN_SLOT_MELLE || m_iActiveWeaponSlot == slot_t::WPN_SLOT_INVALID)
        return;

    // Skip if not local player
    if (I::iEngine->GetPlayerForUserID(pEvent->GetInt("attacker")) != m_iLocalPlayerEntIndex)
        return;

    // damage dealt, new health & victim of that damage
    int         iDamage = pEvent->GetInt("damageamount");
    int         iHealth = pEvent->GetInt("health");
    bool        bCrit   = pEvent->GetBool("crit");
    BaseEntity* pVictim = I::IClientEntityList->GetClientEntityFromUserID(pEvent->GetInt("userid"));
    if (pVictim == nullptr)
        return;

    // Do we have damage records for this victim
    auto it = m_mapHealthRecords.find(pVictim);
    if (it == m_mapHealthRecords.end())
        return;
    
    // How much damage did he actually take ( can't give more damage then health left )
    int iDamageTaken = 0;
    
    // is Death-Ringer spy faking his death
    bool bFakeDeath = pVictim->getCharacterChoice() == TF_SPY && (pVictim->IsFeignDeathReady() == true || pVictim->getPlayerCond() & TF_COND_FEIGN_DEATH);

    // If we "kill-da-victim" or if he faked his death ( using Death-Ringer ) then use our records for calculating damage dealt.
    if (iHealth <= 0 || bFakeDeath)
        iDamageTaken = it->second.iHealth;
    else
        iDamageTaken = iDamage;

    // Add to Total Damage dealt by us
    m_iTotalDamage += iDamageTaken;
    
    // did Victim die to a Crit?
    if (bCrit == true && m_pLocalPlayer->IsCritBoosted() == false) // CritBoosted damage isn't considered
        m_iRangedCritDamage += iDamageTaken;
}


void CritHack_t::ResetDamageRecords()
{
    // Resetting damage records, cause "bool CTFWeaponBase::CanFireRandomCriticalShot( float flCritChance )" 
    //      uses current round damage
    m_iTotalDamage      = 0.0f;
    m_iRangedCritDamage = 0.0f;

    WIN_LOG("We detected round change, we reset damage records :)");
}


void CritHack_t::AddToWeaponsBucket(baseWeapon* pActiveWeapon)
{
    // Nothing gets added if Crit-Boosted.
    if (m_bIsCritBoosted == true)
        return;

    switch (pActiveWeapon->getSlot())
    {
    case WPN_SLOT_PRIMARY:
        m_PrimaryCritData.AddToCritBucket();
        return;
    case WPN_SLOT_SECONDARY:
        m_SecondaryCritData.AddToCritBucket();
        return;
    case WPN_SLOT_MELLE:
        m_MeleeCritData.AddToCritBucket();
        return;
    default:
        break;
    }
}


WeaponCritData_t* CritHack_t::GetWeaponCritData(baseWeapon* pActiveWeapon)
{
    switch (pActiveWeapon->getSlot())
    {
    case WPN_SLOT_PRIMARY:
        return &m_PrimaryCritData;
    case WPN_SLOT_SECONDARY:
        return &m_SecondaryCritData;
    case WPN_SLOT_MELLE:
        return &m_MeleeCritData;
    default:
        return nullptr;
    }
}


void CritHack_t::Reset()
{
    // Resetting Crit Bucket parameters ( server specific )
    m_bCVarsInitialized       = false;
    m_flCritBucketBottom      = 0.0f;
    m_flCritBucketCap         = 0.0f;
    m_flCritBucketDefault     = 0.0f;

                              
    // Resetting...           
    m_iWishSeed               = 0;
    m_nOldCritCount           = DEFAULT_OLD_CRIT_COUNT;
    m_iLastWeaponID           = 0;
    m_pLastShotWeapon         = nullptr;
    m_pLocalPlayer            = nullptr;
    m_iActiveWeaponSlot       = slot_t::WPN_SLOT_INVALID;
    m_iLocalPlayerEntIndex    = 0;
    m_iTotalDamage            = 0;
    m_iRangedCritDamage       = 0;
    m_flCritChance            = 0.0f;
    m_flLastFireTime          = 0.0f;
    m_flLastRapidFireCritCheckTime = 0.0f;
    m_bIsCritBoosted          = false;
    m_pLastWeapon             = nullptr;
    m_bLastShotDeemedCrit     = false;

    // Reseting Weapon's crit data
    m_PrimaryCritData.Reset();
    m_SecondaryCritData.Reset();
    m_MeleeCritData.Reset();

    // Clearing Health Records
    m_mapHealthRecords.clear();
}


//=========================================================================
//                   Server.dll CritHack Testing
//=========================================================================

#ifdef TEST_CRITHACK_FROM_SERVER

MAKE_INTERFACE_SIGNATURE(ServerPredictionSeed, "33 0D ? ? ? ? 3B 8B ? ? ? ? 74 ? 89 8B ? ? ? ? FF 15 ? ? ? ? BA ? ? ? ? 33 C9 FF 15 ? ? ? ? 41 0F 28 C0",
    int*, SERVER_DLL, 0x2, 0x6);

MAKE_HOOK(Server_GetPlayerStats, "4C 8B C1 48 85 D2 75", __fastcall, SERVER_DLL,
    uintptr_t, void* idk1, void* idk2)
{
    uintptr_t pPlayerStats = Hook::Server_GetPlayerStats::O_Server_GetPlayerStats(idk1, idk2);
    
    int nTotalDamage      = *reinterpret_cast<int*>(pPlayerStats + 0x144);
    int nRangedCritDamage = *reinterpret_cast<int*>(pPlayerStats + 0x148);

    // if bot, then no printing
    if (nTotalDamage == 0 && nRangedCritDamage == 0)
        return pPlayerStats;

    // Damage stats for server
    Render::InfoWindow.AddToInfoWindow("ServerTotalDamage", std::format("SERVER Total Damage : {}", nTotalDamage));
    Render::InfoWindow.AddToInfoWindow("ServerCritDamage",  std::format("SERVER Crit Damage  : {}", nRangedCritDamage));
    
    return pPlayerStats;
}


MAKE_HOOK(Server_CanFireRandomCritShot, "40 53 48 83 EC ? 0F 29 74 24 ? 48 8B D9 0F 28 F1 E8 ? ? ? ? 48 8B C8", __fastcall, SERVER_DLL,
    bool, void* idk1, void* idk2)
{
    bool result = Hook::Server_CanFireRandomCritShot::O_Server_CanFireRandomCritShot(idk1, idk2);
    printf("CanFireRandomCritShot says : %s\n", result ? "TRUE" : "FALSE");
    return result;
}

MAKE_HOOK(Server_IsAllowedToWithdrawlFromCritBucket, "40 53 48 83 EC ? FF 81 ? ? ? ? 48 8B D9 0F B7 89", __fastcall, SERVER_DLL,
    bool, void* idk1, float flDamage)
{
    bool result = Hook::Server_IsAllowedToWithdrawlFromCritBucket::O_Server_IsAllowedToWithdrawlFromCritBucket(idk1, flDamage);
    printf("CirtBucket says : %s\n", result ? "TRUE" : "FALSE");
    WIN_LOG("flCost : %.2f", flDamage);
    return result;
}

MAKE_HOOK(Server_TFBaseWeapon_CalcIsAttackCriticalHelper, "48 89 5C 24 ? 55 56 57 41 56 41 57 48 81 EC ? ? ? ? 0F 29 74 24", __fastcall, SERVER_DLL,
    bool, baseWeapon* pActiveWeapon)
{
    printf("<----------------------------------->\n");

    // Calling Original first
    bool bCrit = Hook::Server_TFBaseWeapon_CalcIsAttackCriticalHelper::O_Server_TFBaseWeapon_CalcIsAttackCriticalHelper(pActiveWeapon);
    
    printf("Final Verdict : %s\n", bCrit ? "TRUE" : "FALSE");
    printf("<----------------------------------->\n");

    // Getting weapon stats 
    int   nTotalCritChecks  = *reinterpret_cast<int*>(reinterpret_cast<uintptr_t>(pActiveWeapon) + 0x760);
    int   nTotalCritOccured = *reinterpret_cast<int*>(reinterpret_cast<uintptr_t>(pActiveWeapon) + 0x764);
    int   nCurrentSeed      = *reinterpret_cast<int*>(reinterpret_cast<uintptr_t>(pActiveWeapon) + 0x8E0);
    float flCritbucket      = *reinterpret_cast<float*>(reinterpret_cast<uintptr_t>(pActiveWeapon) + 0x75C);
    float flObsCritChance   = *reinterpret_cast<float*>(reinterpret_cast<uintptr_t>(pActiveWeapon) + 0x9C4);

    // Debugging
#if !defined(PRINT_BULLET_INFO_SERVER)
    WIN_LOG("[ %s ]", bCrit ? "Crit :)" : "No-Crit :(");
    WIN_LOG("Weapon's bucket     : [ %.2f ]", flCritbucket);
    WIN_LOG("Total Crit Checks   : [ %d ]",   nTotalCritChecks);
    WIN_LOG("Total Crit Occured  : [ %d ]",   nTotalCritOccured);
    WIN_LOG("Server Pred. Seed   : [ %d ]",   *I::ServerPredictionSeed);
    WIN_LOG("Updated observed crit chance : [ %.2f ]",   flObsCritChance);
#endif

    return bCrit;
}

#endif

//=========================================================================
//                     PRIVATE METHODS
//=========================================================================
void CritHack_t::_InitializeCVars()
{
    if (m_bCVarsInitialized == true)
        return;

    m_flCritBucketBottom  = I::iCvar->FindVar("tf_weapon_criticals_bucket_bottom")->GetFloat();
    m_flCritBucketCap     = I::iCvar->FindVar("tf_weapon_criticals_bucket_cap")->GetFloat();
    m_flCritBucketDefault = I::iCvar->FindVar("tf_weapon_criticals_bucket_default")->GetFloat();

    m_bCVarsInitialized   = true;

    WIN_LOG("Intitialized CVars for this server");

#if defined(DEBUG_CRITHACK_CVAR)
    Render::InfoWindow.AddToInfoWindow("bucket bottom",  std::format("Bucket bottom {:.2f}",  m_flCritBucketBottom));
    Render::InfoWindow.AddToInfoWindow("bucket cap",     std::format("Bucket cap {:.2f}",     m_flCritBucketCap));
    Render::InfoWindow.AddToInfoWindow("bucket default", std::format("Bucket default {:.2f}", m_flCritBucketDefault));
#endif
}

bool CritHack_t::_IsWeaponEligibleForCritHack(BaseEntity* pLocalPlayer, WeaponCritData_t* pActiveWeapon)
{
    player_class iCharChoice = pLocalPlayer->getCharacterChoice();

    // Skipping Sniper's primary
    if (iCharChoice == TF_SNIPER && pActiveWeapon->m_iSlot == WPN_SLOT_PRIMARY)
        return false;

    // Skipping Spy's every weapon, except primary ( revolver )
    if (iCharChoice == TF_SPY && pActiveWeapon->m_iSlot != WPN_SLOT_PRIMARY)
        return false;

    // Skipping any "Buff-based" secondaries, like Batalion-backup etc...
    if (pActiveWeapon->m_pWeaponInfo->m_nBulletsPerShot <= 0 && pActiveWeapon->m_iSlot != WPN_SLOT_MELLE)
        return false;

    return true;
}

float CritHack_t::_GetCritChance(BaseEntity* pLocalPlayer, WeaponCritData_t* pWeaponCritData)
{
    // Getting players crit multiplier
    float flPlayerCritMult = pLocalPlayer->GetCritMult();
    float flCritChance = 0.0f;

    // Crit Chance for Melee weapons
    if (pWeaponCritData->m_iSlot == WPN_SLOT_MELLE)
    {
        flCritChance = flPlayerCritMult * TF_DAMAGE_CRIT_CHANCE_MELEE;
    }
    // Crit Chance for Rapid-fire weapons
    else if (pWeaponCritData->m_bIsRapidFire == true)
    {
        float flTotalCritChance = std::clamp(TF_DAMAGE_CRIT_CHANCE_RAPID * flPlayerCritMult, 0.01f, 0.99f);
        flCritChance = 1.0f / ((TF_DAMAGE_CRIT_CHANCE_RAPID / flTotalCritChance) - TF_DAMAGE_CRIT_CHANCE_RAPID);
    }
    // Crit Chance for Non-Melee & Non-Rapid-fire Weapons
    else
    {
        flCritChance = TF_DAMAGE_CRIT_CHANCE * flPlayerCritMult;
    }

    // Accouting for attributes
    flCritChance = Sig::ATRIB_HOOK_FLOAT(flCritChance, "mult_crit_chance", static_cast<BaseEntity*>(pWeaponCritData->m_pWeapon), 0, true);

    return flCritChance;
}


bool CritHack_t::_AreWeCritBanned(BaseEntity* pLocalPlayer, WeaponCritData_t* pActiveWeapon, int* iPendingDamage)
{
    // Melees don't get crit banned
    if (pActiveWeapon->m_iSlot == WPN_SLOT_MELLE)
        return false;

    // getting crit chance
    float flCritChance = pLocalPlayer->GetCritMult() * TF_DAMAGE_CRIT_CHANCE;
    flCritChance       = Sig::ATRIB_HOOK_FLOAT(flCritChance, "mult_crit_chance", static_cast<BaseEntity*>(pActiveWeapon->m_pWeapon), 0, true);
    flCritChance       += 0.1f;

    float flNormalizedCritDamage = static_cast<float>(m_iRangedCritDamage) / TF_DAMAGE_CRIT_MULTIPLIER;
    float flObservedCritChance   = flNormalizedCritDamage / (flNormalizedCritDamage + static_cast<float>(m_iTotalDamage - m_iRangedCritDamage));

    // if less than observerd crit chance the no crits for us :(
    if (flObservedCritChance > flCritChance)
    {
        if(iPendingDamage != nullptr)
            *iPendingDamage = m_iTotalDamage - ((flNormalizedCritDamage / flCritChance) + (2.0f * flNormalizedCritDamage)); // I don't know if its right, cause I ain't very good at maths
        return true;
    }

    // else, "shoot floor, big score :)"
    if (iPendingDamage != nullptr)
        *iPendingDamage = 0;
    return false;
}


void CritHack_t::_StoreHealthChanges()
{
    uint32_t nEntities = I::IClientEntityList->NumberOfEntities(false);
    for (int iEnt = 0; iEnt < nEntities; iEnt++)
    {
        auto* pEnt = I::IClientEntityList->GetClientEntity(iEnt);
        
        // Fuck null ptrs
        if (pEnt == nullptr)
            continue;
        
        // Fuck Dormant entities
        if (pEnt->IsDormant() == true)
            continue;

        auto iEntID = IDManager.getID(pEnt);

        // Fuck non-player entities
        if (iEntID != IDclass_t::PLAYER)
            continue;

        // Fuck non-enemy entities
        if (pEnt->isEnemy() == false)
            continue;

        // Fuck Dead enimies
        if (pEnt->getLifeState() != lifeState_t::LIFE_ALIVE)
            continue;
        
        // Record any change in health
        RecordHealth(pEnt);
    }
}


void CritHack_t::RecordHealth(BaseEntity* pEnt)
{
    int iHealth = pEnt->getEntHealth();

    // Entity Present
    auto it = m_mapHealthRecords.find(pEnt);

    // Add to map if new entity
    if (it == m_mapHealthRecords.end())
    {
        m_mapHealthRecords.insert({ pEnt, HealthRecord_t(-1, iHealth) });
        return;
    }
    
    // Did health change?
    if (it->second.iHealth == iHealth)
        return;

    // push in new health
    it->second.iOldHealth = it->second.iHealth;
    it->second.iHealth    = iHealth;
}


//=========================================================================
//                     WEAPON CRIT DATA implementation
//=========================================================================
void WeaponCritData_t::AddToCritBucket()
{
    if (m_flCritCostBase <= 0.0f)
    {
        FAIL_LOG("damage is set to %.2f, not initialized!!", m_flDamagePerShot);
        return;
    }

    // adding to crit bucket, and capping it.
    m_flCritBucket += m_flDamagePerShot;
    if (m_flCritBucket >= FeatureObj::critHack.m_flCritBucketCap)
        m_flCritBucket = FeatureObj::critHack.m_flCritBucketCap;
}


void WeaponCritData_t::WithDrawlFromCritBucket()
{
    // Only withdrawl once per rapid fire crit.
    if (m_bIsRapidFire == true && tfObject.pGlobalVar->curtime < m_flLastWithdrawlTime + TF_DAMAGE_CRIT_DURATION_RAPID)
        return;

    // calculating cost
    float flCritMult = m_pWeapon->getSlot() == WPN_SLOT_MELLE ? 0.5f :
        Maths::RemapValClamped(static_cast<float>(m_nCritRequests) / static_cast<float>(m_pWeapon->GetTotalCritChecks()),
            0.1f, 1.f, 1.f, 3.f);
    float flCritCost = (m_flCritCostBase * TF_DAMAGE_CRIT_MULTIPLIER) * flCritMult;

    // is COST valid ?
    if (flCritCost <= 0.0f)
    {
        FAIL_LOG("crit cost [ %.2f ] was bad :(", flCritCost);
        return;
    }
    WIN_LOG("Deducted cost      [ %.2f ] from crit bucket | new bucket [ %.2f ]", flCritCost, m_flCritBucket - flCritCost);
    WIN_LOG("totalCritOccured : [ %d ] & total crit checks : [ %d ]", m_nCritRequests, m_pWeapon->GetTotalCritChecks());

    // deducting crit cost & capping bucket
    m_flCritBucket -= flCritCost;
    if (m_flCritBucket < FeatureObj::critHack.m_flCritBucketBottom)
        m_flCritBucket = FeatureObj::critHack.m_flCritBucketBottom;

    // Storing last withdrawl time
    m_flLastWithdrawlTime = tfObject.pGlobalVar->curtime;
}


bool WeaponCritData_t::UpdateStats(baseWeapon* pWeapon)
{
    if (m_pWeapon == nullptr)
        m_flCritBucket = FeatureObj::critHack.m_flCritBucketDefault;
    
    // if weapon same as last tick then no worry
    if (pWeapon == m_pWeapon && pWeapon->GetWeaponDefinitionID() == m_iWeaponID)
        return false;

    // reset stats
    m_pWeaponInfo     = pWeapon->GetTFWeaponInfo()->GetWeaponData(0);
    m_pWeapon         = pWeapon;
    m_iWeaponID       = pWeapon->GetWeaponDefinitionID();
    m_flDamagePerShot = pWeapon->GetDamagePerShot();
    m_flCritCostBase  = m_flDamagePerShot;
    m_iSlot           = pWeapon->getSlot();
    m_iWeaponEntIdx   = pWeapon->entindex();

    // is Weapon Rapid fire ?
    m_bIsRapidFire = m_pWeaponInfo->m_bUseRapidFireCrits && (m_iSlot != WPN_SLOT_MELLE);
    if(m_bIsRapidFire)
    {
        // Get number of bullets "to-be" fired in "rapid-fire crit" duration of 2 seconds
        m_flBulletsShotDuringCrit = TF_DAMAGE_CRIT_DURATION_RAPID / m_pWeaponInfo->m_flTimeFireDelay;
        
        // Base crit damage ( for rapid fire weapons, its (Number-of-bullets * Damage-for-each-bullet) )
        m_flCritCostBase = m_flDamagePerShot * m_flBulletsShotDuringCrit;

        // Cap CRIT damage to bucket cap
        if (m_flCritCostBase * TF_DAMAGE_CRIT_MULTIPLIER > FeatureObj::critHack.m_flCritBucketCap)
            m_flCritCostBase = FeatureObj::critHack.m_flCritBucketCap / TF_DAMAGE_CRIT_MULTIPLIER;
    }

    // reset bucket
    m_flCritBucket  = FeatureObj::critHack.m_flCritBucketDefault;
    m_nCritRequests = 0;

    FAIL_LOG("Resetted weapon stats");
    return true;
}

void WeaponCritData_t::IncrementCritRequestCount()
{
    // Just increase the count for NON-rapid fire weapons
    if (m_bIsRapidFire == false)
    {
        ++m_nCritRequests;
        return;
    }

    // If a crit is already going ON & accouted for, then don't add
    if (tfObject.pGlobalVar->curtime < m_flLastCritRequestIncTime + TF_DAMAGE_CRIT_DURATION_RAPID)
        return; 
    
    // else Add, and record current time.
    ++m_nCritRequests;
    m_flLastCritRequestIncTime = tfObject.pGlobalVar->curtime;
    WIN_LOG("Incremented Rapid-Fire Crit-Request count @ [%.4f]", m_flLastCritRequestIncTime);
}

void WeaponCritData_t::Reset()
{
    // Resetting Weapon's Stats
    m_pWeapon         = nullptr;
    m_flDamagePerShot = 0.0f;
    m_iWeaponID       = 0;
    m_iWeaponEntIdx   = 0;
    m_bIsRapidFire    = false;
    m_iSlot           = WPN_SLOT_INVALID;
    m_flCritCostBase  = 0.0f;
    m_pWeaponInfo     = nullptr;
    m_flBulletsShotDuringCrit  = 0.0f;
    m_flLastWithdrawlTime      = 0.0f;
    m_flLastCritRequestIncTime = 0.0f;

    // Resetting Weapon's Crit Bucket...
    m_flCritBucket    = 0.0f;
    m_nCritRequests   = 0;
}