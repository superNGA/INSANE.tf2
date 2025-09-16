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

#include "../FeatureHandler.h"
#include "../../Utility/ClassIDHandler/ClassIDHandler.h"


class BaseEntity;
class baseWeapon;
class CUserCmd;

constexpr float MAX_BACKTRACK_TIME = 0.2f; // in seconds ofcourse.

///////////////////////////////////////////////////////////////////////////
struct BackTrackRecord_t
{
    int m_iFlags = 0;
    int m_iTick = 0;
    vec m_vOrigin;
    qangle m_qViewAngles;
    matrix3x4_t m_bones[MAX_STUDIO_BONES];
};
///////////////////////////////////////////////////////////////////////////


///////////////////////////////////////////////////////////////////////////
class EntityIterator_t
{
public:
    void Run(BaseEntity* pLocalPlayer, baseWeapon* pActiveWeapon, CUserCmd* pCmd);
    void ClearLists();
    void ClearBackTrackData();
    
    std::vector<BaseEntity*>& GetEnemyPlayerList();
    std::vector<BaseEntity*>& GetFriendlyPlayerList();

    std::vector<BaseEntity*>& GetEnemyPlayers();
    std::vector<BaseEntity*>& GetFrendlyPlayers();

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

private:
    // All mighty back track records.
    std::unordered_map<BaseEntity*, std::deque<BackTrackRecord_t>> m_mapEntInfo = {};
    float m_flBackTrackTime = 0.0f; // This is the final record time for backtrack.

    void _ProcessPlayer(BaseEntity* pEnt, int iFriendlyTeam, int iCurrentTick);
    std::vector<BaseEntity*> m_vecPlayerEnemy, m_vecPlayerFriendly;

    void _ProcessSentry(BaseEntity* pEnt, int iFriendlyTeam);
    std::vector<BaseEntity*> m_vecSentryEnemy, m_vecSentryFriendly;

    void _ProcessDispenser(BaseEntity* pEnt, int iFriendlyTeam);
    std::vector<BaseEntity*> m_vecDispenserEnemy, m_vecDispenserFriendly;

    void _ProcessTeleporter(BaseEntity* pEnt, int iFriendlyTeam);
    std::vector<BaseEntity*> m_vecTeleporterEnemy, m_vecTeleporterFriendly;


    // Ent list for playerlist. ( contains dormant entities & localplayer too )
    std::vector<BaseEntity*> m_vecPlayerListEnemy, m_vecPlayerListMates;

    // Entity to Material map.
    std::unordered_map<BaseEntity*, int>  m_mapEntToMateiral = {};

    // Fake lag shit.
    std::deque<DatagramStat_t> m_qSequences = {};

    // Call backs...
    typedef void(*T_CallBack)(BaseEntity*);
    void _CallBack(BaseEntity* pEnt);
    std::vector<void*> m_vecCallBacks = {};

    // Since I can't use switches for class IDs, I am using this as a 
    // Unordered map to get from class id ( dynamic ) to compile time known numbers.
    void _ConstructJumpTableHelper();
    bool m_bJumpTableHelperInit = false;
    std::unordered_map<int, int> m_jumpTableHelperMap = {};
};
///////////////////////////////////////////////////////////////////////////

DECLARE_FEATURE_OBJECT(entityIterator, EntityIterator_t)

DEFINE_TAB(BackTrack, 13)
DEFINE_SECTION(BackTrack, "BackTrack", 6)
DEFINE_FEATURE(
    BackTrack_In_Ms, FloatSlider_t, BackTrack, BackTrack, 1, FloatSlider_t(0.0f, 0.0f, 1000.0f)
)