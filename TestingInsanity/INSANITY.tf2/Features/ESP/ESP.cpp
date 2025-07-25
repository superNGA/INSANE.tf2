#include "ESP.h"

// SDK
#include "../../SDK/class/Source Entity.h"
#include "../../SDK/class/BaseEntity.h"
#include "../../SDK/class/IVEngineClient.h"
#include "../../SDK/class/CUserCmd.h"
#include "../../SDK/class/IVEfx.h"
#include "../../SDK/TF object manager/TFOjectManager.h"

// UTILITY
#include "../Graphics Engine/Graphics Engine/GraphicsEngine.h"
#include "../../Utility/ClassIDHandler/ClassIDHandler.h"
#include "../../Utility/Insane Profiler/InsaneProfiler.h"
#include "../../Extra/math.h"

// Delete this
#include "../MovementSimulation/MovementSimulation.h"


void ESP_t::Run(BaseEntity* pLocalPlayer, CUserCmd* pCmd)
{
    PROFILE_FUNCTION("ESP");

    vec vForward, vUp, vRight;
    Maths::AngleVectors(pCmd->viewangles, &vForward, &vRight, &vUp);

    int iLocalPlayerIndex = I::iEngine->GetLocalPlayer();
    int nEnt              = I::IClientEntityList->NumberOfEntities(false);
    int iFriendlyTeam     = I::IClientEntityList->GetClientEntity(iLocalPlayerIndex)->m_iTeamNum();

    GraphicInfo_t espGraphicInfo(
        Features::ESP::PLAYER::TOP_LEFT.GetData().GetAsBytes(),
        Features::ESP::PLAYER::TOP_RIGHT.GetData().GetAsBytes(),
        Features::ESP::PLAYER::BOTTOM_RIGHT.GetData().GetAsBytes(),
        Features::ESP::PLAYER::BOTTOM_LEFT.GetData().GetAsBytes(),
        Features::ESP::PLAYER::Thickness.GetData().m_flVal,
        Features::ESP::PLAYER::Speed.GetData().m_flVal,
        Features::ESP::PLAYER::glowPower.GetData().m_flVal
    );

    for (int iEntIndex = 0; iEntIndex < nEnt; iEntIndex++)
    {
        if (iEntIndex == iLocalPlayerIndex)
            continue;

        BaseEntity* pEnt = I::IClientEntityList->GetClientEntity(iEntIndex);

        // don't want Nullptrs
        if (pEnt == nullptr)
            continue;

        // don't want Dormants
        if (pEnt->IsDormant() == true)
            continue;

        // don't want dead entities.
        if (pEnt->m_lifeState() != lifeState_t::LIFE_ALIVE)
            continue;

        // don't want team-mates
        if (pEnt->m_iTeamNum() == iFriendlyTeam)
            continue;

        if (pEnt->GetClientClass()->m_ClassID == ClassID::CTFPlayer)
        {
            auto* pEntCollidable = pEnt->GetCollideable();
            if (pEntCollidable == nullptr)
                continue;

            const vec& vOrigin = pEntCollidable->GetCollisionOrigin();
            const vec& vMin = pEntCollidable->OBBMins();
            const vec& vMax = pEntCollidable->OBBMaxs();

            float flEntWidht  = vMax.Dist2Dto(vMin);
            float flEntHeight = Maths::MAX<float>(vMin.z, vMax.z);
            
            const char vNum = '0' + iEntIndex;

            F::graphicsEngine.DrawRect(
                std::format("ENT_{}", iEntIndex), 
                vOrigin + (vRight * (flEntWidht * 0.5f)), // Bottom right corner
                vOrigin + (vRight * (flEntWidht * -0.5f)) + (vUp * flEntHeight), // Top Left corner
                pCmd->viewangles, 1000.0f, &espGraphicInfo);

            /*F::graphicsEngine.DrawBox(
                std::format("ENT_{}", iEntIndex), 
                vOrigin + vMin, vOrigin + vMax,
                pCmd->viewangles, 1000.0f, &espGraphicInfo);*/

            // Recording angle & velocity for all entities.
            // This is a temporary solution
            F::movementSimulation.RecordStrafeData(pEnt, false);
        }
    }
}