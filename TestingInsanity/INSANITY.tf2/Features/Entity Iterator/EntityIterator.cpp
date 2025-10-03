//=========================================================================
//                      ENTITY ITERATOR
//=========================================================================
// by      : INSANE
// created : 02/09/2025
// 
// purpose : Iterates entity list & stores required imformation ( backtrack n more )
//-------------------------------------------------------------------------
#include "EntityIterator.h"

// SDK
#include "../../SDK/class/BaseEntity.h"
#include "../../SDK/class/BaseWeapon.h"
#include "../../SDK/class/CUserCmd.h"
#include "../Tick Shifting/TickShifting.h"
#include "../../Utility/Insane Profiler/InsaneProfiler.h"

// Utility
#include "../../Utility/ConsoleLogging.h"
#include "../../SDK/TF object manager/TFOjectManager.h"



///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
void EntityIterator_t::Run(BaseEntity* pLocalPlayer, baseWeapon* pActiveWeapon, CUserCmd* pCmd)
{
    PROFILE_FUNCTION();

    if (F::tickShifter.ShiftingTicks() == true)
        return;

    SetBackTrackTime(Features::BackTrack::BackTrack::BackTrack_In_Ms.GetData().m_flVal / 1000.0f);

    if (m_bJumpTableHelperInit == false)
    {
        _ConstructJumpTableHelper();
    }

    // Clear lists out before filling up with new data.
    ClearLists();


    int nEntities = I::IClientEntityList->NumberOfEntities(false);


    std::vector<BaseEntity*>* vAllConnectedEnemies = m_vecAllConnectedEnemies.GetWriteBuffer();
    DOUBLEBUFFER_AUTO_RELEASE_WRITEBUFFER_SWAP(&m_vecAllConnectedEnemies, vAllConnectedEnemies);

    std::vector<BaseEntity*>* vAllConnectedTeammates = m_vecAllConnectedTeammates.GetWriteBuffer();
    DOUBLEBUFFER_AUTO_RELEASE_WRITEBUFFER_SWAP(&m_vecAllConnectedTeammates, vAllConnectedTeammates);

    std::vector<BaseEntity*>* vecTeamMates = m_vecPlayerFriendly.GetWriteBuffer();
    DOUBLEBUFFER_AUTO_RELEASE_WRITEBUFFER_SWAP(&m_vecPlayerFriendly, vecTeamMates);
    
    std::vector<BaseEntity*>* vecEnemies = m_vecPlayerEnemy.GetWriteBuffer();
    DOUBLEBUFFER_AUTO_RELEASE_WRITEBUFFER_SWAP(&m_vecPlayerEnemy, vecEnemies);

    for (int iEntIndex = 0; iEntIndex < nEntities; iEntIndex++)
    {
        BaseEntity* pEnt = I::IClientEntityList->GetClientEntity(iEntIndex);

        if (pEnt == nullptr)
            continue;

        int  iClassID        = pEnt->GetClientClass()->m_ClassID;
        bool bFriendlyEntity = pEnt->m_iTeamNum() == pLocalPlayer->m_iTeamNum();

        // Storing all connected players separetly for the player list widget.
        if (iClassID == ClassID::CTFPlayer && vAllConnectedEnemies != nullptr && vAllConnectedTeammates != nullptr)
        {
            if (bFriendlyEntity == true)
            {
                vAllConnectedTeammates->push_back(pEnt);
            }
            else
            {
                vAllConnectedEnemies->push_back(pEnt);
            }
        }

        if (pEnt->IsDormant() == true || pEnt == pLocalPlayer)
            continue;

        // Call back to all registered functions.
        _CallBack(pEnt);

        auto it = m_jumpTableHelperMap.find(iClassID);
        if (it == m_jumpTableHelperMap.end())
            continue;

        switch (it->second)
        {
        case ClassIdIndex::CTFPlayer:         
        {
            _ProcessPlayer((bFriendlyEntity == true ? vecTeamMates : vecEnemies), pEnt, pCmd->tick_count);
            break;
        }
        case ClassIdIndex::CObjectSentrygun:  _ProcessSentry(pEnt, pLocalPlayer->m_iTeamNum());     break;
        case ClassIdIndex::CObjectDispenser:  _ProcessDispenser(pEnt, pLocalPlayer->m_iTeamNum());  break;
        case ClassIdIndex::CObjectTeleporter: _ProcessTeleporter(pEnt, pLocalPlayer->m_iTeamNum()); break;
        default: break;
        }
    }
}


