#include "MovementSimulation.h"

#include <algorithm>

// UTILITY
#include "../../Extra/math.h"
#include "../../SDK/TF object manager/TFOjectManager.h"
#include "../../Utility/Signature Handler/signatures.h"
#include "../../Utility/Interface Handler/Interface.h"
#include "../../Utility/Hook Handler/Hook_t.h"
#include "../ImGui/InfoWindow/InfoWindow_t.h"

//SDK
#include "../../SDK/class/CUserCmd.h"

#define DEBUG_MOVEMENT_SIM_HOOKS false

// Fns
MAKE_SIG(CTFGameMovement_ProcessMovement, "48 85 D2 0F 84 ? ? ? ? 48 89 5C 24 ? 48 89 74 24 ? 57 48 83 EC ? 49 8B F0 48 8B FA 48 8B D9 4D 85 C0", CLIENT_DLL,
    void, void*, BaseEntity*, CMoveData*)
MAKE_INTERFACE_VERSION(iGameMovement, "GameMovement001", void, CLIENT_DLL)

void MovementSimulation_t::Reset()
{
    m_bInitialized = false;

    m_bOldInPrediction       = false;
    m_bOldFirstTimePredicted = false;
    m_bUseStrafePrediction   = false;
    m_flOldFrameTime         = 0.0f;

    m_pPlayer                = nullptr;
    m_iLastFlags             = 0;
    m_vLastSimulatedPos.Init();

    memset(&m_moveData,         0, sizeof(CMoveData));
    memset(&m_playerDataBackup, 0, sizeof(PlayerDataBackup_t));
    memset(&m_dummyCmd,         0, sizeof(CUserCmd));
}

bool MovementSimulation_t::Initialize(BaseEntity* pEnt, bool bStrafePrediction)
{
    if (m_bSimulationRunning == true)
    {
        FAIL_LOG("SIMULATION ALREADY RUNNING !!!! STUPID ASS MONKEY !!! SKILL ISSUES !!!");
        return false;
    }

    m_bUseStrafePrediction = bStrafePrediction;

    // Storing strafing data for this target
    bool bLocalPlayer = pEnt->entindex() == I::iEngine->GetLocalPlayer();
    m_iInitialFlags   = pEnt->m_fFlags();
    _CalculateStrafeAmmount(pEnt);

    // Store Original Prediction data & frame time.
    m_bOldInPrediction       = I::cPrediction->m_bInPrediction;
    m_bOldFirstTimePredicted = I::cPrediction->m_bFirstTimePredicted;
    m_flOldFrameTime         = tfObject.pGlobalVar->frametime;
    m_pPlayer                = pEnt;
    
    // Backing up player data
    m_playerDataBackup.Store(m_pPlayer);

    // Adjusting player data temporarily
    m_pPlayer->m_pCurrentCommand(&m_dummyCmd);
    m_pPlayer->m_hGroundEntity(NULL);

    // Setting up Move Data
    _SetupMove(m_pPlayer, bLocalPlayer);
    
    // Handling Ducking ( Glitching )
    _HandleDuck(m_pPlayer);

    m_bInitialized = true;

    return true;
}

void MovementSimulation_t::RunTick()
{
    if (m_bInitialized == false)
    {
        FAIL_LOG("MOVEMENT SIMULATOR NOT INTIALIZED !!!! STUPID ASS MONKEY !!! SKILL ISSUES !!!");
        return;
    }

    m_bSimulationRunning                  = true;
    I::cPrediction->m_bInPrediction       = true;
    I::cPrediction->m_bFirstTimePredicted = false;
    tfObject.pGlobalVar->frametime        = I::cPrediction->m_bEnginePaused == true ? 0 : tfObject.pGlobalVar->interval_per_tick;

    // strafin' Predicsha'
    if(m_bUseStrafePrediction == true)
        _ApplyStrafe(m_iTick);

    Sig::CTFGameMovement_ProcessMovement(I::iGameMovement, m_pPlayer, &m_moveData);
    m_iLastFlags = m_pPlayer->m_fFlags();
    
    // Drawing line connecting all predicted positions
    if (Features::Aimbot::MovementSim::Debug_MovementSim.IsActive() == true && m_vLastSimulatedPos.IsEmpty() == false)
    {
        I::IDebugOverlay->AddLineOverlay(
            m_vLastSimulatedPos,        // Start
            m_moveData.m_vecAbsOrigin,  // End
            255, 255, 255,              // Color
            false, 10.0f                // Depth test & duration
        );
    }

    m_vLastSimulatedPos = m_moveData.m_vecAbsOrigin;
    m_iTick++;
}

