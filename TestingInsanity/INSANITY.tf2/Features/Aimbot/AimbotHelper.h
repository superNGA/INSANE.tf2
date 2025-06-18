//=========================================================================
//                      AIMBOT HELPER
//=========================================================================
// by      : INSANE
// created : 25/05/2025
// 
// purpose : Handles some common stuff for all different aimbots
//-------------------------------------------------------------------------
#pragma once

// STD Libs
#include <vector>
#include <unordered_map>

// SDK
#include "../../SDK/Class ID Manager/classIDManager.h"
#include "../FeatureHandler.h"

class CUserCmd;
class baseWeapon;
class BaseEntity;

class AimbotHelper_t
{
public:
    void Run(BaseEntity* pLocalPlayer, baseWeapon* pActiveWeapon, CUserCmd* pCmd, bool* pSendPackets);

    struct AimbotTargetData_t
    {
        std::vector<BaseEntity*> m_vecEnemyPlayers;
        std::vector<BaseEntity*> m_vecFriendlyPlayers;

        std::vector<BaseEntity*> m_vecEnemySentry;
        std::vector<BaseEntity*> m_vecFriendlySentry;

        std::vector<BaseEntity*> m_vecEnemyDispensers;
        std::vector<BaseEntity*> m_vecFriendlyDispensers;

        std::vector<BaseEntity*> m_vecEnemyTeleporters;
        std::vector<BaseEntity*> m_vecFriendlyTeleporters;

        std::vector<BaseEntity*> m_vecEnemyRockets;
        std::vector<BaseEntity*> m_vecFriendlyRockets;

        std::vector<BaseEntity*> m_vecEnemyPipeBombs;
        std::vector<BaseEntity*> m_vecFriendlyPipeBombs;
    };

    inline const AimbotTargetData_t& GetAimbotTargetData()
    {
        _ClearAimbotData();
        _ConstructAimbotTargetData();
        return m_aimbotTargetData;
    }

private:
    AimbotTargetData_t m_aimbotTargetData;

    void _ConstructAimbotTargetData();
    void _ClearAimbotData();
};
DECLARE_FEATURE_OBJECT(aimbotHelper, AimbotHelper_t)

DEFINE_TAB(Aimbot, 6)