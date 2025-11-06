#include "TickManipHelper.h"

#include "../../Utility/Signature Handler/signatures.h"
#include "../../Extra/math.h"
#include "AntiAim/AntiAimV2.h"
#include "../ImGui/InfoWindowV2/InfoWindow.h"

// SDK
#include "../../SDK/class/CommonFns.h"
#include "../../SDK/class/BaseEntity.h"
#include "../../SDK/class/BaseWeapon.h"
#include "../../SDK/class/CUserCmd.h"
#include "../../SDK/class/CMultAnimState.h"
#include "../../SDK/class/IVEngineClient.h"
#include "../../SDK/TF object manager/TFOjectManager.h"


MAKE_SIG(CBaseAnimating_InvalidateBoneCache, "8B 05 ? ? ? ? FF C8 C7 81", CLIENT_DLL, int64_t, void*)



///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
void TickManipHelper_t::Run(BaseEntity* pLocalPlayer, baseWeapon* pActiveWeapon, CUserCmd* pCmd, bool* pSendPacket, bool* pCreateMoveResult)
{
    // Get ticks to choke.
    int iFakeLagTicks = Features::Misc::FakeLag::FakeLag_Enable.IsActive()    == false ? 0 : Features::Misc::FakeLag::FakeLag_ChockedTicks.GetData().m_iVal;
    int iAntiAimTicks = Features::AntiAim::AntiAim::AntiAim_Enable.IsActive() == false ? 0 : F::antiAimV2.GetAntiAimTicks();

    int  iTicksToChoke = Maths::MAX<int>(iFakeLagTicks, iAntiAimTicks);
    bool bShooting     = (pCmd->buttons & IN_ATTACK) == true && SDK::CanAttack(pLocalPlayer, pActiveWeapon);

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
    bool bRecordFakeBones = 
        (Features::Misc::FakeLag::FakeLag_Draw.IsActive() == false) || 
        (Features::Misc::FakeLag::FakeLag_Draw.IsActive() == true && *pSendPacket == true);
    if(ShouldDrawSecondModel() == true && bRecordFakeBones == true)
    {
        _StoreBones(F::antiAimV2.GetFakeAngles(), m_fakeAngleBones, pLocalPlayer, pCmd);
    }


    // if we are using custom real angles, we also need to store our real angle bones,
    // so we can draw our model with proper eye & feet yaw n shit like that.
    if (Features::AntiAim::AntiAim::AntiAim_CustomRealAngles.IsActive() == true)
    {
        _StoreBones(F::antiAimV2.GetRealAngles(), m_realAngleBones, pLocalPlayer, pCmd);
    }


    // In case we are shooting, we need to put our cmd view angles to engine angles, so we 
    // shoot where user intends to shoot.
    if (bShooting == true)
        _HandleShots(pCmd);
    

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
    return Features::AntiAim::AntiAim::AntiAim_CustomRealAngles.IsActive() == true;
}


///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
bool TickManipHelper_t::ShouldDrawSecondModel()
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
void TickManipHelper_t::_StoreBones(const qangle& qEyeAngle, matrix3x4_t* pDestination, BaseEntity* pLocalPlayer, CUserCmd* pCmd)
{
    assert(
        (Features::AntiAim::AntiAim::AntiAim_Draw.IsActive() == true || Features::Misc::FakeLag::FakeLag_Draw.IsActive() == true) && 
        "Draw disabled for AntiAim & FakeLag, but still storing bones.");

    // Store old animation state.
    CMultiPlayerAnimState* pAnimState = *reinterpret_cast<CMultiPlayerAnimState**>((uintptr_t)pLocalPlayer + Netvars::DT_TFPlayer::m_hItem - 88);
    static char oldAnimState[sizeof(CMultiPlayerAnimState)];
    memcpy(&oldAnimState, pAnimState, sizeof(CMultiPlayerAnimState));


    // Invalidate old bones, so game actually calculates new ones for us.
    Sig::CBaseAnimating_InvalidateBoneCache(pLocalPlayer);


    // New animation state angles.
    pAnimState->m_flCurrentFeetYaw = qEyeAngle.yaw;
    pAnimState->Update(qEyeAngle.yaw, qEyeAngle.pitch); // <- this is important.

    // store original angles & spoof render angles.
    const qangle qOriginalRenderAngles  = pLocalPlayer->GetRenderAngles();
    pLocalPlayer->GetRenderAngles().yaw = qEyeAngle.yaw;


    // Make game build us another bone matrix
    pLocalPlayer->SetupBones(pDestination, MAX_STUDIO_BONES, BONE_USED_BY_ANYTHING, CUR_TIME);


    // restore render angles & animation state.
    pLocalPlayer->GetRenderAngles() = qOriginalRenderAngles;
    memcpy(pAnimState, &oldAnimState, sizeof(CMultiPlayerAnimState));
}


///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
void TickManipHelper_t::_HandleShots(CUserCmd* pCmd)
{
    I::iEngine->GetViewAngles(pCmd->viewangles);
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
