#include "MovementSimulation.h"

// UTILITY
#include "../../Extra/math.h"
#include "../../SDK/TF object manager/TFOjectManager.h"
#include "../../Utility/signatures.h"
#include "../../Utility/Interface.h"
#include "../../Utility/Hook_t.h"

//SDK
#include "../../SDK/class/CUserCmd.h"

#define DEBUG_MOVEMENT_SIM_HOOKS true

// Fns
MAKE_SIG(CTFGameMovement_ProcessMovement, "48 85 D2 0F 84 ? ? ? ? 48 89 5C 24 ? 48 89 74 24 ? 57 48 83 EC ? 49 8B F0 48 8B FA 48 8B D9 4D 85 C0", CLIENT_DLL,
    void, void*, BaseEntity*, CMoveData*);
MAKE_INTERFACE_VERSION(iGameMovement, "GameMovement001", void, CLIENT_DLL);

static CUserCmd dummyCmd;

void MovementSimulation_t::Reset()
{
    m_bOldInPrediction       = false;
    m_bOldFirstTimePredicted = false;
    m_flOldFrameTime         = 0.0f;
    m_pPlayer                = nullptr;
    m_bInitialized           = false;
}

MAKE_SIG(PlayerMove, "48 89 6C 24 ? 48 89 74 24 ? 57 48 83 EC ? 48 8B 01 48 8B F9 FF 90", CLIENT_DLL, void*, void*)

bool MovementSimulation_t::Initialize(BaseEntity* pEnt, CUserCmd* pCmd)
{
    // Store Original Prediction data & frame time.
    m_bOldInPrediction       = I::cPrediction->m_bInPrediction;
    m_bOldFirstTimePredicted = I::cPrediction->m_bFirstTimePredicted;
    m_flOldFrameTime         = tfObject.pGlobalVar->frametime;
    m_pPlayer                = pEnt;
    
    // Backing up player data
    m_playerDataBackup.Reset();
    m_playerDataBackup.Store(m_pPlayer);

    // Adjusting player data temporarily
    m_pPlayer->m_pCurrentCommand(&dummyCmd);
    m_pPlayer->m_hGroundEntity(NULL);

    // Setting up Move Data
    _ResetMoveData(moveData);
    _SetupMoveDataLocal(moveData, m_pPlayer, pCmd);
    
    // Handling Ducking ( Glitching )
    _HandleDuck(m_pPlayer);

    m_bSimulationRunning = true;
    m_bInitialized = true;
    return true;
}


void MovementSimulation_t::_HandleDuck(BaseEntity* pEnt)
{
   /* int iFlags = pEnt->m_fFlags();
    for (int i = 0; i < 32; i++)
    {
        printf("%d", iFlags & (1 << (31 - i)) ? 1 : 0);
        if ((i + 1) % 4 == 0)
            printf(" ");
        if ((i + 1) % 8 == 0)
            printf(" ");
    }
    printf("\n");*/

    if (pEnt->m_fFlags() & FL_DUCKING)
    {
        pEnt->m_flDucktime(0.0f);
        pEnt->m_flDuckJumpTime(0.0f);
        //pEnt->m_flJumpTime(0.0f);
        pEnt->m_bDucking(false);
        pEnt->m_bDucked(true);
        pEnt->m_bInDuckJump(false);
            
        auto pFlags = reinterpret_cast<uint32_t*>(reinterpret_cast<uintptr_t>(pEnt) + Netvars::DT_BasePlayer::m_fFlags);
        //*pFlags &= ~FL_ANIMDUCKING;
        *pFlags &= ~FL_DUCKING;
        // NOTE : We are also supposed to remove the IN_DUCK bit from UserCmd->Buttons, but 
        // CurrentCommand is already set to an empty CUserCmd
    }
}


void MovementSimulation_t::RunTick()
{
    I::cPrediction->m_bInPrediction       = true;
    I::cPrediction->m_bFirstTimePredicted = false;
    tfObject.pGlobalVar->frametime        = I::cPrediction->m_bEnginePaused == true ? 0 : tfObject.pGlobalVar->interval_per_tick;

    //printf("MoveType : %d\n", *(int*)(reinterpret_cast<uintptr_t>(m_pPlayer) + 0x224));
    Sig::CTFGameMovement_ProcessMovement(I::iGameMovement, m_pPlayer, &moveData);
    
    // Drawing line connecting all predicted positions
    if (m_vLastSimulatedPos.IsEmpty() == false)
    {
        I::IDebugOverlay->AddLineOverlay(
            m_vLastSimulatedPos,        // Start
            moveData.m_vecAbsOrigin,    // End
            255, 255, 255,              // Color
            false, 10.0f                // Depth test & duration
        );
    }
    m_vLastSimulatedPos = moveData.m_vecAbsOrigin;
}


