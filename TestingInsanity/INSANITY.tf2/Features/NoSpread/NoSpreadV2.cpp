#include "NoSpreadV2.h"

#include <bit>
#include <regex>
#include <numeric>

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
#define DEBUG_NOSPREAD true

vec vEyePos;

void NoSpreadV2_t::Run(BaseEntity* pLocalPlayer, baseWeapon* pActiveWeapon, CUserCmd* pCmd, bool* pCreateMoveResult, bool* pSendPacket)
{
    vEyePos = pLocalPlayer->GetEyePos();

    if (Features::NoSpreadV2::NoSpread::NoSpread.IsActive() == false)
        return;


    _CreateTimeStamps(pSendPacket);
    _Draw();

    // DO NOT go futher without syncing time with server, else bad things will happen ( vote kick )
    if (m_bSynced == false)
        return;

    bool bShooting = SDK::CanAttack(pLocalPlayer, pActiveWeapon) == true && (pCmd->buttons & IN_ATTACK);
    if (bShooting == false)
        return;
    
    _FixSpread(pLocalPlayer, pActiveWeapon, pCmd, pCreateMoveResult);
}

void NoSpreadV2_t::Reset()
{
    m_bPerfRequestPending  = false;
    m_dServerTimeDelta     = 0.0;
    m_dRequestSendTime     = 0.0;
    m_dOutgoingLatency     = 0.0;
    m_dCorrectionOffset    = 0.0;
    m_bSynced              = false;
    
    m_qTimeDeltaSamples.clear();
    m_qTimeStamps.clear();
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
    for (int iBullet = 0; iBullet < nBullets; iBullet++, iSeed++)
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


    if (vCorrectedAngles.isEmpty() == true)
        return;

    if(nBullets == 1)
    {
        Maths::VectorAnglesFromSDK(vCorrectedAngles, pCmd->viewangles);
        Maths::WrapYaw(pCmd->viewangles);

        *pCreateMoveResult = false;
        printf("[ CLIENT ] ( %d ) %.2f %.2f %.2f\n", iSeed, vCorrectedAngles.x, vCorrectedAngles.y, vCorrectedAngles.z);
    }
}


void NoSpreadV2_t::_Draw()
{
    float flTime             = ExportFn::Plat_FloatTime() + m_dServerTimeDelta;
    float flLeastCount       = std::nextafter(flTime, std::numeric_limits<float>::infinity()) - flTime;
    float flMantissaStepSize = _GetMantissaStep(ExportFn::Plat_FloatTime() + m_dServerTimeDelta);

    Render::InfoWindow.AddToCenterConsole("LeastCount",  std::format("STEP-SIZE : {} | LC : {:.3}", flMantissaStepSize, flLeastCount), flMantissaStepSize > 1.0f ? GREEN : RED);
    Render::InfoWindow.AddToCenterConsole("SYNC_Status", std::format("Sync : {}", m_bSynced == true ? "SYNCED" : "not-synced"), m_bSynced == true ? GREEN : RED);

    float flServerUpTime = ExportFn::Plat_FloatTime() + m_dServerTimeDelta + m_dCorrectionOffset;
    Render::InfoWindow.AddToCenterConsole("UpTime", 
        std::format("UpTime {}D {}h", static_cast<int>(flServerUpTime / (3600.0f * 24.0f)), static_cast<int>(flServerUpTime / 3600.0f)));
}


