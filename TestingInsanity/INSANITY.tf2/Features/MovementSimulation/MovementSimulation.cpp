#include "MovementSimulation.h"

// UTILITY
#include "../../Extra/math.h"
#include "../../SDK/TF object manager/TFOjectManager.h"
#include "../../Utility/signatures.h"
#include "../../Utility/Interface.h"

/*
NOTES : 
    CMoveData : We need to fill up some stuff, and the rest will be handled by the Engine.
                We also need to sanitize some of it, like ducking and shit cause the code
                for handled ducking is very big and messy.

    Player  :   On the player we run the Movement sim, the Engine will update and mess up 
                stuff, and we need to let it be that way while the simulation is running.
                Once its done, we need to restore it back to original, else bullshit :).
*/

MAKE_SIG(CTFGameMovement, "48 85 D2 0F 84 ? ? ? ? 48 89 5C 24 ? 48 89 74 24 ? 57 48 83 EC ? 49 8B F0 48 8B FA 48 8B D9 4D 85 C0", CLIENT_DLL,
    void, void*, BaseEntity*, CMoveData*);
MAKE_INTERFACE_VERSION(iGameMovement, "GameMovement001", void, CLIENT_DLL);


void MovementSimulation_t::Reset()
{
    m_bOldInPrediction       = false;
    m_bOldFirstTimePredicted = false;
    m_flOldFrameTime         = 0.0f;
    pPlayer = nullptr;
    m_bInitialized = false;
}


bool MovementSimulation_t::Initialize(BaseEntity* pEnt)
{
    // Store Original Prediction data & frame time.
    m_bOldInPrediction       = I::cPrediction->m_bInPrediction;
    m_bOldFirstTimePredicted = I::cPrediction->m_bFirstTimePredicted;
    m_flOldFrameTime         = tfObject.pGlobalVar->frametime;

    m_vOldPos = pEnt->GetAbsOrigin();
    LOG_VEC3(m_vOldPos);

    // Setting everything to 0 & filling in important stuff.
    _ResetMoveData(moveData);
    _SetupMoveData(moveData, pEnt);

    // Absolutely Remove Ducking, use m_fFlags to determing Ducking.

    m_bInitialized = true;
    return true;
}


void MovementSimulation_t::RunTick()
{
    I::cPrediction->m_bInPrediction       = true;
    I::cPrediction->m_bFirstTimePredicted = false;
    tfObject.pGlobalVar->frametime        = tfObject.pGlobalVar->interval_per_tick;

    Sig::CTFGameMovement(I::iGameMovement, pPlayer, &moveData);

    // Drawing what ever position engine generated form our input
    const     vec vOrigin = moveData.GetAbsOrigin();
    constexpr vec vSize(5.0f, 5.0f, 5.0f);
    I::IDebugOverlay->AddBoxOverlay(vOrigin, vOrigin - vSize, vOrigin + vSize, qangle(0.0f, 0.0f, 0.0f), 255, 255, 255, 40, 10.0f);
    LOG_VEC3(vOrigin);
    printf("DIST : %.2f\n", m_vOldPos.DistTo(vOrigin));
}


void MovementSimulation_t::Restore()
{
    I::cPrediction->m_bInPrediction       = m_bOldInPrediction;
    I::cPrediction->m_bFirstTimePredicted = m_bOldFirstTimePredicted;
    tfObject.pGlobalVar->frametime        = m_flOldFrameTime;

    Reset();
    m_bInitialized = false;
};


// Before anyone looks at this and thinks, why TF did I made this
// Just know, that I don't fucking know. I need to get started, and 
// doing something is better than doing nothing :).
void MovementSimulation_t::_ResetMoveData(CMoveData& moveData)
{
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

    moveData.m_vecAbsOrigin             = { 0.0f, 0.0f, 0.0f };

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
}