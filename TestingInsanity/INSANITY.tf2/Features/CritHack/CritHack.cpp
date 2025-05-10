#include "CritHack.h"

// UTILITY
#include "../../Utility/signatures.h"
#include "../../Utility/ExportFnHelper.h"
#include "../../Utility/ConsoleLogging.h"
#include "../../Utility/Interface.h"
#include "../../Utility/Hook_t.h"
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

// DEUBG related
#include "../ImGui/InfoWindow/InfoWindow_t.h"

#define TF_DAMAGE_CRIT_CHANCE_MELEE     0.15f
#define TF_DAMAGE_CRIT_CHANCE           0.02f
#define TF_DAMAGE_CRIT_MULTIPLIER       3.0f
#define TF_DAMAGE_CRIT_CHANCE_RAPID	    0.02f
#define TF_DAMAGE_CRIT_DURATION_RAPID   2.0f
#define WEAPON_RANDOM_RANGE             10000

#define TF_BONUSEFFECT_NONE             4
#define TF_BONUSEFFECT_MINICRIT         1

#define MASK_SIGNED 0x7FFFFFFF

GET_EXPORT_FN(RandomSeed, VSTDLIB_DLL, void, int)
GET_EXPORT_FN(RandomInt, VSTDLIB_DLL, int, int, int)

MAKE_SIG(CBaseEntity_SetPredictionRandomSeed, "48 85 C9 75 ? C7 05 ? ? ? ? ? ? ? ? C3", "client.dll", int64_t, CUserCmd*);
MAKE_SIG(MD5_PseudoRandom, "89 4C 24 ? 55 48 8B EC 48 81 EC", "client.dll", int64_t, int);
MAKE_SIG(CTFWeaponBase_CalcIsAttackCritHelper, "48 89 5C 24 ? 55 56 57 48 81 EC ? ? ? ? 0F 29 74 24", "client.dll", bool, baseWeapon*);
MAKE_SIG(CTFWeaponBaseMelee_CalcIsAttackCritHelper, "40 57 48 83 EC ? 48 8B 05 ? ? ? ? 48 8B F9 83 78 ? ? 75", "client.dll", bool, baseWeapon*);
MAKE_SIG(IsAllowedToWithdrawFromCritBucket, "40 53 48 83 EC ? FF 81", "client.dll", bool, baseWeapon*, float);
MAKE_SIG(mShared_IsCritBoosted, "48 89 5C 24 ? 57 48 83 EC ? 48 8B D9 0F 29 7C 24", CLIENT_DLL, bool, uintptr_t);
MAKE_SIG(CTFWeaponBase_CanFireRandomCrit, "F3 0F 58 0D ? ? ? ? 0F 2F 89", CLIENT_DLL, bool, BaseEntity*, float);

// delete this
MAKE_SIG(ATRIB_HOOK_FLOAT, "4C 8B DC 49 89 5B ? 49 89 6B ? 56 57 41 54 41 56 41 57 48 83 EC ? 48 8B 3D ? ? ? ? 4C 8D 35",
    CLIENT_DLL, float, float , const char* , void* , void* , bool );

MAKE_INTERFACE_SIGNATURE(p_iPredictionSeed, "89 05 ? ? ? ? C3 CC CC CC CC CC CC 8B 02", int, CLIENT_DLL, 0x2, 0x6);

//======================= Debug Macros =======================
//#define DEBUG_CRITHACK_CVAR
//#define DEBUG_CRIT_COMMAND
#define DEGUB_CRITHACK
#define TEST_CRITHACK_FROM_SERVER


