//=========================================================================
//                      ENTITY ITERATOR
//=========================================================================
// by      : INSANE
// created : 02/09/2025
// 
// purpose : Iterates entity list & stores required imformation ( backtrack n more )
//-------------------------------------------------------------------------
#pragma once

#include <unordered_map>
#include <vector>
#include <deque>

#include "../../Utility/Containers/DoubleBuffer.h"
#include "../FeatureHandler.h"
#include "../../Utility/ClassIDHandler/ClassIDHandler.h"


class BaseEntity;
class baseWeapon;
class CUserCmd;

constexpr float MAX_BACKTRACK_TIME = 0.2f; // in seconds ofcourse.

///////////////////////////////////////////////////////////////////////////
struct BackTrackRecord_t
{
    BackTrackRecord_t()
    {
        m_iFlags = 0; m_iTick = 0; 
        m_vOrigin.Init(); m_qViewAngles.Init();
        // ain't setting bones to 0, waste of resoures.
    }
    int         m_iFlags = 0;
    int         m_iTick = 0;
    vec         m_vOrigin;
    qangle      m_qViewAngles;
    matrix3x4_t m_bones[MAX_STUDIO_BONES];
};
///////////////////////////////////////////////////////////////////////////


///////////////////////////////////////////////////////////////////////////
struct LocalPlayerInfo_t
{
    int          m_iClass     = 0;
    lifeState_t  m_iLifeState = lifeState_t::LIFE_DEAD;
    int          m_iTeam      = 0;
    uint32_t     m_iCond      = 0u;
};
///////////////////////////////////////////////////////////////////////////


///////////////////////////////////////////////////////////////////////////
class EntityIterator_t
{
public:
    void Run(BaseEntity* pLocalPlayer, baseWeapon* pActiveWeapon, CUserCmd* pCmd);
    void ClearLists();
    void ClearBackTrackData();
    
    Containers::DoubleBuffer_t<std::vector<BaseEntity*>>& GetAllConnectedEnemiesList();
    Containers::DoubleBuffer_t<std::vector<BaseEntity*>>& GetAllConnectedTeamMatesList();

    Containers::DoubleBuffer_t<std::vector<BaseEntity*>>& GetEnemyPlayers();
    Containers::DoubleBuffer_t<std::vector<BaseEntity*>>& GetFrendlyPlayers();
    
    // Buildings...
    Containers::DoubleBuffer_t<std::vector<BaseEntity*>>& GetEnemySentry();
    Containers::DoubleBuffer_t<std::vector<BaseEntity*>>& GetFriendlySentry();
    
    Containers::DoubleBuffer_t<std::vector<BaseEntity*>>& GetEnemyDispenser();
    Containers::DoubleBuffer_t<std::vector<BaseEntity*>>& GetFrendlyDispenser();
    
    Containers::DoubleBuffer_t<std::vector<BaseEntity*>>& GetEnemyTeleporter();
    Containers::DoubleBuffer_t<std::vector<BaseEntity*>>& GetFrendlyTeleporter();


    // NOTE: The latest record is @ front & the oldest record is @ the back.
    std::deque<BackTrackRecord_t>* GetBackTrackRecord(BaseEntity* pEnt);
    void  SetBackTrackTime(const float flBackTrackTime);
    float GetBackTrackTime() const;

    enum ClassIdIndex : int
    {
        CTFPlayer,
        CObjectSentrygun,
        CObjectDispenser,
        CObjectTeleporter
    };

    void RegisterCallBack(void* pFunction);

    const std::unordered_map<int, int>& GetJumpTableHelper() const;

    void SetEntityMaterial(BaseEntity* pEnt, int iMatIndex);
    int  GetEntityMaterial(BaseEntity* pEnt);
    void ClearEntityMaterialOverrides();



