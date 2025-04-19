//=========================================================================
//                      No Spread
//=========================================================================
// by      : INSANE
// created : 05/04/2025
// 
// purpose : Removes spread (random weapon inaccuracy ) from weapon
//-------------------------------------------------------------------------

#include "NoSpread.h"
#include <algorithm>
#include <regex>
#include <string>
#include <numeric>
#include <bit>

#include "../../SDK/class/Basic Structures.h"
#include "../../SDK/class/CUserCmd.h"
#include "../../SDK/TF object manager/TFOjectManager.h"
#include "../../SDK/Entity Manager/entityManager.h"
#include "../../SDK/class/BaseWeapon.h"
#include "../../SDK/class/FileWeaponInfo.h"
#include "../../Extra/math.h"
#include "../../Utility/signatures.h"
#include "../../SDK/class/I_EngineClientReplay.h"
#include "../ImGui/InfoWindow/InfoWindow_t.h"
#include "../../Utility/ExportFnHelper.h"

GET_EXPORT_FN(Plat_FloatTime, "tier0.dll")
GET_EXPORT_FN(RandomSeed, VSTDLIB_DLL)
GET_EXPORT_FN(RandomFloat, VSTDLIB_DLL)

// weapon porperty related fns
MAKE_SIG(baseWeapon_WeaponIDToAlias, "48 63 C1 48 83 F8 ? 73 ? 85 C9 78 ? 48 8D 0D ? ? ? ? 48 8B 04 C1 C3 33 C0 C3 48 83 E9", CLIENT_DLL)
MAKE_SIG(baseWeapon_LookUpWeaponInfoSlot, "48 8B D1 48 8D 0D ? ? ? ? E9 ? ? ? ? CC 48 89 5C 24 ? 48 89 6C 24", CLIENT_DLL)
MAKE_SIG(baseWeapon_GetWeaponFileHandle, "66 3B 0D", CLIENT_DLL)

MAKE_SIG(BaseWeapon_GetWeaponSpread, "48 89 5C 24 ? 57 48 83 EC ? 4C 63 91", CLIENT_DLL)
MAKE_SIG(CBaseClientState_SendStringCmd, "48 81 EC ? ? ? ? 48 8B 49", ENGINE_DLL)
MAKE_INTERFACE_SIGNATURE(CBaseClientState, "48 8D 0D ? ? ? ? E8 ? ? ? ? 41 8B 57",void, ENGINE_DLL, 0x3)
//MAKE_INTERFACE_SIGNATURE(CBaseClientState, "48 8D 0D ? ? ? ? E8 ? ? ? ? F3 0F 5E 05", void, ENGINE_DLL)

//=========================================================================
//                     PUBLIC METHODS
//=========================================================================

void NoSpread_t::Run(CUserCmd* cmd, bool& result)
{
	if (config.aimbotConfig.bNoSpread == false)
		return;

	pCmd = cmd;

	_RequestPlayerPerf(cmd);

#ifdef DEBUG_NOSPREAD
	Render::InfoWindow.AddToInfoWindow("sync status",	m_eSyncState == SYNC_DONE ? "SYNCED :)" : "sync-ing :(");
	Render::InfoWindow.AddToInfoWindow("delta",			std::format("delta : {:.6f}", m_flDelta));
	Render::InfoWindow.AddToInfoWindow("offset",		std::format("offset : {:.6f}", m_flOffset));
	Render::InfoWindow.AddToInfoWindow("Mantissa step", std::format("Mantisa step : {:.3f}", _CalcMantissaStep(m_flServerTime)));
#endif

	if (_ShouldRun(cmd) == false)
		return;

	BaseEntity* pLocalPlayer = entityManager.getLocalPlayer();
	baseWeapon* pActiveWeapon = entityManager.getActiveWeapon();
	if (pLocalPlayer == nullptr || pActiveWeapon == nullptr)
		return;

	uint32_t iSeed = GetSeed();
	m_iSeed.store(iSeed);

	_FixSpread(cmd, iSeed, pActiveWeapon);
	//_FixSpreadSingleBullet(cmd, iSeed, pActiveWeapon);

	result = false;
}

//=========================================================================
//                     PRIVATE METHODS
//=========================================================================


bool NoSpread_t::_ParsePlayerPerf(std::string sMsg)
{
	if (sMsg[0] == 2)
		sMsg = sMsg.substr(1);

	// for now we are skipping loopback server stuff and just getting the MM server stuff to work.
	std::smatch match;
	if (std::regex_match(sMsg, match, std::regex(R"((\d+.\d+)\s\d+\s\d+\s\d+.\d+\s\d+.\d+\svel\s\d+.\d+)")) == false)
		return false;
	
	if (match.size() != 2)
		return false;
	m_bWaitingForPlayerPerf = false;
	
	float flScrappedTime = std::stof(match[1].str());
	if (flScrappedTime < m_flServerTime)
		return true;
	m_flServerTime = flScrappedTime;

	float flRecieveTime = ExportFn::Plat_FloatTime.Call<double>();
	float flResponseTime = flRecieveTime - m_flRequestTime;

	m_qServerTimes.push_back(m_flServerTime - flRecieveTime + flResponseTime);
	if (m_qServerTimes.size() > MAX_TIME_RECORDS)
		m_qServerTimes.pop_front();
	m_flDelta = std::reduce(m_qServerTimes.begin(), m_qServerTimes.end()) / m_qServerTimes.size();
	m_flDelta += tfObject.pGlobalVar->interval_per_tick * float(config.miscConfig.iServerTimeOffset); // adding offset for finner adjustment.

	float flMantissaStep = _CalcMantissaStep(m_flServerTime);
	if (flMantissaStep > 1.0f && m_qServerTimes.size() >= MAX_TIME_RECORDS)
		m_eSyncState = SYNC_DONE;

	return true;
}