//=========================================================================
//                     PUBLIC METHODS
//=========================================================================
void CritHack_t::Run(CUserCmd* pCmd, baseWeapon* pActiveWeapon, BaseEntity* pLocalPlayer)
{
    m_pLocalPlayer         = pLocalPlayer;
    m_iLocalPlayerEntIndex = pLocalPlayer->entindex();
    m_iActiveWeaponSlot    = pActiveWeapon->getSlot();

    // Store health changes for all enimies
    Store();

    if (Feature::CritHack == false)
        return;

    // Getting CVars done first.
    _InitializeCVars();

    // Is current weapon even eligible for Crit-Hack
    if (_IsWeaponEligibleForCritHack(pLocalPlayer, pActiveWeapon) == false)
    {
        FAIL_LOG("Weapon ain't elligible");
        return;
    }

    // return if we're CritBoosted
    if (pLocalPlayer->IsCritBoosted() == true)
        return;

    // Early exiting Crit-Hack ( User doesn't want to Crit-Hack )
    if (!GetAsyncKeyState(VK_LSHIFT) && // if user is NOT holding crithack key
        (pCmd->buttons & IN_ATTACK) &&  // trying to shoot
        (tfObject.pGlobalVar->curtime > pActiveWeapon->GetNextPrimaryAttackTime())) // and can shot
    {
        _AvoidCrit(pCmd, pActiveWeapon, pLocalPlayer);
        return;
    }

    // Get Current weapons crit data
    WeaponCritData_t* pWeaponCritData = GetWeaponCritData(pActiveWeapon);
    if (pWeaponCritData == nullptr)
        return;
    // Reset crit data if Weapon changed
    pWeaponCritData->UpdateStats(pActiveWeapon);

    // Debugging ( info rendering, on top-left corner. ).
#ifdef DEGUB_CRITHACK
    // Bucket, our's & game's
    Render::InfoWindow.AddToInfoWindow("bucket size",     std::format("bucket : {:.2f}", pWeaponCritData->m_flCritBucket));
    Render::InfoWindow.AddToInfoWindow("gameBucket",      std::format("GAME's bucket : {:.2f}", pActiveWeapon->GetCritBucket()));
    // Crits Checks, our's & game's
    Render::InfoWindow.AddToInfoWindow("Crit requests",   std::format("Crit requests : {}", pWeaponCritData->m_nCritRequests));
    Render::InfoWindow.AddToInfoWindow("Games Crit requests",   std::format("Games Crit requests : {}", pActiveWeapon->GetTotalCritsOccured()));
    // Weapon Stats
    Render::InfoWindow.AddToInfoWindow("damage",          std::format("m_flDamage : {}", pWeaponCritData->m_flDamagePerShot));                                       
    Render::InfoWindow.AddToInfoWindow("crit cost base",  std::format("BaseCrit Cost : {}", pWeaponCritData->m_flCritCostBase));                                       
    // damage dealt
    Render::InfoWindow.AddToInfoWindow("damageDealt",     std::format("total damage dealt : {}", m_iTotalDamage));
    Render::InfoWindow.AddToInfoWindow("CritdamageDealt", std::format("total Crit damage : {}", m_iRangedCritDamage));
#endif

    // More Debugging... :) [ Crits left info ]
    float flMaxBucketSize  = m_flCritBucketCap - m_flCritBucketBottom;
    float flCurBucketSize  = pWeaponCritData->m_flCritBucket - m_flCritBucketBottom;
    float flCritMultiplier = pActiveWeapon->getSlot() == WPN_SLOT_MELLE ? 1.5f : 3.0f;
    int iMaxCrits          = static_cast<int>(flMaxBucketSize / (pWeaponCritData->m_flCritCostBase * flCritMultiplier));
    int iCritsLeft         = static_cast<int>(flCurBucketSize / (pWeaponCritData->m_flCritCostBase * flCritMultiplier));
    Render::InfoWindow.AddToCenterConsole("crit left", std::format("{} / {}", iCritsLeft, iMaxCrits));

    // Adjusting weapon's crit bucket
    RoundStats_t* pStats = pLocalPlayer->GetPlayerRoundData();
    if (pStats != nullptr && m_pLastCritWeapon != nullptr && pStats->m_iCrits != m_nOldCritCount && m_nOldCritCount != DEFAULT_OLD_CRIT_COUNT)
    {
        // If "Last-Crit-Request-Count" and "Cur-Crit-Request-Count" is same and total crits occured changes
        // that means this crit was Un-Intentional and we must compensate for it here.
        if (m_nLastCritRequests == m_pLastCritWeapon->m_nCritRequests)
        {
            WIN_LOG("Compensated for \"Mistake\" Crit");
            ++m_pLastCritWeapon->m_nCritRequests;
        }

        m_pLastCritWeapon->WithDrawlFromCritBucket();
        m_nOldCritCount = pStats->m_iCrits;
        printf("{[]}---> we detected a crit, we deducted the crit <---{[]}\n");
    }

    // can CRIT ?
    bool bAreCritsAllowed = _CanFireCriticalShot(pLocalPlayer, pActiveWeapon);
    Render::InfoWindow.AddToInfoWindow("critban", bAreCritsAllowed ? "crits allowed" : "CRIT BANNED");
    
    // pretty lookin ass crit bucket imformation :) ( Crit Ban related info drawing )
    m_qCritCommands.size() == 0 ?
        Render::InfoWindow.AddToCenterConsole("critBan", "NOT-POSSIBLE", YELLOW) :
        bAreCritsAllowed ?
            Render::InfoWindow.AddToCenterConsole("critBan", "crits_allowed", GREEN) :
            Render::InfoWindow.AddToCenterConsole("critBan", "CRIT BAN", RED);

    if (bAreCritsAllowed == false)
        return;

    // Clering Crit command record if we swapped weapon
    if (m_iLastWeaponID != pWeaponCritData->m_iWeaponID)
    {
        FAIL_LOG("Cleared Crit Records :|");
        _ClearCritCmdRecord();
    }
    m_iLastWeaponID = pActiveWeapon->GetWeaponID();

    // Getting crit command
    _ScanForCritCommandsV2(pCmd, pActiveWeapon, pLocalPlayer);
    int iBestCritCommand = _GetBestCritCommand(pCmd); // <-- This keep refreshing the crit command array by removing the expired ones :)

    // Checking if we have enough balance
    int iCritCostDelta = _CanWithdrawlCritV2(pWeaponCritData);
    if (iCritCostDelta > 0)
    {
        Render::InfoWindow.AddToCenterConsole("tooExpensive", std::format("Too Expensive, Deal [{}]", iCritCostDelta), RED);
        return;
    }
    Render::InfoWindow.AddToCenterConsole("tooExpensive", std::format("we rich nigga!!!"), GREEN);

    // if not attacking, then no crit
    if (!(pCmd->buttons & IN_ATTACK))
        return;

    // CRIT'in :)
    float flNextPrimaryAttackTime = pActiveWeapon->GetNextPrimaryAttackTime();
    if (tfObject.pGlobalVar->curtime >= flNextPrimaryAttackTime && flNextPrimaryAttackTime > m_flLastCritHackTime)
    {
        _ForceCrit(iBestCritCommand, pCmd, pActiveWeapon, pLocalPlayer, pWeaponCritData);
    }
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

    // Do we have damage records for this victim
    auto it = m_mapHealthRecords.find(pVictim);
    if (it == m_mapHealthRecords.end())
        return;
    
    // How much damage did he actually take ( can't give more damage then health left )
    int iDamageTaken = 0;
    
    // did we kill him?
    if (iHealth <= 0)
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
    m_bCVarsInitialized    = false;
    m_flCritBucketBottom   = 0.0f;
    m_flCritBucketCap      = 0.0f;
    m_flCritBucketDefault  = 0.0f;

    // Resetting...
    m_flLastCritHackTime   = 0.0f;
    m_iLastCheckSeed       = 0;
    m_nOldCritCount        = DEFAULT_OLD_CRIT_COUNT;
    m_iLastUsedCritSeed    = 0;
    m_iLastWeaponID        = 0;
    m_nLastCritRequests    = 0;
    m_pLastCritWeapon      = nullptr;
    m_pLocalPlayer         = nullptr;
    m_iActiveWeaponSlot    = slot_t::WPN_SLOT_INVALID;
    m_iLocalPlayerEntIndex = 0;
    m_iTotalDamage         = 0;
    m_iRangedCritDamage    = 0;

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

    printf("Total Damage : %d\n", nTotalDamage);
    printf("Crit Damage  : %d\n", nRangedCritDamage);
    
    return pPlayerStats;
}

