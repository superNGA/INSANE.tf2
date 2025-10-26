#include "BulletTarcers_t.h"

// UTILITY
#include "../../Utility/PullFromAssembly.h"
#include "../../Utility/Hook Handler/Hook_t.h"
#include "../../Utility/Signature Handler/signatures.h"
#include "../../Extra/math.h"

// SDK
#include "../../SDK/class/FireBulletInfo_t.h"
#include "../../SDK/class/BaseEntity.h"
#include "../../SDK/class/BaseWeapon.h"
#include "../../SDK/class/INetworkStringTable.h"
#include "../../SDK/class/CEffectData.h"
#include "../../Features/Entity Iterator/EntityIterator.h"
#include "../../Features/ImGui/NotificationSystem/NotificationSystem.h"


GET_RIP_ADRS_FROM_ASSEMBLY(particleEffectPrecacheTable, INetworkStringTable*, "48 8B 0D ? ? ? ? 48 8B D3 48 8B 01 FF 50 ? 3D", CLIENT_DLL, 3, 7, 7);
MAKE_SIG(UTIL_ParticleTracer, "48 89 5C 24 ? 48 89 74 24 ? 48 89 7C 24 ? 4C 89 74 24 ? 55 48 8D 6C 24 ? 48 81 EC ? ? ? ? 49 63 F1", CLIENT_DLL, void,
    const char*, vec*, vec*, int, int, bool)



///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
const char* TracerHandler_t::GetActiveTracer() const
{
    return m_szActiveTracerIndex;
}


///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
int TracerHandler_t::GetAttackerIndex() const
{
    return m_iAttackerEntIndex;
}


///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
bool TracerHandler_t::IsShotActive() const
{
    return m_bShotActive;
}


///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
void TracerHandler_t::InvalidateTracerCount()
{
    m_bInit = false;
    m_iLastLocalPlayerTracer = -1;
    m_iLastTeammateTracer    = -1;
    m_iLastEnemyTracer       = -1;
    Render::notificationSystem.PushBack("Invalidated tracer count");
}


///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
bool TracerHandler_t::_Initialize()
{
    // Total number of particle effects game has loaded ( usually around 5000 ).
    int nParticleEffects = (*ASM::particleEffectPrecacheTable)->GetNumStrings();
    if (nParticleEffects <= 0)
        return false;


    // Clamping tracer slider for all options...
    {
        Features::Misc::BulletTracers::View_YourTracers.m_Data.m_iMax      = nParticleEffects - 1;
        Features::Misc::BulletTracers::View_YourTracers.m_Data.m_iMin      = -1;
        Features::Misc::BulletTracers::View_TeammatesTracers.m_Data.m_iMax = nParticleEffects - 1;
        Features::Misc::BulletTracers::View_TeammatesTracers.m_Data.m_iMin = -1;
        Features::Misc::BulletTracers::View_EnemyTracers.m_Data.m_iMax     = nParticleEffects - 1;
        Features::Misc::BulletTracers::View_EnemyTracers.m_Data.m_iMin     = -1;
    }

    return true;
}