void MovementSimulation_t::Restore()
{
    I::cPrediction->m_bInPrediction       = m_bOldInPrediction;
    I::cPrediction->m_bFirstTimePredicted = m_bOldFirstTimePredicted;
    tfObject.pGlobalVar->frametime        = m_flOldFrameTime;

    // restore players data from backup data
    m_playerDataBackup.Restore(m_pPlayer);

    // set movement simulator as free :)
    m_bSimulationRunning   = false;
    m_bInitialized         = false;
    m_bUseStrafePrediction = true;

    m_iTick = 0;

    Reset();
};


//=========================================================================
//                     PRIVATE METHODS
//=========================================================================
void MovementSimulation_t::_SetupMove(BaseEntity* pEnt, const bool bLocalPlayer)
{
    m_moveData.m_bFirstRunOfFunctions = false; // Keep it false, else will do extra bullshit and performance loss :(
    m_moveData.m_bGameCodeMovedPlayer = false; // Keep false, else calls CatagorizeMovement() & I don't know WTF is that :)

    m_moveData.m_nPlayerHandle    = pEnt->GetRefEHandle();
    m_moveData.m_vecAbsOrigin     = pEnt->GetAbsOrigin();
    m_moveData.m_vecVelocity      = pEnt->m_vecVelocity();

    m_moveData.m_vecViewAngles    = bLocalPlayer == true ? pEnt->GetAbsAngles() : pEnt->m_angEyeAngles(); // Eye angle ( Same for local & world space (probably) in tf2. )
    m_moveData.m_vecAbsViewAngles = m_moveData.m_vecViewAngles;
    m_moveData.m_vecAngles        = m_moveData.m_vecViewAngles; // The game's Setupmove ( CPrediction::Setupmove ) is setting it from user cmd
    
    qangle qVelocity;
    Maths::VectorAnglesFromSDK(m_moveData.m_vecVelocity, qVelocity);
    const float flSpeed   = m_moveData.m_vecVelocity.Length();
    float flThetaInDegree = m_moveData.m_vecAbsViewAngles.yaw - qVelocity.yaw;

    // if we don't clamp this shit, "move" will rise to 500+ when rocket jumping. ( Fairly accurate otherwise. )
    m_moveData.m_flForwardMove = std::clamp<float>(flSpeed * cos(DEG2RAD(flThetaInDegree)), -MAX_MOVE_USERCMD, MAX_MOVE_USERCMD);
    m_moveData.m_flSideMove    = std::clamp<float>(flSpeed * sin(DEG2RAD(flThetaInDegree)), -MAX_MOVE_USERCMD, MAX_MOVE_USERCMD);

    m_moveData.m_flMaxSpeed         = 10000.0f;
    m_moveData.m_flClientMaxSpeed   = 10000.0f;
    m_moveData.m_flConstraintRadius = 0.0f; // radius 0 means no movement constraints ?
}

void MovementSimulation_t::_HandleDuck(BaseEntity* pEnt)
{
    if (pEnt->m_fFlags() & FL_DUCKING)
    {
        pEnt->m_flDucktime(0.0f);
        pEnt->m_flDuckJumpTime(0.0f);
        pEnt->m_bDucking(false);
        pEnt->m_bDucked(true);
        pEnt->m_bInDuckJump(false);
        m_moveData.m_nButtons |= IN_DUCK;

        // Removing the duck flag, else the Z axis for this entity won't change with ducking.
        *reinterpret_cast<int32_t*>(reinterpret_cast<uintptr_t>(pEnt) + Netvars::DT_BasePlayer::m_fFlags) &= ~IN_DUCK;
    }
}