MAKE_HOOK(Server_TFBaseWeapon_CalcIsAttackCriticalHelper, "48 89 5C 24 ? 55 56 57 41 56 41 57 48 81 EC ? ? ? ? 0F 29 74 24", __fastcall, SERVER_DLL,
    bool, baseWeapon* pActiveWeapon)
{
    // Calling Original first
    bool bCrit = Hook::Server_TFBaseWeapon_CalcIsAttackCriticalHelper::O_Server_TFBaseWeapon_CalcIsAttackCriticalHelper(pActiveWeapon);
    
    // Getting weapon stats 
    int   nTotalCritChecks  = *reinterpret_cast<int*>(reinterpret_cast<uintptr_t>(pActiveWeapon) + 0x760);
    int   nTotalCritOccured = *reinterpret_cast<int*>(reinterpret_cast<uintptr_t>(pActiveWeapon) + 0x764);
    int   nCurrentSeed      = *reinterpret_cast<int*>(reinterpret_cast<uintptr_t>(pActiveWeapon) + 0x8E0);
    float flCritbucket      = *reinterpret_cast<float*>(reinterpret_cast<uintptr_t>(pActiveWeapon) + 0x75C);
    float flObsCritChance   = *reinterpret_cast<float*>(reinterpret_cast<uintptr_t>(pActiveWeapon) + 0x9C4);

    // Debugging
    WIN_LOG("[ %s ]", bCrit ? "Crit :)" : "No-Crit :(");
    WIN_LOG("Weapon's bucket     : [ %.2f ]", flCritbucket);
    WIN_LOG("Total Crit Checks   : [ %d ]",   nTotalCritChecks);
    WIN_LOG("Total Crit Occured  : [ %d ]",   nTotalCritOccured);
    WIN_LOG("Current Weapon Seed : [ %d ]",   nCurrentSeed & MASK_SIGNED);
    WIN_LOG("Server Pred. Seed   : [ %d ]",   *I::ServerPredictionSeed);
    WIN_LOG("Updated observed crit chance : [ %.2f ]",   flObsCritChance);
    
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

#if defined(DEBUG_CRITHACK_CVAR)
    Render::InfoWindow.AddToInfoWindow("bucket bottom",  std::format("Bucket bottom {:.2f}",  m_flCritBucketBottom));
    Render::InfoWindow.AddToInfoWindow("bucket cap",     std::format("Bucket cap {:.2f}",     m_flCritBucketCap));
    Render::InfoWindow.AddToInfoWindow("bucket default", std::format("Bucket default {:.2f}", m_flCritBucketDefault));
#endif
}

