#include "CritHack.h"

// UTILITY
#include "../../Utility/signatures.h"
#include "../../Utility/ExportFnHelper.h"
#include "../../Utility/ConsoleLogging.h"
#include "../../Utility/Interface.h"
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

// DEUBG related
#include "../ImGui/InfoWindow/InfoWindow_t.h"

#define TF_DAMAGE_CRIT_CHANCE_MELEE     0.15f
#define TF_DAMAGE_CRIT_CHANCE           0.02f
#define TF_DAMAGE_CRIT_MULTIPLIER       3.0f
#define TF_DAMAGE_CRIT_CHANCE_RAPID	    0.02f
#define TF_DAMAGE_CRIT_DURATION_RAPID   2.0f
#define WEAPON_RANDOM_RANGE 10000

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


//=========================================================================
//                     PUBLIC METHODS
//=========================================================================
void CritHack_t::Run(CUserCmd* pCmd, baseWeapon* pActiveWeapon, BaseEntity* pLocalPlayer)
{
    m_pLocalPlayer         = pLocalPlayer;
    m_iLocalPlayerEntIndex = pLocalPlayer->entindex();

    if (Feature::CritHack == false)
        return;

    // Getting CVars done first.
    _InitializeCVars();

    // Early exiting crit hack
    if (!GetAsyncKeyState(VK_LSHIFT) && // if user is not holding crithack key
        (pCmd->buttons & IN_ATTACK) &&  // trying to shoot
        (tfObject.pGlobalVar->curtime > pActiveWeapon->GetNextPrimaryAttackTime())) // and can shot
    {
        _AvoidCrit(pCmd, pActiveWeapon, pLocalPlayer);
        return;
    }

    // If Crit Boosted, return
    if (Sig::mShared_IsCritBoosted(reinterpret_cast<uintptr_t>(pLocalPlayer) + netvar.m_Shared) == true)
        return;

    // Get Current weapons crit data
    WeaponCritData_t* pWeaponCritData = GetWeaponCritData(pActiveWeapon);
    if (pWeaponCritData == nullptr)
        return;
    // Reset crit data if Weapon changed
    pWeaponCritData->UpdateStats(pActiveWeapon);

    // Debugging
    Render::InfoWindow.AddToInfoWindow("crit commands",   std::format("Valid Crits commands left : {}", m_qCritCommands.size()));
    Render::InfoWindow.AddToInfoWindow("weaponValidiy",   std::format("{}", pWeaponCritData == nullptr ? "IN VALID WEAPON" : "weapon pointer seems valid"));
    Render::InfoWindow.AddToInfoWindow("bucket size",     std::format("bucket : {:.2f}", pWeaponCritData->m_flCritBucket));
    Render::InfoWindow.AddToInfoWindow("gameBucket",      std::format("GAME's bucket : {:.2f}", pActiveWeapon->GetCritBucket()));
    Render::InfoWindow.AddToInfoWindow("Crit requests",   std::format("Crit requests : {}", pWeaponCritData->m_nCritRequests));
    Render::InfoWindow.AddToInfoWindow("Games Crit requests",   std::format("Games Crit requests : {}", pActiveWeapon->GetTotalCritsOccured()));
    Render::InfoWindow.AddToInfoWindow("damage",          std::format("m_flDamage : {}", pWeaponCritData->m_flDamage));                                       
    Render::InfoWindow.AddToInfoWindow("damageDealt",     std::format("total damage dealt : {}", m_iTotalDamage));
    Render::InfoWindow.AddToInfoWindow("CritdamageDealt", std::format("total Crit damage : {}", m_iRangedCritDamage));

    // More Debugging... :)
    float flMaxBucketSize  = m_flCritBucketCap - m_flCritBucketBottom;
    float flCurBucketSize  = pWeaponCritData->m_flCritBucket - m_flCritBucketBottom;
    float flCritMultiplier = pActiveWeapon->getSlot() == WPN_SLOT_MELLE ? 1.5f : 3.0f;
    int iMaxCrits          = static_cast<int>(flMaxBucketSize / (pWeaponCritData->m_flDamage * flCritMultiplier));
    int iCritsLeft         = static_cast<int>(flCurBucketSize / (pWeaponCritData->m_flDamage * flCritMultiplier));
    Render::InfoWindow.AddToCenterConsole("crit left", std::format("{} / {}", iCritsLeft, iMaxCrits));

    // Adjusting weapon's crit bucket
    RoundStats_t* pStats = pLocalPlayer->GetPlayerRoundData();
    if (pStats != nullptr && m_pLastCritWeapon != nullptr && pStats->m_iCrits != m_nOldCritCount)
    {
        m_pLastCritWeapon->WithDrawlFromCritBucket();
        m_nOldCritCount = pStats->m_iCrits;
        printf("---> we detected a crit, we deducted the crit <---\n");
    }

    // can CRIT ?
    bool bAreCritsAllowed = _CanFireCriticalShot(pLocalPlayer, pActiveWeapon);
    Render::InfoWindow.AddToInfoWindow("critban", bAreCritsAllowed ? "crits allowed" : "CRIT BANNED");
    
    // pretty lookin ass crit bucket imformation :)
    m_qCritCommands.size() == 0 ?
        Render::InfoWindow.AddToCenterConsole("critBan", "NOT-POSSIBLE", YELLOW) :
        bAreCritsAllowed ?
            Render::InfoWindow.AddToCenterConsole("critBan", "CRIT BAN", RED) :
            Render::InfoWindow.AddToCenterConsole("critBan", "crits_allowed", GREEN);

    if (bAreCritsAllowed == false)
        return;

    // setup
    _ScanForCritCommandsV2(pCmd, pActiveWeapon, pLocalPlayer);
    int iBestCritCommand = _GetBestCritCommand(pCmd); // <-- This keep refreshing the crit command array by removing the expired ones :)

    // if not attacking, then no crit
    if (!(pCmd->buttons & IN_ATTACK))
        return;

    float flNextPrimaryAttackTime = pActiveWeapon->GetNextPrimaryAttackTime();
    if (tfObject.pGlobalVar->curtime >= flNextPrimaryAttackTime && flNextPrimaryAttackTime > m_flLastCritHackTime)
    {
        *reinterpret_cast<float*>(reinterpret_cast<uintptr_t>(pLocalPlayer) + netvar.m_Shared + netvar.m_iCritMult) = 0.0f;
        _ForceCrit(iBestCritCommand, pCmd, pActiveWeapon, pLocalPlayer, pWeaponCritData);
    }
}


