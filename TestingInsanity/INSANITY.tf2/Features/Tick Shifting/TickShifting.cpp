//=========================================================================
//                      TICK SHIFTER
//=========================================================================
// by      : INSANE
// created : 26/07/2025
// 
// purpose : Revinds tick base ( Used for DT, Speed HX & more )
//-------------------------------------------------------------------------
#include "TickShifting.h"

#include <algorithm>

//SDK
#include "../../SDK/class/BaseEntity.h"
#include "../../SDK/class/BaseWeapon.h"
#include "../../SDK/class/CUserCmd.h"
#include "../../SDK/TF object manager/TFOjectManager.h"
#include "../../SDK/class/CommonFns.h"
#include "../../SDK/class/CClientState.h"
#include "../../SDK/class/FileWeaponInfo.h"
#include "../ImGui/InfoWindowV2/InfoWindow.h"

#include "../TickManip/TickManipHelper.h"

// UTILITY
#include "../../Utility/Interface Handler/Interface.h"
#include "../../Utility/Signature Handler/signatures.h"
#include "../../Utility/PullFromAssembly.h"
#include "../../Utility/CVar Handler/CVarHandler.h"
#include "../../Utility/Profiler/Profiler.h"
#include "../../Utility/ConsoleLogging.h"
#include "../../Extra/math.h"


#define DEBUG_TICK_SHIFTING false

GET_RIP_ADRS_FROM_ASSEMBLY(CL_RunPrediction, void*, "E8 ? ? ? ? E8 ? ? ? ? F3 0F 10 05", ENGINE_DLL, 1, 5, 5)

/*
THEORY : 
    TF2 servers try to keep our tickbase and the server clock in sync.
    We are allowed to send up to 15 ticks at once in a packet ( originally intended for chocked ticks )
    so the server will simulate all the ticks as long as we have sufficient balance. 
        BALANCE : there is a variable in tf2 servers which keep a track of chocked ticks and we can only send in 
                    that many ticks that have been chocked. ( Forgive my english ) We can increase our balance by 
                    not sending in ticks ( make sure they are not even created, else they will be send in as chocked ticks later )
    
    So lets say we send in x ammount of ticks, the server will revind our tick base x - 1 ticks back so when its finished simulating
        all those ticks, our tick base & server's clock is in sync.
    
    In order to DoubleTap, we need to fire & dump ticks at the same time ( frame ) so that server revinds our tickbase before simulating
        our tick where we fired our shot. So technically we shot in the past. And by the time the server is finished simulating all the ticks
        we dumped our time is FireDelay is already over. So now we can either shoot on the last tick of our dump or in the next packet we send. 

    NOTE : Since we are only allowed to send in 15 ticks in a packet and some weapons requrie more than that like Force-a-Nature with 21 ticks
           . We can send in 2 packets, and if we do it in the same frame, we can effectivly revind our tick base using 2 packet and upto whatever
           the max charge limit is on that server.
*/


constexpr int MAX_NEW_COMMANDS = 15;
typedef void(__fastcall* T_CL_Move)(float flAccumuatedExtraSample,  int64_t bFinalTick);


///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
void TickShifter_t::Run(BaseEntity* pLocalPlayer, baseWeapon* pActiveWeapon, CUserCmd* pCmd, bool* pSendPacket)
{
    PROFILER_RECORD_FUNCTION(CreateMove);

    m_bInitialized = true;

    if (m_bTickShifting == false)
    {
        bool bCanAttack = SDK::CanAttack(pLocalPlayer, pActiveWeapon) == true;

        // if we just ended tick shifting for DT. hold attack button for 1 extra tick, 
        // in order to try n get that DT if it fails in tick shifting.
        if (m_bFinalTickThisPacket == true)
        {
            if (m_bDoubleTap == true)
                pCmd->buttons |= IN_ATTACK;

            *pSendPacket           = true;
            m_bFinalTickThisPacket = false;
            m_bDoubleTap           = false;
            WIN_LOG("Fired second double tap shot.");
            return;
        }

        // Detecting double tap intruction from user.
        m_bDoubleTap = bCanAttack == true && (pCmd->buttons & IN_ATTACK) && Features::TickShifter::TickShifter::DoubleTap.IsActive() == true;

        // Don't send data if user wants to doubletap.
        if (m_bDoubleTap == true)
        {
            LOG("Detected double tap. not sending shit\n");
            *pSendPacket = false;
        }

        // Fire Rate ( with attributes )
        m_flRateOfFire = pActiveWeapon->GetTFWeaponInfo()->GetWeaponData(0)->m_flTimeFireDelay;
        pActiveWeapon->CALL_ATRIB_HOOK_FLOAT(m_flRateOfFire, "mult_postfiredelay");

        _Draw();
    }
    else { _SpoofCmd(pLocalPlayer, pActiveWeapon, pCmd, pSendPacket); }
}


///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
void TickShifter_t::Reset()
{
    m_bInitialized          = false;
    m_bDoubleTap            = false;
    m_bFinalTickThisPacket  = false; 
    m_flRateOfFire          = 0.0f;
    m_nShiftGoal            = 0;
    m_iChargeLevel          = 0; 
    m_bTickShifting         = false;
}


