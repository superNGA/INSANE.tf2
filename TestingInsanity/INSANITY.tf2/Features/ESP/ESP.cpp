#include "ESP.h"

// SDK
#include "../../SDK/class/Source Entity.h"
#include "../../SDK/class/BaseEntity.h"
#include "../../SDK/class/IVEngineClient.h"
#include "../../SDK/class/CUserCmd.h"

// UTILITY
#include "../Graphics Engine/Graphics Engine/GraphicsEngine.h"
#include "../../Utility/ClassIDHandler/ClassIDHandler.h"
#include "../../Utility/Insane Profiler/InsaneProfiler.h"


void ESP_t::Run(CUserCmd* pCmd)
{
    PROFILE_FUNCTION("ESP");

    int iLocalPlayerIndex = I::iEngine->GetLocalPlayer();
    int nEnt              = I::IClientEntityList->NumberOfEntities(false);
    int iFriendlyTeam     = I::IClientEntityList->GetClientEntity(iLocalPlayerIndex)->m_iTeamNum();

    GraphicInfo_t espGraphicInfo(
        Features::ESP::PLAYER::TOP_LEFT.GetData().GetAsBytes(),
        Features::ESP::PLAYER::TOP_RIGHT.GetData().GetAsBytes(),
        Features::ESP::PLAYER::BOTTOM_RIGHT.GetData().GetAsBytes(),
        Features::ESP::PLAYER::BOTTOM_LEFT.GetData().GetAsBytes(),
        Features::ESP::PLAYER::Thickness.GetData().m_flVal
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

        bool bEnemy = (pEnt->m_iTeamNum() != iFriendlyTeam);
        if (bEnemy == false)
            continue;

        if (pEnt->GetClientClass()->m_ClassID == ClassID::CTFPlayer)
        {
            const vec& vOrigin = pEnt->GetCollideable()->GetCollisionOrigin();
            const vec& vMin = pEnt->GetCollideable()->OBBMins();
            const vec& vMax = pEnt->GetCollideable()->OBBMaxs();
            
            const char vNum = '0' + iEntIndex;

            F::graphicsEngine.DrawRect(std::format("ENT_{}", iEntIndex), vOrigin + vMin, vOrigin + vMax, pCmd->viewangles, 1000.0f, &espGraphicInfo);
        }
    }
}