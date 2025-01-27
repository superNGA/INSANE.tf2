#pragma once
#include <Windows.h>
#include <cstdint>
#include <cmath>

#define MAX_STUDIO_BONES	128
#define HITBOX_BONES		0x00000100

/* bone type defs */
#define BONE_HEAD 6             // Head
#define BONE_CHEST 7			// Chest
#define BONE_LEFT_SHOULDER 9	// left shoulder
#define BONE_RIGHT_SHOULDER 10	// right shoulder
#define BONE_LEFT_FOOT 34		// left foot
#define BONE_RIGHT_FOOT 35		// right foot
#define BONE_LEFT_KNEE 15		// left knee
#define BONE_RIGHT_KNEE 16		// right knee
#define BONE_FACE 33

/* game constants */
#define TF2_GRAVITY 800.0f
#define DIRECT_HIT_ROCKET_LAUNCHER 1980.0f

struct qangle
{
	qangle() : pitch(0.0f), yaw(0.0f), roll(0.0f){}
	qangle(float PITCH, float YAW, float ROLL) : pitch(PITCH), yaw(YAW), roll(ROLL) {}

	float pitch, yaw, roll;
};

struct vec
{
	vec() : x(0.0f), y(0.0f), z(0.0f) {}
	vec(float X, float Y, float Z) : x(X), y(Y), z(Z) {}
	float x, y, z;

	vec operator+(vec other)
	{
		return vec(x + other.x, y + other.y, z + other.z);
	}
	vec operator-(vec other) const
	{
		return vec(x - other.x, y - other.y, z - other.z);
	}

	const float mag()
	{
		return sqrt(x * x + y * y + z * z);
	}

	bool isEmpty(){
		if (x || y || z) return false;
		return true;
	}
};

/* most used to store screen coordinates in this software, nothing too big */
struct vec2
{
	/* constructor maxing :) */
	vec2() : x(0.0f), y(0.0f){}
	vec2(float X, float Y)	: x(X), y(Y){}
	float x, y;

	qangle to_qangle()
	{
		return qangle(x, y, 0.0f);
	}
};

/* view matrix, stores world-to-view or world-to-screen transformation matrix given
by the engine*/
struct view_matrix
{
	float m[4][4];
};

/* enum for indicating which frame stage the engine is at. Used in FrameStageNotify */
enum client_frame_stage
{
	FRAME_UNDEFINED = -1,					// (haven't run any frames yet)
	FRAME_START,
	FRAME_NET_UPDATE_START,					// A network packet is being recieved
	FRAME_NET_UPDATE_POSTDATAUPDATE_START,	// Data has been received and we're going to start calling PostDataUpdate
	FRAME_NET_UPDATE_POSTDATAUPDATE_END,	// Data has been received and we've called PostDataUpdate on all data recipients
	FRAME_NET_UPDATE_END,					// We've received all packets, we can now do interpolation, prediction, etc..
	FRAME_RENDER_START,						// We're about to start rendering the scene
	FRAME_RENDER_END						// We've finished rendering the scene.
};

/* skeleton matrix */
class matrix3x4_t
{
public:
	float m[3][4];

	/* gets the bone's world coordinates */
	vec get_bone_coordinates(){
		return vec(m[0][3], m[1][3], m[2][3]);
	}
};

/* in game playable classes */
enum player_class
{
	TF_SCOUT = 1,
	TF_SNIPER,
	TF_SOLDIER,
	TF_DEMOMAN,
	TF_MEDIC,
	TF_HEAVY,
	TF_PYRO,
	TF_SPY,
	TF_ENGINEER
};

/* use engine to retrieve this information about player */
struct player_info_t
{
	char		name[32];
	int			userID;
	char		guid[33];
	uint32_t	friendsID;
	char		friendsName[32];
	bool		fakeplayer;
	bool		ishltv;
};

/* Dummy stuctures */
struct CBaseHandle {};
struct ICollideable {};
struct model_t {};
struct ShadowType_t {};
struct ShouldTransmitState_t {};
struct DataUpdateType_t {};
struct bf_read {};
struct CMouthInfo {};
struct SpatializationInfo_t {};
struct ClientThinkHandle_t {};
struct ITraceFilter {};
struct client_textmessage_t {};
struct IMaterial {};
struct SurfInfo {};
struct ButtonCode_t {};
struct AudioState_t {};
struct color32 {};
struct UNUSED{};