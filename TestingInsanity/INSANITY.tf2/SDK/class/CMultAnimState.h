#pragma once
#include "Basic Structures.h"
#include "Activity.h"

enum PlayerAnimEvent_t
{
	PLAYERANIMEVENT_ATTACK_PRIMARY,
	PLAYERANIMEVENT_ATTACK_SECONDARY,
	PLAYERANIMEVENT_ATTACK_GRENADE,
	PLAYERANIMEVENT_RELOAD,
	PLAYERANIMEVENT_RELOAD_LOOP,
	PLAYERANIMEVENT_RELOAD_END,
	PLAYERANIMEVENT_JUMP,
	PLAYERANIMEVENT_SWIM,
	PLAYERANIMEVENT_DIE,
	PLAYERANIMEVENT_FLINCH_CHEST,
	PLAYERANIMEVENT_FLINCH_HEAD,
	PLAYERANIMEVENT_FLINCH_LEFTARM,
	PLAYERANIMEVENT_FLINCH_RIGHTARM,
	PLAYERANIMEVENT_FLINCH_LEFTLEG,
	PLAYERANIMEVENT_FLINCH_RIGHTLEG,
	PLAYERANIMEVENT_DOUBLEJUMP,

	// Cancel.
	PLAYERANIMEVENT_CANCEL,
	PLAYERANIMEVENT_SPAWN,

	// Snap to current yaw exactly
	PLAYERANIMEVENT_SNAP_YAW,

	PLAYERANIMEVENT_CUSTOM,				// Used to play specific activities
	PLAYERANIMEVENT_CUSTOM_GESTURE,
	PLAYERANIMEVENT_CUSTOM_SEQUENCE,	// Used to play specific sequences
	PLAYERANIMEVENT_CUSTOM_GESTURE_SEQUENCE,

	// TF Specific. Here until there's a derived game solution to this.
	PLAYERANIMEVENT_ATTACK_PRE,
	PLAYERANIMEVENT_ATTACK_POST,
	PLAYERANIMEVENT_GRENADE1_DRAW,
	PLAYERANIMEVENT_GRENADE2_DRAW,
	PLAYERANIMEVENT_GRENADE1_THROW,
	PLAYERANIMEVENT_GRENADE2_THROW,
	PLAYERANIMEVENT_VOICE_COMMAND_GESTURE,
	PLAYERANIMEVENT_DOUBLEJUMP_CROUCH,
	PLAYERANIMEVENT_STUN_BEGIN,
	PLAYERANIMEVENT_STUN_MIDDLE,
	PLAYERANIMEVENT_STUN_END,
	PLAYERANIMEVENT_PASSTIME_THROW_BEGIN,
	PLAYERANIMEVENT_PASSTIME_THROW_MIDDLE,
	PLAYERANIMEVENT_PASSTIME_THROW_END,
	PLAYERANIMEVENT_PASSTIME_THROW_CANCEL,

	PLAYERANIMEVENT_ATTACK_PRIMARY_SUPER,

	PLAYERANIMEVENT_COUNT
};

// Gesture Slots.
enum
{
	GESTURE_SLOT_ATTACK_AND_RELOAD,
	GESTURE_SLOT_GRENADE,
	GESTURE_SLOT_JUMP,
	GESTURE_SLOT_SWIM,
	GESTURE_SLOT_FLINCH,
	GESTURE_SLOT_VCD,
	GESTURE_SLOT_CUSTOM,

	GESTURE_SLOT_COUNT,
};

struct GestureSlot_t
{
	int					m_iGestureSlot;
	Activity			m_iActivity;
	bool				m_bAutoKill;
	bool				m_bActive;
	void*				m_pAnimLayer; // pointer to anim layer
};

struct MultiPlayerPoseData_t
{
	int			m_iMoveX;
	int			m_iMoveY;
	int			m_iAimYaw;
	int			m_iAimPitch;
	int			m_iBodyHeight;
	int			m_iMoveYaw;
	int			m_iMoveScale;

	float		m_flEstimateYaw;
	float		m_flLastAimTurnTime;

	void Init()
	{
		m_iMoveX = 0;
		m_iMoveY = 0;
		m_iAimYaw = 0;
		m_iAimPitch = 0;
		m_iBodyHeight = 0;
		m_iMoveYaw = 0;
		m_iMoveScale = 0;
		m_flEstimateYaw = 0.0f;
		m_flLastAimTurnTime = 0.0f;
	}
};