void CritHack_t::HandleEvent(IGameEvent* pEvent)
{
    // if just spawned in the game, then it will be nullptr, then we don't save damage dealt ( sometimes it store bullshit damage as soon as we join )
    if (m_pLocalPlayer == nullptr)
        return;

    // if not ALIVE then no damage storing
    if (m_pLocalPlayer->getLifeState() != lifeState_t::LIFE_ALIVE)
        return;

    // Skip if not local player
    if (m_iLocalPlayerEntIndex > 0 && I::iEngine->GetPlayerForUserID(pEvent->GetInt("attacker")) != m_iLocalPlayerEntIndex)
        return;

    // damage dealt by us
    auto iDamage = pEvent->GetInt("damageamount");
    
    // total damage dealt till this point
    m_iTotalDamage += iDamage;
    
    // if this attack was crit
    if (pEvent->GetBool("crit"))
        m_iRangedCritDamage += iDamage;
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
    m_bCVarsInitialized = false;

    // this is useless, cause if m_bCVarsInitialized is false, we are gonna update them anyways.
    // I still did, cause this doesn't affest performance much :|
    m_flCritBucketBottom  = 0.0f;
    m_flCritBucketCap     = 0.0f;
    m_flCritBucketDefault = 0.0f;

    // Reseting Weapon's crit data
    m_PrimaryCritData.Reset();
    m_SecondaryCritData.Reset();
    m_MeleeCritData.Reset();

    // Clearing Dealt Damage records
    m_iTotalDamage          = 0;
    m_iRangedCritDamage     = 0;
    m_iLocalPlayerEntIndex  = 0;
    m_pLastCritWeapon       = nullptr;

    m_iLastCheckSeed        = 0;
    m_pLocalPlayer          = nullptr;
}

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