void MovementSimulation_t::Restore()
{
    I::cPrediction->m_bInPrediction       = m_bOldInPrediction;
    I::cPrediction->m_bFirstTimePredicted = m_bOldFirstTimePredicted;
    tfObject.pGlobalVar->frametime        = m_flOldFrameTime;

    m_bSimulationRunning = false;
    m_vLastSimulatedPos.Init();

    // RESTORING PLAYERS DATA
    m_pPlayer->m_pCurrentCommand(m_playerDataBackup.pOldCmd);
    
    m_pPlayer->m_fFlags(m_playerDataBackup.m_fFlags);

    m_pPlayer->m_flDucktime(m_playerDataBackup.m_flDuckTime);
    m_pPlayer->m_flJumpTime(m_playerDataBackup.m_flJumpTime);
    m_pPlayer->m_flDuckJumpTime(m_playerDataBackup.m_flDuckJumpTime);
    m_pPlayer->m_bDucking(m_playerDataBackup.m_bDucking);
    m_pPlayer->m_bDucked(m_playerDataBackup.m_bDucked);
    m_pPlayer->m_bInDuckJump(m_playerDataBackup.m_bInDuckJump);
    m_pPlayer->m_hGroundEntity(m_playerDataBackup.m_hGroundEntity);
    
    m_pPlayer->m_vecOrigin(m_playerDataBackup.m_vOrigin);
    m_pPlayer->m_vecVelocity(m_playerDataBackup.m_vVelocity);
    m_pPlayer->m_vecViewOffset(m_playerDataBackup.m_vEyeOffset);

    Reset();
    m_bInitialized = false;
};


//=========================================================================
//                     PRIVATE METHODS
//=========================================================================

// Before anyone looks at this and thinks, why TF did I made this
// Just know, that I don't fucking know. I need to get started, and 
// doing something is better than doing nothing :).
void MovementSimulation_t::_ResetMoveData(CMoveData& moveData)
{
    memset(&moveData, 0, sizeof(CMoveData));

    /*
    moveData.m_bFirstRunOfFunctions     = false;
    moveData.m_bGameCodeMovedPlayer     = false;
    moveData.m_nPlayerHandle            = 0;
    moveData.m_nImpulseCommand          = 0;

    moveData.m_vecViewAngles            = {0.0f, 0.0f, 0.0f};
    moveData.m_vecAbsViewAngles         = { 0.0f, 0.0f, 0.0f };
    moveData.m_nButtons                 = 0;
    moveData.m_nOldButtons              = 0;
    moveData.m_flForwardMove            = 0.0f;
    moveData.m_flOldForwardMove         = 0.0f;
    moveData.m_flSideMove               = 0.0f;
    moveData.m_flUpMove                 = 0.0f;

    moveData.m_flMaxSpeed               = 0.0f;
    moveData.m_flClientMaxSpeed         = 0.0f;

    moveData.m_vecVelocity              = { 0.0f, 0.0f, 0.0f };
    moveData.m_vecAngles                = { 0.0f, 0.0f, 0.0f };
    moveData.m_vecOldAngles             = { 0.0f, 0.0f, 0.0f };
    
    moveData.m_outStepHeight            = 0.0f;
    moveData.m_outWishVel               = { 0.0f, 0.0f, 0.0f };
    moveData.m_outJumpVel               = { 0.0f, 0.0f, 0.0f };

    moveData.m_vecConstraintCenter      = { 0.0f, 0.0f, 0.0f };
    moveData.m_flConstraintRadius       = 0.0f;
    moveData.m_flConstraintWidth        = 0.0f;
    moveData.m_flConstraintSpeedFactor  = 0.0f;

    moveData.m_vecAbsOrigin             = { 0.0f, 0.0f, 0.0f };*/

}


void MovementSimulation_t::_SetupMove(BaseEntity* pEnt)
{
    moveData.m_bFirstRunOfFunctions = false; // Keep it false, else will do extra bullshit and performance loss :(
    moveData.m_bGameCodeMovedPlayer = false; // Keep false, else calls CatagorizeMovement() & I don't know WTF is that :)

    moveData.m_nPlayerHandle    = pEnt->GetRefEHandle();
    moveData.m_vecAbsOrigin     = pEnt->GetAbsOrigin();
    moveData.m_vecVelocity      = pEnt->m_vecVelocity();

    moveData.m_vecViewAngles    = pEnt->GetAbsAngles();
    moveData.m_vecAbsViewAngles = pEnt->GetAbsAngles();
    
    // Calculating body angle ( not same as shooting angles )
    // Since we gonna have to make strafe prediction separately, this doesn't matter
    Maths::VectorAnglesFromSDK(moveData.m_vecVelocity, moveData.m_vecAngles);

    // Setup Forward & Side move and we are golden :)
}


