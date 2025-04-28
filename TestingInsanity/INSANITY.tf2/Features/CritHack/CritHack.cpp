#include "CritHack.h"

// UTILITY
#include "../../Utility/signatures.h"
#include "../../Utility/ExportFnHelper.h"
#include "../../Utility/ConsoleLogging.h"

// SDK
#include "../../SDK/class/CUserCmd.h"
#include "../../SDK/class/BaseWeapon.h"
#include "../../SDK/class/Source Entity.h"
#include "../../SDK/class/FileWeaponInfo.h"
#include "../../SDK/class/CVar.h"

// DEUBG related
#include "../ImGui/InfoWindow/InfoWindow_t.h"

#define TF_DAMAGE_CRIT_CHANCE_MELEE 0.15f
#define WEAPON_RANDOM_RANGE 10000

GET_EXPORT_FN(RandomSeed, VSTDLIB_DLL, void, int)
GET_EXPORT_FN(RandomInt, VSTDLIB_DLL, int, int, int)

MAKE_SIG(CBaseEntity_SetPredictionRandomSeed, "48 85 C9 75 ? C7 05 ? ? ? ? ? ? ? ? C3", "client.dll", int64_t, CUserCmd*);
MAKE_SIG(MD5_PseudoRandom, "89 4C 24 ? 55 48 8B EC 48 81 EC", "client.dll", int64_t, int);
MAKE_SIG(CTFWeaponBase_CalcIsAttackCritHelper, "48 89 5C 24 ? 55 56 57 48 81 EC ? ? ? ? 0F 29 74 24", "client.dll", bool, baseWeapon*);
MAKE_SIG(CTFWeaponBaseMelee_CalcIsAttackCritHelper, "40 57 48 83 EC ? 48 8B 05 ? ? ? ? 48 8B F9 83 78 ? ? 75", "client.dll", bool, baseWeapon*);
MAKE_SIG(IsAllowedToWithdrawFromCritBucket, "40 53 48 83 EC ? FF 81", "client.dll", bool, baseWeapon*, float);

// delete this
MAKE_SIG(ATRIB_HOOK_FLOAT, "4C 8B DC 49 89 5B ? 49 89 6B ? 56 57 41 54 41 56 41 57 48 83 EC ? 48 8B 3D ? ? ? ? 4C 8D 35",
    CLIENT_DLL, float, float , const char* , void* , void* , bool );


//======================= Debug Macros =======================
//#define DEBUG_CRITHACK_CVAR
//#define DEBUG_CRIT_COMMAND


//=========================================================================
//                     PUBLIC METHODS
//=========================================================================
void CritHack_t::Run(CUserCmd* pCmd, baseWeapon* pActiveWeapon, BaseEntity* pLocalPlayer)
{
    if (Feature::CritHack == false)
        return;

    static slot_t iLastWeaponSlot;

    if (iLastWeaponSlot != pActiveWeapon->getSlot())
    {
        m_qCritCommands.clear();
    #if defined(DEBUG_CRIT_COMMAND)
        FAIL_LOG("Weapon Changed, cleared log!");
    #endif
        iLastWeaponSlot = pActiveWeapon->getSlot();
    }

    Render::InfoWindow.AddToInfoWindow("bucket", std::format("Bucket : {:.2f}", pActiveWeapon->GetCritBucket()));
    Render::InfoWindow.AddToInfoWindow("player crit chance", std::format("player crit chance : {}", static_cast<float>(pLocalPlayer->GetCritMult()) * TF_DAMAGE_CRIT_CHANCE_MELEE));
    
    Render::InfoWindow.AddToInfoWindow("local player", std::format("local player : {}", pLocalPlayer->entindex()));
    Render::InfoWindow.AddToInfoWindow("active weapon", std::format("active weapon : {}", pActiveWeapon->entindex()));

    // setup
    _InitializeCVars();
    _ScanForCritCommands(pCmd, pActiveWeapon, pLocalPlayer);
    int iBestCritCommand = _GetBestCritCommand(pCmd);

    static int hitTick = 0;

    if (!(pCmd->buttons & IN_ATTACK))
    {
        hitTick = 0;
        return;
    }
    ++hitTick;

    // crit hacks
    if (pActiveWeapon->getSlot() == WPN_SLOT_MELLE && hitTick == 1)
        _MeleeCritHack(iBestCritCommand, pCmd, pActiveWeapon, pLocalPlayer);
    
}