bool CritHack_t::_IsWeaponEligibleForCritHack(BaseEntity* pLocalPlayer, baseWeapon* pActiveWeapon)
{
    player_class iCharChoice = pLocalPlayer->getCharacterChoice();
    slot_t       iWeaponSlot = pActiveWeapon->getSlot();

    // Skipping Sniper's primary
    if (iCharChoice == TF_SNIPER && iWeaponSlot == WPN_SLOT_PRIMARY)
        return false;

    // Skipping Spy's every weapon, except primary ( revolver )
    if (iCharChoice == TF_SPY && iWeaponSlot != WPN_SLOT_PRIMARY)
        return false;

    // Skipping any "Buff-based" secondaries, like Batalion-backup etc...
    if (pActiveWeapon->GetTFWeaponInfo()->GetWeaponData(0).m_nBulletsPerShot <= 0 && iWeaponSlot != WPN_SLOT_MELLE)
        return false;

    return true;
}


void CritHack_t::_ScanForCritCommandsV2(CUserCmd* pCmd, baseWeapon* pActiveWeapon, BaseEntity* pLocalPlayer)
{
    // Is cur weapon rapid fire
    bool bRapidFire = pActiveWeapon->GetTFWeaponInfo()->GetWeaponData(0).m_bUseRapidFireCrits;

    // Storing data to restore to
    float flOldCritBucket    = pActiveWeapon->GetCritBucket();
    int   iOldCritChecks     = pActiveWeapon->GetTotalCritChecks();
    int   iOldCritsOccured   = pActiveWeapon->GetTotalCritsOccured();
    int   iOldPredictionSeed = *I::p_iPredictionSeed;
    int   iOldSeed           = pCmd->random_seed;

    // Looping through future commands
    constexpr int CRIT_SEED_SEARCH_RANGE     = 256;
    constexpr int MAX_CRIT_SEED_SEARCH_SCOPE = 1024;
    int i       = 0;
    int iOffset = max(0, m_iLastCheckSeed - pCmd->command_number); // make sure the offset is not negative

    for (i = 0; i < CRIT_SEED_SEARCH_RANGE; i++)
    {
        // if going too much out of search range ( i.e. 1024 ticks in future )
        if (i + iOffset > MAX_CRIT_SEED_SEARCH_SCOPE)
            return;

        // don't do no more if got enough
        if (m_qCritCommands.size() >= MAX_CRIT_COMMANDS)
            break;

        // Setting prediction seed to future one
        int iFutureSeed       = pCmd->command_number + iOffset + i;
        pCmd->random_seed     = Sig::MD5_PseudoRandom(iFutureSeed) & MASK_SIGNED;
        *I::p_iPredictionSeed = pCmd->random_seed;
        
        // if this seed crits
        bool bCrit = bRapidFire ? _IsCommandCritRapidFire(pCmd->random_seed, pActiveWeapon, pLocalPlayer) : pActiveWeapon->CanCrit();
        if(bCrit)
        {
            //WIN_LOG("Found Critting seed @ %d [ PREDICTION SEED : %d ]", pCmd->random_seed, GetPredictionSeed());
            m_qCritCommands.push_back(iFutureSeed);
        }

        // resetting shit for next iteration
        pActiveWeapon->SetCritBucket(flOldCritBucket);
        pActiveWeapon->SetTotalCritChecks(iOldCritChecks);
    }

    m_iLastCheckSeed = pCmd->command_number + i;

    // Resetting back to original
    pActiveWeapon->SetCritBucket(flOldCritBucket);
    pActiveWeapon->SetTotalCritChecks(iOldCritChecks);
    pActiveWeapon->SetTotalCritsOccured(iOldCritsOccured);
    pCmd->random_seed     = iOldSeed;
    *I::p_iPredictionSeed = pCmd->random_seed;
}


