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
	
	uint32_t iSeed = _GetSeed(cmd);
	cmd->random_seed = iSeed; // Remove this bullshit.
	float flBaseSpread = tfObject.pGetWeaponSpread((void*)pActiveWeapon);
	//printf("EST. seed : %d | base spread : %f\n", iSeed, flBaseSpread);
	
	tfObject.pRandomSeed(iSeed);
	// random X & Y
	float flRandomX = tfObject.pRandomFloat(-0.5, 0.5) + tfObject.pRandomFloat(-0.5, 0.5);
	float flRandomY = tfObject.pRandomFloat(-0.5, 0.5) + tfObject.pRandomFloat(-0.5, 0.5);
	// spread X & Y
	float flSpreadX = flRandomX * flBaseSpread;
	float flSpreadY = flRandomY * flBaseSpread;

	// resetting the random seed just in case this is not done by the game, but From what I see in IDA
	// this is done by the game, so I would be removing it once the feature is done.
	tfObject.pRandomSeed(iSeed);

	vec vecDirShooting, vecRight, vecUp;
	AngleVectors(cmd->viewangles, &vecDirShooting, &vecRight, &vecUp); // this part is working, maybe we can even invert the pitch so it is 100% correct but we will se about that.

	vec vecDir = vecDirShooting + (vecRight * flSpreadX) + (vecUp * flSpreadY);
	vecDir.Normalize();
	qangle qFinal;
	VectorAngles(vecDir, qFinal);
	qangle qFixedAngles = cmd->viewangles + (cmd->viewangles - qFinal);
	
	// clamping pitch
	if (qFixedAngles.pitch < -89.0f) qFixedAngles.pitch = -89.0f;
	else if (qFixedAngles.pitch > 89.0f) qFixedAngles.pitch = +89.0f;

	// clamping yaw
	if (qFixedAngles.yaw > 180.f) qFixedAngles.yaw = 180.0f;
	else if (qFixedAngles.yaw < -180.f) qFixedAngles.yaw = -180.0f;

	cmd->viewangles = qFixedAngles;
	//qangle delta = qFinal - cmd->viewangles;
	//cmd->viewangles = cmd->viewangles - delta;

	result = false;
}


//=========================================================================
//                     PRIVATE METHODS
//=========================================================================

uint32_t NoSpread_t::_GetSeed(CUserCmd* cmd)
{
    return tfObject.MD5_PseudoRandom(cmd->command_number) & 255; // <- this is the real shit.
}

uint32_t NoSpread_t::_GetSeed()
{
	static bool setup = false;
	static uintptr_t fn;
	if (!setup)
	{
		// double(*)()
		fn = (uintptr_t)GetProcAddress(GetModuleHandle("tier0.dll"), "Plat_FloatTime");
	}
	
	double time = ((double(*)())fn)();
	double dFloatTime = time; // removed + deltaTime
	float flTime = float(dFloatTime * 1000.0);
	return std::bit_cast<int32_t>(flTime) & 255;
}


/*
// random seed calc...
	int64_t nRandomSeed = tfObject.MD5_PseudoRandom(cmd->command_number) & 0x7FFFFFFF;

	// Getting required fns...
	static bool bCapturedNoSpreadStuff = false;
	static T_randomSeed RandomSeed = nullptr;
	static T_RandomFloatEx RandomFloat = nullptr;
	if (bCapturedNoSpreadStuff == false)
	{
		auto HWND = GetModuleHandle(VSTDLIB_DLL);
		if (HWND == 0)
			cons.Log(FG_RED, "CreateMove", "no handle :(");
		else
		{
			RandomSeed = (T_randomSeed)GetProcAddress(HWND, (LPCSTR)60);
			if (RandomSeed == nullptr)
				cons.Log(FG_RED, "CreateMove", "no randomseed() :(");
			else
				cons.Log(FG_GREEN, "CreateMove", "got RandomSeed()");

			RandomFloat = (T_RandomFloatEx)GetProcAddress(HWND, "RandomFloat");
			if (RandomFloat == nullptr)
				cons.Log(FG_RED, "CreateMove", "no RandomFloat() :(");
			else
				cons.Log(FG_GREEN, "CreateMove", "got RandomFloat()");
		}

		bCapturedNoSpreadStuff = true;
	}
	if ((cmd->buttons & IN_ATTACK) && tfObject.pRandomGausianFloat && tfObject.pGetWeaponSpread && RandomSeed && RandomFloat)
	{
		float flBaseSpread = tfObject.pGetWeaponSpread(pActiveWeapon);

		RandomSeed(nRandomSeed); // maybe try +1 ( i.e. on tick ahead if this dones't work )
		//float flRandomX = tfObject.pRandomGausianFloat(-0.5, 0.5) + tfObject.pRandomGausianFloat(-0.5, 0.5);
		//float flRandomY = tfObject.pRandomGausianFloat(-0.5, 0.5) + tfObject.pRandomGausianFloat(-0.5, 0.5);
		float flRandomX = RandomFloat(-0.5, 0.5) + RandomFloat(-0.5, 0.5);
		float flRandomY = RandomFloat(-0.5, 0.5) + RandomFloat(-0.5, 0.5);
		float flSpreadX = flRandomX * flBaseSpread;
		float flSpreadY = flRandomY * flBaseSpread;

		/* TODO :
		* -> understand that the forward vector bullshit is.
		* -> see how game is adding spread.
		*/

		//vec vecDirShooting, vecRight, vecUp;
		//AngleVectors(cmd->viewangles, &vecDirShooting, &vecRight, &vecUp);
		//
		//vecRight.x *= flSpreadX;
		//vecRight.y *= flSpreadX;
		//vecRight.z *= flSpreadX;
		//vecUp.x *= flSpreadY;
		//vecUp.y *= flSpreadY;
		//vecUp.z *= flSpreadY;
		//
		//// Vector vecDir = vecDirShooting + xSpread * vecRight + ySpread * vecUp; // <- games implementation
		//vec vecNoSpreadDir = vecDirShooting + vecRight + vecUp; // getting spread direction
		//vecNoSpreadDir.Normalize();
		//qangle noSpeadAngles(vecNoSpreadDir.x, vecNoSpreadDir.y, vecNoSpreadDir.z);
		//cmd->viewangles = cmd->viewangles - noSpeadAngles;
		//
		//printf("%.2f %.2f | base spread : %.2f\n", noSpeadAngles.pitch, noSpeadAngles.yaw, flBaseSpread);
		//result = false;
		//	}
		//*/