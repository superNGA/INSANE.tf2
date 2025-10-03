#include "NoSpreadV2.h"

#include <bit>
#include <regex>
#include <numeric>
#include <sstream>

// SDK
#include "../../SDK/class/BaseEntity.h"
#include "../../SDK/class/BaseWeapon.h"
#include "../../SDK/class/CUserCmd.h"
#include "../../SDK/class/ETFWeaponType.h"
#include "../../SDK/class/FileWeaponInfo.h"
#include "../../SDK/class/CommonFns.h"
#include "../../SDK/class/I_EngineClientReplay.h"
#include "../../SDK/class/IVDebugOverlay.h"
#include "../../SDK/class/CClientState.h"
#include "../../SDK/TF object manager/TFOjectManager.h"
#include "../../Extra/math.h"

// Other Features
#include "../Tick Shifting/TickShifting.h"

// UTILITY
#include "../../Utility/ConsoleLogging.h"
#include "../../Utility/CVar Handler/CVarHandler.h"
#include "../../Utility/Export Fn Handler/ExportFnHelper.h"
#include "../../Utility/Signature Handler/signatures.h"
#include "../../Utility/Hook Handler/Hook_t.h"
#include "../ImGui/InfoWindow/InfoWindow_t.h"


// EXPORT FNS
GET_EXPORT_FN(RandomSeed, VSTDLIB_DLL, void, int)
GET_EXPORT_FN(RandomFloat, VSTDLIB_DLL, float, float, float)
GET_EXPORT_FN_NO_ARGS(Plat_FloatTime, TIER0_DLL, double)


// FNs
MAKE_SIG(BaseWeapon_GetWeaponSpread,     "48 89 5C 24 ? 57 48 83 EC ? 4C 63 91", CLIENT_DLL, float, void*)
MAKE_SIG(CBaseClientState_SendStringCmd, "48 81 EC ? ? ? ? 48 8B 49",            ENGINE_DLL, void, CClientState*, const char*)
MAKE_SIG(MD5_PseudoRandom,               "89 4C 24 ? 55 48 8B EC 48 81 EC",      CLIENT_DLL, int64_t, int);


// Debugging hooks ( server.dll )
#define DEBUG_NOSPREAD      false
#define SPOOF_PLATFLOATTIME false

vec vEyePos;

void NoSpreadV2_t::Run(BaseEntity* pLocalPlayer, baseWeapon* pActiveWeapon, CUserCmd* pCmd, bool* pCreateMoveResult, bool* pSendPacket)
{
    vEyePos = pLocalPlayer->GetEyePos();

    if (Features::NoSpreadV2::NoSpread::NoSpread.IsActive() == false)
        return;

    _Sync(pSendPacket);
    _Draw();

    if (m_bSynced == false)
        return;

    bool bShooting = SDK::CanAttack(pLocalPlayer, pActiveWeapon) == true && (pCmd->buttons & IN_ATTACK);
    if (bShooting == false)
        return;

    _FixSpread(pLocalPlayer, pActiveWeapon, pCmd, pCreateMoveResult);
}

void NoSpreadV2_t::Reset()
{
    m_bSendTimeStamps = false;
    m_bSynced = false;
    m_dServerClientDelta = 0.0;
    m_iFailSyncCounter = 0;

    m_dPerfRequestTime = 0.0;
    m_dRequestLatency = 0.0;
    m_bPerfRequestPending = false;
    
    m_qTimeStamps.clear();
}



