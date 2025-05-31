#pragma once

#include <vector>

// UTILITY
#include "../FeatureHandler.h"

// SDK
#include "../../SDK/class/BaseEntity.h"
#include "../../SDK/class/CMoveData.h"
#include "../../SDK/class/CPrediction.h"

class MovementSimulation_t
{
public:
    void Reset();

    bool Initialize(BaseEntity* pEnt);
    void RunTick();
    void Restore();

private:
    bool  m_bOldInPrediction       = false;
    bool  m_bOldFirstTimePredicted = false;
    float m_flOldFrameTime         = 0.0f;

    BaseEntity* pPlayer = nullptr;
    CMoveData moveData;
    void _ResetMoveData(CMoveData& moveData);
    void _SetupMoveData(CMoveData& moveData, BaseEntity* pEnt);

    vec m_vOldPos;

    bool m_bInitialized = false;
};
DECLARE_FEATURE_OBJECT(movementSimulation, MovementSimulation_t)

DEFINE_SECTION(MovementSim, "Aimbot", 2);
DEFINE_FEATURE(Debug_MovementSim, bool, "MovementSim", "Aimbot", 1,
    false)