void NoSpread_t::_RequestPlayerPerf(CUserCmd* cmd)
{
	if (m_bWaitingForPlayerPerf == true)
		return;

	if (m_eSyncState == SYNC_DONE)
		return;

	/*if (cmd->tick_count - m_iRequestTick < DELTA_UPDATE_FREQUENCY && m_eSyncState == SYNC_DONE)
		return;*/

	Sig::CBaseClientState_SendStringCmd.Call<void>(I::CBaseClientState, "playerperf\n");
	m_flRequestTime		= static_cast<float>(ExportFn::Plat_FloatTime.Call<double>());
	m_iRequestTick		= cmd->tick_count;
	m_flRequestLatency	= I::iEngineClientReplay->GetNetChannel()->GetLatency(FLOW_OUTGOING);
	m_bWaitingForPlayerPerf = true;
}


uint32_t NoSpread_t::GetSeed()
{
	// compensating for lantecy.
	double flflTime = ExportFn::Plat_FloatTime.Call<double>() + m_flDelta;// +m_flOffset + I::iEngineClientReplay->GetNetChannel()->GetLatency(FLOW_OUTGOING);
	float  flTime	= float(flflTime * 1000.0f);
	return std::bit_cast<int32_t>(flTime) & 255;
}


bool NoSpread_t::_ShouldRun(CUserCmd* cmd)
{
	if (config.aimbotConfig.bNoSpread == false)
		return false;

	if ((cmd->buttons & IN_ATTACK) == false)
		return false;

	// perform a melle cheak on active weapon here.

	return true;
}


bool NoSpread_t::_FixSpread(CUserCmd* cmd, uint32_t seed, baseWeapon* pActiveWeapon)
{
	auto wpnInfo = pActiveWeapon->GetTFWeaponInfo();
	int32_t iBullets = wpnInfo->GetWeaponData(0).m_nBulletsPerShot;
	if (iBullets <= 0)
		return false;

	vec vAverageSpread(0.0f, 0.0f, 0.0f);
	float flBaseSpread = Sig::BaseWeapon_GetWeaponSpread.Call<float>((void*)pActiveWeapon);

	std::vector<vec> vecBulletCorrections;

	for (int bullet = 0; bullet < iBullets; bullet++)
	{
		ExportFn::RandomSeed.Call<void>(seed + bullet);
		
		// random X & Y
		float flRandomX = ExportFn::RandomFloat.Call<float>(-0.5f, 0.5f) + ExportFn::RandomFloat.Call<float>(-0.5f, 0.5f);
		float flRandomY = ExportFn::RandomFloat.Call<float>(-0.5f, 0.5f) + ExportFn::RandomFloat.Call<float>(-0.5f, 0.5f);

		vec vecDirShooting, vecRight, vecUp;
		Maths::AngleVectors(cmd->viewangles, &vecDirShooting, &vecRight, &vecUp);

		vec vbulletSpread = vecDirShooting + (vecRight * (flRandomX * flBaseSpread)) + (vecUp * (flRandomY * flBaseSpread));
		vbulletSpread.Normalize();

		vAverageSpread = vAverageSpread + vbulletSpread; // adding up to the average.
		vecBulletCorrections.push_back(vbulletSpread);   // pushing into record.
	}

	vAverageSpread = vAverageSpread / static_cast<float>(iBullets); // Calculating the actual average.
	vec vFixedSpread(FLT_MAX, FLT_MAX, FLT_MAX);
	for (auto& spread : vecBulletCorrections)
	{
		if (spread.DistTo(vAverageSpread) < vFixedSpread.DistTo(vAverageSpread))
		{
			vFixedSpread = spread;
		}
	}
	//printf("calculated spread : %.2f %.2f %.2f\n", vFixedSpread.x, vFixedSpread.y, vFixedSpread.z);

	qangle qFinal;
	//Maths::VectorAngles(vAverageSpread, qFinal);
	Maths::VectorAngles(vFixedSpread, qFinal);
	//Maths::VectorAnglesFromSDK(vFixedSpread, qFinal);

	cmd->viewangles = cmd->viewangles + (cmd->viewangles - qFinal);
	Maths::ClampQAngle(cmd->viewangles);
	//Maths::ClampAngles(cmd->viewangles);

	return true;
}

// I totally made it myself. like 100%. you feel me, like fucking 100%. I am really good at fucking maths and shi..
float NoSpread_t::_CalcMantissaStep(float flInput)
{
	// Calculate the delta to the next representable value
	float nextValue = std::nextafter(flInput, std::numeric_limits<float>::infinity());
	float mantissaStep = (nextValue - flInput) * 1000;

	// Get the closest mantissa (next power of 2)
	return powf(2, ceilf(logf(mantissaStep) / logf(2)));
}

std::string NoSpread_t::_GetServerUpTime(float flServerEngineTime)
{
	uint64_t iServerEngineTime = static_cast<uint64_t>(flServerEngineTime);
	int iDays	 = iServerEngineTime / 86400;
	int iHours	 = iServerEngineTime / 3600 % 24;
	int iMinutes = iServerEngineTime / 60 % 60;
	int iSeconds = iServerEngineTime % 60;

	if (iDays != 0)
		return std::format("{}d {}h", iDays, iHours);
	else if (iHours != 0)
		return std::format("{}h {}m", iHours, iMinutes);
	else
		return std::format("{}m {}s", iMinutes, iSeconds);
}