void MovementSimulation_t::RecordStrafeData(BaseEntity* pEnt, const bool bIsLocalPlayer)
{
    // Sometimes in loopback servers, local players Eye Angle is incorrect & delayed, hence we use absolute angle here.
    float flYaw = bIsLocalPlayer == true ? pEnt->GetAbsAngles().yaw : pEnt->m_angEyeAngles().yaw;

    auto it = m_mapStrafeSamples.find(pEnt);
    if (it == m_mapStrafeSamples.end())
    {
        m_mapStrafeSamples.emplace(
            pEnt,  // Key
            std::deque<StrafeData_t>{StrafeData_t{ flYaw }} // Strafe data queue. 
        ); 
        return;
    }
    

    // removing older items
    if (it->second.size() > MAX_STRAFE_SAMPLES)
    {
        it->second.pop_front();
    }

    it->second.push_back(StrafeData_t{ flYaw });
}


void MovementSimulation_t::ClearStrafeData()
{
    m_mapStrafeSamples.clear();
    m_flDeltaYaw    = 0.0f;
    m_iInitialFlags = 0;
}


void MovementSimulation_t::_CalculateStrafeAmmount(BaseEntity* pEnt)
{
    auto it = m_mapStrafeSamples.find(pEnt);
    if (it == m_mapStrafeSamples.end())
        return; // We don't have no data for this target.


    const std::deque<StrafeData_t>& qStrafeSamples = it->second;
    int nSamples = qStrafeSamples.size();
    if (nSamples < 3)
        return; // we don't have enough data for this target.


    // Calculating rate of change of view angle.
    float flAvgDeltaYaw = 0.0f;

    for (int iSampleIndex = 1; iSampleIndex < nSamples; iSampleIndex++)
    {
        // Calculating sum of change in yaw.
        float flDeltaYaw = qStrafeSamples[iSampleIndex].m_flYaw -   qStrafeSamples[iSampleIndex - 1].m_flYaw;

        // Handling sus angles ( i.e. angle going over the -180 to 180 seem )
        if (fabs(flDeltaYaw) > 180.0f)
        {
            // Just getting the angle from the smaller side.
            flDeltaYaw = std::remainderf(360.0f, flDeltaYaw) * -1.0f;

            //FAIL_LOG("Handled sus angle");
        }
        
        flAvgDeltaYaw += flDeltaYaw;
    }

    m_flDeltaYaw = flAvgDeltaYaw / static_cast<float>(nSamples - 1);
}


void MovementSimulation_t::_ApplyStrafe(uint32_t iTick)
{
    // adjusting view angles
    m_moveData.m_vecViewAngles.yaw    += m_flDeltaYaw;
    m_moveData.m_vecAbsViewAngles.yaw += m_flDeltaYaw;
    m_moveData.m_vecAngles.yaw        += m_flDeltaYaw;


    // NOTE : In Air conditions work a little differently and I have made a 
    //      quirky solution, I hope it works. ( for ground, changing view angles work like magic )
    if (m_iInitialFlags & FL_ONGROUND)
        return;

    
    // We must have forward & side move inputs as non zero. else no strafing would occur.
    constexpr float flMinMoveForStrafe = 1.0f;
    if (fabs(m_moveData.m_flSideMove) < flMinMoveForStrafe || fabs(m_moveData.m_flForwardMove) < flMinMoveForStrafe)
        return;

    // Target must be changing angles to strafe.
    if (fabs(m_flDeltaYaw) < 0.001f)
        return;

    // For strafing, the change-in-yaw per tick & angle of the "move" resultant vector must be of same sign.
    // Determined experimentally cause I ain't very methamatical :(
    float flMoveAngle   = RAD2DEG(atan2f(m_moveData.m_flForwardMove, m_moveData.m_flSideMove)) - 90.0f;
    bool  bHaveSameSign = (flMoveAngle * m_flDeltaYaw) > 0.0f;
    if (bHaveSameSign == false)
        return;

    vec vVelOriginal = m_moveData.m_vecVelocity;
    vVelOriginal.z   = 0.0f;
    float flSpeed2D  = vVelOriginal.Length2D();

    qangle qVelocity;
    Maths::VectorAnglesFromSDK(vVelOriginal, qVelocity);
    qVelocity.yaw += m_flDeltaYaw;

    // Making sure velocity yaw remains in valid range.
    {
        if (qVelocity.yaw > 180.0f)
        {
            qVelocity.yaw = -180.0f + std::fmodf(qVelocity.yaw, 180.0f);
        }
        else if (qVelocity.yaw < -180.0f)
        {
            qVelocity.yaw = 180 - std::fmod(fabs(qVelocity.yaw), 180.0f);
        }
    }

    // Converting back to velocity
    Maths::AngleVectors(qVelocity, &vVelOriginal);
    vVelOriginal.NormalizeInPlace();
    vVelOriginal *= flSpeed2D;
    vVelOriginal.z = m_moveData.m_vecVelocity.z;
    m_moveData.m_vecVelocity = vVelOriginal;
}


