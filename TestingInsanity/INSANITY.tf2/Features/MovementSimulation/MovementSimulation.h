#pragma once

#include <vector>
#include <unordered_map>
#include <deque>

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

    bool Initialize(BaseEntity* pEnt, bool bStrafePrediction = true);
    void RunTick();
    void Restore();

    bool m_bSimulationRunning = false;

    inline const vec& GetSimulationPos() const { return m_moveData.m_vecAbsOrigin; }
    inline const vec& GetSimulationVel() const { return m_moveData.m_vecVelocity; }
    inline uint32_t GetSimulationFlags() const { return m_iLastFlags; }
    
    void RecordStrafeData(BaseEntity* pEnt, const bool bIsLocalPlayer);
    void ClearStrafeData();

private:
    bool     m_bInitialized           = false;
    bool     m_bUseStrafePrediction   = false;
    bool     m_bOldInPrediction       = false;
    bool     m_bOldFirstTimePredicted = false;
    float    m_flOldFrameTime         = 0.0f;
    vec      m_vLastSimulatedPos;
    uint32_t m_iLastFlags = 0;

    BaseEntity* m_pPlayer = nullptr;
    
    CMoveData          m_moveData;
    PlayerDataBackup_t m_playerDataBackup;
    CUserCmd           m_dummyCmd;
    uint32_t           m_iTick = 0;

    void _SetupMove(BaseEntity* pEnt, const bool bLocalPlayer = false);
    void _HandleDuck(BaseEntity* pEnt);
    

    // Strafe prediction data ( this data doesn't get resetted upon restoring )
    struct StrafeData_t
    {
        float  m_flYaw;
    };
    static constexpr uint32_t MAX_STRAFE_SAMPLES = 16u;
    std::unordered_map<BaseEntity*, std::deque<StrafeData_t>> m_mapStrafeSamples = {};

    void _CalculateStrafeAmmount(BaseEntity* pEnt);
    void _ApplyStrafe(uint32_t iTick);

    float m_flDeltaYaw;
    uint32_t m_iInitialFlags = 0;
};

DECLARE_FEATURE_OBJECT(movementSimulation, MovementSimulation_t)

DEFINE_SECTION(MovementSim, "Aimbot", 2);

DEFINE_FEATURE(Debug_MovementSim, bool, MovementSim, Aimbot, 1, false)
DEFINE_FEATURE(Ticks_To_Simulate, IntSlider_t, MovementSim, Aimbot, 2, 
    IntSlider_t(10, 1, 128))

DEFINE_FEATURE(Enable_Strafe_Prediction, bool, MovementSim, Aimbot, 3, true)


/*

DONE : 
    -> Maintain a reliable stream of view angle and velocity change data.
    -> reset and clear out data upon leaving the game.
    -> make ground strafe prediction.
        -> Handle angle edge cases.
    -> test ground strafe prediction.
*/


/*

TODO : 
    -> make ground strafe prediction.
        -> Handle noisy & glitchy data.
    -> Make air strafe prediction.
        -> Only do if angle and velocity change.
        -> Maybe add accelration too.

*/


/*

STRAFE PREDICTION : 
    Not as easy and definetly not rocket science.
        STRAFING : when a nigga is mid-air or on the ground, they change their view
        angles and give in some input (move buttons) which will increase their velocity
        in their wish direction. That velcity gets added to their current velocity.
        and it capped to whatever is allowed by the server.
        
        While on ground, the input buttons change the direction imediately,
        and all we need to account for is the view angle, cause their input
        buttons will take effect immediatly but thier view angles will slowly change
        their trajactory over a couple of ticks. ( like a smooth curve )

        While in air, their change in view angles won't solely dictate change 
        in their direction. A person in air can change their view angles all they want
        but their direction won't change, cause they need to give in input button
        i.e. either of the side move or forward move to "Strafe".
        So, that means if someone is changing their view angles we check if they have
        any change in velocity ACCORDING to that change in view angle. If no, then
        they have given in no input.

    THEORY : 
        if on ground, just check for change in view angle & keep adding that
        change in view angle over each tick.

        If in Air, get change in view angle. If no change, just simulate simply.
        if there is change in view angle, check if there is any change in velocity.
        if there is a change in velocity with respect of the angle change, keep 
        appliying that change to simulate strafing.

*/