#pragma once
#include <Windows.h>
#include <cstdint>
#include <cmath>

#define MAX_STUDIO_BONES	128
#define HITBOX_BONES		0x00000100

#define BONE_USED_MASK				0x0007FF00
#define BONE_USED_BY_ANYTHING		0x0007FF00
#define BONE_USED_BY_HITBOX			0x00000100	// bone (or child) is used by a hit box
#define BONE_USED_BY_ATTACHMENT		0x00000200	// bone (or child) is used by an attachment point
#define BONE_USED_BY_VERTEX_MASK	0x0003FC00
#define BONE_USED_BY_VERTEX_LOD0	0x00000400	// bone (or child) is used by the toplevel model via skinned vertex
#define BONE_USED_BY_VERTEX_LOD1	0x00000800	
#define BONE_USED_BY_VERTEX_LOD2	0x00001000  
#define BONE_USED_BY_VERTEX_LOD3	0x00002000
#define BONE_USED_BY_VERTEX_LOD4	0x00004000
#define BONE_USED_BY_VERTEX_LOD5	0x00008000
#define BONE_USED_BY_VERTEX_LOD6	0x00010000
#define BONE_USED_BY_VERTEX_LOD7	0x00020000
#define BONE_USED_BY_BONE_MERGE		0x00040000	// bone is available for bone merge to occur against it

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

#define M_PI 3.14159265358979323846

/* METH */
#define DEG2RAD (M_PI / 180.0f)
#define DEG2RAD(x) (static_cast<float>(x) * (M_PI / 180.0f))

#define RAD2DEG (180.0f / M_PI)
#define RAD2DEG(x) (static_cast<float>(x) * (180.0f / M_PI))

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
#define	MASK_NO_SOLID				(CONTENTS_MOVEABLE|CONTENTS_MONSTER|CONTENTS_WINDOW|CONTENTS_DEBRIS|CONTENTS_HITBOX)
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
	qangle(float flAngle) : pitch(flAngle), yaw(flAngle), roll(flAngle) {}
	qangle(float PITCH, float YAW, float ROLL) : pitch(PITCH), yaw(YAW), roll(ROLL) {}

	float pitch, yaw, roll;

	void Init()
	{
		pitch = 0.0f; yaw = 0.0f; roll = 0.0f;
	}

	qangle operator+(qangle other) {
		return qangle(pitch + other.pitch, yaw + other.yaw, 0.0f);
	}

	qangle operator-(qangle other) {
		return qangle(pitch - other.pitch, yaw - other.yaw, 0.0f);
	}
	qangle& operator=(qangle other)
	{
		pitch = other.pitch; yaw = other.yaw; roll = other.roll;
		return *this;
	}
};

struct vec
{
	vec() : x(0.0f), y(0.0f), z(0.0f) {}
	constexpr vec(float X) : x(X), y(X), z(X) {}
	constexpr vec(float X, float Y, float Z) : x(X), y(Y), z(Z) {}
	float x, y, z;

	inline void Init()
	{
		x = 0.0f; y = 0.0f; z = 0.0f;
	}

	vec operator+(const vec other) const
	{
		return vec(x + other.x, y + other.y, z + other.z);
	}
	vec operator+(float other) const
	{
		return vec(x + other, y + other, z + other);
	}
	vec operator-(const vec& other) const
	{
		return vec(x - other.x, y - other.y, z - other.z);
	}

	constexpr vec operator*(float other) const
	{
		return vec(x * other, y * other, z * other);
	}

	vec operator/(float other) 
	{
		return vec(x / other, y / other, z / other);
	}

	vec& operator +=(vec other)
	{
		x += other.x; y += other.y; z += other.z;
		return *this;
	}

	vec& operator-=(vec other)
	{
		x -= other.x; y -= other.y; z -= other.z;
		return *this;
	}

	vec& operator *=(float other)
	{
		x *= other; y *= other; z *= other;
		return *this;
	}

	void operator= (vec other)
	{
		x = other.x; y = other.y; z = other.z;
	}

	bool operator==(const vec& other) const
	{
		return (x == other.x && y == other.y && z == other.z);
	}

	float length()
	{
		return sqrtf(x * x + y * y + z * z);
	}

	float Length2D() const
	{
		return sqrtf(x * x + y * y);
	}

	float DistTo(const vec& other) const
	{
		return (*this - other).length();
	}

	float Dist2Dto(const vec& other) const
	{
		return sqrtf((x - other.x) * (x - other.x) + (y - other.y) * (y - other.y));
	}

	const float mag()
	{
		return sqrt(x * x + y * y + z * z);
	}

	float magnitude() {
		return sqrt(x * x + y * y + z * z);
	}
	
	bool IsEmpty() const
	{
		return fabs(x) < 0.0001f && fabs(y) < 0.0001f && fabs(z) < 0.0001f;
	}

	bool isEmpty()
	{
		if (x || y || z) return false;
		return true;
	}

	void Normalize()
	{
		float flLength = sqrtf(x * x + y * y + z * z);
		x = x / flLength;
		y = y / flLength;
		z = z / flLength;
	}

	float Dot(const vec& other) const
	{
		return (x * other.x + y * other.y + z * other.z);
	}

	vec ScaleMultiply(float other)
	{
		return vec(x * other, y * other, z * other);
	}

	vec CrossProduct(vec other) const
	{
		return vec(
			y * other.z - z * other.y,
			z * other.x - x * other.z,
			x * other.y - y * other.x
		);
	}

	vec& NormalizeInPlace()
	{
		float flMagnitude = sqrtf(x * x + y * y + z * z);
		x /= flMagnitude; y /= flMagnitude; z /= flMagnitude;
		return *this;
	}
};