//=========================================================================
//                     PLAYER DATA BACKUP
//=========================================================================
PlayerDataBackup_t::PlayerDataBackup_t()
{
    Reset();
}

void PlayerDataBackup_t::Store(BaseEntity* pPlayer)
{
    pOldCmd             = pPlayer->m_pCurrentCommand();
    m_fFlags            = pPlayer->m_fFlags();

    m_flJumpTime        = pPlayer->m_flJumpTime();
    m_flDuckJumpTime    = pPlayer->m_flDuckJumpTime();
    m_flDuckTime        = pPlayer->m_flDucktime();
    m_bDucking          = pPlayer->m_bDucking();
    m_bDucked           = pPlayer->m_bDucked();
    m_bInDuckJump       = pPlayer->m_bInDuckJump();
    m_hGroundEntity     = pPlayer->m_hGroundEntity();

    m_vOrigin           = pPlayer->m_vecOrigin();
    m_vEyeOffset        = pPlayer->m_vecViewOffset();
    m_vVelocity         = pPlayer->m_vecVelocity();

    m_flModelScale      = pPlayer->m_flModelScale();
}

void PlayerDataBackup_t::Restore(BaseEntity* pPlayer)
{
    pPlayer->m_pCurrentCommand(pOldCmd);
    pPlayer->m_fFlags(m_fFlags);

    pPlayer->m_flDucktime(m_flDuckTime);
    pPlayer->m_flJumpTime(m_flJumpTime);
    pPlayer->m_flDuckJumpTime(m_flDuckJumpTime);
    pPlayer->m_bDucking(m_bDucking);
    pPlayer->m_bDucked(m_bDucked);
    pPlayer->m_bInDuckJump(m_bInDuckJump);
    pPlayer->m_hGroundEntity(m_hGroundEntity);

    pPlayer->m_vecOrigin(m_vOrigin);
    pPlayer->m_vecVelocity(m_vVelocity);
    pPlayer->m_vecViewOffset(m_vEyeOffset);

    pPlayer->m_flModelScale(m_flModelScale);
}

void PlayerDataBackup_t::Reset()
{
    pOldCmd = nullptr;
    m_fFlags = 0;
    m_flDuckTime = 0.0f;
    m_flJumpTime = 0.0f;
    m_flDuckJumpTime = 0.0f;   // 5
    m_bDucked = false;
    m_bDucking = false;
    m_bInDuckJump = false;
    m_hGroundEntity = 0;
    m_flModelScale = 0.0f;   // 10

    m_vOrigin.Init();
    m_vVelocity.Init();
    m_vEyeOffset.Init();
}

//=========================================================================
//                     DEBUG HOOKS
//=========================================================================
#if (DEBUG_MOVEMENT_SIM_HOOKS == true)

// RunCommand is correct
MAKE_HOOK_VTABLE(CPrediction_RunCommand, "VClientPrediction001", 17, __fastcall, CLIENT_DLL, void,
    void* pEntity, void* pCmd, void* pMoveHelper)
{
    Hook::CPrediction_RunCommand::O_CPrediction_RunCommand(pEntity, pCmd, pMoveHelper);
}

