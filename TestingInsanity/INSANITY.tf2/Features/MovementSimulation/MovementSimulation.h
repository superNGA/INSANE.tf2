#pragma once

#include <vector>

// UTILITY
#include "../FeatureHandler.h"

// SDK
#include "../../SDK/class/BaseEntity.h"
#include "../../SDK/class/CPrediction.h"
#include "../../SDK/class/CMoveData.h"
#include "../../SDK/class/CPlayerLocalData.h"

class CUserCmd;

/*
Flaws : 
    -> Gets fucked up in duck transition.
    -> Crashed in approching spawn gate.
*/

struct PlayerDataBackup_t
{
    PlayerDataBackup_t();
    
    void Store(BaseEntity* pPlayer);
    void Reset();

    CUserCmd*  pOldCmd;
    uint32_t   m_fFlags;
    
    float m_flDuckTime;
    float m_flJumpTime;
    float m_flDuckJumpTime;
    bool m_bDucking;
    bool m_bDucked;
    bool m_bInDuckJump;

    uint32_t m_hGroundEntity;

    vec m_vOrigin;
    vec m_vVelocity;
    vec m_vEyeOffset;
};

class MovementSimulation_t
{
public:
    MovementSimulation_t() { Reset(); }
    void Reset();

    bool Initialize(BaseEntity* pEnt, CUserCmd* pCmd);
    void RunTick();
    void Restore();

    bool m_bSimulationRunning = false;

private:
    bool  m_bOldInPrediction       = false;
    bool  m_bOldFirstTimePredicted = false;
    float m_flOldFrameTime         = 0.0f;

    BaseEntity* m_pPlayer = nullptr;
    CMoveData moveData;
    void _ResetMoveData(CMoveData& moveData);

    void _SetupMove(BaseEntity* pEnt);

    void _SetupMoveData(CMoveData& moveData, BaseEntity* pEnt);
    void _SetupMoveDataLocal(CMoveData& moveData, BaseEntity* pEnt, CUserCmd* pCmd);

    void _HandleDuck(BaseEntity* pEnt);

    vec m_vOldPos;
    vec m_vLastSimulatedPos;

    bool m_bInitialized = false;

    PlayerDataBackup_t m_playerDataBackup;
};
DECLARE_FEATURE_OBJECT(movementSimulation, MovementSimulation_t)

DEFINE_SECTION(MovementSim, "Aimbot", 2);

DEFINE_FEATURE(Debug_MovementSim, bool, MovementSim, Aimbot, 1, false)
DEFINE_FEATURE(Ticks_To_Simulate, IntSlider_t, MovementSim, Aimbot, 2, 
    IntSlider_t(10, 1, 30))