void CritHack_t::_ScanForCritCommands(CUserCmd* pCmd, baseWeapon* pActiveWeapon, BaseEntity* pLocalPlayer)
{
    for (int i = 0; i < MAX_CRIT_COMMANDS; i++)
    {
        if (m_qCritCommands.size() >= MAX_CRIT_COMMANDS)
            break;

        int nFutureSeed = Sig::MD5_PseudoRandom(pCmd->command_number + i) & 0x7FFFFFFF;
        
        if (_IsCommandCritRapidFire(nFutureSeed, pActiveWeapon, pLocalPlayer) == true)
        {
            m_qCritCommands.push_back(pCmd->command_number + i);

        #if defined(DEBUG_CRIT_COMMAND)
            WIN_LOG("Found crit command @ [ %d ] seed [ %d ]", pCmd->command_number + i, nFutureSeed);
        #endif
        }
    } 

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

    // Does crit bucket allow ?
    if (_CanWithdrawlCrit(pActiveWeapon, pWeaponCritData) == false)
    {
        FAIL_LOG("Not enough balance in crit bucket");
        return;
    }

    int iOriginalCommand = pCmd->command_number;

    pCmd->command_number = iCritCommand;
    pCmd->random_seed    = Sig::MD5_PseudoRandom(iCritCommand) & 0x7FFFFFFF;

    // Setting prediction seed to what we want
    *I::p_iPredictionSeed = pCmd->random_seed;
    m_flLastCritHackTime  = tfObject.pGlobalVar->curtime;
    m_nOldCritCount       = pLocalPlayer->GetPlayerRoundData()->m_iCrits;
    m_pLastCritWeapon     = pWeaponCritData;
    
}


bool CritHack_t::_IsCritShotPossible(BaseEntity* pLocalPlayer, baseWeapon* pActiveWeapon, WeaponCritData_t* pWeaponCritData)
{
    bool bWaitingForCritStream = tfObject.pGlobalVar->curtime < pActiveWeapon->GetLastRapidFireCritCheckTime() + 1.0f;
    if (bWaitingForCritStream == true)
    {
        FAIL_LOG("Waiting for crit stream");
        return false;
    }

    // if Crit Bucket says yes &
    // if Player cirt ratio says yes.
    return (_CanFireCriticalShot(pLocalPlayer, pActiveWeapon) && _CanWithdrawlCrit(pActiveWeapon, pWeaponCritData));
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
        FAIL_LOG("observed cirt chance is [ %.4f ] < [ %.4f ]", flObservedCritChance, flCritChance);
        return false;
    }

    // else, "shoot floor, big score :)"
    return true;
}


bool CritHack_t::_CanWithdrawlCrit(baseWeapon* pActiveWeapon, WeaponCritData_t* pWeaponCritData) const
{
    // Crit multiplier, depending if Melee or not
    float flMult = (pActiveWeapon->getSlot() == WPN_SLOT_MELLE) ?
        0.5f : 1.0f;

    // if bucket has enough balance
    float flCost = (pWeaponCritData->m_flDamage * TF_DAMAGE_CRIT_MULTIPLIER) * flMult;
    if (pWeaponCritData->m_flCritBucket - flCost < m_flCritBucketBottom)
        return false;

    return true;
}


//=========================================================================
//                     WEAPON CRIT DATA implementation
//=========================================================================
void WeaponCritData_t::AddToCritBucket()
{
    if (m_flDamage <= 0.0f)
    {
        FAIL_LOG("damage is set to %.2f, not initialized!!", m_flDamage);
        return;
    }

    // adding to crit bucket, and capping it.
    m_flCritBucket += m_flDamage;
    if (m_flCritBucket >= Features::critHack.m_flCritBucketCap)
        m_flCritBucket = Features::critHack.m_flCritBucketCap;
}


void WeaponCritData_t::WithDrawlFromCritBucket()
{
    // add this crit request. ( assuming that this actually give us crit. )
    m_nCritRequests++;

    // calculating cost
    float flCritMult = m_pWeapon->getSlot() == WPN_SLOT_MELLE ? 0.5f : 1.0f;
    float flCritCost = (m_flDamage * TF_DAMAGE_CRIT_MULTIPLIER) * flCritMult;

    // is COST valid ?
    if (flCritCost <= 0.0f)
    {
        FAIL_LOG("crit cost [ %.2f ] was bad :(", flCritCost);
        return;
    }
    WIN_LOG("Deducted cost [ %.2f ] from crit bucket | new bucket [ %.2f ]", flCritCost, m_flCritBucket - flCritCost);

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
    m_pWeapon   = pWeapon;
    m_iWeaponID = pWeapon->GetWeaponID();
    m_flDamage  = pWeapon->GetDamagePerShot();

    // reset bucket
    m_flCritBucket  = Features::critHack.m_flCritBucketDefault;
    m_nCritRequests = 0;

    FAIL_LOG("Resetted weapon stats");
}

void WeaponCritData_t::Reset()
{
    m_pWeapon       = nullptr;
    m_flDamage      = 0.0f;
    m_iWeaponID     = 0;
    m_flCritBucket  = 0.0f;
    m_nCritRequests = 0;
}