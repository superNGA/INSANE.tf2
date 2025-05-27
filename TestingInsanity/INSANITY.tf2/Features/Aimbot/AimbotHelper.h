//=========================================================================
//                      AIMBOT HELPER
//=========================================================================
// by      : INSANE
// created : 25/05/2025
// 
// purpose : Handles some common stuff for all different aimbots
//-------------------------------------------------------------------------
#pragma once

#include <vector>

#include "../FeatureHandler.h"

class CUserCmd;
class baseWeapon;
class BaseEntity;

class AimbotHelper_t
{
public:
    void Run(BaseEntity* pLocalPlayer, baseWeapon* pActiveWeapon, CUserCmd* pCmd, bool* pSendPackets);

    // Holds potential targets
    struct AimbotTargetData_t
    {
        std::vector<BaseEntity*> m_vecEnemyPlayers;
        std::vector<BaseEntity*> m_vecEnemySentry;
        std::vector<BaseEntity*> m_vecEnemyBuildings;
        std::vector<BaseEntity*> m_vecEnemyProjectiles;
    };
    inline const AimbotTargetData_t& GetAimbotTargetData() 
    { 
        _ClearAimbotData();
        _ConstructAimbotTargetData(); 
        return m_AllTargets; 
    }

private:
    AimbotTargetData_t m_AllTargets;
    void _ConstructAimbotTargetData();
    void _ClearAimbotData();
};
DECLARE_FEATURE_OBJECT(aimbotHelper, AimbotHelper_t)

DEFINE_TAB(Aimbot, 6)