void MovementSimulation_t::_SetupMoveData(CMoveData& moveData, BaseEntity* pEnt)
{
    moveData.m_bFirstRunOfFunctions = false;
    moveData.m_bGameCodeMovedPlayer = false;

    moveData.m_nPlayerHandle = pEnt->GetRefEHandle();
    moveData.m_vecAbsOrigin  = pEnt->GetAbsOrigin();
    moveData.m_vecAngles     = pEnt->GetRenderAngles(); // Confused between ABS angles and Render angles.
    moveData.m_vecVelocity   = pEnt->m_vecVelocity();

    // The UC guy has only set the yaw for vecAngles, but not the pitch. 
    // I yanked both of em here :).
    qangle qVelocity;
    Maths::VectorAnglesFromSDK(moveData.m_vecVelocity, qVelocity);
    moveData.m_vecAbsViewAngles = { qVelocity.pitch, qVelocity.yaw, 0.0f };
    moveData.m_vecViewAngles    = moveData.m_vecAbsViewAngles;

    // Forward Move & Side Move
    vec vForward, vRight;
    Maths::AngleVectors(moveData.m_vecViewAngles, &vForward, &vRight);

    // Ripped of starigh from @Sonixz
    moveData.m_flForwardMove = (moveData.m_vecVelocity.y - vRight.y / vRight.x * moveData.m_vecVelocity.x) / (vForward.y - vRight.y / vRight.x * vForward.x);
    moveData.m_flSideMove    = (moveData.m_vecVelocity.x - vForward.x * moveData.m_flForwardMove) / vRight.x;

    printf("Forward move : %.2f, SideMove : %.2f\n", moveData.m_flForwardMove, moveData.m_flSideMove);
}

void MovementSimulation_t::_SetupMoveDataLocal(CMoveData& moveData, BaseEntity* pEnt, CUserCmd* pCmd)
{
    moveData.m_bFirstRunOfFunctions = false;
    moveData.m_bGameCodeMovedPlayer = false;

    moveData.m_nPlayerHandle        = pEnt->GetRefEHandle();
    moveData.m_vecAbsOrigin         = pEnt->GetAbsOrigin();
    moveData.m_vecAngles            = pEnt->GetAbsAngles();
    moveData.m_vecVelocity          = pEnt->m_vecVelocity();

    moveData.m_vecViewAngles        = pCmd->viewangles;
    moveData.m_vecAbsViewAngles     = pCmd->viewangles;

    moveData.m_flForwardMove        = pCmd->forwardmove;
    moveData.m_flSideMove           = pCmd->sidemove;

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
    if (FeatureObj::movementSimulation.m_bSimulationRunning == true)
        FAIL_LOG("|---------->CGameMovement::ReduceTimers()");

    return Hook::CGameMovement_ReduceTimers::O_CGameMovement_ReduceTimers(idk1);
}

//======================= PLAYER MOVE =======================

// CGameMovement -> PlayerMove
MAKE_HOOK(CGameMovement_PlayerMove, "48 89 6C 24 ? 48 89 74 24 ? 57 48 83 EC ? 48 8B 01 48 8B F9 FF 90", __fastcall, CLIENT_DLL,
    int64_t, void* pVTable)
{
    if (FeatureObj::movementSimulation.m_bSimulationRunning == true)
    {
        printf("|------>CGameMovement_PlayerMove\n");
    }

    return Hook::CGameMovement_PlayerMove::O_CGameMovement_PlayerMove(pVTable);
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

MAKE_HOOK(CGameMovement_FullWalkMove, "48 8B C4 48 89 58 ? 48 89 68 ? 48 89 70 ? 48 89 78 ? 41 56 48 83 EC ? 0F 29 70 ? 4C 8D 71", __fastcall, CLIENT_DLL, void*, void* idk1)
{
    printf("|--------->CGameMovement::FullWalkMove()\n");
    return Hook::CGameMovement_FullWalkMove::O_CGameMovement_FullWalkMove(idk1);
}



#endif

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
}

void PlayerDataBackup_t::Reset()
{
    pOldCmd = nullptr;
    m_fFlags = 0;
    m_flDuckTime = 0.0f;
    m_flJumpTime = 0.0f;
    m_flDuckJumpTime = 0.0f;
    m_bDucked = false;
    m_bDucking = false;
    m_bInDuckJump = false;
    m_hGroundEntity = 0;

    m_vOrigin.Init();
    m_vEyeOffset.Init();
    m_vVelocity.Init();
}