// Now look carefully, I didn't set up "current cmd" and it didn't even crash.
// cause it never reached that part, it just got yanked out cause it was stuck.
// maybe cause position was fucked up to start with? IDK
MAKE_HOOK(CGameMovement_Duck, "48 8B C4 48 89 58 ? 48 89 68 ? 48 89 70 ? 48 89 78 ? 41 54 41 56 41 57 48 81 EC ? ? ? ? 44 0F 29 48", __fastcall, CLIENT_DLL,
    int64_t, void* pVtable)
{
    FAIL_LOG("<---- Handling Ducking! ---->");
    return Hook::CGameMovement_Duck::O_CGameMovement_Duck(pVtable);
}

MAKE_HOOK(CTraceFilterObject_Constructor, "48 8D 05 ? ? ? ? 48 89 51 ? 48 89 01 48 8B C1 44 89 41", __fastcall, CLIENT_DLL,
    void*, void* pTraceFilter, int64_t idk1, int idk2, int64_t idk3)
{
    if (FeatureObj::movementSimulation.m_bSimulationRunning == true)
    {
        LOG("Construnting CTraceFilterObject @ [ %p ] %lld , %d , %lld", pTraceFilter, idk1, idk2, idk3);
    }
    else
    {
        WIN_LOG("Game's call to Contrustor::CTraceFilterObject @ [ %p ] %lld , %d , %lld", pTraceFilter, idk1, idk2, idk3);
    }

    return Hook::CTraceFilterObject_Constructor::O_CTraceFilterObject_Constructor(pTraceFilter, idk1, idk2, idk3);
}

MAKE_HOOK_VTABLE(CEngineTrace_TraceRay, "EngineTraceClient003", 4, __fastcall, ENGINE_DLL, void,
    void* idk1, void* pRay, int iMask, void* pTraceFilter, void* pTrace)
{
    // Gets called from CTFGameMovement::TracePlayerBBox();
    if (FeatureObj::movementSimulation.m_bSimulationRunning == true)
    {            
        LOG("OUR CALL : ray* %p | mask : %d | traceFilter : %p | trace : %p", pRay, iMask, pTraceFilter, pTrace);
        __debugbreak();
    }

    return Hook::CEngineTrace_TraceRay::O_CEngineTrace_TraceRay(idk1, pRay, iMask, pTraceFilter, pTrace);
}

MAKE_HOOK(CGameMovement_CatagorizeMovement, "40 56 48 81 EC ? ? ? ? 48 8B 41 ? 48 8B F1 C7 80", __fastcall, CLIENT_DLL,
    void*, void* idk1)
{
    if (FeatureObj::movementSimulation.m_bSimulationRunning == true)
        FAIL_LOG("|---------->CATAGARIZE_MOVEMENT()");

    return Hook::CGameMovement_CatagorizeMovement::O_CGameMovement_CatagorizeMovement(idk1);
}

MAKE_HOOK(CGameMovement_ReduceTimers, "48 8B 05 ? ? ? ? 0F 57 C0 33 D2", __fastcall, CLIENT_DLL,
    void*, void* idk1)
{
    auto result = Hook::CGameMovement_ReduceTimers::O_CGameMovement_ReduceTimers(idk1);
    
    if (FeatureObj::movementSimulation.m_bSimulationRunning == true)
        printf("|---------->CGameMovement::ReduceTimers()  DONE\n");

    return result;
}

MAKE_HOOK(CGameMovement_CheckStuck, "40 55 56 57 48 8D 6C 24 ? 48 81 EC ? ? ? ? 48 8B F1", __fastcall, CLIENT_DLL,
    void*, void* idk1)
{
    if (FeatureObj::movementSimulation.m_bSimulationRunning == true)
        FAIL_LOG("|---------->CGameMovement::CheckStuck()");

    auto result = Hook::CGameMovement_CheckStuck::O_CGameMovement_CheckStuck(idk1);
    
    if (FeatureObj::movementSimulation.m_bSimulationRunning == true)
        WIN_LOG("|---------->CGameMovement::CheckStuck()  DONE");

    return result;
}

