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

/* METH */
#define DEG2RAD (M_PI / 180.0f)
#define RAD2DEG (180.0f / M_PI)

/* game constants */
#define TF2_GRAVITY 800.0f
#define DIRECT_HIT_ROCKET_LAUNCHER 1980.0f


// engine trace specific defines
#define	CONTENTS_SOLID					0x1
#define	CONTENTS_WINDOW					0x2
#define	CONTENTS_GRATE					0x8
#define	CONTENTS_SLIME					0x10
#define	CONTENTS_WATER					0x20
#define	CONTENTS_BLOCKLOS				0x40
#define CONTENTS_OPAQUE					0x80
#define CONTENTS_IGNORE_NODRAW_OPAQUE	0x2000
#define CONTENTS_MOVEABLE				0x4000
#define	CONTENTS_PLAYERCLIP				0x10000
#define	CONTENTS_MONSTERCLIP			0x20000
#define	CONTENTS_MONSTER				0x2000000
#define	CONTENTS_DEBRIS					0x4000000
#define CONTENTS_HITBOX					0x40000000

#define	MASK_ALL					(0xFFFFFFFF)
#define	MASK_SHOT					(CONTENTS_SOLID|CONTENTS_MOVEABLE|CONTENTS_MONSTER|CONTENTS_WINDOW|CONTENTS_DEBRIS|CONTENTS_HITBOX)
#define	MASK_SOLID					(CONTENTS_SOLID|CONTENTS_MOVEABLE|CONTENTS_WINDOW|CONTENTS_MONSTER|CONTENTS_GRATE)
#define	MASK_PLAYERSOLID			(CONTENTS_SOLID|CONTENTS_MOVEABLE|CONTENTS_PLAYERCLIP|CONTENTS_WINDOW|CONTENTS_MONSTER|CONTENTS_GRATE)
#define	MASK_NPCSOLID				(CONTENTS_SOLID|CONTENTS_MOVEABLE|CONTENTS_MONSTERCLIP|CONTENTS_WINDOW|CONTENTS_MONSTER|CONTENTS_GRATE)
#define	MASK_WATER					(CONTENTS_WATER|CONTENTS_MOVEABLE|CONTENTS_SLIME)
#define	MASK_OPAQUE					(CONTENTS_SOLID|CONTENTS_MOVEABLE|CONTENTS_OPAQUE)
#define MASK_OPAQUE_AND_NPCS		(MASK_OPAQUE|CONTENTS_MONSTER)
#define MASK_BLOCKLOS				(CONTENTS_SOLID|CONTENTS_MOVEABLE|CONTENTS_BLOCKLOS)
#define MASK_BLOCKLOS_AND_NPCS		(MASK_BLOCKLOS|CONTENTS_MONSTER)
#define	MASK_VISIBLE					(MASK_OPAQUE|CONTENTS_IGNORE_NODRAW_OPAQUE)
#define MASK_VISIBLE_AND_NPCS		(MASK_OPAQUE_AND_NPCS|CONTENTS_IGNORE_NODRAW_OPAQUE)

#define DISPSURF_FLAG_SURFACE		(1<<0)
#define DISPSURF_FLAG_WALKABLE		(1<<1)
#define DISPSURF_FLAG_BUILDABLE		(1<<2)
#define DISPSURF_FLAG_SURFPROP1		(1<<3)
#define DISPSURF_FLAG_SURFPROP2		(1<<4)

enum renderGroup_t
{
	RENDER_GROUP_OPAQUE_STATIC_HUGE = 0,		
	RENDER_GROUP_OPAQUE_ENTITY_HUGE = 1,		
	RENDER_GROUP_OPAQUE_STATIC = 6,
	RENDER_GROUP_OPAQUE_ENTITY,					
	RENDER_GROUP_TRANSLUCENT_ENTITY,
	RENDER_GROUP_TWOPASS,						
	RENDER_GROUP_VIEW_MODEL_OPAQUE,				
	RENDER_GROUP_VIEW_MODEL_TRANSLUCENT,		
	RENDER_GROUP_OPAQUE_BRUSH,					
	RENDER_GROUP_OTHER,							
	RENDER_GROUP_COUNT
};