///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
__forceinline void ClearDoubleBufferVector(Containers::DoubleBuffer_t<std::vector<BaseEntity*>>& data)
{
    std::vector<BaseEntity*>* pBuffer = data.GetWriteBuffer();
    if (pBuffer != nullptr)
    {
        pBuffer->clear();
        data.ReturnWriteBuffer(pBuffer, false);
    }
}


///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
void EntityIterator_t::ClearLists()
{
    // Non-Dormant entities.
    ClearDoubleBufferVector(m_vecPlayerEnemy);
    ClearDoubleBufferVector(m_vecPlayerFriendly);
    m_vecDispenserEnemy.clear();  m_vecDispenserFriendly.clear();
    m_vecSentryEnemy.clear();     m_vecSentryFriendly.clear();
    m_vecTeleporterEnemy.clear(); m_vecTeleporterFriendly.clear();

    // Dormant & Non-Dormant players.
    ClearDoubleBufferVector(m_vecAllConnectedEnemies);
    ClearDoubleBufferVector(m_vecAllConnectedTeammates);
}


///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
void EntityIterator_t::ClearBackTrackData()
{
    m_mapEntInfo.clear();
}


///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
Containers::DoubleBuffer_t<std::vector<BaseEntity*>>& EntityIterator_t::GetAllConnectedEnemiesList()
{
    return m_vecAllConnectedEnemies;
}


///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
Containers::DoubleBuffer_t<std::vector<BaseEntity*>>& EntityIterator_t::GetAllConnectedTeamMatesList()
{
    return m_vecAllConnectedTeammates;
}


///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
Containers::DoubleBuffer_t<std::vector<BaseEntity*>>& EntityIterator_t::GetEnemyPlayers()
{
    return m_vecPlayerEnemy;
}


///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
Containers::DoubleBuffer_t<std::vector<BaseEntity*>>& EntityIterator_t::GetFrendlyPlayers()
{
    return m_vecPlayerFriendly;
}


///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
std::deque<BackTrackRecord_t>* EntityIterator_t::GetBackTrackRecord(BaseEntity* pEnt)
{
    auto it = m_mapEntInfo.find(pEnt);
    if (it == m_mapEntInfo.end())
        return nullptr;

    return &it->second;
}


///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
void EntityIterator_t::SetBackTrackTime(const float flBackTrackTime)
{
    m_flBackTrackTime = std::clamp<float>(flBackTrackTime, 0.0f, CVars::sv_maxunlag + MAX_BACKTRACK_TIME);
}


///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
float EntityIterator_t::GetBackTrackTime() const
{
    return m_flBackTrackTime;
}


///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
void EntityIterator_t::RegisterCallBack(void* pFunction)
{
    m_vecCallBacks.push_back(pFunction);
}


///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
const std::unordered_map<int, int>& EntityIterator_t::GetJumpTableHelper() const
{
    return m_jumpTableHelperMap;
}


///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
void EntityIterator_t::SetEntityMaterial(BaseEntity* pEnt, int iMatIndex)
{
    if (pEnt == nullptr)
        return;

    auto it = m_mapEntToMateiral.find(pEnt);
    if (it == m_mapEntToMateiral.end())
    {
        m_mapEntToMateiral.insert({ pEnt, iMatIndex });
        return;
    }

    it->second = iMatIndex;

    LOG("Set entity material to \"%d\"", iMatIndex);
}


///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
int EntityIterator_t::GetEntityMaterial(BaseEntity* pEnt)
{
    if (pEnt == nullptr)
        return -1;

    auto it = m_mapEntToMateiral.find(pEnt);
    if (it == m_mapEntToMateiral.end())
        return -1;

    return it->second;
}