void CritHack_t::_AvoidCrit(CUserCmd* pCmd, baseWeapon* pActiveWeapon, BaseEntity* pLocalPlayer)
{
    // storing original crit bucket parameters
    float flOldCritBucket    = pActiveWeapon->GetCritBucket();
    int   iOldCritChecks     = pActiveWeapon->GetTotalCritChecks();
    int   iOldCritsOccured   = pActiveWeapon->GetTotalCritsOccured();
    int   iOldPredictionSeed = *I::p_iPredictionSeed;
    int   iOldSeed = pCmd->random_seed;

    // trying to find a non-crit command number nearby
    constexpr int NONCRIT_SEARCH_RANGE = 10;
    for (int i = 0; i < NONCRIT_SEARCH_RANGE; i++)
    {
        // New ( future ) prediction seed
        *I::p_iPredictionSeed = Sig::MD5_PseudoRandom(pCmd->command_number + i) & MASK_SIGNED;
        
        // can this seed crit
        if (pActiveWeapon->CanCrit() == false)
        {
            pCmd->command_number += i;
            pCmd->random_seed    = Sig::MD5_PseudoRandom(pCmd->command_number + i) & MASK_SIGNED;
            
            if (i > 0)
                WIN_LOG("!!!!!!!!!!!!!!!!!!!!!!!!!AVOIDED A FUCKING CRIT MY NIGAAA!!!!!!!!!!!!!!!!!!!!!!!!!");

            break;
        }
        
    }

    // Resetting
    pActiveWeapon->SetCritBucket(flOldCritBucket);
    pActiveWeapon->SetTotalCritChecks(iOldCritChecks);
    pActiveWeapon->SetTotalCritsOccured(iOldCritsOccured);
    *I::p_iPredictionSeed = iOldPredictionSeed;
}


