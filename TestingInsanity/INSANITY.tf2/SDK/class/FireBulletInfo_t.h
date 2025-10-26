#pragma once
#include "Basic Structures.h"
class BaseEntity;


struct FireBulletInfo_t
{
    int         m_iShots;
    vec         m_vecSrc;
    vec         m_vecDirShooting;
    vec         m_vecSpread;
    float       m_flDistance;
    int         m_iAmmoType;
    int         m_iTracerFreq;
    float       m_flDamage;
    int         m_iPlayerDamage;	// Damage to be used instead of m_flDamage if we hit a player
    int         m_nFlags;			// See FireBulletsFlags_t
    float       m_flDamageForceScale;
    BaseEntity* m_pAttacker;
    BaseEntity* m_pAdditionalIgnoreEnt;
    bool        m_bPrimaryAttack;
    bool        m_bUseServerRandomSeed;
};