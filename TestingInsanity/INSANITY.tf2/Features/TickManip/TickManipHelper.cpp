//=========================================================================
//                      TICK MANIPULATION HELPER
//=========================================================================
// by      : INSANE
// created : 11/05/2025
// 
// purpose : Handles "SendPacket"'s value & runs fakelag.
//-------------------------------------------------------------------------
#include "TickManipHelper.h"

#include "../../Utility/Signature Handler/signatures.h"
#include "../../Extra/math.h"
#include "AntiAim/AntiAimV2.h"
#include "../ImGui/InfoWindowV2/InfoWindow.h"
#include "../ImGui/NotificationSystem/NotificationSystem.h"


// SDK
#include "../../SDK/class/CommonFns.h"
#include "../../SDK/class/BaseEntity.h"
#include "../../SDK/class/BaseWeapon.h"
#include "../../SDK/class/CUserCmd.h"
#include "../../SDK/class/CMultAnimState.h"
#include "../../SDK/class/IVEngineClient.h"
#include "../../SDK/class/IMaterial.h"
#include "../../SDK/class/IVDebugOverlay.h"
#include "../../SDK/TF object manager/TFOjectManager.h"


MAKE_SIG(CBaseAnimating_InvalidateBoneCache, "8B 05 ? ? ? ? FF C8 C7 81", CLIENT_DLL, int64_t, void*)



///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
void TickManipHelper_t::Run(BaseEntity* pLocalPlayer, baseWeapon* pActiveWeapon, CUserCmd* pCmd, bool* pSendPacket, bool* pCreateMoveResult)
{
    // Get ticks to choke.
    int iFakeLagTicks = Features::Misc::FakeLag::FakeLag_Enable.IsActive()    == false ? 0 : Features::Misc::FakeLag::FakeLag_ChockedTicks.GetData().m_iVal;
    int iAntiAimTicks = Features::AntiAim::AntiAim::AntiAim_Enable.IsActive() == false ? 0 : F::antiAimV2.GetAntiAimTicks();

    int iTicksToChoke = Maths::MAX<int>(iFakeLagTicks, iAntiAimTicks);

    m_qOriginalAngles   = pCmd->viewangles;
    m_flOrigForwardMove = pCmd->forwardmove; m_flOrigSideMove = pCmd->sidemove;

    // Choke...
    if (m_iTicksChocked < iTicksToChoke)
    {
        m_iTicksChocked++;
        *pSendPacket = false;
    }
    else
    {
        m_iTicksChocked = 0;
        *pSendPacket    = true;
    }


    F::antiAimV2.Run(pLocalPlayer, pActiveWeapon, pCmd, pSendPacket, pCreateMoveResult);


    // Store bones to draw our fake angles model.
    // Either Fake-Lag is off or Fake-Lag is on and we are send packet is true.
    if(_ShouldRecordSecondModelBones(*pSendPacket) == true)
    {
        qangle qEyeAngle = 
            Features::AntiAim::AntiAim::AntiAim_Draw.IsActive() == true ?
            F::antiAimV2.GetFakeAngles() : F::antiAimV2.GetRealAngles();

        _StoreBones(qEyeAngle, m_fakeAngleBones, pLocalPlayer);
    }


    // In case we are shooting, we need to put our cmd view angles to engine angles, so we 
    // shoot where user intends to shoot.
    _DetectAndHandleShots(pLocalPlayer, pActiveWeapon, pCmd, pSendPacket);


    _DrawWidget();
}


///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
void TickManipHelper_t::Reset()
{
    m_iTicksChocked = 0;
}


///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
bool TickManipHelper_t::UseCustomBonesLocalPlayer() const
{
    return
        Features::AntiAim::AntiAim::AntiAim_Enable.IsActive()           == true &&
        Features::AntiAim::AntiAim::AntiAim_CustomRealAngles.IsActive() == true;
}


///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
bool TickManipHelper_t::ShouldDrawSecondModel() const
{
    bool bAntiAimDrawEnabled =
        Features::AntiAim::AntiAim::AntiAim_Enable.IsActive() == true &&
        Features::AntiAim::AntiAim::AntiAim_Draw.IsActive()   == true;

    bool bFakeLagDrawEnabled =
        Features::Misc::FakeLag::FakeLag_Enable.IsActive() == true &&
        Features::Misc::FakeLag::FakeLag_Draw.IsActive()   == true;

    return bAntiAimDrawEnabled == true || bFakeLagDrawEnabled == true;
}


///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
matrix3x4_t* TickManipHelper_t::GetFakeAngleBones()
{
    return m_fakeAngleBones;
}


///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
matrix3x4_t* TickManipHelper_t::GetRealAngleBones()
{
    return m_realAngleBones;
}


///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
bool TickManipHelper_t::CalculatingBones() const
{
    return m_bCalculatingBones;
}


///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
bool TickManipHelper_t::_ShouldRecordSecondModelBones(const bool bSendPacket) const
{
    // Atlest one of the features from antiaim_draw and fakelag_draw shoudl should be enabled.
    // else no recording.
    if (ShouldDrawSecondModel() == false)
        return false;

    if (Features::Misc::FakeLag::FakeLag_Enable.IsActive() == true && Features::Misc::FakeLag::FakeLag_Draw.IsActive() == true && bSendPacket == false)
        return false;

    return true;
}


