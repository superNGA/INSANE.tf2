//=========================================================================
//                      AIM-BOT MANAGMENT SYSTEM
//=========================================================================
// by      : INSANE
// created : 25/02/2025
// 
// purpose : Manages all aimbot mechanims. hit-scan / projectile, or sorts 
// of shit
//-------------------------------------------------------------------------

#pragma once

//=======================FORWARD DECLERATIONS=======================
class matrix3x4_t;
class I_client_entity;
typedef I_client_entity BaseEntity;

class Aimbot_t
{
public:
    bool processPlayerAimbot(BaseEntity* entity, const matrix3x4_t* boneMatrix, bool isWeaponProjectile);
};
inline Aimbot_t ABMS;