struct qangle
{
	qangle() : pitch(0.0f), yaw(0.0f), roll(0.0f){}
	qangle(float PITCH, float YAW, float ROLL) : pitch(PITCH), yaw(YAW), roll(ROLL) {}

	float pitch, yaw, roll;

	qangle operator+(qangle other) {
		return qangle(pitch + other.pitch, yaw + other.yaw, 0.0f);
	}

	qangle operator-(qangle other) {
		return qangle(pitch - other.pitch, yaw - other.yaw, 0.0f);
	}
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
	vec operator+(float other)
	{
		return vec(x + other, y + other, z + other);
	}
	vec operator-(vec other) const
	{
		return vec(x - other.x, y - other.y, z - other.z);
	}

	const float mag()
	{
		return sqrt(x * x + y * y + z * z);
	}

	float magnitude() {
		return sqrt(x * x + y * y + z * z);
	}
	
	bool isEmpty(){
		if (x || y || z) return false;
		return true;
	}

	void Normalize()
	{
		float mag = this->mag();
		x /= mag;
		y /= mag;
		z /= mag;
	}

	float Dot(vec other) const
	{
		return (x * other.x + y * other.y + z * other.z);
	}

	vec ScaleMultiply(float other)
	{
		return vec(x * other, y * other, z * other);
	}
};


