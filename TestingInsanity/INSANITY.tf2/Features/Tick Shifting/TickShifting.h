//=========================================================================
//                      TICK SHIFTER
//=========================================================================
// by      : INSANE
// created : 26/07/2025
// 
// purpose : Revinds tick base ( Used for DT, Speed HX & more )
//-------------------------------------------------------------------------
#pragma once

#include "../FeatureHandler.h"
#include "../../Utility/CVar Handler/CVarHandler.h"


class BaseEntity;
class baseWeapon;
class CUserCmd;


/////////////////////////////////////////////////////////////////////////////////////////////////////
class TickShifter_t
{
public:
    void Run(BaseEntity* pLocalPlayer, baseWeapon* pActiveWeapon, CUserCmd* pCmd, bool* pSendPacket);
    void Reset();

    void HandleTick(void* pOriginalCLMove, float flAccumulatedExtraSample, bool bOriginalFinalTick);

    inline int  GetMaxCharge()  const { return CVars::sv_maxusrcmdprocessticks; }
    inline int  GetCharge()     const { return m_iChargeLevel; }
    inline bool ShiftingTicks() const { return m_bTickShifting; }
    bool        CanShiftThisTick() const;
    bool        ForceDumpCharge(int iTicks);
    bool        DoubleTapping() const;

private:
    bool m_bInitialized = false;    

    // Charging...
    bool _ConsumeTickForCharge();
    int  m_iChargeLevel  = 0;
    bool m_bTickShifting = false;

    void _SpoofCmd(BaseEntity* pLocalPlayer, baseWeapon* pActiveWeapon, CUserCmd* pCmd, bool* pSendPacket) const;
    
    // Shifting Ticks...
    int  _DetermineShiftGoal();
    bool _ShouldDumpCharge() const;
    void _DumpCharge(int nTicks, void* pOriginalCLMove, float flAccumulatedExtraSample);
    bool m_bDoubleTap = false;

    std::chrono::high_resolution_clock::time_point m_lastChargeDumpTime;
    bool  m_bFirstTick           = false;
    bool  m_bFinalTickThisPacket = false;
    float m_flRateOfFire         = 0.0f;
    int   m_nShiftGoal           = 0;

    // Drawing 
    void _Draw();
};
/////////////////////////////////////////////////////////////////////////////////////////////////////


DECLARE_FEATURE_OBJECT(tickShifter, TickShifter_t)

DEFINE_TAB(TickShifter, 3)
DEFINE_SECTION(TickShifter, "TickShifter", 1)

DEFINE_FEATURE(ForceCharge,         "Force charge",       bool,          TickShifter, TickShifter, 1, false, FeatureFlag_HoldOnlyKeyBind | FeatureFlag_SupportKeyBind, "Charges aggresively ( will cause a little glitching )")
DEFINE_FEATURE(RechargeDelay_InSec, "Recharge delay",     FloatSlider_t, TickShifter, TickShifter, 2, FloatSlider_t(1.0f, 0.0f, 10.0f), FeatureFlag_SupportKeyBind, "Waits for some time before recharging.")
DEFINE_FEATURE(Recharge_Aggression, "Recharge aggresion", IntSlider_t,   TickShifter, TickShifter, 3, IntSlider_t(2, 1, 66), FeatureFlag_SupportKeyBind, "How aggesively do you wanna recharge ( How much time u willing to spend recharging )")
DEFINE_FEATURE(DoubleTap,           "DoubleTap",          bool,          TickShifter, TickShifter, 4, false, FeatureFlag_SupportKeyBind, "Fire multiple bullets at once")
DEFINE_FEATURE(Force_ChargeDump,    "Dump charge",        bool,          TickShifter, TickShifter, 5, false, FeatureFlag_SupportKeyBind | FeatureFlag_HoldOnlyKeyBind, "Dump whatever charge we have build up. ( can be used creativly :) )")
DEFINE_FEATURE(DoubleTab_AutoStop,  "Auto-Stop",          bool,          TickShifter, TickShifter, 6, false, FeatureFlag_None, "Stop while double tapping, to prevent warping while DT.")