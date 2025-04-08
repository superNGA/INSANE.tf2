//=========================================================================
//                      No Spread
//=========================================================================
// by      : INSANE
// created : 05/04/2025
// 
// purpose : Removes spread (random weapon inaccuracy ) from weapon
//-------------------------------------------------------------------------

#include "NoSpread.h"
#include "../../SDK/class/Basic Structures.h"
#include "../../SDK/class/CUserCmd.h"
#include "../../SDK/TF object manager/TFOjectManager.h"
#include "../../SDK/Entity Manager/entityManager.h"
#include "../../SDK/class/BaseWeapon.h"
#include "../../SDK/class/FileWeaponInfo.h"
#include "../../Extra/math.h"
#include <algorithm>

NoSpread_t noSpread;

//=========================================================================
//                     PUBLIC METHODS
//=========================================================================

void NoSpread_t::run(CUserCmd* cmd, bool& result)
{
    if (config.aimbotConfig.bNoSpread == false)
        return;

    BaseEntity* pLocalPlayer  = entityManager.getLocalPlayer();
    baseWeapon* pActiveWeapon = entityManager.getActiveWeapon();
    if (pLocalPlayer == nullptr || pActiveWeapon == nullptr)
        return;

	if (!(cmd->buttons & IN_ATTACK))
		return;
	
	uint32_t iSeed   = _GetSeed(cmd);
	cmd->random_seed = iSeed;
	float flBaseSpread = tfObject.pGetWeaponSpread((void*)pActiveWeapon);
	printf("EST. seed : %d | base spread : %f\n", iSeed, flBaseSpread);
	
	tfObject.pRandomSeed(iSeed);
	// random X & Y
	float flRandomX = tfObject.pRandomFloat(-0.5, 0.5) + tfObject.pRandomFloat(-0.5, 0.5);
	float flRandomY = tfObject.pRandomFloat(-0.5, 0.5) + tfObject.pRandomFloat(-0.5, 0.5);

	vec vecDirShooting, vecRight, vecUp;
	Maths::AngleVectors(cmd->viewangles, &vecDirShooting, &vecRight, &vecUp);

	vec vecDir = vecDirShooting + (vecRight * flRandomX * flBaseSpread) + (vecUp * flRandomY * flBaseSpread);
	// vecDis seems to be a unit vector at all times, so this can be avoided but still keeping it here just for the seak of certanity...
	vecDir.Normalize(); 

	qangle qFinal;
	Maths::VectorAngles(vecDir, qFinal);
	Maths::ClampQAngle(qFinal);
	
	cmd->viewangles = cmd->viewangles + (cmd->viewangles - qFinal);
	Maths::ClampQAngle(cmd->viewangles);
	result = false;
}


//=========================================================================
//                     PRIVATE METHODS
//=========================================================================

uint32_t NoSpread_t::_GetSeed(CUserCmd* cmd)
{
    return tfObject.MD5_PseudoRandom(cmd->command_number) & 255; // <- this is the real shit.
}