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

	_DemandPlayerPerf(cmd);

	if (_ShouldRun(cmd) == false)
		return;

	BaseEntity* pLocalPlayer = entityManager.getLocalPlayer();
	baseWeapon* pActiveWeapon = entityManager.getActiveWeapon();
	if (pLocalPlayer == nullptr || pActiveWeapon == nullptr)
		return;

	//_DemandPlayerPerf(cmd);

	uint32_t iSeed = _GetSeed(cmd);

	_FixSpread(cmd, iSeed, pActiveWeapon);

#ifdef DEBUG_NOSPREAD
	Render::InfoWindow.AddToInfoWindow("serverType",			 (m_bLoopBack ? "local server" : "OFFICAL SERVER! :)"));
	//Render::InfoWindow.AddToInfoWindow("Mantissa step",			 std::format("Mantissa step : {:.6f}", m_flMantissaStep));
	Render::InfoWindow.AddToInfoWindow("Request time",			 std::format("Request time : {:.6f}", m_flRequestTime));
	Render::InfoWindow.AddToInfoWindow("delta time",			 std::format("delta time : {:.6f}", m_flDeltaClientServer));
	Render::InfoWindow.AddToInfoWindow("Sync Status",			 std::format("SYNC STATUS : {}", m_syncStatus == SYNC_DONE ? "SYNCED :)" : "syncing :("));
	Render::InfoWindow.AddToInfoWindow("Approax server up time", std::format("Server has been up for {}", _GetServerUpTime(m_flServerEngineTime)));
	Render::InfoWindow.AddToInfoWindow("time deltas count",		 std::format("Stored {} deltas", m_vecTimeDeltas.size()));
	Render::InfoWindow.AddToInfoWindow("Last Aquisition delay",	 std::format("Last Aquisition delay {}", m_flAquisitionDelay));
	//printf("using seed : %d\n", iSeed);
#endif

	result = false;
}

//=========================================================================
//                     PRIVATE METHODS
//=========================================================================


bool NoSpread_t::_ShouldRun(CUserCmd* cmd)
{
	if (config.aimbotConfig.bNoSpread == false)
		return false;

	if ((cmd->buttons & IN_ATTACK) == false)
		return false;

	// perform a melle cheak on active weapon here.

	return true;
}

// the problem is most likely not here! 
uint32_t NoSpread_t::_GetSeed(CUserCmd* cmd) const
{
	double flPredictedServerTime = m_flServertime + m_flSyncOffset + m_flResponseTime;
	//double flPredictedServerTime = ExportFn::Plat_FloatTime.Call<double>() + m_flSyncOffset + m_flResponseTime;
	float flTime = static_cast<float>(flPredictedServerTime * 1000.0);
	return std::bit_cast<int32_t>(flTime) & 255;

	//float time{ (m_flServertime + m_flSyncOffset + m_flResponseTime) * 1000.0f };
	//return *reinterpret_cast<int*>((char*)&time) & 255;
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

		vec vbulletSpread = vecDirShooting + ((vecRight * flRandomX) * flBaseSpread) + ((vecUp * flRandomY) * flBaseSpread);
		vbulletSpread.Normalize();

		vAverageSpread = vAverageSpread + vbulletSpread; // adding up to the average.
		vecBulletCorrections.push_back(vbulletSpread);   // pushing into record.
	}

	// this averging out logic from amalgun, seems like will only work with single bullet weapons and shotguns. 
	// ( don't know how well minigums will work with this. )
	vAverageSpread = vAverageSpread / static_cast<float>(iBullets); // Calculating the actual average.
	vec vFixedSpread(FLT_MAX, FLT_MAX, FLT_MAX);
	for (auto& spread : vecBulletCorrections)
	{
		if (spread.DistTo(vAverageSpread) < vFixedSpread.DistTo(vAverageSpread))
		{
			vFixedSpread = spread;
		}
	}

	qangle qFinal;
	Maths::VectorAngles(vAverageSpread, qFinal);
	//Maths::VectorAnglesFromSDK(vFixedSpread, qFinal);
	//Maths::ClampQAngle(qFinal);

	cmd->viewangles = cmd->viewangles + (cmd->viewangles - qFinal);
	Maths::ClampQAngle(cmd->viewangles);
	//Maths::ClampAngles(cmd->viewangles);

	return true;
}


