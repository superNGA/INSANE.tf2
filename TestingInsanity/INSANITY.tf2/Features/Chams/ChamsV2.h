#pragma once

#include "../FeatureHandler.h"


class BaseEntity;
struct DrawModelState_t;
struct ModelRenderInfo_t;
struct matrix3x4_t;

///////////////////////////////////////////////////////////////////////////
class ChamsV2_t
{
public:
    void Run(void* pVTable, DrawModelState_t* modelState, ModelRenderInfo_t* renderInfo, matrix3x4_t* boneMatrix, void* pOriginalDME);

private:
    void _DrawBackTrack(void* pVTable, DrawModelState_t* modelState, ModelRenderInfo_t* renderInfo, void* pOriginalDME, BaseEntity* pEnt);

    void _SetupMatDropDowns();

};
///////////////////////////////////////////////////////////////////////////

DECLARE_FEATURE_OBJECT(chamsV2, ChamsV2_t)

DEFINE_TAB(Materials, 12)
DEFINE_SECTION(Materials, "Materials", 1)

static const char* pDummyList[] = {"NULL"};
DEFINE_FEATURE(Player_Enemy,         DropDown_t, Materials, Materials, 1, DropDown_t(pDummyList, 1));
DEFINE_FEATURE(Player_TeamMates,     DropDown_t, Materials, Materials, 2, DropDown_t(pDummyList, 1));
DEFINE_FEATURE(Sentry_Enemy,         DropDown_t, Materials, Materials, 3, DropDown_t(pDummyList, 1));
DEFINE_FEATURE(Sentry_TeamMates,     DropDown_t, Materials, Materials, 4, DropDown_t(pDummyList, 1));
DEFINE_FEATURE(Dispenser_Enemy,      DropDown_t, Materials, Materials, 5, DropDown_t(pDummyList, 1));
DEFINE_FEATURE(Dispenser_TeamMates,  DropDown_t, Materials, Materials, 6, DropDown_t(pDummyList, 1));
DEFINE_FEATURE(Teleporter_Enemy,     DropDown_t, Materials, Materials, 7, DropDown_t(pDummyList, 1));
DEFINE_FEATURE(Teleporter_TeamMates, DropDown_t, Materials, Materials, 8, DropDown_t(pDummyList, 1));


static const char* pBackTrackChamOptions[] = { "None", "Last only", "All" };
DEFINE_FEATURE(BackTrack_Cham_Setting, DropDown_t, BackTrack, BackTrack, 2,
    DropDown_t(pBackTrackChamOptions, 3))

DEFINE_FEATURE(BackTrack_Cham, DropDown_t, BackTrack, BackTrack, 3, DropDown_t(pDummyList, 1));