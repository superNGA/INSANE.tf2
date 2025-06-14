#pragma once

#include <vector>

// UTILITY
#include "../FeatureHandler.h"

// SDK
#include "../../SDK/class/BaseEntity.h"
#include "../../SDK/class/CPrediction.h"
#include "../../SDK/class/CMoveData.h"
#include "../../SDK/class/CUserCmd.h"
#include "../../SDK/class/CPlayerLocalData.h"

class CUserCmd;

// TODO : Ducking is messed up. Glitches while ducking in transition.

struct PlayerDataBackup_t
{
    PlayerDataBackup_t();
    void Store(BaseEntity* pPlayer);
    void Restore(BaseEntity* pPlayer);
    void Reset();

    CUserCmd*  pOldCmd;
    uint32_t   m_fFlags;
    
    float      m_flDuckTime;
    float      m_flJumpTime;
    float      m_flDuckJumpTime;
    bool       m_bDucking;
    bool       m_bDucked;
    bool       m_bInDuckJump;

    uint32_t   m_hGroundEntity;

    vec        m_vOrigin;
    vec        m_vVelocity;
    vec        m_vEyeOffset;

    float      m_flModelScale;
};

class MovementSimulation_t
{
public:
    MovementSimulation_t() { Reset(); }
    void Reset();

    bool Initialize(BaseEntity* pEnt);
    void RunTick();
    void Restore();

    bool m_bSimulationRunning = false;

    inline const vec& GetSimulationPos() const { return m_moveData.m_vecAbsOrigin; }
    
private:
    bool    m_bInitialized           = false;
    bool    m_bOldInPrediction       = false;
    bool    m_bOldFirstTimePredicted = false;
    float   m_flOldFrameTime         = 0.0f;
    vec     m_vLastSimulatedPos;

    BaseEntity* m_pPlayer = nullptr;
    
    CMoveData          m_moveData;
    PlayerDataBackup_t m_playerDataBackup;
    CUserCmd           m_dummyCmd;

    void _SetupMove(BaseEntity* pEnt);
    void _HandleDuck(BaseEntity* pEnt);
};

DECLARE_FEATURE_OBJECT(movementSimulation, MovementSimulation_t)

DEFINE_SECTION(MovementSim, "Aimbot", 2);

DEFINE_FEATURE(Debug_MovementSim, bool, MovementSim, Aimbot, 1, false)
DEFINE_FEATURE(Ticks_To_Simulate, IntSlider_t, MovementSim, Aimbot, 2, 
    IntSlider_t(10, 1, 30))