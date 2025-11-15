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
#include "../../SDK/class/IVDebugOverlay.h"
#include "../../SDK/TF object manager/TFOjectManager.h"
#include "../Tick Shifting/TickShifting.h"
#include "../MovementSimulation/MovementSimulation.h"

// Utility
#include "../../Utility/ConsoleLogging.h"
#include "../../Utility/Profiler/Profiler.h"
#include "../../Utility/Signature Handler/signatures.h"

MAKE_SIG(CBaseAnimating_InvalidateBoneCache, "8B 05 ? ? ? ? FF C8 C7 81", CLIENT_DLL, int64_t, void*)



///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
void EntityIterator_t::Run(BaseEntity* pLocalPlayer, baseWeapon* pActiveWeapon, CUserCmd* pCmd)
{
    PROFILER_RECORD_FUNCTION(CreateMove);

    if (F::tickShifter.ShiftingTicks() == true)
        return;

    SetBackTrackTime(Features::BackTrack::BackTrack::BackTrack_In_Ms.GetData().m_flVal / 1000.0f);

    if (m_bJumpTableHelperInit == false)
    {
        _ConstructJumpTableHelper();
    }

    // Store Local-Player's information before doing anything else.
    _UpdateLocalPlayerInfo(pLocalPlayer, pActiveWeapon);


    // Clear lists out before filling up with new data.
    ClearLists();


    int nEntities = I::IClientEntityList->GetHighestEntityIndex();


    std::vector<BaseEntity*>* vAllConnectedEnemies = m_vecAllConnectedEnemies.GetWriteBuffer();
    DOUBLEBUFFER_AUTO_RELEASE_WRITEBUFFER_SWAP(&m_vecAllConnectedEnemies, vAllConnectedEnemies);

    std::vector<BaseEntity*>* vAllConnectedTeammates = m_vecAllConnectedTeammates.GetWriteBuffer();
    DOUBLEBUFFER_AUTO_RELEASE_WRITEBUFFER_SWAP(&m_vecAllConnectedTeammates, vAllConnectedTeammates);

    std::vector<BaseEntity*>* vecTeamMates = m_vecPlayerFriendly.GetWriteBuffer();
    DOUBLEBUFFER_AUTO_RELEASE_WRITEBUFFER_SWAP(&m_vecPlayerFriendly, vecTeamMates);
    
    std::vector<BaseEntity*>* vecEnemies = m_vecPlayerEnemy.GetWriteBuffer();
    DOUBLEBUFFER_AUTO_RELEASE_WRITEBUFFER_SWAP(&m_vecPlayerEnemy, vecEnemies);

    // Enemy Handles...
    std::vector<int32_t>* pVecEnemyHandles = m_vecEnemyPlayerHandles.GetWriteBuffer();
    DOUBLEBUFFER_AUTO_RELEASE_WRITEBUFFER_SWAP(&m_vecEnemyPlayerHandles, pVecEnemyHandles);


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

        auto it = m_jumpTableHelperMap.find(iClassID);
        if (it == m_jumpTableHelperMap.end())
            continue;


        switch (it->second)
        {
        case ClassIdIndex::CTFPlayer:         
        {
            _ProcessPlayer((bFriendlyEntity == true ? vecTeamMates : vecEnemies), pEnt, pCmd->tick_count);
            
            if(bFriendlyEntity == false)
                pVecEnemyHandles->push_back(pEnt->GetRefEHandle());

            break;
        }
        case ClassIdIndex::CObjectSentrygun:  _ProcessSentry    (pEnt, pLocalPlayer->m_iTeamNum()); break;
        case ClassIdIndex::CObjectDispenser:  _ProcessDispenser (pEnt, pLocalPlayer->m_iTeamNum()); break;
        case ClassIdIndex::CObjectTeleporter: _ProcessTeleporter(pEnt, pLocalPlayer->m_iTeamNum()); break;
        case ClassIdIndex::CTFStickBomb:      _ProcessPipeBomb  (pEnt, pLocalPlayer->m_iTeamNum()); break;
        default: break;
        }
    }


    // Now swap them buffers plz.
    m_vecDispenserEnemy.SwapBuffer();  m_vecDispenserFriendly.SwapBuffer();
    m_vecSentryEnemy.SwapBuffer();     m_vecSentryFriendly.SwapBuffer();
    m_vecTeleporterEnemy.SwapBuffer(); m_vecTeleporterFriendly.SwapBuffer();
    m_vecEnemyPipeBombs.SwapBuffer();


    // Now since we have processed the entity list & also swapped all buffers, we can 
    // claim that all list's are valid enough to be used.
    m_bRecordsValid = true;
}