__declspec(align(16)) struct vecAligned : public vec {

	float w; // to ensure alligment

	vecAligned(float X, float Y, float Z, float W = 0.0f) : vec(X, Y, Z), w(W){}
	vecAligned() : vec(0.0f, 0.0f, 0.0f), w(0.0f){}

	vecAligned& operator= (const vec& other) 
	{
		x = other.x;
		y = other.y; 
		z = other.z;
		return *this;
	}

	vecAligned& operator= (const vecAligned& other)
	{
		x = other.x;
		y = other.y;
		z = other.z;
		w = other.w;
		return *this;
	}

	void operator*=(const float other)
	{
		x *= other; y *= other; z *= other; w *= other;
	}
	void operator-=(const float other)
	{
		x -= other; y -= other; z -= other; w -= other;
	}
	void operator+=(const float other)
	{
		x += other; y += other; z += other; w += other;
	}

	vecAligned operator+ (const vecAligned& other)
	{
		return vecAligned(x + other.x, y + other.y, z + other.z);
	}
	vecAligned operator+ (const vec& other)
	{
		return vecAligned(x + other.x, y + other.y, z + other.z);
	}
	vecAligned operator+ (const float other)
	{
		return vecAligned(x + other, y + other, z + other);
	}

	vecAligned operator- (const vecAligned& other)
	{
		return vecAligned(x - other.x, y - other.y, z - other.z);
	}
	vecAligned operator- (const vec& other)
	{
		return vecAligned(x - other.x, y - other.y, z - other.z);
	}
	vecAligned operator- (const float other)
	{
		return vecAligned(x - other, y - other, z - other);
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

struct clr_t
{
	clr_t() : r(0.0f), g(0.0f), b(0.0f), a(0.0f){}
	clr_t(float red, float green, float blue,float alpha) : r(red), g(green), b(blue), a(alpha){}
	float r, g, b, a;
};

struct TFclr_t
{
	char clr[4];
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

enum class lifeState_t : int8_t 
{
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
class bf_read;
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

//=========================================================================
//                     m_fFlags bits
//=========================================================================

#define	FL_ONGROUND				(1<<0)	// At rest / on the ground
#define FL_DUCKING				(1<<1)	// Player flag -- Player is fully crouched
#define FL_ANIMDUCKING			(1<<2)	// Player flag -- Player is in the process of crouching or uncrouching but could be in transition
// examples:                                   Fully ducked:  FL_DUCKING &  FL_ANIMDUCKING
//           Previously fully ducked, unducking in progress:  FL_DUCKING & !FL_ANIMDUCKING
//                                           Fully unducked: !FL_DUCKING & !FL_ANIMDUCKING
//           Previously fully unducked, ducking in progress: !FL_DUCKING &  FL_ANIMDUCKING
#define	FL_WATERJUMP			(1<<3)	// player jumping out of water
#define FL_ONTRAIN				(1<<4) // Player is _controlling_ a train, so movement commands should be ignored on client during prediction.
#define FL_INRAIN				(1<<5)	// Indicates the entity is standing in rain
#define FL_FROZEN				(1<<6) // Player is frozen for 3rd person camera
#define FL_ATCONTROLS			(1<<7) // Player can't move, but keeps key inputs for controlling another entity
#define	FL_CLIENT				(1<<8)	// Is a player
#define FL_FAKECLIENT			(1<<9)	// Fake client, simulated server side; don't send network messages to them
// NON-PLAYER SPECIFIC (i.e., not used by GameMovement or the client .dll ) -- Can still be applied to players, though
#define	FL_INWATER				(1<<10)	// In water

// NOTE if you move things up, make sure to change this value
#define PLAYER_FLAG_BITS		11

#define	FL_FLY					(1<<11)	// Changes the SV_Movestep() behavior to not need to be on ground
#define	FL_SWIM					(1<<12)	// Changes the SV_Movestep() behavior to not need to be on ground (but stay in water)
#define	FL_CONVEYOR				(1<<13)
#define	FL_NPC					(1<<14)
#define	FL_GODMODE				(1<<15)
#define	FL_NOTARGET				(1<<16)
#define	FL_AIMTARGET			(1<<17)	// set if the crosshair needs to aim onto the entity
#define	FL_PARTIALGROUND		(1<<18)	// not all corners are valid
#define FL_STATICPROP			(1<<19)	// Eetsa static prop!		
#define FL_GRAPHED				(1<<20) // worldgraph has this ent listed as something that blocks a connection
#define FL_GRENADE				(1<<21)
#define FL_STEPMOVEMENT			(1<<22)	// Changes the SV_Movestep() behavior to not do any processing
#define FL_DONTTOUCH			(1<<23)	// Doesn't generate touch functions, generates Untouch() for anything it was touching when this flag was set
#define FL_BASEVELOCITY			(1<<24)	// Base velocity has been applied this frame (used to convert base velocity into momentum)
#define FL_WORLDBRUSH			(1<<25)	// Not moveable/removeable brush entity (really part of the world, but represented as an entity for transparency or something)
#define FL_OBJECT				(1<<26) // Terrible name. This is an object that NPCs should see. Missiles, for example.
#define FL_KILLME				(1<<27)	// This entity is marked for death -- will be freed by game DLL
#define FL_ONFIRE				(1<<28)	// You know...
#define FL_DISSOLVING			(1<<29) // We're dissolving!
#define FL_TRANSRAGDOLL			(1<<30) // In the process of turning into a client side ragdoll.
#define FL_UNBLOCKABLE_BY_PLAYER (1<<31) // pusher that can't be blocked by the player