#include "Movement.h"
//#include "../config.h"

// SDK
#include "../../SDK/class/Source Entity.h"
#include "../../SDK/class/CUserCmd.h"
#include "../../SDK/class/BaseWeapon.h"
#include "../../SDK/class/IInputSystem.h"
#include "../../SDK/class/IVDebugOverlay.h"
#include "../../SDK/class/CommonFns.h"
#include "../../SDK/class/CPrediction.h"
#include "../../SDK/TF object manager/TFOjectManager.h"
#include "../Tick Shifting/TickShifting.h"

// UTILITY
#include "../../Utility/CVar Handler/CVarHandler.h"
#include "../../Utility/Profiler/Profiler.h"
#include "../../Utility/Hook Handler/Hook_t.h"
#include "../../Extra/math.h"



///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
Movement_t::Movement_t()
{
    m_bInitializedKeyCodes = false;
    m_iJumpKeyCode		   = 0;

    Reset();
}


///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
void Movement_t::Run(BaseEntity* pLocalPlayer, baseWeapon* pActiveWeapon, CUserCmd* pCmd, bool& result)
{
    PROFILER_RECORD_FUNCTION(CreateMove);

    _InitializeKeyCodes();

    // MOVE!!!
    _Bhop		(pCmd, result, pLocalPlayer);
    _RocketJump (pCmd, result, pActiveWeapon, pLocalPlayer);
    _ThirdPerson(pCmd, result, pLocalPlayer);
    _AutoStrafer(pLocalPlayer, pCmd);
    _AutoStop	(pLocalPlayer, pCmd, pActiveWeapon);
} 


///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
void Movement_t::Reset()
{
    m_bLastBhopState = false;
}


///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
void Movement_t::_Bhop(CUserCmd* pCmd, bool& result, BaseEntity* pLocalPlayer)
{
    bool bActive		  = Features::Movement::Movement::Bhop.IsActive();
    bool bOnGround		  = pLocalPlayer->m_fFlags() & (1 << 0);
    bool bKeysOverlapping = (m_iJumpKeyCode == Features::Movement::Movement::Bhop.m_iKey);

    if (bOnGround == true && bActive == true)
        pCmd->buttons |= IN_JUMP;

    // If keybind for jump & Bhop are same, then disable jumping Mid-Air ( double jumping ) until
    // user releases the jump key.
    if (bOnGround == false && bKeysOverlapping == true && m_bLastBhopState == bActive)
    {
        pCmd->buttons &= ~IN_JUMP;
    }

    // Store this bhop state
    m_bLastBhopState = bActive;
}


///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
void Movement_t::_RocketJump(CUserCmd* pCmd, bool& result, baseWeapon* pActiveWeapon, BaseEntity* pLocalPlayer)
{
    static int iRocketJumpingStage = 0;
    if (Features::Movement::Movement::AutoRocketJump.IsActive() == false)
    {
        iRocketJumpingStage = 0;
        return;
    }

    // Can Rocket jump?
    if (pLocalPlayer->m_iClass() != TF_SOLDIER || pActiveWeapon->getSlot() != WPN_SLOT_PRIMARY)
        return;

    // If Got no ammo, then return
    if (pActiveWeapon->m_iClip1() <= 0)
        return;

    switch (iRocketJumpingStage)
    {
    case 0 :
        pCmd->buttons |= IN_JUMP;
        pCmd->buttons |= IN_DUCK;
        ++iRocketJumpingStage;
        // Canceling reload
        if (pActiveWeapon->m_iReloadMode() != reload_t::WPN_RELOAD_START)
            pCmd->buttons |= IN_ATTACK;
        break;

    case 1 :
        pCmd->viewangles.yaw += 180.0f;
        Maths::ClampQAngle(pCmd->viewangles);
        pCmd->buttons |= IN_ATTACK;
        ++iRocketJumpingStage;
        result = false;
        break;

    default:
        break;
    }
}


///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
void Movement_t::_ThirdPerson(CUserCmd* pCmd, bool& result, BaseEntity* pLocalPlayer)
{
    pLocalPlayer->m_nForceTauntCam(Features::Movement::Movement::ThirdPerson.IsActive());
}


///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
void Movement_t::_AutoStrafer(BaseEntity* pLocalPlayer, CUserCmd* pCmd)
{
    if (Features::Movement::Movement::DirectionStrafe.IsActive() == false)
        return;

    // is in air ?
    if (pLocalPlayer->m_fFlags() & FL_ONGROUND)
        return;

    // Does user wanna strafe ?
    if (pCmd->forwardmove == 0.0f && pCmd->sidemove == 0.0f)
        return;

    vec vVelocity = pLocalPlayer->m_vecVelocity();
    vVelocity.z   = 0.0f;

    // Getting the angles where user wants to strafe to.
    qangle qFinalAngles;
    {
        vec vMoves(pCmd->forwardmove, pCmd->sidemove, 0.0f);
        qangle qMoves;
        Maths::VectorAnglesFromSDK(vMoves, qMoves);

        qMoves.yaw *= -1.0f;
        Maths::WrapYaw(qMoves);

        qFinalAngles.yaw = pCmd->viewangles.yaw + qMoves.yaw;
        Maths::WrapYaw(qFinalAngles);
    }

    // Current Velocity angles.
    qangle qVelocity;  Maths::VectorAnglesFromSDK(vVelocity, qVelocity); Maths::WrapYaw(qVelocity);
    
    // Angle from Current Velocity to "Where we wanna go" ( signed )
    float flDelta = Maths::DeltaAngle(qVelocity.yaw, qFinalAngles.yaw);
    if (fabs(flDelta) <= 5.0f)
        return;

    // Constructing Wish Direction ( 90 degrees more than velocity direction, in the side where we wanna go. )
    float flWishDirYaw = qVelocity.yaw;
    flWishDirYaw      += (flDelta >= 0.0f ? 90.0f : -90.0f) + (flDelta * Features::Movement::Movement::AutoStrafe_Agression.GetData().m_flVal);
    flWishDirYaw	   = Maths::DeltaAngle(pCmd->viewangles.yaw, flWishDirYaw);

    // Converting Wish direction to appropriate Forward & Side move.
    constexpr float flMaxMoveMagnitude = MAX_MOVE_USERCMD * 1.41421356237f;
    pCmd->forwardmove = std::clamp<float>(cosf(DEG2RAD(flWishDirYaw)) * flMaxMoveMagnitude, -450.0f, 450.0f);
    pCmd->sidemove    = std::clamp<float>(sinf(DEG2RAD(flWishDirYaw)) * flMaxMoveMagnitude, -450.0f, 450.0f) * -1.0f;
}