int64_t iLastCheckInterval = 0;

// Gets called twice each tick, but will only get called once in failing tick
MAKE_HOOK(CGameMovement_CheckInterval, "85 D2 74 ? 83 EA ? 74 ? 83 FA ? 74 ? B8", __fastcall, CLIENT_DLL,
    int64_t, void* idk1, int idk2)
{
    auto result = Hook::CGameMovement_CheckInterval::O_CGameMovement_CheckInterval(idk1, idk2);
    
    iLastCheckInterval = result;
    if (FeatureObj::movementSimulation.m_bSimulationRunning == true)
        WIN_LOG("|---------->CGameMovement::CheckInterval()  DONE : %d", iLastCheckInterval);

    return result;
}

MAKE_HOOK(CGameMovement_EntIndex, "8B 41 ? C3 CC CC CC CC CC CC CC CC CC CC CC CC 8B 91 ? ? ? ? 85 D2 7F ? 33 C0", __fastcall, CLIENT_DLL,
    int, void* idk1)
{
    if (FeatureObj::movementSimulation.m_bSimulationRunning == true)
        FAIL_LOG("Requesting Ent index");

    int iEntIndex = Hook::CGameMovement_EntIndex::O_CGameMovement_EntIndex(idk1);

    if (FeatureObj::movementSimulation.m_bSimulationRunning == true)
        WIN_LOG("Ent Index : %d [ res. : %d ]", iEntIndex, (dummyCmd.command_number + iEntIndex) % iLastCheckInterval);

    return iEntIndex;
}

// Ain't getting called
//MAKE_HOOK(CGameMovement_TracePlayerBBox, "48 89 6C 24 ? 48 89 74 24 ? 48 89 7C 24 ? 41 56 48 81 EC ? ? ? ? 48 8B 05 ? ? ? ? 45 8B F1 49 8B F0", __fastcall, CLIENT_DLL,
//    void* , void* idk1, int idk2, int idk3, int idk4, int idk5, void* idk6)
//{
//    if (FeatureObj::movementSimulation.m_bSimulationRunning == true)
//        FAIL_LOG("Trying to trace player Bounding Box");
//
//    auto result = Hook::CGameMovement_TracePlayerBBox::O_CGameMovement_TracePlayerBBox(idk1, idk2, idk3, idk4, idk5, idk6);
//
//    if (FeatureObj::movementSimulation.m_bSimulationRunning == true)
//        WIN_LOG("successFully trace player bbox");
//
//    return result;
//}

MAKE_HOOK(CGameMovement_TryPlayerMove, "4C 8B DC 49 89 5B ? 49 89 53", __fastcall, CLIENT_DLL,
    void* , void* rax, void* idk1, void* idk2, float idk3)
{
    if (FeatureObj::movementSimulation.m_bSimulationRunning == true)
        FAIL_LOG("TryPlayerMove");

    auto result = Hook::CGameMovement_TryPlayerMove::O_CGameMovement_TryPlayerMove(rax, idk1, idk2, idk3);

    if (FeatureObj::movementSimulation.m_bSimulationRunning == true)
        WIN_LOG("TryPlayerMove DONE");

    return result;
}

//======================= PLAYER MOVE =======================

// CGameMovement -> PlayerMove
MAKE_HOOK(CGameMovement_PlayerMove, "48 89 6C 24 ? 48 89 74 24 ? 57 48 83 EC ? 48 8B 01 48 8B F9 FF 90", __fastcall, CLIENT_DLL,
    int64_t, void* pVTable)
{
    if (FeatureObj::movementSimulation.m_bSimulationRunning == true)
    {
        printf("|------>CGameMovement_PlayerMove\n");
        //printf("Breaking...\n");
        //__debugbreak();
    }

    auto result = Hook::CGameMovement_PlayerMove::O_CGameMovement_PlayerMove(pVTable);

    WIN_LOG("!! playermove done !!");
    
    return result;
}