///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
void EntityIterator_t::ClearEntityMaterialOverrides()
{
    m_mapEntToMateiral.clear();
}


///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
void EntityIterator_t::ClearSequenceData()
{
    m_qSequences.clear();
    m_iLastAddedSequence = -1;
}


///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
std::deque<EntityIterator_t::DatagramStat_t>& EntityIterator_t::GetDatagramSequences()
{
    return m_qSequences;
}


///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
void EntityIterator_t::_ProcessPlayer(std::vector<BaseEntity*>* vecListToPushIn, BaseEntity* pEnt, int iCurrentTick)
{
    if (pEnt->m_lifeState() != lifeState_t::LIFE_ALIVE)
        return;
    
    vecListToPushIn->push_back(pEnt);

    auto it = m_mapEntInfo.find(pEnt);
    if (it == m_mapEntInfo.end())
    {
        m_mapEntInfo.insert({ pEnt, std::deque<BackTrackRecord_t>() });
        LOG("Added backtrack record for entity.");
    }

    // Adding this back track record.
    BackTrackRecord_t record;
    pEnt->SetupBones(record.m_bones, MAX_STUDIO_BONES, BONE_USED_BY_ANYTHING, CUR_TIME);
    record.m_iFlags      = pEnt->m_fFlags();
    record.m_iTick       = iCurrentTick;
    record.m_vOrigin     = pEnt->GetAbsOrigin();
    record.m_qViewAngles = pEnt->m_angEyeAngles();

    auto it2 = m_mapEntInfo.find(pEnt);
    std::deque<BackTrackRecord_t>& allRecords = it2->second;

    allRecords.push_front(record);

    // Keep at least one record.
    const int iMaxRecords = TIME_TO_TICK(GetBackTrackTime()) == 0 ? 1 : TIME_TO_TICK(GetBackTrackTime());
    // removing expired ones.
    for (int iTick = 0; iTick < 70; iTick++)
    {
        if (allRecords.size() <= iMaxRecords)
            break;

        allRecords.pop_back();
    }
}


///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
void EntityIterator_t::_ProcessSentry(BaseEntity* pEnt, int iFriendlyTeam)
{
    if (pEnt->m_iTeamNum() == iFriendlyTeam)
    {
        m_vecSentryFriendly.push_back(pEnt);
    }
    else
    {
        m_vecSentryEnemy.push_back(pEnt);
    }
}


///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
void EntityIterator_t::_ProcessDispenser(BaseEntity* pEnt, int iFriendlyTeam)
{
    if (pEnt->m_iTeamNum() == iFriendlyTeam)
    {
        m_vecDispenserFriendly.push_back(pEnt);
    }
    else
    {
        m_vecDispenserEnemy.push_back(pEnt);
    }
}


///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
void EntityIterator_t::_ProcessTeleporter(BaseEntity* pEnt, int iFriendlyTeam)
{
    if (pEnt->m_iTeamNum() == iFriendlyTeam)
    {
        m_vecTeleporterFriendly.push_back(pEnt);
    }
    else
    {
        m_vecTeleporterEnemy.push_back(pEnt);
    }
}


///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
void EntityIterator_t::_CallBack(BaseEntity* pEnt)
{
    for (void* pFunction : m_vecCallBacks)
    {
        reinterpret_cast<T_CallBack>(pFunction)(pEnt);
    }
}


///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
void EntityIterator_t::_ConstructJumpTableHelper()
{
    m_jumpTableHelperMap.insert({ ClassID::CTFPlayer,         CTFPlayer });
    m_jumpTableHelperMap.insert({ ClassID::CObjectSentrygun,  CObjectSentrygun });
    m_jumpTableHelperMap.insert({ ClassID::CObjectDispenser,  CObjectDispenser });
    m_jumpTableHelperMap.insert({ ClassID::CObjectTeleporter, CObjectTeleporter });

    m_bJumpTableHelperInit = true;
}
