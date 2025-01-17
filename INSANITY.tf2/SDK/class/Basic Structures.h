#pragma once
#include <Windows.h>
#include <cstdint>

#define MAX_STUDIO_BONES	128
#define HITBOX_BONES		0x00000100

/* bone type defs */
#define BONE_HEAD 6              // Head
#define BONE_NECK 5              // Neck
#define BONE_SPINE_UPPER 4       // Upper Spine
#define BONE_SPINE_MIDDLE 3      // Middle Spine
#define BONE_SPINE_LOWER 2       // Lower Spine
#define BONE_PELVIS 1            // Pelvis or Hip
#define BONE_LEFT_SHOULDER 7     // Left Shoulder
#define BONE_LEFT_ELBOW 8        // Left Elbow
#define BONE_LEFT_HAND 9         // Left Hand
#define BONE_RIGHT_SHOULDER 10   // Right Shoulder
#define BONE_RIGHT_ELBOW 11      // Right Elbow
#define BONE_RIGHT_HAND 12       // Right Hand
#define BONE_LEFT_HIP 13         // Left Hip
#define BONE_LEFT_KNEE 14        // Left Knee
#define BONE_LEFT_FOOT 15        // Left Foot
#define BONE_RIGHT_HIP 16        // Right Hip
#define BONE_RIGHT_KNEE 17       // Right Knee
#define BONE_RIGHT_FOOT 18       // Right Foot


struct qangle
{
	float pitch, yaw, roll;
};

struct vec
{
	float x, y, z;
};

struct vec2
{
	/* constructor maxing :) */
	vec2() : x(0.0f), y(0.0f){}
	vec2(float X, float Y)	: x(X), y(Y){}
	float x, y;
};

/* view matrix, stores world-to-view or world-to-screen transformation matrix given
by the engine*/
struct view_matrix
{
	float m[4][4];
};

/* skeleton matrix */
class matrix3x4_t
{
public:
	float m[3][4];

	/* gets the bone's world coordinates */
	vec get_bone_coordinates(){
		return vec(m[0][4], m[1][4], m[2][4]);
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