/*
NOTE : There are 2 types of data in playerPerf dumps. 
one like this -> "%.3f %d %d %.3f %.3f vel %.2f\n". This is the simulation info
one like this -> "%.3f %d %d\n". This is Cmd info.

In the "CBasePlayer::ProcessUsercmds" function, the prediction seed is created and in the 
same function our command info is stored which also holds the current server time as the first argument.
Hence we will be using the Cmd info & not simulation info.
*/
bool NoSpreadV2_t::ExtractTimeStamps(std::string& szPlayerPerf)
{
    // Did we even send in a player perf request ?
    if (m_bPerfRequestPending == false)
        return false;

    // Is this our time stamped request.
    if (m_bSendTimeStamps == false)
        return false;

    // Do we even got any data?
    if (m_qTimeStamps.size() == 0)
    {
        FAIL_LOG("Nigga the time stamp list is empty!!");
        return false;
    }

    //Sanity checks.
    if (szPlayerPerf.empty() == true)
        return false;

    // This is necessary.
    if (szPlayerPerf[0] == 2)
        szPlayerPerf.erase(0, 1);

    // Checking if input message is simulation info
    {
        std::smatch match;
        std::regex playerPerfTemplate(R"((\d+.\d+)\s\d+\s\d+\s\d+.\d+\s\d+.\d+\svel\s\d+.\d+)");
        if (std::regex_match(szPlayerPerf, match, playerPerfTemplate) == true)
            return true;
    }
    
    
    double dTimeStampDeltaSum = 0.0;

    std::smatch match;
    std::istringstream playerPerfStream(szPlayerPerf);
    std::string szLine;
    int  iCommandIndex          = 0; 
    bool m_bFoundFirstTimeStamp = false;
    int  iTimeStampCmdSize      = CHOCK_SIZE + 1;
    while (std::getline(playerPerfStream, szLine))
    {
        if (std::regex_match(szLine, match, std::regex(R"((\d+.\d+)\s(\d+)\s\d+)")) == false)
            return true;

        if (match.size() != 3)
            return true;
        
        // Filter out the timestamped entires.
        int    iCmdSize    = std::stoi(match[2]);
        double dServerTime = std::stod(match[1]);
        float flLeastCount = _GetLeastCount(static_cast<float>(dServerTime));
        Maths::RoundToCeil(dServerTime, flLeastCount);

        if (iCmdSize == iTimeStampCmdSize)
            m_bFoundFirstTimeStamp = true;

        if (m_bFoundFirstTimeStamp == false)
            continue;

        bool bIsTimeStamp = iCommandIndex % TIME_STAMP_FREQUENCY == 0;

        if (bIsTimeStamp == true)
        {
            if(iCmdSize != iTimeStampCmdSize)
            {
                FAIL_LOG("Bad input. Unexpected player perf");
                return true;
            }

            LOG("server [ %.6f ] client [ %.6f ] | Delta { %.6f } | LC : %.2f\n", dServerTime, m_qTimeStamps.back(), dServerTime - m_qTimeStamps.back(), flLeastCount);
            dTimeStampDeltaSum += dServerTime - m_qTimeStamps.back();
            m_qTimeStamps.pop_back();

            if (m_qTimeStamps.size() == 0)
            {
                m_bSynced            = true; m_bSendTimeStamps = false;
                m_dServerClientDelta = dTimeStampDeltaSum / static_cast<double>(MAX_TIME_STAMP_COUNT);
                return true;
            }
        }

        iCommandIndex++;
    }

    return true;
}



void NoSpreadV2_t::_FixSpread(BaseEntity* pLocalPlayer, baseWeapon* pActiveWeapon, CUserCmd* pCmd, bool* pCreateMoveResult)
{
    // WPN base spread
    float flSpread = Sig::BaseWeapon_GetWeaponSpread(pActiveWeapon);

    //  shootings angles.
    vec vForward, vRight, vUp;
    Maths::AngleVectors(pCmd->viewangles, &vForward, &vRight, &vUp);

    // Bullet per shot ( with attributes )
    int nBullets = pActiveWeapon->GetTFWeaponInfo()->GetWeaponData(0)->m_nBulletsPerShot;
    pActiveWeapon->CALL_ATRIB_HOOK_INT(nBullets, "mult_bullets_per_shot");

    vec vCorrectedAngles;

    int iSeed = _GetSeed(pCmd);
    printf("[ CLIENT ] seed : %d\n", iSeed);
    for (int iBullet = 0; iBullet < /*nBullets*/1; iBullet++, iSeed++)
    {
        // Generating random number.
        ExportFn::RandomSeed(iSeed);
        float x = ExportFn::RandomFloat(-0.5f, 0.5f) + ExportFn::RandomFloat(-0.5f, 0.5f);
        float y = ExportFn::RandomFloat(-0.5f, 0.5f) + ExportFn::RandomFloat(-0.5f, 0.5f);

        // Perfect shot?
        bool bPerfectShot = iBullet == 0 && _IsShotPerfect(pLocalPlayer, pActiveWeapon) == true;
        if (bPerfectShot == true) { x = 0.0f; y = 0.0f; }

        // Spread calc
        vec vSpreadX = vRight * (flSpread * x);
        vec vSpreadY = vUp    * (flSpread * y);
        vCorrectedAngles = vForward - vSpreadX - vSpreadY; vCorrectedAngles.NormalizeInPlace();
    }

    if (vCorrectedAngles.IsZero() == true)
        return;

    /*if(nBullets == 1)
    {
        Maths::VectorAnglesFromSDK(vCorrectedAngles, pCmd->viewangles);
        Maths::WrapYaw(pCmd->viewangles);

        *pCreateMoveResult = false;
        printf("[ CLIENT ] ( %d ) %.2f %.2f %.2f\n", iSeed, vCorrectedAngles.x, vCorrectedAngles.y, vCorrectedAngles.z);
    }*/

    Maths::VectorAnglesFromSDK(vCorrectedAngles, pCmd->viewangles);
    Maths::WrapYaw(pCmd->viewangles);

    *pCreateMoveResult = false;
    // printf("[ CLIENT ] ( %d ) %.2f %.2f %.2f\n", iSeed, vCorrectedAngles.x, vCorrectedAngles.y, vCorrectedAngles.z);
}



