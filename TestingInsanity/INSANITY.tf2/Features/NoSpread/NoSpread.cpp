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

GET_EXPORT_FN(Plat_FloatTime, "tier0.dll");

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
	if (_ShouldRun(cmd) == false)
		return;

	BaseEntity* pLocalPlayer = entityManager.getLocalPlayer();
	baseWeapon* pActiveWeapon = entityManager.getActiveWeapon();
	if (pLocalPlayer == nullptr || pActiveWeapon == nullptr)
		return;

	_DemandPlayerPerf();

	uint32_t iSeed = _GetSeed();

	_FixSpread(cmd, iSeed, pActiveWeapon);

#ifdef DEBUG_NOSPREAD
	Render::InfoWindow.AddToInfoWindow("serverType",			 (m_bLoopBack ? "local server" : "OFFICAL SERVER! :)"));
	Render::InfoWindow.AddToInfoWindow("Mantissa step",			 std::format("Mantissa step : {:.6f}", m_flMantissaStep));
	Render::InfoWindow.AddToInfoWindow("Request time",			 std::format("Request time : {:.6f}", m_flRequestTime));
	Render::InfoWindow.AddToInfoWindow("delta time",			 std::format("delta time : {:.6f}", m_flDeltaClientServer));
	Render::InfoWindow.AddToInfoWindow("request latency",		 std::format("outgoing latency at request : {}ms", m_flLatencyAtRecordTime));
	Render::InfoWindow.AddToInfoWindow("Approax server up time", std::format("Server has been up for {}", _GetServerUpTime(m_flServerEngineTime)));
	Render::InfoWindow.AddToInfoWindow("time deltas count",		 std::format("Stored {} deltas", m_vecTimeDeltas.size()));
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

	return true;
}

uint32_t NoSpread_t::_GetSeed()
{
	double flPredictedServerTime = ExportFn::Plat_FloatTime.Call<double>() + m_flDeltaClientServer;// +(I::iEngineClientReplay->GetNetChannel()->GetLatency(NET_FLOW_TYPE::FLOW_OUTGOING));
	float flTime = static_cast<float>(flPredictedServerTime * 1000.0);
	return std::bit_cast<int32_t>(flTime) & 255;
}

bool NoSpread_t::_FixSpread(CUserCmd* cmd, uint32_t seed, baseWeapon* pActiveWeapon)
{
	// testing with bullet count
	const char* weaponAlias = Sig::baseWeapon_WeaponIDToAlias.Call<char*>(pActiveWeapon->getWeaponIndex());
	auto wpnInfo = Sig::baseWeapon_LookUpWeaponInfoSlot.Call<int16_t>(weaponAlias);
	auto wpnFile = static_cast<CTFWeaponInfo*>(Sig::baseWeapon_GetWeaponFileHandle.Call<FileWeaponInfo_t*>(wpnInfo));
	LOG("%s has %d bullets", weaponAlias, wpnFile->GetWeaponData(1).m_nBulletsPerShot);

	// working stuff here.
	float flBaseSpread = Sig::BaseWeapon_GetWeaponSpread.Call<float>((void*)pActiveWeapon);
	
	tfObject.pRandomSeed(seed);
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

	return true;
}


void NoSpread_t::_DemandPlayerPerf()
{
	if (m_bWaitingForPlayerPerf == true)
		return;

	if (m_vecTimeDeltas.size() >= MAX_TIME_RECORDS) // if we have enought of these fucking records
		return;

	if (m_bIsSynced == true)
		return;

	Sig::CBaseClientState_SendStringCmd.Call<void>(I::CBaseClientState, "playerperf\n");
	m_flRequestTime = static_cast<float>(ExportFn::Plat_FloatTime.Call<double>());
	m_flLatencyAtRecordTime = I::iEngineClientReplay->GetNetChannel()->GetLatency(NET_FLOW_TYPE::FLOW_OUTGOING);
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

bool NoSpread_t::ParsePlayerPerf(std::string strPlayerperf)
{
	m_bWaitingForPlayerPerf = false;

	// LoopBack server check
	auto* pNetChannel = I::iEngineClientReplay->GetNetChannel();
	m_bLoopBack = pNetChannel != nullptr && pNetChannel->IsLoopback();
	if (m_bLoopBack == true)
	{
		m_flDeltaClientServer = 0.0f;
		m_bIsSynced = true;
		return true;
	}

	// Actual server stuff
	std::smatch match;
	if (strPlayerperf[0] == 2)
		strPlayerperf = strPlayerperf.substr(1);
	
	// extracing first float. i.e. the server's engine time
	if (std::regex_match(strPlayerperf, match, std::regex(R"((\d+.\d+)\s\d+\s\d+\s\d+.\d+\s\d+.\d+\svel\s\d+.\d+)")) == false)
		return false;

	// checking if match found is valid to prevent false positives ( kinda redundant but its ok )
	if (match.size() != 2)
		return false;

	float flNewServerEngineTime = std::stof(match[1]);

	// skipping older times, and keeping only the latest time.
	if (flNewServerEngineTime < m_flServerEngineTime)
		return true;
	m_flServerEngineTime = flNewServerEngineTime; // if got latest time, then store it.

	// calculating delta :)
	m_vecTimeDeltas.push_back(m_flServerEngineTime - m_flRequestTime + tfObject.pGlobalVar->interval_per_tick); // +m_flLatencyAtRecordTime + I::iEngineClientReplay->GetNetChannel()->GetLatency(NET_FLOW_TYPE::FLOW_INCOMING);
	if (m_vecTimeDeltas.empty() == false && m_vecTimeDeltas.size() > MAX_TIME_RECORDS)
		m_vecTimeDeltas.pop_front();

	// storing the average delta
	m_flDeltaClientServer = std::reduce(m_vecTimeDeltas.begin(), m_vecTimeDeltas.end()) / m_vecTimeDeltas.size();

	// calculating the mantissa step.
	m_flMantissaStep = _CalcMantissaStep(m_flServerEngineTime);

	if (m_flMantissaStep > 1.0f && m_vecTimeDeltas.size() >= MAX_TIME_RECORDS)
		m_bIsSynced = true;

	return true;
}