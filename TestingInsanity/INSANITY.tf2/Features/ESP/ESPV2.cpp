#include "ESPV2.h"

// SDK
#include "../../SDK/class/BaseEntity.h"
#include "../../SDK/class/IVEngineClient.h"

#include "../../Utility/Insane Profiler/InsaneProfiler.h"
#include "../Graphics Engine V2/Draw Objects/Cube/Cube.h"
#include "../Entity Iterator/EntityIterator.h"


///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
void ESPV2_t::Run(BaseEntity* pLocalPlayer)
{
    PROFILE_FUNCTION();

    if (m_bMapCleared == false)
        return;

    std::vector<BaseEntity*>* vecEnemies = F::entityIterator.GetEnemyPlayers().GetReadBuffer();
    DOUBLEBUFFER_AUTO_RELEASE_READBUFFER(&F::entityIterator.GetEnemyPlayers(), vecEnemies);

    // Buffer not recieved.
    if (vecEnemies == nullptr)
        return;

    if (vecEnemies->size() <= 0)
        return;


    // Filtering provided entity list.
    for (BaseEntity* pEnt : *vecEnemies)
    {
        if (pEnt == nullptr)
            continue;

        if (pEnt->IsDormant() == true)
            continue;

        if (pEnt == pLocalPlayer)
            continue;

        auto it = m_vecEntToDrawObj.find(pEnt);
        if (it == m_vecEntToDrawObj.end())
        {
            Cube3D_t* pCube = new Cube3D_t();
            m_vecEntToDrawObj.insert({ pEnt, pCube });
            LOG("Registered entity in draw list.");
        }
    }


    // Now drawing entity.
    for (auto& it : m_vecEntToDrawObj)
    {
        IDrawObj_t* pDrawObj = it.second;
        BaseEntity* pEnt     = it.first;
        auto* pCube = reinterpret_cast<Cube3D_t*>(pDrawObj);

        if (pEnt == nullptr)
        {
            pCube->SetVisible(false);
            continue;
        }

        if (pEnt->IsDormant() == true || pEnt->m_lifeState() != lifeState_t::LIFE_ALIVE)
        {
            pCube->SetVisible(false);
            continue;
        }

        // Sometimes game switch shit around, and our team-mate might end up having esp on his ass.
        if (pEnt->m_iTeamNum() == pLocalPlayer->m_iTeamNum())
        {
            FAIL_LOG("Attempting deletion of bad entity");
            m_vecEntToDrawObj.erase(pEnt);
            F::graphics.FindAndRemoveDrawObj(pDrawObj);
            pDrawObj->DeleteThis();
            FAIL_LOG("ESP draw obj occured on team mate. Discarding it.");
        }

        ICollideable_t* pCollidable = pEnt->GetCollideable(); 
        vec             vOrigin     = pEnt->GetRenderOrigin();
        pCube->SetVisible(true);
        pCube->SetVertex(vOrigin + pCollidable->OBBMins(), vOrigin + pCollidable->OBBMaxs());
        pCube->SetColor(255, 255, 255, 255);
        pCube->SetRGBAnimSpeed(1.0f);
    }
}


///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
void ESPV2_t::Free()
{
    LOG("Starting freeing all entities");
    for (const auto& it : m_vecEntToDrawObj)
    {
        if(F::graphics.FindAndRemoveDrawObj(it.second) == true)
            it.second->DeleteThis();
    }
    LOG("Deleted all entities");

    m_vecEntToDrawObj.clear();
    LOG("Cleared map");

    m_bMapCleared = true;
}
