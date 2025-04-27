#pragma once
#include <deque>
#include "../features.h"
#include "../../SDK/class/Source Entity.h"

class CUserCmd;
class baseWeapon;

/*
* TODO : 
* -> try timming hits
* -> make semi-automatic crit hack
* -> optimize in the end
*/

#define MAX_CRIT_COMMANDS 10

class CritHack_t
{
public:
    void Run(CUserCmd* pCmd, baseWeapon* pActiveWeapon, BaseEntity* pLocalPlayer);
    void Reset();

private:
    void _InitializeCVars();
    void _ScanForCritCommands(CUserCmd* pCmd, baseWeapon* pActiveWeapon, BaseEntity* pLocalPlayer);
    int _GetBestCritCommand(CUserCmd* pCmd);

    void _MeleeCritHack(int iCritCommand, CUserCmd* pCmd, baseWeapon* pActiveWeapon, BaseEntity* pLocalPlayer);
    bool _isSeedCritMelee(int iSeed, baseWeapon* pActiveWeapon, BaseEntity* pLocalPlayer);


    bool  m_bCVarsInitialized   = false;

    float m_flCritBucketBottom  = 0;
    float m_flCritBucketCap     = 0;
    float m_flCritBucketDefault = 0;

    std::deque<int> m_qCritCommands = {};

};
ADD_FEATURE(critHack, CritHack_t);

MAKE_FEATURE_BOOL(CritHack, "CritHack->toggleCritHack", 1);