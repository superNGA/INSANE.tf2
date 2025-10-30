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
#include "../../SDK/class/IEngineTrace.h"
#include "../../SDK/TF object manager/TFOjectManager.h"
#include "../Tick Shifting/TickShifting.h"

// RENDERER
#include "../Graphics Engine V2/Draw Objects/Box/Box.h"

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
    _PeekAssist (pLocalPlayer, pCmd, pActiveWeapon);
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
void Movement_t::_PeekAssist(BaseEntity* pLocalPlayer, CUserCmd* pCmd, baseWeapon* pActiveWeapon)
{
    static bool s_bReturnOriginSaved = false;
    static bool s_bReturnActive      = false;
    static vec  s_vReturnOrigin(0.0f, 0.0f, 0.0f);
    static vec  s_vMarkerOrigin(0.0f);
    static qangle s_qSurfaceNormal(0.0f);
    static BoxFilled3D_t* s_pMarker = nullptr;
    
    // Initializing marker draw obj if null
    if (s_pMarker == nullptr)
    {
        s_pMarker = new BoxFilled3D_t();
        
        if (s_pMarker == nullptr)
            return;
    }


    // no return origin is inactive.
    if (Features::Movement::Movement::PeekAssist.IsActive() == false)
    {
        s_bReturnOriginSaved = false;
        s_bReturnActive      = false;
        s_pMarker->SetVisible(false);
        return;
    }


    // While double tapping don't interfere, only do anything when double tapping is over.
    if (F::tickShifter.DoubleTapping() == true)
        return;


    // Only store return origin on the first iteration.
    if (s_bReturnOriginSaved == false)
    {
        s_vReturnOrigin = pLocalPlayer->GetAbsOrigin();

        // ray trace to get the surface normal ( for drawing marker )
        ITraceFilter_IgnoreSpawnVisualizer filter(pLocalPlayer); trace_t trace;
        vec vTraceEndPos = pLocalPlayer->GetAbsOrigin(); vTraceEndPos.z -= 200.0f; // To make sure the end pos is in the fucking ground.
        I::EngineTrace->UTIL_TraceRay(pLocalPlayer->GetEyePos(), vTraceEndPos, MASK_SHOT, &filter, &trace);
        
        // Store point on ground ( where ray ended ) cause GetAbsOrigin() is a little above the ground.
        s_vMarkerOrigin = trace.m_end;
        
        // if we don't do this, the circle will draw in the direction or normal & not perpendicular to it.
        Maths::VectorAnglesFromSDK(trace.m_plane.normal, s_qSurfaceNormal);
        s_qSurfaceNormal.pitch += 90.0f;
        
        s_bReturnOriginSaved = true;
    }

    
    // Start return to saved origin once we shoot.
    bool bShotThisTick = SDK::CanAttack(pLocalPlayer, pActiveWeapon) == true && (pCmd->buttons & IN_ATTACK) == true;
    if (bShotThisTick == true)
    {
        s_bReturnActive = true;
    }


    // Drawing marker for return position.
    {
        s_pMarker->SetVisible(true);
        s_pMarker->SetRounding(0.5f); // to make it a circle.
        s_pMarker->SetBlur(-1);       // on 3D objects, this looks like shit.

        // Points
        float flMarkerRadius = Features::Movement::Movement::PeekAssist_Size.GetData().m_flVal; 
        s_pMarker->SetVertex(s_vMarkerOrigin, vec(-flMarkerRadius), vec(flMarkerRadius), s_qSurfaceNormal);

        // Colors
        RGBA_t clr = Features::Movement::Movement::PeekAssist_Clr.GetData().GetAsBytes(); 
        s_pMarker->SetColor(clr.r, clr.g, clr.b, clr.a);
        s_pMarker->SetRGBAnimSpeed(Features::Movement::Movement::PeekAssist_RGB.GetData().m_flVal);
    }
    


    // Nothing to do if we don't need to return this tick.
    if (s_bReturnActive == false)
        return;


    // Calculating vector point for current position to target position.
    vec vTargetPosFlat(s_vReturnOrigin);              vTargetPosFlat.z = 0.0f;
    vec vOriginFlat   (pLocalPlayer->GetAbsOrigin()); vOriginFlat.z    = 0.0f;
    vec vOriginToTarget = vTargetPosFlat - vOriginFlat; vOriginToTarget.NormalizeInPlace();


    // View angles. Flat.
    qangle qViewAnglesFlat = pCmd->viewangles; qViewAnglesFlat.pitch = 0.0f;
    vec vForward, vRight, vUp; Maths::AngleVectors(qViewAnglesFlat, &vForward, &vRight, &vUp);
    vForward.NormalizeInPlace(); vRight.NormalizeInPlace(); // NOTE : Up component isn't useful here.


    // Set forward & move in opposite direction of velocity.
    pCmd->forwardmove = std::clamp<float>(MAX_MOVE_USERCMD * vOriginToTarget.Dot(vForward), -MAX_MOVE_USERCMD, MAX_MOVE_USERCMD);
    pCmd->sidemove    = std::clamp<float>(MAX_MOVE_USERCMD * vOriginToTarget.Dot(vRight),   -MAX_MOVE_USERCMD, MAX_MOVE_USERCMD);
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