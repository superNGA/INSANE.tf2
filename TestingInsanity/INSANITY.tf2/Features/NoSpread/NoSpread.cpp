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

#define TIME_FOR_TICKS(x) ((float)(x) * (1.0f / 64.0f))

NoSpread_t::NoSpread_t()
{
	m_bStoredServerTime.store(false);
	m_flServerEngineTime.store(0.0f);
	m_flClientEngineTime.store(0.0f);
}

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
	
	// asking for server for playerPerf
	if (m_bStoredServerTime.load() == false)
		_AskForPlayerPerf();

	uint32_t iSeed   = _GetSeed();
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

uint32_t NoSpread_t::_GetSeed()
{
	if (m_bStoredServerTime.load() == false)
	{
		cons.Log(FG_RED, "NO SPREAD", "server time no stored yet");
		return 0;
	}

	float delta = m_flServerEngineTime.load() - m_flClientEngineTime.load();
	m_flCurServerEngineTime.store(PlatFloatTime() + delta);
	float allo = m_flCurServerEngineTime - std::fmod(m_flCurServerEngineTime.load(), m_flStepSize);
	m_flCurServerEngineTime.store(allo);
	float flSpreadTime = allo * 1000.0f;

	return std::bit_cast<int32_t>(flSpreadTime) & 255;
}

uint32_t NoSpread_t::_GetLocalSeed()
{
	if (PlatFloatTime == nullptr)
		return 0;
	return std::bit_cast<int32_t>((float)PlatFloatTime()) & 255;
}


float NoSpread_t::_GetMantissa(float flInput)
{
	// Interpret float as integer bits
	uint32_t bits = *(uint32_t*)&flInput;

	// Extract mantissa (lower 23 bits)
	uint32_t mantissaBits = bits & 0x007FFFFF;

	// Convert mantissa bits to float fraction (normalized)
	float mantissa = 1.0f; // implicit leading 1

	for (int i = 0; i < 23; ++i)
	{
		if (mantissaBits & (1 << (22 - i)))
		{
			mantissa += std::pow(2.0f, -(i + 1));
		}
	}

	return mantissa;
}


// this seems to be working now. 
bool NoSpread_t::_AskForPlayerPerf()
{
	void* pCS = tfObject.pCBaseClientState.load();
	if (pCS == nullptr || tfObject.pSendStringCommand == nullptr)
	{
		cons.Log(FG_RED, "NO SPREAD", "CBaseClientState global object not initialized yet");
		return false;
	}

	tfObject.pSendStringCommand(pCS, "playerperf\n");
	return true;
}


bool NoSpread_t::ParsePlayerPerf(std::string strPlayerperf)
{
	/*if (m_bStoredServerTime.load() == true)
		return true;*/

	// given string is not playerperf
	if (strPlayerperf.find("vel") == std::string::npos)
		return false;

	//const char* szPlayerPerf = strPlayerperf.c_str();
	// Extracting server engine time from string.
	bool bServerTimeStarted = false;	
	bool bPointOccured		= false;
	float flExtractedTime	= 0.0f;
	uint16_t iDecimalIndex	= 1;

	static float flLastExtractedTime = 0.0f;

	for (char x : strPlayerperf)
	{
		if (x == ' ' && bServerTimeStarted)
			break;

		if (x == '.')
			bPointOccured = true;

		float nX = (float)(x - '0');
		if (nX >= 0 && nX <= 9)
		{
			if(bPointOccured == false)
			{
				bServerTimeStarted = true;
				flExtractedTime *= 10.0f;
				flExtractedTime += (float)nX;
			}
			else
			{
				nX = nX / (float)pow(10.0f, (float)iDecimalIndex);
				iDecimalIndex++;
				flExtractedTime += (float)nX;
			}
		}

		if (iDecimalIndex == 4)
			break;
	}

	if (flExtractedTime == 0.0f)
		return false;

	flExtractedTime += tfObject.pGlobalVar->interval_per_tick;

	// getting client time at the time of storing server time.
	static bool setup = false;
	static uintptr_t fn = NULL;
	if (!setup)
	{
		fn = (uintptr_t)GetProcAddress(GetModuleHandle("tier0.dll"), "Plat_FloatTime");
		setup = true;
	}
	if(fn != NULL)
	{
		m_flClientEngineTime.store(((double(*)())fn)());
		PlatFloatTime = (T_PlatFloatTime)fn;
	}
	
	if(m_bStoredServerTime.load() == false)
		m_flServerEngineTime.store(flExtractedTime);

	if (flLastExtractedTime != 0.0f)
		m_flStepSize = abs(flLastExtractedTime - flExtractedTime);
	flLastExtractedTime = flExtractedTime;

	m_bStoredServerTime.store(true);
	m_iStorageTick = tfObject.pGlobalVar->tickcount;
	printf("EXTRACTED SERVER ENGINE TIME : %f @ client time : %f | MANTISSA : %f | determined step : %f\n", m_flServerEngineTime.load(), m_flClientEngineTime.load(), _GetMantissa(flExtractedTime), m_flStepSize);
	return true;
}