int NoSpreadV2_t::_GetSeed(CUserCmd* pCmd) const
{
    if (CVars::sv_usercmd_custom_random_seed == 0)
        return Sig::MD5_PseudoRandom(pCmd->command_number) & 0xFF;

    float flTime = static_cast<float>(ExportFn::Plat_FloatTime() + m_dServerTimeDelta + m_dCorrectionOffset + I::iEngineClientReplay->GetCurrentLatency(FLOW_OUTGOING));

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
    // Dumping PlayerPerf once we have created timestamps.
    static bool bDumpedPlayerPerf = false;
    if (m_qTimeStamps.size() >= MAX_TIME_STAMP_COUNT && bDumpedPlayerPerf == false)
    {
        WIN_LOG("/////////////////  Dumped Player Perf  /////////////////");
        
        Sig::CBaseClientState_SendStringCmd(I::cClientState, "playerperf\n");
        bDumpedPlayerPerf = true;
    }
    else
    {
        bDumpedPlayerPerf = false;
    }

    // Make sure no other features are messing with TickBase or bSendPacket / chocking commands.
    if (_ShouldCreateTimeStamps() == false)
        return;

    // NOTE : Go ahead & chock every other tick & note down time when releasing each choke. 
    // We can see those chocked ticks in PlayerPerf dump. from there we can match them & sync time.
    constexpr int TIME_STAMP_FREQUENCY = 2, CHOCK_SIZE = 1;

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


void NoSpreadV2_t::_AskPlayerPerf()
{
    if (I::iEngineClientReplay->GetNetChannel()->IsLoopback() == true)
    {
        m_dServerTimeDelta = 0.0;
        return;
    }

    if (m_bPerfRequestPending == true)
        return;

    // NOTE : The string command is send immediately & not at the end of CL_Move or anything. 
    //        Hence we are recording time & ping here.
    Sig::CBaseClientState_SendStringCmd(I::cClientState, "playerperf\n");
    m_dRequestSendTime    = ExportFn::Plat_FloatTime();
    m_dOutgoingLatency    = I::iEngineClientReplay->GetAvgLatency(FLOW_OUTGOING);
    m_bPerfRequestPending = true;
}


//-------------------------------------------------------------------------
// NOTE : It takes around 8 to 20 ticks of time to recieve the "PlayerPerf" message
// from server.
//-------------------------------------------------------------------------
bool NoSpreadV2_t::ExtractTime(std::string& szPlayerPerf)
{
    if (m_bPerfRequestPending == false)
        return false;

    double dOutgoingLatency = m_dOutgoingLatency;
    double dRequestSendTime = m_dRequestSendTime;
    m_bPerfRequestPending   = false;

    // Sanity checks.
    if (szPlayerPerf.empty() == true)
        return false;

    // This is necessary.
    if (szPlayerPerf[0] == 2)
        szPlayerPerf.erase(0, 1);


    std::smatch match;
    std::regex playerPerfTemplate(R"((\d+.\d+)\s(\d+)\s\d+\s\d+.\d+\s\d+.\d+\svel\s\d+.\d+)");
    if (std::regex_match(szPlayerPerf, match, playerPerfTemplate) == false)
        return true;
    
    
    // Only process the latest time.
    static double flLastParsedTime = 0.0f;
    double flTime = std::stod(match[1].str());

    if (flTime < flLastParsedTime)
        return true;
    flLastParsedTime = flTime;

    double flLeastCount = static_cast<double>(_GetLeastCount(static_cast<float>(flTime)));

    // calulating difference & rounding to least count.
    // NOTE : We have removed the latency, assuming that would remove any random spikes due to network delay.
    //        Now I am expecting "dServerClientDelta" to hold the difference in time between the clinet and the server at
    //        this very moment.
    double dServerClientDelta = flTime - dRequestSendTime - dOutgoingLatency;
    dServerClientDelta        = std::round(dServerClientDelta / flLeastCount) * static_cast<double>(flLeastCount);

    // add to sample list & remove extra samples.
    m_qTimeDeltaSamples.push_back(dServerClientDelta);
    if (m_qTimeDeltaSamples.size() > MAX_TIME_DELTA_SAMPLE)
        m_qTimeDeltaSamples.pop_front();

    // get the avg of all samples & round it.
    double dAvgDeltaTime = std::accumulate(m_qTimeDeltaSamples.begin(), m_qTimeDeltaSamples.end(), 0.0) / static_cast<double>(m_qTimeDeltaSamples.size());
    dAvgDeltaTime        = std::round(dAvgDeltaTime / flLeastCount) * static_cast<double>(flLeastCount);

    // Checking sync
    if(m_qTimeDeltaSamples.size() >= MAX_TIME_DELTA_SAMPLE && dAvgDeltaTime > 0.0)
    {
        double dPredictedServerTime = dRequestSendTime + dAvgDeltaTime + m_dCorrectionOffset + dOutgoingLatency;
        
        // Rounding both times to smallest possible change. 
        dPredictedServerTime = std::round(dPredictedServerTime / flLeastCount) * static_cast<double>(flLeastCount);
        flTime               = std::round(flTime               / flLeastCount) * static_cast<double>(flLeastCount);

        m_bSynced = fabs(flTime - dPredictedServerTime) < flLeastCount * 2.0f;
        
        if (m_bSynced == false)
        {
            // Storing new correction offset.
            m_dCorrectionOffset += flTime - dPredictedServerTime;
            printf("Correction offset : %.5f , i.e. %.2f * LC\n", m_dCorrectionOffset, m_dCorrectionOffset / flLeastCount);
        }
    }

    // Storing the correction times
    m_dServerTimeDelta  = dAvgDeltaTime;

    return true;
}





//=========================================================================
//                     DUBUGGING HOOKS
//=========================================================================
#if defined(DEBUG_NOSPREAD)

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
    LOG("[ SERVER ] Bullet Dir : %.2f %.2f %.2f\n", pBulletInfo->m_vecDirShooting.x, pBulletInfo->m_vecDirShooting.y, pBulletInfo->m_vecDirShooting.z);

    I::IDebugOverlay->AddLineOverlay(vEyePos, vEyePos + (pBulletInfo->m_vecDirShooting * 500.0f), 255, 255, 255, true, 5.0f);

    Hook::CTFPlayer_FireBullet::O_CTFPlayer_FireBullet(pPlayer, pWeapon, pBulletInfo, a4, a5, a6);
}


MAKE_HOOK(Server_FX_FireBullet, "48 89 5C 24 ? 4C 89 4C 24 ? 55 56 41 54", __fastcall, SERVER_DLL, void*,
    baseWeapon* pWeapon, int iPlayer, vec* vecOrigin, qangle* vecAngles, int iWeapon, int iMode, int iSeed, float flSpread, float flDamage, bool bCritical)
{
    LOG("[ SERVER ] Seed : %d", iSeed);

    return Hook::Server_FX_FireBullet::O_Server_FX_FireBullet(pWeapon, iPlayer, vecOrigin, vecAngles, iWeapon, iMode, iSeed, flSpread, flDamage, bCritical);
}


//MAKE_HOOK(PlatFloatTime, "48 83 EC ? 80 3D ? ? ? ? ? 75 ? E8 ? ? ? ? 80 3D ? ? ? ? ? 74 ? F2 0F 10 05 ? ? ? ? F2 0F 58 05 ? ? ? ? F2 0F 11 05 ? ? ? ? 48 83 C4",
//    __stdcall, TIER0_DLL, double)
//{
//    double result = Hook::PlatFloatTime::O_PlatFloatTime();
//    return result + static_cast<double>(Features::NoSpreadV2::NoSpread::LoopBack_Time_Offset.GetData().m_flVal);
//}


#endif