///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
static void ClearDoubleBufferVector(Containers::DoubleBuffer_t<std::vector<BaseEntity*>>& data)
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
    
    // Dormant & Non-Dormant players.
    ClearDoubleBufferVector(m_vecAllConnectedEnemies);
    ClearDoubleBufferVector(m_vecAllConnectedTeammates);

    // Enemy Handles...
    std::vector<int32_t>* pVecEnemyHandles = m_vecEnemyPlayerHandles.GetWriteBuffer();
    if(pVecEnemyHandles != nullptr)
    {
        pVecEnemyHandles->clear(); m_vecEnemyPlayerHandles.ReturnWriteBuffer(pVecEnemyHandles, false);
    }

    // Buildings...
    ClearDoubleBufferVector(m_vecSentryEnemy);
    ClearDoubleBufferVector(m_vecSentryFriendly);
    
    ClearDoubleBufferVector(m_vecDispenserEnemy);
    ClearDoubleBufferVector(m_vecDispenserFriendly);
    
    ClearDoubleBufferVector(m_vecTeleporterEnemy);
    ClearDoubleBufferVector(m_vecTeleporterFriendly);

    ClearDoubleBufferVector(m_vecEnemyPipeBombs);
}


///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
void EntityIterator_t::ClearBackTrackData()
{
    m_mapEntInfo.clear();
}


///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
bool EntityIterator_t::AreListsValid() const
{
    return m_bRecordsValid;
}


