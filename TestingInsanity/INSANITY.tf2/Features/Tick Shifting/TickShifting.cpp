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
#include "../ImGui/InfoWindow/InfoWindow_t.h"

// UTILITY
#include "../../Utility/Interface Handler/Interface.h"
#include "../../Utility/Signature Handler/signatures.h"
#include "../../Utility/CVar Handler/CVarHandler.h"
#include "../../Utility/ConsoleLogging.h"
#include "../../Extra/math.h"

#define DEBUG_TICK_SHIFTING false

typedef void(__fastcall* T_CL_Move)(float flAccumuatedExtraSample,  int64_t bFinalTick);


void TickShifter_t::Run(BaseEntity* pLocalPlayer, baseWeapon* pActiveWeapon, CUserCmd* pCmd, bool* pSendPacket)
{
    if (m_bTickShifting == false)
    {
        m_iTickBaseBackup = pLocalPlayer->m_nTickBase(); m_flSimTimeBackup = pLocalPlayer->m_flSimulationTime();
        m_bShooting       = SDK::CanAttack(pLocalPlayer, pActiveWeapon) == true && (pCmd->buttons & IN_ATTACK);
        m_flRateOfFire    = pActiveWeapon->GetTFWeaponInfo()->GetWeaponData(0)->m_flTimeFireDelay;
        pActiveWeapon->CALL_ATRIB_HOOK_FLOAT(m_flRateOfFire, "mult_postfiredelay");

        _Draw();

        if (m_bFinalTick == true)
        {
            std::cout << "shot firred111111111111111111111111111111111\n";
            pCmd->buttons |= IN_ATTACK;
            m_bFinalTick = false;
        }
    }
    else { _SpoofCmd(pLocalPlayer, pCmd, pSendPacket); }
}



void TickShifter_t::Reset()
{
    m_iTickBaseBackup = 0; m_flSimTimeBackup = 0.0f;
    m_bShooting         = false;
    m_bFinalTick        = false;
    m_flRateOfFire      = 0.0f;
    m_iDumpTick         = 0;

    m_nShiftedTickIndex = 0;
    m_nShiftGoal        = 0;
    m_nShiftedTicks     = 0;

    m_iChargeLevel = 0; m_bTickShifting = false;
}


void TickShifter_t::HandleTick(void* pOriginalCLMove, float flAccumulatedExtraSample, bool bOriginalFinalTick)
{
    static int iShiftGoal = 0;

    // Consume this tick for charging & don't call the original.
    if (_ConsumeTickForCharge() == true && iShiftGoal <= 0)
    {
        m_iChargeLevel++;
        return;
    }
    
    // Call the original.
    reinterpret_cast<T_CL_Move>(pOriginalCLMove)(flAccumulatedExtraSample, bOriginalFinalTick);


    if (_CanDumpCharge() == false && iShiftGoal <= 0)
        return;

    if (iShiftGoal <= 0)
    {
        iShiftGoal = _DetermineShiftGoal();
        printf("Shift goal set to [ %d ]\n", iShiftGoal);
    }

    if (iShiftGoal > 0)
    {
        m_bTickShifting = true;
        
        m_nShiftedTicks = iShiftGoal; //m_nShiftedTicks = Maths::MIN<int>(iShiftGoal, 7); 
        _DumpCharge(m_nShiftedTicks, pOriginalCLMove, flAccumulatedExtraSample);
        iShiftGoal -= m_nShiftedTicks; //Maths::MAX<int>(iShiftGoal - m_nShiftedTicks, 0); // compensate what we have done form the goal ( not like it's gonna fail or anything :| )

        m_bTickShifting = false;
        
        if (iShiftGoal <= 0)
        {
            m_bShooting          = false;
            m_iDumpTick          = GLOBAL_TICKCOUNT;
            m_lastChargeDumpTime = std::chrono::high_resolution_clock::now();

            // Delete this
            WIN_LOG("//////////////////////////   Charge Dumped!   //////////////////////////");
        }
    }
}


