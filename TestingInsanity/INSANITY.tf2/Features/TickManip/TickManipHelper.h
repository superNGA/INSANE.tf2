//=========================================================================
//                      TICK MANIPULATION HELPER
//=========================================================================
// by      : INSANE
// created : 11/05/2025
// 
// purpose : Handles "SendPacket"'s value & runs fakelag.
//-------------------------------------------------------------------------
#pragma once
#include "../FeatureHandler.h"


class BaseEntity;
class baseWeapon;
class CUserCmd;


///////////////////////////////////////////////////////////////////////////
class TickManipHelper_t
{
public:
    void Run(BaseEntity* pLocalPlayer, baseWeapon* pActiveWeapon, CUserCmd* pCmd, bool* pSendPacket, bool* pCreateMoveResult);
    void Reset();

    bool         UseCustomBonesLocalPlayer() const;
    bool         ShouldDrawSecondModel();
    matrix3x4_t* GetFakeAngleBones();
    matrix3x4_t* GetRealAngleBones();

private:

    // Calculating & storing bones.
    void   _StoreBones(const qangle& qEyeAngle, matrix3x4_t* pDestination, BaseEntity* pLocalPlayer, CUserCmd* pCmd);
    matrix3x4_t m_fakeAngleBones[MAX_STUDIO_BONES];
    matrix3x4_t m_realAngleBones[MAX_STUDIO_BONES];


    // View angle override while manually shooting.
    void _HandleShots(CUserCmd* pCmd);


    // UI
    void _DrawWidget() const;
    int m_iTicksChocked = 0;
};
///////////////////////////////////////////////////////////////////////////

DECLARE_FEATURE_OBJECT(tickManipHelper, TickManipHelper_t)

DEFINE_TAB(Misc, 11)
DEFINE_SECTION(FakeLag, "Misc", 1)
DEFINE_FEATURE(FakeLag_Enable,       "Enable",     bool,        FakeLag, Misc, 1, false, FeatureFlag_None)
DEFINE_FEATURE(FakeLag_ChockedTicks, "Ticks",      IntSlider_t, FakeLag, Misc, 2, IntSlider_t(0, 0, 24), FeatureFlag_None)
DEFINE_FEATURE(FakeLag_Draw,         "Draw Model", bool,        FakeLag, Misc, 3, false, FeatureFlag_None)
DEFINE_FEATURE(FakeLag_DrawWidget,   "Draw Info",  bool,        FakeLag, Misc, 4, false, FeatureFlag_None)