struct DebugPlayerAnimData_t
{
	float		m_flSpeed;
	float		m_flAimPitch;
	float		m_flAimYaw;
	float		m_flBodyHeight;
	vec2	m_vecMoveYaw;
};

struct MultiPlayerMovementData_t
{
	// Set speeds to -1 if they are not used.
	float		m_flWalkSpeed;
	float		m_flRunSpeed;
	float		m_flSprintSpeed;
	float		m_flBodyYawRate;
};

class CMultiPlayerAnimState
{
public:
	virtual void				GANDU_COLLEGE();
	virtual void				ClearAnimationState();
	virtual void				DoAnimationEvent(PlayerAnimEvent_t event, int nData = 0);
	virtual void				CalcMainActivity();
	virtual void				Update(float eyeYaw, float eyePitch);
	virtual void				Release(void);
	virtual Activity			TranslateActivity(Activity actDesired);

	virtual void				SetRunSpeed(float flSpeed);	    // { m_MovementData.m_flRunSpeed = flSpeed; }
	virtual void				SetWalkSpeed(float flSpeed);	// { m_MovementData.m_flWalkSpeed = flSpeed; }
	virtual void				SetSprintSpeed(float flSpeed);	// { m_MovementData.m_flSprintSpeed = flSpeed; }

	// Debug
	virtual void				ShowDebugInfo(void);
	virtual void				DebugShowAnimState(int iStartLine);

	// Feet.
	// If you are forcing aim yaw, your code is almost definitely broken if you don't include a delay between 
	// teleporting and forcing yaw. This is due to an unfortunate interaction between the command lookback window,
	// and the fact that m_flEyeYaw is never propogated from the server to the client.
	// TODO: Fix this after Halloween 2014.
	virtual void				Init(void* pPlayer, MultiPlayerMovementData_t& movementData);

	// Allow inheriting classes to override SelectWeightedSequence
	virtual int					SelectWeightedSequence(Activity activity);
	virtual void				RestartMainSequence();

	virtual void				GetOuterAbsVelocity(vec& vel);

	virtual bool				HandleJumping(Activity& idealActivity);
	virtual bool				HandleDucking(Activity& idealActivity);
	virtual bool				HandleMoving(Activity& idealActivity);
	virtual bool				HandleSwimming(Activity& idealActivity);
	virtual bool				HandleDying(Activity& idealActivity);

	// Gesture Slots
	virtual void				RestartGesture(int iGestureSlot, Activity iGestureActivity, bool bAutoKill = true);
	virtual float				GetGesturePlaybackRate(void) { return 1.0f; }
	virtual void				PlayFlinchGesture(Activity iActivity);

	virtual float				CalcMovementSpeed(bool* bIsMoving);
	virtual float				CalcMovementPlaybackRate(bool* bIsMoving);
	// Pose parameters.
	virtual void				ComputePoseParam_MoveYaw(void* pStudioHdr);
	virtual void				ComputePoseParam_AimPitch(void* pStudioHdr);
	virtual void				ComputePoseParam_AimYaw(void* pStudioHdr);
	virtual void				EstimateYaw(void);

	virtual float				GetCurrentMaxGroundSpeed();
	virtual void				ComputeSequences(void* pStudioHdr);
	virtual bool				ShouldUpdateAnimState();

	GestureSlot_t*				m_aGestureSlots;
	bool						m_bForceAimYaw;
	void*						m_pPlayer;

	qangle						m_angRender;

	// Pose parameters.
	bool						m_bPoseParameterInit;
	MultiPlayerPoseData_t		m_PoseParameterData;
	DebugPlayerAnimData_t		m_DebugAnimData;

	bool						m_bCurrentFeetYawInitialized;
	float						m_flLastAnimationStateClearTime;

	float						m_flEyeYaw;
	float						m_flEyePitch;
	float						m_flGoalFeetYaw;
	float						m_flCurrentFeetYaw;
	float						m_flLastAimTurnTime;

	MultiPlayerMovementData_t	m_MovementData;

	// Jumping.
	bool						m_bJumping;
	float						m_flJumpStartTime;
	bool						m_bFirstJumpFrame;

	// Swimming.
	bool						m_bInSwim;
	bool						m_bFirstSwimFrame;

	// Dying
	bool						m_bDying;
	bool						m_bFirstDyingFrame;

	// Last activity we've used on the lower body. Used to determine if animations should restart.
	Activity					m_eCurrentMainSequenceActivity;

	// Specific full-body sequence to play
	int							m_nSpecificMainSequence;
};