///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
void TickShifter_t::HandleTick(void* pOriginalCLMove, float flAccumulatedExtraSample, bool bOriginalFinalTick)
{
    // Consume this tick for charging & don't call the original.
    if (_ConsumeTickForCharge() == true && m_nShiftGoal <= 0 && m_bInitialized == true)
    {
        m_iChargeLevel++;
        return;
    }

    
    // Call the original.
    reinterpret_cast<T_CL_Move>(pOriginalCLMove)(flAccumulatedExtraSample, bOriginalFinalTick);

    // If we have non-zero charge & want to dump.
    if (_ShouldDumpCharge() == false)
        return;

    // Get our shift goal.
    if (m_nShiftGoal <= 0 && m_bTickShifting == false)
    {
        m_nShiftGoal = _DetermineShiftGoal();
        LOG("Shift Goal -> [ %d ]", m_nShiftGoal);
    }

    // Got enough charge for the shift goal. ( if not then skip, we will charge next tick )
    if (m_nShiftGoal > m_iChargeLevel)
    {
        m_nShiftGoal = 0;
        return;
    }


    if (m_nShiftGoal > 0)
    {
        m_bTickShifting = true;
        
        // Dumping ticks.
        _DumpCharge(m_nShiftGoal, pOriginalCLMove, flAccumulatedExtraSample);
        m_nShiftGoal = 0;
        
        if (m_nShiftGoal <= 0)
        {
            m_bTickShifting      = false;
            m_nShiftGoal         = 0;
            m_lastChargeDumpTime = std::chrono::high_resolution_clock::now();

            // Delete this
            WIN_LOG("//////////////////////////   Charge Dumped!   //////////////////////////");
        }
    }
}


///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
bool TickShifter_t::_ConsumeTickForCharge()
{
    // No double tapping while fake lagging.
    if (Features::Misc::FakeLag::FakeLag_Enable.IsActive() == true)
        return false;

    if (m_bTickShifting == true)
        return false;

    // Already charged ?
    if (m_iChargeLevel >= CVars::sv_maxusrcmdprocessticks)
        return false;

    // User want to recharge regardless of everything
    if (Features::TickShifter::TickShifter::ForceCharge.IsActive() == true)
        return true;

    // check if wait-delay ammount of time passed since we last dumped charge.
    uint64_t iTimeSinceDumpInMs = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - m_lastChargeDumpTime).count();
    if (iTimeSinceDumpInMs < static_cast<uint64_t>(fabs(Features::TickShifter::TickShifter::RechargeDelay_InSec.GetData().m_flVal) * 1000.0f))
        return false;

    // Recharging according to aggression prefrences. ( ex. every 7th tick or every other tick )
    int iRechargeAggression = Features::TickShifter::TickShifter::Recharge_Aggression.GetData().m_iVal;
    int iTicksSinceDump     = TIME_TO_TICK(static_cast<float>(iTimeSinceDumpInMs) / 1000.0f);
    if (iTicksSinceDump % iRechargeAggression == 0)
        return true;

    return false;
}


///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
void TickShifter_t::_SpoofCmd(BaseEntity* pLocalPlayer, baseWeapon* pActiveWeapon, CUserCmd* pCmd, bool* pSendPacket) const
{
    // don't send anything until the last tick of the packet.
    *pSendPacket = m_bFinalTickThisPacket;

    if (m_bDoubleTap == true)
    {
        if (m_bFinalTickThisPacket == true)
        {
            pCmd->buttons |= IN_ATTACK;
        }
        else
        {
            pCmd->buttons &= ~IN_ATTACK;
        }
    }
}


///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
int TickShifter_t::_DetermineShiftGoal()
{
    int iRateOfFireInTicks = static_cast<int>(0.5f + (m_flRateOfFire / TICK_INTERVAL)) + 1;

    // DT
    if (m_bDoubleTap == true && m_flRateOfFire > 0.0f)
        return std::clamp<int>(iRateOfFireInTicks, 0, CVars::sv_maxusrcmdprocessticks);

    // not DT
    return m_iChargeLevel;
}


///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
bool TickShifter_t::_ShouldDumpCharge() const
{
    if (Features::Misc::FakeLag::FakeLag_Enable.IsActive() == true)
        return false;

    // Got any charge ?
    if (m_iChargeLevel <= 0)
        return false;

    // Dump regardless ? 
    if (Features::TickShifter::TickShifter::Force_ChargeDump.IsActive() == true || m_bDoubleTap == true)
        return true;

    return false;
}