void NoSpreadV2_t::_Draw()
{
    float flTime             = ExportFn::Plat_FloatTime() + m_dServerClientDelta;
    float flLeastCount       = _GetLeastCount(flTime);
    float flMantissaStepSize = _GetMantissaStep(ExportFn::Plat_FloatTime() + m_dServerClientDelta);

    Render::InfoWindow.AddToCenterConsole("LeastCount", std::format("STEP-SIZE : {} | LC : {:.3}", flMantissaStepSize, flLeastCount), flMantissaStepSize > 1.0f ? GREEN : RED);
    Render::InfoWindow.AddToCenterConsole("SYNC_Status", std::format("Sync : {}", m_bSynced == true ? "SYNCED" : "not-synced"), m_bSynced == true ? GREEN : RED);
}


int NoSpreadV2_t::_GetSeed(CUserCmd* pCmd) const
{
    if (CVars::sv_usercmd_custom_random_seed == 0)
        return Sig::MD5_PseudoRandom(pCmd->command_number) & 0xFF;

    float flTime = static_cast<float>(ExportFn::Plat_FloatTime() + m_dServerClientDelta);

    return std::bit_cast<uint32_t>(flTime * 1000.0f) & 0xFF;
}


bool NoSpreadV2_t::_IsShotPerfect(BaseEntity* pLocalPlayer, baseWeapon* pActiveWeapon)
{
    int nBullets = pActiveWeapon->GetTFWeaponInfo()->GetWeaponData(0)->m_nBulletsPerShot;
    pActiveWeapon->CALL_ATRIB_HOOK_INT(nBullets, "mult_bullets_per_shot");

    float flTimeSinceLastShot = TICK_TO_TIME(pLocalPlayer->m_nTickBase()) - pActiveWeapon->m_flLastFireTime();

    if (nBullets > 1 && flTimeSinceLastShot > 0.25f)
        return true;
    if (nBullets == 1 && flTimeSinceLastShot > 1.25f)
        return true;

    return false;
}


float NoSpreadV2_t::_GetMantissaStep(float x)
{
    float flLeastCount = std::nextafter(x, std::numeric_limits<float>::infinity()) - x;
    flLeastCount      *= 1000.0f;

    return powf(2.0f, ceilf(logf(flLeastCount) / logf(2.0f)));
}

float NoSpreadV2_t::_GetLeastCount(float x)
{
    return std::nextafterf(x, std::numeric_limits<float>::infinity()) - x;
}



void NoSpreadV2_t::_Sync(bool* pSendPacket)
{
    if (m_bSynced == false)
    {
        if (_ShouldCreateTimeStamps() == true)
            _CreateTimeStamps(pSendPacket);
    }
    else
    {
        _VerifySync();
    }
}


bool NoSpreadV2_t::_ShouldCreateTimeStamps() const
{
    // Don't mess with bSendPacket while charging, else can cause undefined behaviour.
    if (F::tickShifter.GetCharge() < F::tickShifter.GetMaxCharge())
    {
        FAIL_LOG("Waiting for tick shifter to finish charging");
        return false;
    }

    if (F::tickShifter.ShiftingTicks() == true)
    {
        FAIL_LOG("Shifting ticks, can't create timestamps");
        return false;
    }

    return true;
}