int CritHack_t::_GetBestCritCommand(CUserCmd* pCmd)
{
    for (int iCmd : m_qCritCommands)
    {
        if (iCmd < pCmd->command_number)
        {
            m_qCritCommands.pop_front();
        }
        else
        {
            m_qCritCommands.pop_front();
            return iCmd;
        }
    }
}


void CritHack_t::_ForceCrit(int iCritCommand, CUserCmd* pCmd, baseWeapon* pActiveWeapon, BaseEntity* pLocalPlayer, WeaponCritData_t* pWeaponCritData)
{
    // do we have a valid seed ?
    if (iCritCommand - pCmd->command_number < 0 || m_qCritCommands.size() == 0)
    {
        FAIL_LOG("No Valid Crit command at this point :(");
        return;
    }

    int iOriginalCommand  = pCmd->command_number;
                          
    pCmd->command_number  = iCritCommand;
    pCmd->random_seed     = Sig::MD5_PseudoRandom(iCritCommand) & 0x7FFFFFFF;

    // Setting prediction seed to what we want
    *I::p_iPredictionSeed = pCmd->random_seed;
    m_flLastCritHackTime  = tfObject.pGlobalVar->curtime;
    m_nOldCritCount       = pLocalPlayer->GetPlayerRoundData()->m_iCrits;
    m_pLastCritWeapon     = pWeaponCritData;
    m_nLastCritRequests   = pWeaponCritData->m_nCritRequests;
    ++pWeaponCritData->m_nCritRequests; // Increamenting crit request each time we change seed. Just like how the game does it

    if (pCmd->random_seed == m_iLastCheckSeed)
        FAIL_LOG("....----> REUSED LAST SEED <----....");
    else
        WIN_LOG("....----> CRITTED [ %d ] CRITTED <----....", pCmd->random_seed);

    m_iLastUsedCritSeed   = pCmd->random_seed;
}

bool CritHack_t::_IsCommandCritRapidFire(int iSeed, baseWeapon* pActiveWeapon, BaseEntity* pLocalPlayer)
{
    float flPlayerCritMult  = pLocalPlayer->GetCritMult();
    float flTotalCritChance = std::clamp(TF_DAMAGE_CRIT_CHANCE_RAPID * flPlayerCritMult, 0.01f, 0.99f);
    float flNonCritDuration = (TF_DAMAGE_CRIT_DURATION_RAPID / flTotalCritChance) - TF_DAMAGE_CRIT_DURATION_RAPID;
    float flStartCritChance = 1 / flNonCritDuration;

    flStartCritChance = Sig::ATRIB_HOOK_FLOAT(flStartCritChance, "mult_crit_chance", static_cast<BaseEntity*>(pActiveWeapon), 0, true);

    int iMask = (pActiveWeapon->entindex() << 8) | (pLocalPlayer->entindex());
    iSeed     = iSeed ^ iMask;
    ExportFn::RandomSeed(iSeed);

    return flStartCritChance * WEAPON_RANDOM_RANGE > ExportFn::RandomInt(0, WEAPON_RANDOM_RANGE - 1);
}


bool CritHack_t::_CanFireCriticalShot(BaseEntity* pLocalPlayer, baseWeapon* pActiveWeapon)
{
    // Melees don't get crit banned
    if (pActiveWeapon->getSlot() == WPN_SLOT_MELLE)
        return true;

    // getting crit chance
    float flCritChance = pLocalPlayer->GetCritMult() * TF_DAMAGE_CRIT_CHANCE;
    flCritChance       = Sig::ATRIB_HOOK_FLOAT(flCritChance, "mult_crit_chance", static_cast<BaseEntity*>(pActiveWeapon), 0, true);
    flCritChance       += 0.1f;

    float flNormalizedCritDamage = static_cast<float>(m_iRangedCritDamage) / TF_DAMAGE_CRIT_MULTIPLIER;
    float flObservedCritChance   = flNormalizedCritDamage / (flNormalizedCritDamage + static_cast<float>(m_iTotalDamage - m_iRangedCritDamage));

    // if less than observerd crit chance the no crits for us :(
    if (flObservedCritChance > flCritChance)
    {
        //FAIL_LOG("observed cirt chance is [ %.4f ] < [ %.4f ]", flObservedCritChance, flCritChance);
        return false;
    }

    // else, "shoot floor, big score :)"
    return true;
}

