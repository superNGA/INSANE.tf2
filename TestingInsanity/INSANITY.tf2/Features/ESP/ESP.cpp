#include "ESP.h"

#include "../Visual Engine/VisualEngine.h"
#include "../../SDK/class/Source Entity.h"
#include "../../SDK/class/BaseEntity.h"
#include "../../SDK/class/IVEngineClient.h"
#include "../../Utility/ClassIDHandler/ClassIDHandler.h"

void ESP_t::Run()
{
    int iLocalPlayerIndex = I::iEngine->GetLocalPlayer();
    int nEnt = I::IClientEntityList->NumberOfEntities(false);
    int iFriendlyTeam = I::IClientEntityList->GetClientEntity(iLocalPlayerIndex)->m_iTeamNum();

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
        int  iEntID = pEnt->GetClientClass()->m_ClassID;

        if (iEntID == ClassID::CTFPlayer)
        {
            const vec& vOrigin = pEnt->GetCollideable()->GetCollisionOrigin();
            const vec& vMin = pEnt->GetCollideable()->OBBMins();
            const vec& vMax = pEnt->GetCollideable()->OBBMaxs();
            
            const char vNum = '0' + iEntIndex;

            F::insaneOverlay.DrawRect(std::format("ENT_{}", iEntIndex), vOrigin + vMin, vOrigin + vMax, 1000.0f);
        }
    }
}