///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
void TickManipHelper_t::_StoreBones(const qangle& qEyeAngle, matrix3x4_t* pDestination, BaseEntity* pLocalPlayer)
{
    m_bCalculatingBones = true;

    // Store old animation state.
    CMultiPlayerAnimState* pAnimState = *reinterpret_cast<CMultiPlayerAnimState**>((uintptr_t)pLocalPlayer + Netvars::DT_TFPlayer::m_hItem - 88);
    static char oldAnimState[sizeof(CMultiPlayerAnimState)];
    memcpy(&oldAnimState, pAnimState, sizeof(CMultiPlayerAnimState));


    // Invalidate old bones, so game actually calculates new ones for us.
    Sig::CBaseAnimating_InvalidateBoneCache(pLocalPlayer);


    // New animation state angles.
    pAnimState->m_flCurrentFeetYaw = qEyeAngle.yaw;
    pAnimState->Update(qEyeAngle.yaw, qEyeAngle.pitch);


    // store original angles & spoof render angles.
    const qangle qOriginalRenderAngles = pLocalPlayer->GetRenderAngles();
    pLocalPlayer->GetRenderAngles().yaw = qEyeAngle.yaw;

    
    // sometimes, the pose parameter index 1, is set to 0 or 1, instead of 0.5, and that 
    // will make us look either to the right or left, with the correct feet position,
    // as in rotating spine to look either way.
    pLocalPlayer->GetPoseParameter()[1] = 0.5f;


    // Storing animation parameters to prevent animation from speeding up.
    int   iOldSequence   = pLocalPlayer->m_nSequence();
    float flOldCycle     = pLocalPlayer->m_flCycle();
    float flOldFrameTime = tfObject.pGlobalVar->frametime;


    // Make game build us another bone matrix
    pLocalPlayer->SetupBones(pDestination, MAX_STUDIO_BONES, BONE_USED_BY_ANYTHING, CUR_TIME);


    // Restoring anim parameters to prevent anim speed up.
    tfObject.pGlobalVar->frametime = flOldFrameTime;
    pLocalPlayer->m_nSequence(iOldSequence);
    pLocalPlayer->m_flCycle(flOldCycle);


    // restore render angles & animation state.
    pLocalPlayer->GetRenderAngles() = qOriginalRenderAngles;
    memcpy(pAnimState, &oldAnimState, sizeof(CMultiPlayerAnimState));


    m_bCalculatingBones = false;
}


///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
void TickManipHelper_t::_DetectAndHandleShots(BaseEntity* pLocalPlayer, baseWeapon* pActiveWeapon, CUserCmd* pCmd, bool* pSendPacket)
{
    // If antiaim is turned off, then our moved if fixed, no need to fuck with anything here.
    if (Features::AntiAim::AntiAim::AntiAim_Enable.IsActive() == false)
        return;

    bool bShooting    = SDK::CanAttack(pLocalPlayer, pActiveWeapon) == true && (pCmd->buttons & IN_ATTACK) == true;
    bool bResetAngles = false;

    if (pActiveWeapon->getSlot() == WPN_SLOT_MELLE)
    {
        if (pLocalPlayer->m_iClass() != TF_SPY)
        {
            float flCurTime = TICK_TO_TIME(pLocalPlayer->m_nTickBase());
            bResetAngles    = SDK::InAttack(pLocalPlayer, pActiveWeapon) == true && flCurTime >= pActiveWeapon->m_flSmackTime();
        }
    }
    // TODO : Use switch here.
    // In case we are playing pyro, we must reset angles on all the tick and not only on alternating ticks.
    else if (pLocalPlayer->m_iClass() == TF_PYRO && pActiveWeapon->getSlot() == WPN_SLOT_PRIMARY)
    {
        if (pCmd->buttons & IN_ATTACK)
            bResetAngles = true;
    }
    else if (pActiveWeapon->RequiresCharging() == true)
    {
        // In case our wpn require charing, we need to do this weird check, where we need to 
        // set angles on the tick where we have released the attack button ( hence bShooting == false ) but 
        // all our netvars are in good state / represeting shooting. IDK, but it seems to work.
        bResetAngles = pActiveWeapon->m_flChargeBeginTime() > 0.001f && bShooting == false;
    }
    else
    {
        bResetAngles = bShooting;
    }


    // We good, no need to set view angles to engine angles.
    if (bResetAngles == false)
        return;

    pCmd->viewangles = m_qOriginalAngles;
    pCmd->forwardmove = m_flOrigForwardMove; pCmd->sidemove = m_flOrigSideMove;
}


///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
void TickManipHelper_t::_DrawWidget() const
{
    if(Features::Misc::FakeLag::FakeLag_DrawWidget.IsActive() == true)
    {
        // Slider
        Render::infoWindowV2.AddOrUpdate("FakeLag", m_iTicksChocked,
            Features::Misc::FakeLag::FakeLag_ChockedTicks.GetData().m_iMin,
            Features::Misc::FakeLag::FakeLag_ChockedTicks.GetData().m_iMax, 0);

        // Counter
        Render::infoWindowV2.AddOrUpdate("FakeLag", 
            std::format("{} / {}", m_iTicksChocked, Features::Misc::FakeLag::FakeLag_ChockedTicks.GetData().m_iMax), 
            0, InfoWindowWidget_t::Alignment_Middle); 
    }
    else
    {
        Render::infoWindowV2.Hide("FakeLag");
    }
}