    // Fake latency stuff..
    struct DatagramStat_t
    {
        int   m_nInSequenceNr = 0;
        int   m_nInReliableState = 0;
        float m_flTimeStamp = 0.0f;
    };
    void ClearSequenceData();
    std::deque<DatagramStat_t>& GetDatagramSequences();
    int m_iLastAddedSequence = -1;


    const LocalPlayerInfo_t& GetLocalPlayerInfo();

private:
    // All mighty back track records.
    std::unordered_map<BaseEntity*, std::deque<BackTrackRecord_t>> m_mapEntInfo = {};
    float m_flBackTrackTime = 0.0f; // This is how much time the last backtrack record is behind from the actual player's location.


    // Storing all Non-Dormant player's in thread safe containers...
    void _ProcessPlayer(std::vector<BaseEntity*>* vecListToPushIn, BaseEntity* pEnt, int iCurrentTick);
    Containers::DoubleBuffer_t<std::vector<BaseEntity*>> m_vecPlayerEnemy, m_vecPlayerFriendly;


    // Storing all Non-Dormant building's in thread safe containers...
    void _ProcessSentry(BaseEntity* pEnt, int iFriendlyTeam);
    Containers::DoubleBuffer_t<std::vector<BaseEntity*>> m_vecSentryEnemy, m_vecSentryFriendly;

    void _ProcessDispenser(BaseEntity* pEnt, int iFriendlyTeam);
    Containers::DoubleBuffer_t<std::vector<BaseEntity*>> m_vecDispenserEnemy, m_vecDispenserFriendly;

    void _ProcessTeleporter(BaseEntity* pEnt, int iFriendlyTeam);
    Containers::DoubleBuffer_t<std::vector<BaseEntity*>> m_vecTeleporterEnemy, m_vecTeleporterFriendly;


    // Entity stored in these thread safe containers, might or might not be dormant. 
    // These are strictly for using in the player list ( UI feature ) and are meant to be used with care.
    // Ent list for playerlist. ( contains dormant entities & localplayer too )
    Containers::DoubleBuffer_t<std::vector<BaseEntity*>> m_vecAllConnectedEnemies, m_vecAllConnectedTeammates;


    // Entity to Material map.
    // These contain material index ( with respect to material gen. ) for each entity.
    // This is used to give selected entities custom & distinguishable material...
    std::unordered_map<BaseEntity*, int>  m_mapEntToMateiral = {};


    // Fake lag shit.
    // This is used for create a fake lag effect, i.e. used to extend our backtrack window.
    std::deque<DatagramStat_t> m_qSequences = {};


    // This contains local-player's information which is requested by other threads
    // at times when we cannot gaurantee that the local player is valid or not. 
    // So, to get thread safety we will store information here, and accept the fact that 
    // sometimes the data might be little delay n such...
    LocalPlayerInfo_t m_localPlayerInfo;
    void _UpdateLocalPlayerInfo(BaseEntity* pLocalPlayer, baseWeapon* pActiveWeapon);


    // Call-backs...
    typedef void(*T_CallBack)(BaseEntity*);
    void _CallBack(BaseEntity* pEnt);
    std::vector<void*> m_vecCallBacks = {};


    // NOTE : I get my classID's as runtime ( look @ ClassIDHandler.h ).
    // Since I can't use switches for class IDs, I am using this as a 
    // Unordered map to get entity's classID's in O(1) time ( maybe :) ).
    void _ConstructJumpTableHelper();
    bool m_bJumpTableHelperInit = false;
    std::unordered_map<int, int> m_jumpTableHelperMap = {};
};
///////////////////////////////////////////////////////////////////////////

DECLARE_FEATURE_OBJECT(entityIterator, EntityIterator_t)

DEFINE_TAB(BackTrack, 4)
DEFINE_SECTION(BackTrack, "BackTrack", 6)
DEFINE_FEATURE(
    BackTrack_In_Ms, "BackTrack", FloatSlider_t, BackTrack, BackTrack, 1, FloatSlider_t(0.0f, 0.0f, 1000.0f)
)