MAKE_HOOK(CTFGameMovement_PlayerMove, "48 89 5C 24 ? 48 89 74 24 ? 57 48 81 EC ? ? ? ? 48 8B D9 BA", __fastcall, CLIENT_DLL,
    int64_t, void* pVTable)
{
    if (FeatureObj::movementSimulation.m_bSimulationRunning == true)
    {
        printf("|------>CTFGameMovement_PlayerMove\n");
    }

    return Hook::CTFGameMovement_PlayerMove::O_CTFGameMovement_PlayerMove(pVTable);
}


//======================= PROCESS MOVES =======================
// CTFGameMovement -> ProcessMovement
MAKE_HOOK(CTFGameMovement_ProcessMovement, "48 85 D2 0F 84 ? ? ? ? 48 89 5C 24 ? 48 89 74 24 ? 57 48 83 EC ? 49 8B F0 48 8B FA 48 8B D9 4D 85 C0", __fastcall, CLIENT_DLL,
    void, void* idk1, void* idk2, void* idk3)
{
    if (FeatureObj::movementSimulation.m_bSimulationRunning == true)
    {
        printf("->CTFGameMovement_ProcessMovement\n");
    }

    Hook::CTFGameMovement_ProcessMovement::O_CTFGameMovement_ProcessMovement(idk1, idk2, idk3);
}

// CGameMovement -> ProcessMovement
MAKE_HOOK(CGameMovement_ProcessMovement, "48 89 5C 24 ? 48 89 74 24 ? 57 48 83 EC ? 48 8B 05 ? ? ? ? 49 8B D8", __fastcall, CLIENT_DLL,
    void, void* idk1, void* idk2, void* idk3)
{
    if (FeatureObj::movementSimulation.m_bSimulationRunning == true)
    {
        printf("->C_GameMovement_ProcessMovement\n");
    }

    Hook::CGameMovement_ProcessMovement::O_CGameMovement_ProcessMovement(idk1, idk2, idk3);
}

// Dayem!, even this didn't get called :(
MAKE_HOOK(ResetGetPointsContentCache, "48 8D 41 ? B9", __fastcall, CLIENT_DLL, void*, void* idk1)
{
    if (FeatureObj::movementSimulation.m_bSimulationRunning == true)
        printf("|--->ResetGetPointsContentCache\n");

    return Hook::ResetGetPointsContentCache::O_ResetGetPointsContentCache(idk1);
}

MAKE_HOOK(CGameMovement_FullWalkMove, "48 89 5C 24 ? 57 48 83 EC ? 0F 29 74 24 ? 48 8B D9 E8 ? ? ? ? 0F 57 F6 84 C0", __fastcall, CLIENT_DLL, void*, void* idk1)
{
    FAIL_LOG("|--------->CGameMovement::FullWalkMove()");
    auto result = Hook::CGameMovement_FullWalkMove::O_CGameMovement_FullWalkMove(idk1);
    WIN_LOG("|--------->CGameMovement::FullWalkMove() DONE");
    return result;
}

MAKE_HOOK(CGameMovement_AirMove, "48 8B C4 53 48 81 EC ? ? ? ? 0F 29 70 ? 4C 8D 48", __fastcall, CLIENT_DLL, void*, void* idk1)
{
    FAIL_LOG("|---------------->CGameMovement::AirMove()");
    auto result = Hook::CGameMovement_AirMove::O_CGameMovement_AirMove(idk1);
    WIN_LOG("|---------------->CGameMovement::AirMove() DONE");
    return result;
}

MAKE_HOOK(CGameMovement_AirAccelerate, "48 89 5C 24 ? 57 48 83 EC ? 48 8B 41 ? 48 8B FA 0F 29 74 24", __fastcall, CLIENT_DLL, 
    void, void* rax, void* idk1, float idk2, float idk3)
{
    printf("Calling AirAccelerate\n");
    Hook::CGameMovement_AirAccelerate::O_CGameMovement_AirAccelerate(rax, idk1, idk2, idk3);
}

#endif