///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
void EntityIterator_t::InvalidateLists()
{
    m_bRecordsValid = false;
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
Containers::DoubleBuffer_t<std::vector<int32_t>>& EntityIterator_t::GetEnemyHandles()
{
    return m_vecEnemyPlayerHandles;
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
Containers::DoubleBuffer_t<std::vector<BaseEntity*>>& EntityIterator_t::GetEnemySentry()
{
    return m_vecSentryEnemy;
}


///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
Containers::DoubleBuffer_t<std::vector<BaseEntity*>>& EntityIterator_t::GetFriendlySentry()
{
    return m_vecSentryFriendly;
}


///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
Containers::DoubleBuffer_t<std::vector<BaseEntity*>>& EntityIterator_t::GetEnemyDispenser()
{
    return m_vecDispenserEnemy;
}


///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
Containers::DoubleBuffer_t<std::vector<BaseEntity*>>& EntityIterator_t::GetFrendlyDispenser()
{
    return m_vecDispenserFriendly;
}


///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
Containers::DoubleBuffer_t<std::vector<BaseEntity*>>& EntityIterator_t::GetEnemyTeleporter()
{
    return m_vecTeleporterEnemy;
}


///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
Containers::DoubleBuffer_t<std::vector<BaseEntity*>>& EntityIterator_t::GetFrendlyTeleporter()
{
    return m_vecTeleporterFriendly;
}


///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
Containers::DoubleBuffer_t<std::vector<BaseEntity*>>& EntityIterator_t::GetEnemyPipeBombs()
{
    return m_vecEnemyPipeBombs;
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
float EntityIterator_t::GetBackTrackTimeInSec() const
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
const LocalPlayerInfo_t& EntityIterator_t::GetLocalPlayerInfo()
{
    return m_localPlayerInfo;
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


    // Recording strafe prediction informatoin.
    F::movementSimulation.RecordStrafeData(pEnt, false);


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
    const int iMaxRecords = TIME_TO_TICK(GetBackTrackTimeInSec()) == 0 ? 1 : TIME_TO_TICK(GetBackTrackTimeInSec());
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
        std::vector<BaseEntity*>* vecFriendlySentires = m_vecSentryFriendly.GetWriteBuffer();
        DOUBLEBUFFER_AUTO_RELEASE_WRITEBUFFER_NOSWAP(&m_vecSentryFriendly, vecFriendlySentires);
        vecFriendlySentires->push_back(pEnt);
    }
    else
    {
        std::vector<BaseEntity*>* vecEnemySentires = m_vecSentryEnemy.GetWriteBuffer();
        DOUBLEBUFFER_AUTO_RELEASE_WRITEBUFFER_NOSWAP(&m_vecSentryEnemy, vecEnemySentires);
        vecEnemySentires->push_back(pEnt);
    }
}


///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
void EntityIterator_t::_ProcessDispenser(BaseEntity* pEnt, int iFriendlyTeam)
{
    if (pEnt->m_iTeamNum() == iFriendlyTeam)
    {
        std::vector<BaseEntity*>* vecFriendlyDispenser = m_vecDispenserFriendly.GetWriteBuffer();
        DOUBLEBUFFER_AUTO_RELEASE_WRITEBUFFER_NOSWAP(&m_vecDispenserFriendly, vecFriendlyDispenser);
        vecFriendlyDispenser->push_back(pEnt);
    }
    else
    {
        std::vector<BaseEntity*>* vecEnemyDispenser = m_vecDispenserEnemy.GetWriteBuffer();
        DOUBLEBUFFER_AUTO_RELEASE_WRITEBUFFER_NOSWAP(&m_vecDispenserEnemy, vecEnemyDispenser);
        vecEnemyDispenser->push_back(pEnt);
    }
}


///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
void EntityIterator_t::_ProcessTeleporter(BaseEntity* pEnt, int iFriendlyTeam)
{
    if (pEnt->m_iTeamNum() == iFriendlyTeam)
    {
        std::vector<BaseEntity*>* vecFriendlyTeleporter = m_vecTeleporterFriendly.GetWriteBuffer();
        DOUBLEBUFFER_AUTO_RELEASE_WRITEBUFFER_NOSWAP(&m_vecTeleporterFriendly, vecFriendlyTeleporter);
        vecFriendlyTeleporter->push_back(pEnt);
    }
    else
    {
        std::vector<BaseEntity*>* vecEnemyTeleporter = m_vecTeleporterEnemy.GetWriteBuffer();
        DOUBLEBUFFER_AUTO_RELEASE_WRITEBUFFER_NOSWAP(&m_vecTeleporterEnemy, vecEnemyTeleporter);
        vecEnemyTeleporter->push_back(pEnt);
    }
}


///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
void EntityIterator_t::_ProcessPipeBomb(BaseEntity* pEnt, int iFriendlyTeam)
{
    if (pEnt->m_iTeamNum() != iFriendlyTeam)
    {
        std::vector<BaseEntity*>* vecEnemyPipeBombs = m_vecEnemyPipeBombs.GetWriteBuffer();
        DOUBLEBUFFER_AUTO_RELEASE_WRITEBUFFER_NOSWAP(&m_vecEnemyPipeBombs, vecEnemyPipeBombs);
        vecEnemyPipeBombs->push_back(pEnt);
    }
}


///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
void EntityIterator_t::_UpdateLocalPlayerInfo(BaseEntity* pLocalPlayer, baseWeapon* pActiveWeapon)
{
    m_localPlayerInfo.m_iClass            = pLocalPlayer->m_iClass();
    m_localPlayerInfo.m_iLifeState        = pLocalPlayer->m_lifeState();
    m_localPlayerInfo.m_iTeam             = pLocalPlayer->m_iTeamNum();
    m_localPlayerInfo.m_iCond             = pLocalPlayer->GetPlayerCond();
    m_localPlayerInfo.m_vOrigin           = pLocalPlayer->GetAbsOrigin();
    m_localPlayerInfo.m_iEntIndex         = pLocalPlayer->entindex();
    m_localPlayerInfo.m_iActiveWeaponSlot = pActiveWeapon->getSlot();
    m_localPlayerInfo.m_refEHandle        = pLocalPlayer->GetRefEHandle();
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
    if (F::classIDHandler.IsInitialized() == false)
        return;

    m_jumpTableHelperMap.clear();

    m_jumpTableHelperMap.emplace(ClassID::CTFPlayer,                    ClassIdIndex::CTFPlayer        );
    m_jumpTableHelperMap.emplace(ClassID::CObjectSentrygun,             ClassIdIndex::CObjectSentrygun );
    m_jumpTableHelperMap.emplace(ClassID::CObjectDispenser,             ClassIdIndex::CObjectDispenser );
    m_jumpTableHelperMap.emplace(ClassID::CObjectTeleporter,            ClassIdIndex::CObjectTeleporter);
    m_jumpTableHelperMap.emplace(ClassID::CTFGrenadePipebombProjectile, ClassIdIndex::CTFStickBomb     );

    m_bJumpTableHelperInit = true;
}