__declspec(align(16)) struct vecAligned : public vec {

	float w; // to ensure alligment

	vecAligned(float X, float Y, float Z, float W = 0.0f) : vec(X, Y, Z), w(W){}
	vecAligned() : vec(0.0f, 0.0f, 0.0f), w(0.0f){}

	vecAligned operator=(const vec& other) {
		return vecAligned(other.x, other.y, other.z);
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

enum lifeState_t {
	LIFE_ALIVE = 0,
	LIFE_DYING,
	LIFE_DEAD,
	LIFE_RESPAWNABLE,
	LIFE_DISCARDBODY
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
	INVALID = 0,
	TF_SCOUT = 1,
	TF_SNIPER,
	TF_SOLDIER,
	TF_DEMOMAN,
	TF_MEDIC,
	TF_HEAVY,
	TF_PYRO,
	TF_SPY,
	TF_ENGINEER,
	NOT_PLAYER
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

typedef unsigned char byte;
struct cplane_t
{
	vec	normal;
	float	dist;
	byte	type;			// for fast side tests
	byte	signbits;		// signx + (signy<<1) + (signz<<1)
	byte	pad[2];
};

struct csurface_t
{
	const char*		name;
	short			surfaceProps;
	unsigned short	flags;		
};

/* Dummy stuctures */
struct CBaseHandle {};
struct ICollideable {};
struct ShadowType_t {};
struct ShouldTransmitState_t {};
struct DataUpdateType_t {};
struct bf_read {};
struct CMouthInfo {};
struct SpatializationInfo_t {};
struct ClientThinkHandle_t {};
struct ITraceFilter {};
struct client_textmessage_t {};
class IMaterial;
struct SurfInfo {};
struct ButtonCode_t {};
struct AudioState_t {};
struct color32 {};
struct UNUSED{};

struct model_t
{
	int					fnHandle; // is a 4 byte pointer handle
	const char*			strName;

	int					nLoadFlags;		// mark loaded/not loaded
	int					nServerCount;	// marked at load
	IMaterial**			ppMaterials;	// null-terminated runtime material cache; ((intptr_t*)(ppMaterials))[-1] == nMaterials

	int					type; // ENUM HA BC
	int					flags;			// MODELFLAG_???

	// volume occupied by the model graphics	
	vec					mins, maxs;
	float				radius;
};

// player condition enum ( used for : m_playercond )
enum FLAG_playerCond
{
	TF_COND_INVALID = -1,
	TF_COND_AIMING = 0,		// Sniper aiming, Heavy minigun.
	TF_COND_ZOOMED,
	TF_COND_DISGUISING,
	TF_COND_DISGUISED,
	TF_COND_STEALTHED,		// Spy specific
	TF_COND_INVULNERABLE,
	TF_COND_TELEPORTED,
	TF_COND_TAUNTING,
	TF_COND_INVULNERABLE_WEARINGOFF,
	TF_COND_STEALTHED_BLINK,
	TF_COND_SELECTED_TO_TELEPORT,
	TF_COND_CRITBOOSTED,	// DO NOT RE-USE THIS -- THIS IS FOR KRITZKRIEG AND REVENGE CRITS ONLY
	TF_COND_TMPDAMAGEBONUS,
	TF_COND_FEIGN_DEATH,
	TF_COND_PHASE,
	TF_COND_STUNNED,		// Any type of stun. Check iStunFlags for more info.
	TF_COND_OFFENSEBUFF,
	TF_COND_SHIELD_CHARGE,
	TF_COND_DEMO_BUFF,
	TF_COND_ENERGY_BUFF,
	TF_COND_RADIUSHEAL,
	TF_COND_HEALTH_BUFF,
	TF_COND_BURNING,
	TF_COND_HEALTH_OVERHEALED,
	TF_COND_URINE,
	TF_COND_BLEEDING,
	TF_COND_DEFENSEBUFF,	// 35% defense! No crit damage.
	TF_COND_MAD_MILK,
	TF_COND_MEGAHEAL,
	TF_COND_REGENONDAMAGEBUFF,
	TF_COND_MARKEDFORDEATH,
	TF_COND_NOHEALINGDAMAGEBUFF,
	TF_COND_SPEED_BOOST,				// = 32
	TF_COND_CRITBOOSTED_PUMPKIN,		// Brandon hates bits
	TF_COND_CRITBOOSTED_USER_BUFF,
	TF_COND_CRITBOOSTED_DEMO_CHARGE,
	TF_COND_SODAPOPPER_HYPE,
	TF_COND_CRITBOOSTED_FIRST_BLOOD,	// arena mode first blood
	TF_COND_CRITBOOSTED_BONUS_TIME,
	TF_COND_CRITBOOSTED_CTF_CAPTURE,
	TF_COND_CRITBOOSTED_ON_KILL,		// =40. KGB, etc.
	TF_COND_CANNOT_SWITCH_FROM_MELEE,
	TF_COND_DEFENSEBUFF_NO_CRIT_BLOCK,	// 35% defense! Still damaged by crits.
	TF_COND_REPROGRAMMED,				// Bots only
	TF_COND_CRITBOOSTED_RAGE_BUFF,
	TF_COND_DEFENSEBUFF_HIGH,			// 75% defense! Still damaged by crits.
	TF_COND_SNIPERCHARGE_RAGE_BUFF,		// Sniper Rage - Charge time speed up
	TF_COND_DISGUISE_WEARINGOFF,		// Applied for half-second post-disguise
	TF_COND_MARKEDFORDEATH_SILENT,		// Sans sound
	TF_COND_DISGUISED_AS_DISPENSER,
	TF_COND_SAPPED,						// =50. Bots only
	TF_COND_INVULNERABLE_HIDE_UNLESS_DAMAGED,
	TF_COND_INVULNERABLE_USER_BUFF,
	TF_COND_HALLOWEEN_BOMB_HEAD,
	TF_COND_HALLOWEEN_THRILLER,
	TF_COND_RADIUSHEAL_ON_DAMAGE,
	TF_COND_CRITBOOSTED_CARD_EFFECT,
	TF_COND_INVULNERABLE_CARD_EFFECT,
	TF_COND_MEDIGUN_UBER_BULLET_RESIST,
	TF_COND_MEDIGUN_UBER_BLAST_RESIST,
	TF_COND_MEDIGUN_UBER_FIRE_RESIST,		// =60
	TF_COND_MEDIGUN_SMALL_BULLET_RESIST,
	TF_COND_MEDIGUN_SMALL_BLAST_RESIST,
	TF_COND_MEDIGUN_SMALL_FIRE_RESIST,
	TF_COND_STEALTHED_USER_BUFF,			// Any class can have this
	TF_COND_MEDIGUN_DEBUFF,
	TF_COND_STEALTHED_USER_BUFF_FADING,
	TF_COND_BULLET_IMMUNE,
	TF_COND_BLAST_IMMUNE,
	TF_COND_FIRE_IMMUNE,
	TF_COND_PREVENT_DEATH,					// =70
	TF_COND_MVM_BOT_STUN_RADIOWAVE, 		// Bots only
	TF_COND_HALLOWEEN_SPEED_BOOST,
	TF_COND_HALLOWEEN_QUICK_HEAL,
	TF_COND_HALLOWEEN_GIANT,
	TF_COND_HALLOWEEN_TINY,
	TF_COND_HALLOWEEN_IN_HELL,
	TF_COND_HALLOWEEN_GHOST_MODE,			// =77
	TF_COND_MINICRITBOOSTED_ON_KILL,
	TF_COND_OBSCURED_SMOKE,
	TF_COND_PARACHUTE_DEPLOYED,				// =80
	TF_COND_BLASTJUMPING,
	TF_COND_HALLOWEEN_KART,
	TF_COND_HALLOWEEN_KART_DASH,
	TF_COND_BALLOON_HEAD,					// =84 larger head, lower-gravity-feeling jumps
	TF_COND_MELEE_ONLY,						// =85 melee only
	TF_COND_SWIMMING_CURSE,					// player movement become swimming movement
	TF_COND_FREEZE_INPUT,					// freezes player input
	TF_COND_HALLOWEEN_KART_CAGE,			// attach cage model to player while in kart
	TF_COND_DONOTUSE_0,
	TF_COND_RUNE_STRENGTH,
	TF_COND_RUNE_HASTE,
	TF_COND_RUNE_REGEN,
	TF_COND_RUNE_RESIST,
	TF_COND_RUNE_VAMPIRE,
	TF_COND_RUNE_REFLECT,
	TF_COND_RUNE_PRECISION,
	TF_COND_RUNE_AGILITY,
	TF_COND_GRAPPLINGHOOK,
	TF_COND_GRAPPLINGHOOK_SAFEFALL,
	TF_COND_GRAPPLINGHOOK_LATCHED,
	TF_COND_GRAPPLINGHOOK_BLEEDING,
	TF_COND_AFTERBURN_IMMUNE,
	TF_COND_RUNE_KNOCKOUT,
	TF_COND_RUNE_IMBALANCE,
	TF_COND_CRITBOOSTED_RUNE_TEMP,
	TF_COND_PASSTIME_INTERCEPTION,
	TF_COND_SWIMMING_NO_EFFECTS,			// =107_DNOC_FT
	TF_COND_PURGATORY,
	TF_COND_RUNE_KING,
	TF_COND_RUNE_PLAGUE,
	TF_COND_RUNE_SUPERNOVA,
	TF_COND_PLAGUE,
	TF_COND_KING_BUFFED,
	TF_COND_TEAM_GLOWS,						// used to show team glows to living players
	TF_COND_KNOCKED_INTO_AIR,
	TF_COND_COMPETITIVE_WINNER,
	TF_COND_COMPETITIVE_LOSER,
	TF_COND_HEALING_DEBUFF,
	TF_COND_PASSTIME_PENALTY_DEBUFF,		// when carrying the ball without any teammates nearby	
	TF_COND_GRAPPLED_TO_PLAYER,
	TF_COND_GRAPPLED_BY_PLAYER
};