///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
void TickShifter_t::_DumpCharge(int nTicks, void* pOriginalCLMove, float flAccumulatedExtraSample)
{
    // Fn cast the original CLMove poitner.
    T_CL_Move pCLMove = reinterpret_cast<T_CL_Move>(pOriginalCLMove);
    
    // we can only do 15 commands in 1 packet. so we shall split it by sending 1 packet after 15 commands.
    bool bShouldSplitPacket = nTicks > MAX_NEW_COMMANDS - 1;

    for (int iTick = 0; iTick < nTicks; iTick++)
    {   
        m_bFirstTick           = iTick == 0;
        m_bFinalTickThisPacket = (iTick == nTicks - 1) || (bShouldSplitPacket == true && iTick == nTicks / 2);

        // I actually don't know why I have placed a fucking 0 for the accumulatedExtraSample but it seems to work.
        pCLMove(flAccumulatedExtraSample, m_bFinalTickThisPacket);

        // if you don't want your velocity / position to be static during tick shifting, you better call one
        // CL_RunPrediction per tick you shift.
        ((void(*)())ASM::CL_RunPrediction)();
    }

    // Deducting shifted ticks from charge.
    m_iChargeLevel = Maths::MAX<int>(m_iChargeLevel - nTicks, 0);
}


///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
void TickShifter_t::_Draw()
{
    int64_t iTimeSinceDumpInMs = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - m_lastChargeDumpTime).count();

    Render::infoWindowV2.AddOrUpdate("TickShifter", std::format("{} / {}", m_iChargeLevel, CVars::sv_maxusrcmdprocessticks), 0, InfoWindowWidget_t::Alignment_Middle);
    Render::infoWindowV2.AddOrUpdate("TickShifter", m_iChargeLevel, 0, CVars::sv_maxusrcmdprocessticks, 0);
    Render::infoWindowV2.AddOrUpdate("TickShifter", 
        std::format("waiting {:.2}",
        Maths::MAX<float>(0.0f, Features::TickShifter::TickShifter::RechargeDelay_InSec.GetData().m_flVal - (iTimeSinceDumpInMs / 1000.0f))), 1, InfoWindowWidget_t::Alignment_Middle);
}


///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
bool TickShifter_t::CanShiftThisTick() const
{
    return m_bTickShifting == false && m_iChargeLevel > 0 && m_bInitialized == true;
}


///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
bool TickShifter_t::ForceDumpCharge(int iTicks)
{
    if (CanShiftThisTick() == false)
        return false;

    m_bDoubleTap = false; m_nShiftGoal = iTicks;
    return true;
}


///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
bool TickShifter_t::DoubleTapping() const
{
    return ShiftingTicks() == true && m_bDoubleTap == true;
}


//=========================================================================
//                     DEBUGGING HOOKS
//=========================================================================
#if (DEBUG_TICK_SHIFTING == true)

#include "../../Utility/Hook Handler/Hook_t.h"

MAKE_HOOK(CPlayerMove_RunCommand, "48 89 5C 24 ? 48 89 74 24 ? 57 41 54 41 55 41 56 41 57 48 83 EC ? 8B 9A", __fastcall, SERVER_DLL, void*,
    void* pVTable, BaseEntity* pPlayer, CUserCmd* pCmd, void* pMoveHelper)
{

    int& nShiftableTicks = *reinterpret_cast<int*>(reinterpret_cast<int*>(pPlayer) + 0x282);
    Render::InfoWindow.AddToCenterConsole("Server_Ticks_DT", std::format("ServerCharge : {}", nShiftableTicks), nShiftableTicks < CVars::sv_maxusrcmdprocessticks ? RED : GREEN);
    
    int iTickBase = *reinterpret_cast<int*>(reinterpret_cast<char*>(pPlayer) + (0x45D * 4));

    printf("[ SERVER ] CMD->Shooting : [ %s ] | TickBase [ %d ]\n", (pCmd->buttons & IN_ATTACK) ? "TRUE" : "false", iTickBase);

    return Hook::CPlayerMove_RunCommand::O_CPlayerMove_RunCommand(pVTable, pPlayer, pCmd, pMoveHelper);

}

MAKE_HOOK(CPlayerMove_StartCommand_Helper, "48 89 5C 24 ? 57 48 83 EC ? 49 8B D8 48 8B FA E8", __fastcall, SERVER_DLL, int64_t,
    int64_t a1, BaseEntity* pPlayer, CUserCmd* pCmd)
{
    //LOG("HERE!");
    return Hook::CPlayerMove_StartCommand_Helper::O_CPlayerMove_StartCommand_Helper(a1, pPlayer, pCmd);
}

MAKE_HOOK(CBasePlayer_AdjustPlayerTickBase, "85 D2 0F 88 ? ? ? ? 55", __fastcall, SERVER_DLL, void, BaseEntity* pPlayer, int nSimulationTicks)
{
    int iOriginalTickBase = *reinterpret_cast<int*>(reinterpret_cast<char*>(pPlayer) + (0x45D * 4));

    Hook::CBasePlayer_AdjustPlayerTickBase::O_CBasePlayer_AdjustPlayerTickBase(pPlayer, nSimulationTicks);

    int iShiftedTickBase = *reinterpret_cast<int*>(reinterpret_cast<char*>(pPlayer) + (0x45D * 4));
    printf("Simulation ticks [ %d ] , TickBaseDelta [ %d ]\n", nSimulationTicks, iShiftedTickBase - iOriginalTickBase);
}

#endif