///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
void Movement_t::_AutoStop(BaseEntity* pLocalPlayer, CUserCmd* pCmd, baseWeapon* pActiveWeapon)
{
    if (Features::TickShifter::TickShifter::DoubleTab_AutoStop.IsActive() == false)
        return;

    if (F::tickShifter.DoubleTapping() == false)
        return;

    // some basic sanity checks.
    if ((pLocalPlayer->m_fFlags() & FL_ONGROUND) == false || pActiveWeapon->getSlot() == WPN_SLOT_MELLE)
        return;


    // View angles. Flat.
    qangle qViewAnglesFlat = pCmd->viewangles; qViewAnglesFlat.pitch = 0.0f;
    vec vForward, vRight, vUp; Maths::AngleVectors(qViewAnglesFlat, &vForward, &vRight, &vUp);
    vForward.NormalizeInPlace(); vRight.NormalizeInPlace(); // NOTE : Up component isn't useful here.


    // Velocity. Flat.
    vec  vVelFlat  = pLocalPlayer->m_vecAbsVelocity(); vVelFlat.z = 0.0f;
    vVelFlat.NormalizeInPlace();

    // Set forward & move in opposite direction of velocity.
    pCmd->forwardmove = std::clamp<float>(-1.0f * MAX_MOVE_USERCMD * vVelFlat.Dot(vForward), -MAX_MOVE_USERCMD, MAX_MOVE_USERCMD);
    pCmd->sidemove    = std::clamp<float>(-1.0f * MAX_MOVE_USERCMD * vVelFlat.Dot(vRight),   -MAX_MOVE_USERCMD, MAX_MOVE_USERCMD);
}


///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
void Movement_t::_InitializeKeyCodes()
{
    if (m_bInitializedKeyCodes == true)
        return;

    m_iJumpKeyCode = I::iInputSystem->VirtualKeyForInput("+jump");

    m_bInitializedKeyCodes = true;
}



//=========================================================================
//                     DEBUGGING HOOKS
//=========================================================================
#if (false)

MAKE_HOOK(CPrediction_FinishMove, "48 89 5C 24 ? 48 89 6C 24 ? 48 89 74 24 ? 48 89 7C 24 ? 41 56 48 83 EC ? 41 8B 41", __fastcall, CLIENT_DLL, void*,
    void* pThis, BaseEntity* pPlayer, CUserCmd* ucmd, void* move)
{
    LOG("Finished move. Shifting : %d", F::tickShifter.ShiftingTicks());

    return Hook::CPrediction_FinishMove::O_CPrediction_FinishMove(pThis, pPlayer, ucmd, move);
}

//MAKE_HOOK_VTABLE(CPrediction_RunCommand, "VClientPrediction001", 17, __fastcall, CLIENT_DLL, void,
//    void* pEntity, void* pCmd, void* pMoveHelper)
//{
//    LOG("Running command");
//    Hook::CPrediction_RunCommand::O_CPrediction_RunCommand(pEntity, pCmd, pMoveHelper);
//}

MAKE_HOOK(CPrediction__Update, "40 53 41 54 41 55 41 56 41 57 48 83 EC ? 45 8B F9", __fastcall, CLIENT_DLL, void*,
    __int64* a1, unsigned __int8 a2, char a3, int a4, int a5)
{
    LOG("Updating local player");
    return Hook::CPrediction__Update::O_CPrediction__Update(a1, a2, a3, a4, a5);
}

MAKE_HOOK(CL_RunPrediction, "4C 8B DC 49 89 5B ? 57 48 83 EC ? 48 8B 1D ? ? ? ? 33 FF 49 89 7B ? 48 8B 03 48 85 C0 74 ? 48 8D 0D ? ? ? ? 45 33 C9 49 89 4B ? 49 8D 53 ? 48 8D 0D ? ? ? ? 45 33 C0 49 89 4B ? 48 8D 0D ? ? ? ? 49 89 4B ? 48 8D 0D ? ? ? ? C7 44 24 ? ? ? ? ? 49 89 4B ? 48 8B C8 49 89 7B ? FF 90 ? ? ? ? 48 8B 7C 24 ? 48 8B 1D ? ? ? ? 83 3D", __stdcall, ENGINE_DLL, void*)
{
    LOG("Running local player prediction!");
 
    return Hook::CL_RunPrediction::O_CL_RunPrediction();
}

#endif