void NoSpread_t::_DemandPlayerPerf(CUserCmd* cmd)
{
	if (m_bWaitingForPlayerPerf == true)
		return;

	//LOG("Asked for player perf");

	Sig::CBaseClientState_SendStringCmd.Call<void>(I::CBaseClientState, "playerperf\n");
	m_flRequestTime			= static_cast<float>(ExportFn::Plat_FloatTime.Call<double>());
	m_bWaitingForPlayerPerf = true;
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


// The fucking loopback server's engine time is also at a constant difference of 0.1 to 0.01 MF
bool NoSpread_t::ParsePlayerPerf(std::string strPlayerperf)
{
	// Actual server stuff
	std::smatch match;
	if (strPlayerperf[0] == 2)
		strPlayerperf = strPlayerperf.substr(1);

	// extracing first float. i.e. the server's engine time
	if (std::regex_match(strPlayerperf, match, std::regex(R"((\d+.\d+)\s\d+\s\d+\s\d+.\d+\s\d+.\d+\svel\s\d+.\d+)")))
		printf("[ %f ] <- request time\n[ %f ] receieved time | Time for 1 tick : %f\n", m_flRequestTime, std::stof(match[1]), tfObject.pGlobalVar->interval_per_tick);

	m_bWaitingForPlayerPerf = false;

	// LoopBack server check
	auto* pNetChannel = I::iEngineClientReplay->GetNetChannel();
	m_bLoopBack = pNetChannel != nullptr && pNetChannel->IsLoopback();
	if (m_bLoopBack == true)
	{
		m_flDeltaClientServer = 0.0f;
		m_bIsSynced = false;
		return true;
	}

	//// Actual server stuff
	//std::smatch match;
	//if (strPlayerperf[0] == 2)
	//	strPlayerperf = strPlayerperf.substr(1);
	//
	//// extracing first float. i.e. the server's engine time
	//if (std::regex_match(strPlayerperf, match, std::regex(R"((\d+.\d+)\s\d+\s\d+\s\d+.\d+\s\d+.\d+\svel\s\d+.\d+)")) == false)
	//	return false;

	//// checking if match found is valid to prevent false positives ( kinda redundant but its ok )
	//if (match.size() != 2)
	//	return false;

	////delete this
	//printf("[ %f ] <- request time\n[ %f ] receieved time | Time for 1 tick : %f\n", m_flRequestTime, std::stof(match[1]), tfObject.pGlobalVar->interval_per_tick);

	float flNewServerEngineTime = std::stof(match[1]);

	// skipping older times, and keeping only the latest time.
	if (flNewServerEngineTime < m_flServerEngineTime)
		return true;
	m_flServerEngineTime = flNewServerEngineTime; // if got latest time, then store it.

	// calculating deltas :)
	m_flAquisitionDelay = static_cast<float>(ExportFn::Plat_FloatTime.Call<double>()) - m_flRequestTime; // time difference between requesting and reciving message from server.
	m_vecTimeDeltas.push_back(m_flServerEngineTime - m_flRequestTime + tfObject.pGlobalVar->interval_per_tick); // +m_flLatencyAtRecordTime + I::iEngineClientReplay->GetNetChannel()->GetLatency(NET_FLOW_TYPE::FLOW_INCOMING);
	if (m_vecTimeDeltas.empty() == false && m_vecTimeDeltas.size() > MAX_TIME_RECORDS)
		m_vecTimeDeltas.pop_front();

	/// NOTE : Although amalgun has used 1 tick as the delay "Aquisition Delay" from observing the tick counts between aquiring and requesting
	///			seems to be a significantly more then 1 tick. The "tick delta" was ranging from 8 to 20 ticks.

	// storing the average delta
	m_flDeltaClientServer = std::reduce(m_vecTimeDeltas.begin(), m_vecTimeDeltas.end()) / m_vecTimeDeltas.size();

	// calculating the mantissa step.
	m_flMantissaStep = _CalcMantissaStep(m_flServerEngineTime);

	if (m_flMantissaStep > 1.0f && m_vecTimeDeltas.size() >= MAX_TIME_RECORDS)
		m_bIsSynced = true;

	return true;
}


/*
ASSUMPTION :
	we will believe that, when we ask for playerperf at tick 1, it will give us playerperf message at tick 1 but the data will be of last simulating time, 
	i.e. tick 0. so add 1 tick worth of time to the receieved engine time to get time "which must have been at the time of asking playerperf" i.e. at tick 1.
*/
bool NoSpread_t::ParsePlayerPerfExperimental(std::string szMsg)
{
	// std::regex_match(szMsg, match, std::regex(R"((\d+.\d+)\s\d+\s\d+\s\d+.\d+\s\d+.\d+\svel\s\d+.\d+)"))
	if (szMsg[0] == 2)
		szMsg = szMsg.substr(1);

	std::smatch match;
	if (std::regex_match(szMsg, match, std::regex(R"((\d+.\d+)\s\d+\s\d+\s\d+.\d+\s\d+.\d+\svel\s\d+.\d+)")) == false)
		return false;

	/*if (match.size() != 2)
		return false;*/

	m_bWaitingForPlayerPerf = false;
	//FAIL_LOG("ready to ask player perf again NIGA");

	float flNewTime = std::stof(match[1].str());

	// if refering to old time, then skip
	if (flNewTime < m_flServertime)
		return true;

	m_flPrevServertime  = m_flServertime;
	m_flServertime		= flNewTime;

	m_flResponseTime	= ExportFn::Plat_FloatTime.Call<double>() - m_flRequestTime;

	if (m_flPrevServertime <= 0.0f)
		return true;

	// if this not zero then this iteration is not our first iteration.
	if (m_flEstimatedServerTime > 0.0f)
	{
		float flDelta = m_flServertime - m_flEstimatedServerTime;
		if (flDelta == 0.0f) // if we estimated the server time successfully
		{
			m_flSyncOffset  = m_flEstimatedServerTimeDelta;
			m_bIsSynced		= true;
			m_syncStatus	= SYNC_DONE;

			static bool printed = false;
			if (!printed)
			{
				WIN_LOG("%.6f = %.6f | SYNCED :) now if not no spread then maths wrong.", m_flEstimatedServerTime, m_flServertime);
				printed = false;
			}
		}
		else if(m_syncStatus == SYNC_DONE)
		{
			m_syncStatus = SYNC_LOST;
		}
	}

	m_flEstimatedServerTimeDelta = m_flServertime - m_flPrevServertime;
	m_flEstimatedServerTime		 = m_flServertime + m_flEstimatedServerTimeDelta;

	return true;
}