int CritHack_t::_CanWithdrawlCritV2(WeaponCritData_t* pWeaponCritData)
{
    float flMult = (pWeaponCritData->m_iSlot == WPN_SLOT_MELLE) ?
        0.5f :
        Maths::RemapValClamped(static_cast<float>(pWeaponCritData->m_nCritRequests + 1) / static_cast<float>(pWeaponCritData->m_pWeapon->GetTotalCritChecks()), // compensating for this check
            0.1f, 1.f, 1.f, 3.f);

    float flCost = (pWeaponCritData->m_flCritCostBase * TF_DAMAGE_CRIT_MULTIPLIER) * flMult;
    if (flCost > pWeaponCritData->m_flCritBucket) // This looks flawed, we might have enough balance but still get denied the crit.
        return static_cast<int>(flCost - pWeaponCritData->m_flCritBucket);

    return 0;
}


void CritHack_t::Store()
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
    if (m_flCritBucket >= Features::critHack.m_flCritBucketCap)
        m_flCritBucket = Features::critHack.m_flCritBucketCap;
}


void WeaponCritData_t::WithDrawlFromCritBucket()
{
    // add this crit request. ( assuming that this actually give us crit. )
    //m_nCritRequests++;

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
    if (m_flCritBucket < Features::critHack.m_flCritBucketBottom)
        m_flCritBucket = Features::critHack.m_flCritBucketBottom;
}


void WeaponCritData_t::UpdateStats(baseWeapon* pWeapon)
{
    if (m_pWeapon == nullptr)
        m_flCritBucket = Features::critHack.m_flCritBucketDefault;
    
    // if weapon same as last tick then no worry
    if (pWeapon == m_pWeapon)
        return;

    // reset stats
    m_pWeapon         = pWeapon;
    m_iWeaponID       = pWeapon->GetWeaponID();
    m_flDamagePerShot = pWeapon->GetDamagePerShot();
    m_flCritCostBase  = m_flDamagePerShot;
    m_iSlot           = pWeapon->getSlot();

    // is Weapon Rapid fire ?
    const WeaponData_t& pWeaponInfo = pWeapon->GetTFWeaponInfo()->GetWeaponData(0);
    m_bIsRapidFire                  = pWeaponInfo.m_bUseRapidFireCrits && (m_iSlot != WPN_SLOT_MELLE);
    if(m_bIsRapidFire)
    {
        // get total damage for all bullets "to-be" fired in "rapid-fire crit" duration
        m_flBulletsShotDuringCrit = TF_DAMAGE_CRIT_DURATION_RAPID / pWeaponInfo.m_flTimeFireDelay;
        
        // Base crit damage
        m_flCritCostBase = m_flDamagePerShot * m_flBulletsShotDuringCrit;

        // Cap CRIT damage to bucket cap
        if (m_flCritCostBase * TF_DAMAGE_CRIT_MULTIPLIER > Features::critHack.m_flCritBucketCap)
            m_flCritCostBase = Features::critHack.m_flCritBucketCap / TF_DAMAGE_CRIT_MULTIPLIER;
    }

    // reset bucket
    m_flCritBucket  = Features::critHack.m_flCritBucketDefault;
    m_nCritRequests = 0;

    FAIL_LOG("Resetted weapon stats");
}

void WeaponCritData_t::Reset()
{
    // Resetting Weapon's Stats
    m_pWeapon         = nullptr;
    m_flDamagePerShot = 0.0f;
    m_iWeaponID       = 0;
    m_bIsRapidFire    = false;
    m_iSlot           = WPN_SLOT_INVALID;
    m_flCritCostBase  = 0.0f;
    m_flBulletsShotDuringCrit = 0.0f;
    
    // Resetting Weapon's Crit Bucket...
    m_flCritBucket    = 0.0f;
    m_nCritRequests   = 0;
}