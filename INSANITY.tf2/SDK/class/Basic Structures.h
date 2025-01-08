#pragma once
#include <Windows.h>
#include <cstdint>


struct qangle
{
	float pitch, yaw, roll;
};

struct vec
{
	float x, y, z;
};


/* Dummy stuctures */
struct CBaseHandle {};
struct ICollideable {};
struct model_t {};
struct ShadowType_t {};
struct matrix3x4_t {};
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
struct player_info_t {};
struct ButtonCode_t {};
struct AudioState_t {};
struct color32 {};
struct UNUSED{};