void NoSpreadV2_t::_CreateTimeStamps(bool* pSendPacket)
{
    if (m_bSendTimeStamps == true)
        return;

    if (m_qTimeStamps.size() >= MAX_TIME_STAMP_COUNT)
    {
        _AskPlayerPerf();
        m_bSendTimeStamps = true;

        // Delete this
        WIN_LOG("/////////////////  Dumped Player Perf  /////////////////");
    }

    // NOTE : Go ahead & chock every other tick & note down time when releasing each choke. 
    // We can see those chocked ticks in PlayerPerf dump. from there we can match them & sync time.
    static int  iTick           = 0;
    static int  iChockTickCount = 0;
    static bool bChockActive    = false;
    static bool bStoreTime      = false;

    // Reset chock tick count
    if (bChockActive == false)
    {
        *pSendPacket    = true;
        iChockTickCount = 0;
        iTick++;
        bChockActive = iTick % TIME_STAMP_FREQUENCY == 0;

        // storing current time if we just released a choke
        if (bStoreTime == true)
        {
            LOG("Stord time : [ %.2f ]", ExportFn::Plat_FloatTime());
            m_qTimeStamps.push_back(ExportFn::Plat_FloatTime());
            bStoreTime = false;
        }
    }
    else // Chock command & notify once we have chocked enough.
    {
        *pSendPacket = false;
        iChockTickCount++;
        if (iChockTickCount >= CHOCK_SIZE)
        {
            bChockActive = false;
            bStoreTime   = true;
        }
    }

}


void NoSpreadV2_t::_VerifySync()
{
    // this is a performance thing
    static int iVerificationTick = 0;
    iVerificationTick++;

    if(iVerificationTick % 5 == 0)
        _AskPlayerPerf();
}


void NoSpreadV2_t::_AskPlayerPerf()
{
    if (m_bPerfRequestPending == true)
        return;

    // NOTE : The string command is send immediately & not at the end of CL_Move or anything. 
    //        Hence we are recording time & ping here.
    Sig::CBaseClientState_SendStringCmd(I::cClientState, "playerperf\n");
    m_dRequestLatency     = I::iEngineClientReplay->GetCurrentLatency(FLOW_OUTGOING);
    m_dPerfRequestTime    = ExportFn::Plat_FloatTime();
    m_bPerfRequestPending = true;
}


//-------------------------------------------------------------------------
// NOTE : It takes around 8 to 20 ticks of time to recieve the "PlayerPerf" message
// from server.
//-------------------------------------------------------------------------
bool NoSpreadV2_t::VerifyServerClientDelta(std::string& szPlayerPerf)
{
    if (m_bPerfRequestPending == false)
        return false;

    double dOutgoingLatency = m_dRequestLatency;
    double dRequestSendTime = m_dPerfRequestTime;

    if (m_bSynced == false)
        return true;

    // Sanity checks.
    if (szPlayerPerf.empty() == true)
        return false;

    // This is necessary.
    if (szPlayerPerf[0] == 2)
        szPlayerPerf.erase(0, 1);

    std::smatch match;
    std::istringstream stream(szPlayerPerf);
    std::string line;
    std::getline(stream, line);

    // Skip simulation info entries
    {
        std::smatch match;
        std::regex playerPerfTemplate(R"((\d+.\d+)\s\d+\s\d+\s\d+.\d+\s\d+.\d+\svel\s\d+.\d+)");
        if (std::regex_match(line, match, playerPerfTemplate) == true)
            return true;
    }
    
    if (std::regex_match(line, match, std::regex(R"((\d+.\d+)\s(\d+)\s\d+)")) == false)
        return true;
    
    m_bPerfRequestPending = false;

    // Only process the latest time.
    static double flLastParsedTime = 0.0f;
    double flTime = std::stod(match[1].str());
    if (flTime < flLastParsedTime)
        return true;
    flTime += TICK_INTERVAL;
    flLastParsedTime = flTime;

    float flLeastCount = _GetLeastCount(static_cast<float>(flTime));
    flTime = Maths::RoundToCeil(flTime, flLeastCount);

    // Checking sync
    {
        float  flLatencySpike      = m_dRequestLatency / I::iEngineClientReplay->GetAvgLatency(FLOW_OUTGOING);

        double dExpectedServerTime = m_dPerfRequestTime + m_dServerClientDelta;
        dExpectedServerTime = Maths::RoundToFloor(dExpectedServerTime, flLeastCount);

        static bool bLastSyncFailed = false;
        double dError = flTime - dExpectedServerTime;
        if (dError > flLeastCount * 2.0f)
        {
            //FAIL_LOG("Out of sync. error [ %.4f ] ping spike [ %.2f ]", dError, flLatencySpike);
            if(bLastSyncFailed == true)
                m_iFailSyncCounter++;

            if(m_iFailSyncCounter >= 3)
            {
                m_iFailSyncCounter = 0;
                m_bSynced          = false;
                bLastSyncFailed    = false;
            }
            else
            {
                bLastSyncFailed = true;
            }
        }
        else
        {
            m_iFailSyncCounter = 0;
            bLastSyncFailed = false;
            //WIN_LOG("Synced !!! [ ERROR : %.4f ]", dError);
        }
    }

    return true;
}