///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
void TracerHandler_t::BeginTracer(int iPlayerIndex, baseWeapon* pWeapon)
{
    if (m_bInit == false)
    {
        m_bInit = _Initialize();
        
        if (m_bInit == false)
            return;
    }


    const LocalPlayerInfo_t& localPlayerInfo = F::entityIterator.GetLocalPlayerInfo();
    m_szActiveTracerIndex                    = nullptr;
    int iEffectIndex                         = -1;

    if (iPlayerIndex == localPlayerInfo.m_iEntIndex)
    {
        iEffectIndex = Features::Misc::BulletTracers::View_YourTracers.GetData().m_iVal;
        if (iEffectIndex < 0)
            return;
        
        // In case, choose something from the dropdown, Find that particle effect and put its index in the slider.
        int iLocalPlayerTracerIndex = Features::Misc::BulletTracers::View_YourFancyTracers.GetData();
        if (iLocalPlayerTracerIndex != m_iLastLocalPlayerTracer)
        {
            m_iLastLocalPlayerTracer = iLocalPlayerTracerIndex;
            const char* szTracerName = Features::Misc::BulletTracers::View_YourFancyTracers.GetString();
            iEffectIndex             = (*ASM::particleEffectPrecacheTable)->FindStringIndex(szTracerName);

            // Clamp before setting...
            Features::Misc::BulletTracers::View_YourTracers.m_Data.m_iVal = std::clamp<int>(
                iEffectIndex, -1,
                Features::Misc::BulletTracers::View_YourTracers.m_Data.m_iMax);
        }
    }
    else
    {
        BaseEntity* pEnemy = I::IClientEntityList->GetClientEntity(iPlayerIndex);
        if (pEnemy == nullptr)
            return;

        if (pEnemy->m_iTeamNum() != localPlayerInfo.m_iTeam)
        {
            iEffectIndex = Features::Misc::BulletTracers::View_EnemyTracers.GetData().m_iVal;
            if (iEffectIndex < 0)
                return;


            // In case, choose something from the dropdown, Find that particle effect and put its index in the slider.
            int iEnemyTracerIndex = Features::Misc::BulletTracers::View_EnemyFancyTracers.GetData();
            if (iEnemyTracerIndex != m_iLastEnemyTracer)
            {
                m_iLastEnemyTracer       = iEnemyTracerIndex;
                const char* szTracerName = Features::Misc::BulletTracers::View_EnemyFancyTracers.GetString();
                iEffectIndex             = (*ASM::particleEffectPrecacheTable)->FindStringIndex(szTracerName);

                // Clamp before setting...
                Features::Misc::BulletTracers::View_EnemyTracers.m_Data.m_iVal = std::clamp<int>(
                    iEffectIndex, -1,
                    Features::Misc::BulletTracers::View_EnemyTracers.m_Data.m_iMax);
            }
        }
        else
        {
            iEffectIndex = Features::Misc::BulletTracers::View_TeammatesTracers.GetData().m_iVal;
            if (iEffectIndex < 0)
                return;


            // In case, choose something from the dropdown, Find that particle effect and put its index in the slider.
            int iTeammateTracerIndex = Features::Misc::BulletTracers::View_TeamatesFancyTracers.GetData();
            if (iTeammateTracerIndex != m_iLastTeammateTracer)
            {
                m_iLastTeammateTracer    = iTeammateTracerIndex;
                const char* szTracerName = Features::Misc::BulletTracers::View_TeamatesFancyTracers.GetString();
                iEffectIndex             = (*ASM::particleEffectPrecacheTable)->FindStringIndex(szTracerName);

                // Clamp before setting...
                Features::Misc::BulletTracers::View_TeammatesTracers.m_Data.m_iVal = std::clamp<int>(
                    iEffectIndex, -1,
                    Features::Misc::BulletTracers::View_TeammatesTracers.m_Data.m_iMax);
            }
        }
    }


    if (iEffectIndex < 0)
        return;

    m_szActiveTracerIndex = (*ASM::particleEffectPrecacheTable)->GetString(iEffectIndex);
    m_bShotActive         = true;
    m_iAttackerEntIndex   = iPlayerIndex;
}


///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
void TracerHandler_t::EndTracer()
{
    m_szActiveTracerIndex = nullptr;
    m_bShotActive         = false;
}


///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
MAKE_HOOK(CTFPlayer_FireBullet, "48 89 74 24 ? 55 57 41 55 41 56 41 57 48 8D AC 24 ? ? ? ? 48 81 EC ? ? ? ? F3 41 0F 10 58", __fastcall, CLIENT_DLL, void*,
    void* pAttacker, void* pWeapon, FireBulletInfo_t* pBulletInfo, bool bDoEffect, int iDamageType)
{
    if(F::tracerHandler.IsShotActive() == false)
    {
        return Hook::CTFPlayer_FireBullet::O_CTFPlayer_FireBullet(pAttacker, pWeapon, pBulletInfo, bDoEffect, iDamageType);
    }

    if(F::tracerHandler.GetActiveTracer() == nullptr || F::tracerHandler.GetAttackerIndex() < 0)
    {
        return Hook::CTFPlayer_FireBullet::O_CTFPlayer_FireBullet(pAttacker, pWeapon, pBulletInfo, bDoEffect, iDamageType);
    }


    vec vEnd = pBulletInfo->m_vecSrc + (pBulletInfo->m_vecDirShooting * pBulletInfo->m_flDistance);

    Sig::UTIL_ParticleTracer(
        F::tracerHandler.GetActiveTracer(),
        &pBulletInfo->m_vecSrc, &vEnd,
        F::tracerHandler.GetAttackerIndex(),
        0, false);


    return Hook::CTFPlayer_FireBullet::O_CTFPlayer_FireBullet(pAttacker, pWeapon, pBulletInfo, bDoEffect, iDamageType);
}