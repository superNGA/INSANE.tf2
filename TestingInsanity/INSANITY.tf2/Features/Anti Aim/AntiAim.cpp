#include "AntiAim.h"
//======================= SDK =======================
#include "../../SDK/class/CUserCmd.h"
#include "../../SDK/class/IVEngineClient.h"
#include "../../SDK/class/CMultAnimState.h"
#include "../../SDK/class/BaseEntity.h"

//======================= Internal stuff =======================
#include "../../SDK/TF object manager/TFOjectManager.h"
#include "../../Utility/Interface Handler/Interface.h"
#include "../../Utility/Signature Handler/signatures.h"
#include "../../Utility/Profiler/Profiler.h"
#include "../../Extra/math.h"

MAKE_SIG(CBaseAnimating_InvalidateBoneCache, "8B 05 ? ? ? ? FF C8 C7 81", CLIENT_DLL, int64_t, void*);

//=========================================================================
//                     PUBLIC METHODS
//=========================================================================
void AntiAim_t::Run(CUserCmd* cmd, bool& bResult, bool* bSendPacket, BaseEntity* pLocalPlayer)
{
	PROFILER_RECORD_FUNCTION(CreateMove);

	if (Features::AntiAim::AntiAim::AntiAim.IsActive() == false)
		return;

	auto ent = pLocalPlayer;
	if (ent == nullptr)
		return;

	auto* pAnimState = *reinterpret_cast<CMultiPlayerAnimState**>((uintptr_t)ent + Netvars::DT_TFPlayer::m_hItem - 88);

	m_qAAAngles.pitch = -89.0f;
	m_qAAAngles.yaw	  = 180.0f;

	Maths::ClampQAngle(m_qAAAngles);

	if (*bSendPacket == true)
		cmd->viewangles = m_qAAAngles;

	StoreAABones(pLocalPlayer);

	_FixMovement(cmd);
	bResult = false;
}


//=========================================================================
//                     PRIVATE METHODS
//=========================================================================
void AntiAim_t::StoreAABones(BaseEntity* pLocalPlayer)
{
	if (pLocalPlayer == nullptr)
		return;
	auto* pAnimState = *reinterpret_cast<CMultiPlayerAnimState**>((uintptr_t)pLocalPlayer + Netvars::DT_TFPlayer::m_hItem - 88);
	//
	//// storing old data
	//float flOldFrameTime = tfObject.pGlobalVar->frametime;

	// PoseParameters are "CInterpolatedVarArray" which are supposed to arrays of 24 floats,
	// But it also has virtual functions from the class it inherits from. So maybe we need to store more than 24 bytes.
	//constexpr uint32_t POSE_PARAMETER_SIZE = (sizeof(float) * 24) + 0x8; //<- compensating for virtual table pointer.
	//char flOldPose[POSE_PARAMETER_SIZE];
	//memcpy(flOldPose, (void*)((uintptr_t)pLocalPlayer + netvar.m_flPoseParameter), POSE_PARAMETER_SIZE);
	//int nOldSeqence = *reinterpret_cast<int*>((uintptr_t)pLocalPlayer + netvar.m_nSequence);
	//float flOldCycle = *reinterpret_cast<float*>((uintptr_t)pLocalPlayer + netvar.m_flCycle);
	//
	char oldAnimState[sizeof(CMultiPlayerAnimState)];
	memcpy(&oldAnimState, pAnimState, sizeof(CMultiPlayerAnimState));

	// setupBones
	//tfObject.pGlobalVar->frametime = 0.0f;
	
	pAnimState->m_flCurrentFeetYaw  = m_qAAAngles.yaw;
	pAnimState->Update(m_qAAAngles.yaw, m_qAAAngles.pitch); // <- this is important.

	Sig::CBaseAnimating_InvalidateBoneCache(pLocalPlayer->GetBaseEntity());
	
	const qangle qOriRenderAngles = pLocalPlayer->GetRenderAngles();
	pLocalPlayer->GetRenderAngles().yaw = m_qAAAngles.yaw;
	pLocalPlayer->SetupBones(pBone, MAX_STUDIO_BONES, BONE_USED_BY_ANYTHING, tfObject.pGlobalVar->curtime);
	pLocalPlayer->GetRenderAngles() = qOriRenderAngles;

	// resetting back to original
	//tfObject.pGlobalVar->frametime = flOldFrameTime;
	//*reinterpret_cast<float*>((uintptr_t)pLocalPlayer + netvar.m_flCycle) = flOldCycle;
	//memcpy((void*)((uintptr_t)pLocalPlayer + netvar.m_flPoseParameter), flOldPose, POSE_PARAMETER_SIZE);
	//*reinterpret_cast<int*>((uintptr_t)pLocalPlayer + netvar.m_nSequence) = nOldSeqence;
	//
	memcpy(pAnimState, &oldAnimState, sizeof(CMultiPlayerAnimState));
}


inline float normalizeAngle(float angle)
{
	if (angle < 0.0f)
		return angle + 360.0f;
	return angle;
}


void AntiAim_t::_FixMovement(CUserCmd* pCmd)
{
	qangle qEngineAngles;
	I::iEngine->GetViewAngles(qEngineAngles);

	float fakeAnglesInDeg	 = 360.0f - normalizeAngle(pCmd->viewangles.yaw);
	float realAnglesInDeg	 = 360.0f - normalizeAngle(qEngineAngles.yaw);
							 
	float deltaAngleInRad	 = DEG2RAD((realAnglesInDeg - fakeAnglesInDeg));

	float orignalForwardMove = pCmd->forwardmove;
	float orignalSideMove	 = pCmd->sidemove;

	pCmd->forwardmove		 = cos(deltaAngleInRad) * orignalForwardMove - sin(deltaAngleInRad) * orignalSideMove;
	pCmd->sidemove			 = cos(deltaAngleInRad) * orignalSideMove + sin(deltaAngleInRad) * orignalForwardMove;
}