//=========================================================================
//                     DUBUGGING HOOKS
//=========================================================================
#if ( DEBUG_NOSPREAD == true )

struct BulletInfo_t
{
    int         m_iShots;
    vec         m_vecSrc;
    vec         m_vecDirShooting;
    vec         m_vecSpread;
    float       m_flDistance;
    int         m_iAmmoType;
    int         m_iTracerFreq;
    float       m_flDamage;
    int         m_iPlayerDamage;	// Damage to be used instead of m_flDamage if we hit a player
    int         m_nFlags;			// See FireBulletsFlags_t
    float       m_flDamageForceScale;
    BaseEntity* m_pAttacker;
    BaseEntity* m_pAdditionalIgnoreEnt;
    bool        m_bPrimaryAttack;
    bool        m_bUseServerRandomSeed;
};


MAKE_HOOK(CTFPlayer_FireBullet, "48 8B C4 44 88 48 ? 4C 89 40 ? 48 89 50", __fastcall, SERVER_DLL, void,
    BaseEntity* pPlayer, void* pWeapon, BulletInfo_t* pBulletInfo, char a4, unsigned int a5, unsigned int a6)
{
    //LOG("[ SERVER ] Bullet Dir : %.2f %.2f %.2f\n", pBulletInfo->m_vecDirShooting.x, pBulletInfo->m_vecDirShooting.y, pBulletInfo->m_vecDirShooting.z);

    I::IDebugOverlay->AddLineOverlay(vEyePos, vEyePos + (pBulletInfo->m_vecDirShooting * 500.0f), 255, 255, 255, true, 5.0f);

    Hook::CTFPlayer_FireBullet::O_CTFPlayer_FireBullet(pPlayer, pWeapon, pBulletInfo, a4, a5, a6);
}


MAKE_HOOK(Server_FX_FireBullet, "48 89 5C 24 ? 4C 89 4C 24 ? 55 56 41 54", __fastcall, SERVER_DLL, void*,
    baseWeapon* pWeapon, int iPlayer, vec* vecOrigin, qangle* vecAngles, int iWeapon, int iMode, int iSeed, float flSpread, float flDamage, bool bCritical)
{
    LOG("[ SERVER ] Seed : %d", iSeed);

    return Hook::Server_FX_FireBullet::O_Server_FX_FireBullet(pWeapon, iPlayer, vecOrigin, vecAngles, iWeapon, iMode, iSeed, flSpread, flDamage, bCritical);
}


#endif

#if ( SPOOF_PLATFLOATTIME == true )

MAKE_HOOK(PlatFloatTime, "48 83 EC ? 80 3D ? ? ? ? ? 75 ? E8 ? ? ? ? 80 3D ? ? ? ? ? 74 ? F2 0F 10 05 ? ? ? ? F2 0F 58 05 ? ? ? ? F2 0F 11 05 ? ? ? ? 48 83 C4",
    __stdcall, TIER0_DLL, double)
{
    double result = Hook::PlatFloatTime::O_PlatFloatTime();
    return result + 86400.0;
}


#endif