void CritHack_t::Reset()
{
    m_bCVarsInitialized = false;

    // this is useless, cause if m_bCVarsInitialized is false, we are gonna update them anyways.
    // I still did, cause this doesn't affest performance much :|
    m_flCritBucketBottom  = 0.0f;
    m_flCritBucketCap     = 0.0f;
    m_flCritBucketDefault = 0.0f;
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

    m_bCVarsInitialized = true;

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

        //int nFutureSeed = Sig::MD5_PseudoRandom(pCmd->command_number + i) & 0xFF;
        int nFutureSeed = Sig::MD5_PseudoRandom(pCmd->command_number + i) & 0x7FFFFFFF;
        
        if (_isSeedCritMelee(nFutureSeed, pActiveWeapon, pLocalPlayer, false) == true)
        {
            m_qCritCommands.push_back(pCmd->command_number + i);

        #if defined(DEBUG_CRIT_COMMAND)
            WIN_LOG("Found crit command @ [ %d ] seed [ %d ]", pCmd->command_number + i, nFutureSeed);
        #endif
        }
    } 

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


void CritHack_t::_MeleeCritHack(int iCritCommand, CUserCmd* pCmd, baseWeapon* pActiveWeapon, BaseEntity* pLocalPlayer)
{
#if defined(DEBUG_CRIT_COMMAND)
    printf("shooting using command num [ %d ] & seed [ %d ] | cur -> %d & %d\n", iCritCommand, Sig::MD5_PseudoRandom(iCritCommand) & 0xFF,
        pCmd->command_number, pCmd->random_seed);
#endif
    pCmd->command_number = iCritCommand;
    //pCmd->random_seed = Sig::MD5_PseudoRandom(iCritCommand) & 0xFF;
    pCmd->random_seed = Sig::MD5_PseudoRandom(iCritCommand) & 0x7FFFFFFF;
    Sig::CBaseEntity_SetPredictionRandomSeed(pCmd);
    WIN_LOG("rebuild check : %s", _isSeedCritMelee(pCmd->random_seed, pActiveWeapon, pLocalPlayer, true) ? "TRUE" : "FALSE");
}


bool CritHack_t::_isSeedCritMelee(int iSeed, baseWeapon* pActiveWeapon, BaseEntity* pLocalPlayer, bool debug)
{
    // Damage calculation
    //const WeaponData_t& pWeaponData = pActiveWeapon->GetTFWeaponInfo()->GetWeaponData(0);
    //float flDamage = pWeaponData.m_nDamage;
    //flDamage = Sig::ATRIB_HOOK_FLOAT(flDamage, "mult_dmg", static_cast<BaseEntity*>(pActiveWeapon), 0, true);

    //flDamage *= pWeaponData.m_nBulletsPerShot; <- since we are making it for melee weapons, this will zero out the damage

    // Calculaing Crit Chance
    float flCritChance = static_cast<float>(pLocalPlayer->GetCritMult()) * TF_DAMAGE_CRIT_CHANCE_MELEE;
    flCritChance = Sig::ATRIB_HOOK_FLOAT(flCritChance, "mult_crit_chance", static_cast<BaseEntity*>(pActiveWeapon), 0, true);

    if (debug)
        printf("flCritChance : %.6f\n", flCritChance);

    // Setting seed
    int iMask = (pActiveWeapon->entindex() << 16) | (pLocalPlayer->entindex() << 8);
    iSeed = iSeed ^ iMask;
    ExportFn::RandomSeed(iSeed);

    int randomInt = ExportFn::RandomInt(0, WEAPON_RANDOM_RANGE - 1);
    bool bCrit = randomInt < static_cast<int>(flCritChance * static_cast<float>(WEAPON_RANDOM_RANGE));

    if(debug)
        printf("randomint : %d & our chance : %d\n", randomInt, static_cast<int>(flCritChance * static_cast<float>(WEAPON_RANDOM_RANGE)));

    return bCrit;
}