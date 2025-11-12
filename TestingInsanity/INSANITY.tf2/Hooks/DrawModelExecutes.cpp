// Utility
#include "../Utility/Hook Handler/Hook_t.h"
#include "../Utility/ConsoleLogging.h"
// #include "../Utility/ClassIDHandler/ClassIDHandler.h"

// SDK
#include "../SDK/class/IMaterialSystem.h"
#include "../SDK/class/IStudioRender.h"
#include "../SDK/class/IMaterial.h"
#include "../SDK/class/BaseEntity.h"
#include "../SDK/class/CBaseLightCache.h"
#include "../SDK/class/IVEngineClient.h"
#include "../SDK/class/CMultAnimState.h"


#include "../Features/TickManip/TickManipHelper.h"
#include "../Features/Aimbot/AimbotHitscanV2/AimbotHitscanV2.h"
#include "../Features/DMEHandler/DMEHandler.h"
#include "../Features/ModelPreview/ModelPreview.h"
#include "../Features/Material Gen/MaterialGen.h"
#include "../Features/Entity Iterator/EntityIterator.h"
#include "../Features/Aimbot/Aimbot Melee/AimbotMelee.h"
#include "../Features/WorldColorModulation/WorldColorModulation.h"
#include "../Features/TickManip/AntiAim/AntiAimV2.h"
#include "EndScene/EndScene.h"



///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
MAKE_HOOK(DrawModelExecute, "4C 89 4C 24 ? 48 89 4C 24 ? 55 53 56 57 41 54", __fastcall, ENGINE_DLL, int64_t, 
    void* pVTable, DrawModelState_t* modelState, ModelRenderInfo_t* renderInfo, matrix3x4_t* boneMatrix)
{
    // Don't do material altering at unstable states, else causes crahes.
    if (directX::UI::UI_has_been_shutdown == true)
    {
        return Hook::DrawModelExecute::O_DrawModelExecute(pVTable, modelState, renderInfo, boneMatrix);
    }


    // Color/Alpha Modulate world materials & props n shit.
    F::worldColorMod.Run();


    // Handle this model.
    return F::DMEHandler.Run(pVTable, modelState, renderInfo, boneMatrix, Hook::DrawModelExecute::O_DrawModelExecute);
}


///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
MAKE_HOOK(SetupBones, "48 8B C4 44 89 40 ? 48 89 50 ? 55 53", __fastcall, CLIENT_DLL, void*,
    I_client_renderable* pRenderable, void* pBonesOut, int nMaxBones, int iMask, int iCurTime)
{
    // OBSERVATIONS : Setup bones is only calculated for player entities. Setup bones is only called for
    // weapon entites when we shoot, and its called about 7 to 8 times. Changing IClientRenderable::GetRenderAngles during
    // those setupBone calls will rotate our weapon in desired direction but for only those couple of frames, when 
    // weapon is drawn using those bones, else is uses its owner's bone matrix maybe.
    //

    // Check if we need to bone matrix @ custom angles?
    if (F::tickManipHelper.UseCustomBonesLocalPlayer() == false)
        return Hook::SetupBones::O_SetupBones(pRenderable, pBonesOut, nMaxBones, iMask, iCurTime);


    // if we calculating bones ourselves, don't shit on it.
    if (F::tickManipHelper.CalculatingBones() == true)
        return Hook::SetupBones::O_SetupBones(pRenderable, pBonesOut, nMaxBones, iMask, iCurTime);


    BaseEntity* pEnt         = pRenderable->GetBaseEntFromRenderable();
    bool        bLocalPlayer = pEnt->GetRefEHandle() == F::entityIterator.GetLocalPlayerInfo().m_refEHandle;

    if (bLocalPlayer == true && pBonesOut != nullptr)
    {
        // Make ourselves look up.
        CMultiPlayerAnimState* pAnimState = *reinterpret_cast<CMultiPlayerAnimState**>((uintptr_t)pEnt + Netvars::DT_TFPlayer::m_hItem - 88);
        qangle                 qAngles    = F::antiAimV2.GetRealAngles();
        pAnimState->Update(qAngles.yaw, qAngles.pitch);

        // Handle pose parameter index 1
        pEnt->GetPoseParameter()[1] = 0.5f;
    }

    return Hook::SetupBones::O_SetupBones(pRenderable, pBonesOut, nMaxBones, iMask, iCurTime);
}