bool TickShifter_t::_ConsumeTickForCharge()
{
    if (m_bTickShifting == true)
        return false;

    // Already charged ?
    if (m_iChargeLevel >= CVars::sv_maxusrcmdprocessticks)
        return false;

    // User want to recharge regardless of everything
    if (Features::TickShifter::TickShifter::ForceCharge.IsActive() == true)
        return true;

    // Has wait-delay ammount of time passed since we last dumped charge?
    uint64_t iDeltaTime = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - m_lastChargeDumpTime).count();
    if (iDeltaTime < static_cast<uint64_t>(fabs(Features::TickShifter::TickShifter::RechargeDelay_InSec.GetData().m_flVal) * 1000.0f))
        return false;

    // Recharging according to aggesion prefrences. ( ex. every 7th tick or every other tick )
    if ((m_iDumpTick - GLOBAL_TICKCOUNT) % Features::TickShifter::TickShifter::Recharge_Aggression.GetData().m_iVal == 0)
        return true;

}


void TickShifter_t::_SpoofCmd(BaseEntity* pLocalPlayer, CUserCmd* pCmd, bool* pSendPacket)
{
    *pSendPacket = m_bFinalTick == true || I::cClientState->chokedcommands >= 21;

    if (m_bShooting == true)
    {
        pCmd->forwardmove = 0.0f; pCmd->sidemove = 0.0f;
    }
}


int TickShifter_t::_DetermineShiftGoal()
{
    int iRateOfFireInTicks = static_cast<int>(0.5f + (m_flRateOfFire / TICK_INTERVAL));

    // DT
    if (m_bShooting == true && m_flRateOfFire > 0.0f)
        return iRateOfFireInTicks > CVars::sv_maxusrcmdprocessticks ? CVars::sv_maxusrcmdprocessticks : iRateOfFireInTicks;

    // not DT
    return m_iChargeLevel;
}


bool TickShifter_t::_CanDumpCharge() const
{
    // Got any charge ?
    if (m_iChargeLevel <= 0)
        return false;

    // Dump regardless ? 
    if (Features::TickShifter::TickShifter::Force_ChargeDump.IsActive() == true)
        return true;

    // Double Tapin ?
    if (Features::TickShifter::TickShifter::DoubleTap.IsActive() == true && m_bShooting == true)
        return true;

    return false;
}


void TickShifter_t::_DumpCharge(int nTicks, void* pOriginalCLMove, float flAccumulatedExtraSample)
{
    // Fn cast the original CLMove poitner.
    T_CL_Move pCLMove = reinterpret_cast<T_CL_Move>(pOriginalCLMove);

    for (int iTick = 0; iTick < nTicks; iTick++)
    {
        m_bFinalTick = (iTick == nTicks - 1);
        pCLMove(flAccumulatedExtraSample, iTick == nTicks - 1 ? 1LL : 0LL);
    }

    // Deducting shifted ticks from charge.
    m_iChargeLevel = Maths::MAX<int>(m_iChargeLevel - nTicks, 0);
}


void TickShifter_t::_Draw()
{
    // Drawing Charge
    Render::InfoWindow.AddToCenterConsole("TS_Charge", std::format("Charge: {}", m_iChargeLevel), m_iChargeLevel < CVars::sv_maxusrcmdprocessticks ? RED : GREEN);
    
    // Drawing Charge delay
    int64_t iTimeSinceDumpInMs = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - m_lastChargeDumpTime).count();
    Render::InfoWindow.AddToCenterConsole("TS_ChargeWait", std::format("waiting {:.2}", 
        Maths::MAX<float>(0.0f, Features::TickShifter::TickShifter::RechargeDelay_InSec.GetData().m_flVal - (iTimeSinceDumpInMs / 1000.0f))));
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

    printf("[ SERVER ] CMD Tick count -> [ %d ] | TickBase -> [ %d ]\n", 
        pCmd->tick_count, 
        *reinterpret_cast<int*>(reinterpret_cast<char*>(pPlayer) + (0x45D * 4)));

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