#pragma once
#include <Windows.h>
#include <atomic>
#include <mutex>
#include <string>
#include <unordered_map>
#include <vector>

#define M_PI 3.14159265358979323846

/* game module names */
#define CLIENT_DLL	"client.dll"
#define ENGINE_DLL	"engine.dll"
#define VGUI2_DLL	"vgui2.dll"

/* game interfaces names */
#define IVENGIENCLIENT013		"VEngineClient014"
#define ICLIENTENTITYLIST		"VClientEntityList003"
#define VENGINERANDOM001		"VEngineRandom001"
#define IGAMEEVENTLISTNER		"GAMEEVENTSMANAGER002"
#define IVDEBUGOVERLAY			"VDebugOverlay003"
#define ENGINETRACE_SERVER		"EngineTraceServer003"
#define ENGINETRACE_CLIENT		"EngineTraceClient003"
#define ENGINE_CLIENT_REPLAY	"EngineClientReplay001"
#define IENGINETRACE			"EngineTraceClient003"
#define IVDEBUGOVERLAY			"VDebugOverlay003"
#define VGUI_PANEL				"VGUI_Panel009"

/* this is config file */
#include "Features/config.h"

/* game classes */
#include "SDK/class/I_BaseEntityDLL.h"
#include "SDK/class/Source Entity.h"
#include "SDK/class/IVEngineClient.h"
#include "SDK/class/I_EngineClientReplay.h"
#include "SDK/class/GlowManager.h"
#include "SDK/class/IEngineTrace.h"
#include "SDK/class/IVDebugOverlay.h"

/* entity information template struct */
#include "SDK/entInfo_t.h"

/* console system for debug mode */
#ifdef _DEBUG
#include "Libraries/Console System/Console_System.h"
extern Console_System cons;
#endif // _DEBUG

// Forward Declares
class baseWeapon;

/* just a little typedef, cause typing out map type is very annoying*/
typedef std::unordered_map < std::string , int32_t> T_map;

/* this holds necesarry information about the byte array of a resource.
all resources must initialize this.*/
struct raw_image_data
{
	raw_image_data(unsigned char* bytes, size_t size)
		:image_bytes(bytes), image_bytearray_size(size) {}

	unsigned char* image_bytes;
	size_t image_bytearray_size;
};

namespace TF_objects
{
	inline glowManager* pGlowManager = nullptr;
	inline std::atomic<IEngineTrace*> pEngineTrace;
	// inline IEngineTrace* pEngineTrace = nullptr;
}

/* functions found via signature scanning. TO BE CALLED MANUALLY :) */
namespace TF2_functions
{
	// 40 53 48 83 EC ? 48 8B DA E8 ? ? ? ? 48 8B C8 48 8B D3 48 83 C4 ? 5B E9 ? ? ? ? CC CC 48 89 74 24
	typedef int64_t(__fastcall* T_lookUpBone)(void* pEnt, const char* boneName);
	extern T_lookUpBone lookUpBone;

	// 48 8B 02 48 8B CA 48 FF A0 ? ? ? ? CC CC CC 48 8B 02 48 8B CA 48 FF A0 ? ? ? ? CC CC CC 48 8B 02 48 8B CA 48 FF A0 ? ? ? ? CC CC CC 48 83 EC
	typedef char* (__fastcall* T_getName)(void*, int64_t);
	inline T_getName FN_getName = nullptr;

	typedef void(__fastcall* T_addToLeafSystem)(void*, renderGroup_t);
	extern T_addToLeafSystem FN_addToLeafSystem;
};

/* this will hold information about when threads are started & teminated */
namespace thread_termination_status
{
	inline bool thread1 = false;
	inline bool thread2 = false;
	inline bool thread3 = false;

	inline bool thread1_primed = false;
}

/* this will be popullated and maintained by thread 2,
it shall hold important information about possible targets & me */
namespace entities
{
	inline std::atomic<view_matrix> M_worldToScreen;
	inline std::atomic<view_matrix> M_worldToView;

	/* this struct holds screen corrdinated of each of the extreme body parts location of the entity.
	essentially storing the height and width of the entity */
	struct entity_dimensions
	{
		vec2 head, left_foot, right_foot, left_shoulder, right_shoulder;
	};

	inline global_var_base* pGlobalVars = nullptr;
	
	/* imfo about local player */
	namespace local
	{
		inline std::atomic<I_client_entity*>	pLocalPlayer(nullptr);
		inline std::atomic<player_class>		localplayer_class;
		inline std::atomic<baseWeapon*>			active_weapon(nullptr);
		inline int32_t				ID_active_weapon;
		inline int16_t				team_num;
		inline vec					pos;
		inline std::atomic<vec>		eye_pos;
		inline bool					b_hasProjectileWeapon = false;
		inline std::atomic<qangle>	viewAngles;
	}

	inline int32_t* ARR_maxHealth = nullptr;

	/* AIMBOT TARGET's vars */
	inline std::atomic<qangle> aimbotTargetAngles;
	inline std::atomic<bool> shouldDoAimbot(false);

	/* info about all possible / alive targets */
	class C_targets
	{
	private:
		/* holds all valid entities, which are to be processed inside FrameStageNotify */
		std::vector<entInfo_t> vecEntities;
		std::vector<entInfo_t> RENDER_vecEntities;
		std::mutex MTX_vecEntities;

		int8_t flag = 0;
	public:
		//==============HANDLING VECTOR============================
		void clear_vecEntities() {
			std::lock_guard<std::mutex> lock(MTX_vecEntities);
			vecEntities.clear();
		}

		void insert_vecEntities(const entInfo_t entInfo) {
			std::lock_guard<std::mutex> lock(MTX_vecEntities);
			vecEntities.push_back(entInfo);
		}

		/* made to return a copy intentionaly, so it can altered according to needs */
		std::vector<entInfo_t> get_vecEntities(bool renderable = false) {
			std::lock_guard<std::mutex> lock(MTX_vecEntities);
			if (!renderable) return vecEntities;
			return RENDER_vecEntities;
		}
		
		/* taking it simply instead of refrence might not be optimal? IDK nigga */
		void update_vecEntities(const std::vector<entInfo_t> new_vecEntInfo, bool renderable = false) {
			std::lock_guard<std::mutex> lock(MTX_vecEntities);
			if (!renderable) {
				vecEntities.clear();
				vecEntities = new_vecEntInfo;
			}
			else {
				RENDER_vecEntities.clear();
				RENDER_vecEntities = new_vecEntInfo;
			}
		}

		/* enum holding information about flag bits */
		enum BIT_cTarget {
			DOING_FIRST_HALF = 0, // is vecEntities popullated ?
			DOING_SECOND_HALF // is RENDER_vecEntities popullated ?
		};

		/* Gets the flag bit */
		bool getFlagBit(BIT_cTarget bitIndex) {
			return flag & (1 << bitIndex);
		}

		/* sets desired flag bit*/
		void setFlagBit(BIT_cTarget bitIndex) {
			flag |= (1 << bitIndex);
		}

		/* Clears desired flag bit */
		void clearFlagBit(BIT_cTarget bitIndex) {
			flag &= ~(1 << bitIndex);
		}
	};
	inline C_targets entManager;

	// this is a anti-race condition mechanism for maps, but I cannot garantte returning 
	// the latest buffer each call.
	class allEntManager_t {
	public:
		typedef std::unordered_map<int16_t, glowObject_t> allEntMap;

		// give pointer to current read buffer
		allEntMap* getReadBuffer() {

			b_isReadBufferActive.store(true);
			return readBuffer.load();
		}

		// gives pointer to current write buffer
		allEntMap* getWriteBuffer() {

			b_isWriteBufferActive.store(true);

			allEntMap* CHE_writeBuffer = writeBuffer.load();
			if (CHE_writeBuffer->size() > 100) {
				#ifdef _DEBUG
				cons.Log(FG_YELLOW, "Ent Map Manager", "Map size too big, clearing it now");
				#endif 
				CHE_writeBuffer->clear();
			}

			// clear write buffer here, if size if above some limit
			return writeBuffer.load();
		}

		// must call this once you are done reading or writting buffer
		// @param set isRead to true if you sending back read buffer, else set it to false
		void sendBack(bool isRead) {

			// Managing flags
			if (isRead) {
				b_isReadBufferActive.store(false);
			}
			else {
				b_isWriteBufferActive.store(false);
			}

			// if non of the buffer is active, then swap em'
			if (!b_isReadBufferActive.load() && !b_isWriteBufferActive.load()) {
				
				// swapping buffer
				if (readBuffer.load() == &allEntMap_01) {
					readBuffer.store(&allEntMap_02);
					writeBuffer.store(&allEntMap_01);
				}
				else {
					readBuffer.store(&allEntMap_01);
					writeBuffer.store(&allEntMap_02);
				}
			}
		}

	private:
		allEntMap allEntMap_01; // default read buffer
		allEntMap allEntMap_02; // default write buffer

		// read and write buffer pointers
		std::atomic<allEntMap*> readBuffer	= &allEntMap_01;
		std::atomic<allEntMap*> writeBuffer	= &allEntMap_02;

		// flags indicating state of read and write buffer to prevent race conditions
		std::atomic<bool> b_isReadBufferActive = false;
		std::atomic<bool> b_isWriteBufferActive = false;
	};
	inline allEntManager_t allEntManager;

	class boneManager_t
	{
	private:
		/* this is a 16-bit bit field, each booleans/bit holds whether that character models bones
		are cached or not. and if they are not cached, they will be aquired and stored. */
		int16_t BF_boneIndexCached = 0;
		
		/* used to toggle BF_boneIndexCached's bits */
		void setBit_boneIndexCached(player_class characterModel) {
			BF_boneIndexCached |= (1 << characterModel);
		}
		void clearBit_boneIndexCached(player_class characterModel) {
			BF_boneIndexCached &= ~(1 << characterModel);
		}
		bool getBit_boneIndexCached(player_class characterModel) {
			return BF_boneIndexCached & (1 << characterModel);
		}

		/* bone indexes to be cached */
		boneInfo_t scoutBone;
		boneInfo_t sniperBone;
		boneInfo_t soldierBone;
		boneInfo_t demomanBone;

		boneInfo_t medicBone;
		boneInfo_t heavyBone;
		boneInfo_t pyroBone;
		boneInfo_t engiBone;

		boneInfo_t spyBone;
		boneInfo_t nonPlayerEntities;

	public:
		/* loop up bone function, if bone IDs are not cached, the it caches them*/
		boneInfo_t* getBone(entInfo_t* ent, player_class characterModel) {
			
			if (ent->classID != PLAYER) {
				return &nonPlayerEntities;
			}

			void* pEnt = (void*)ent->p_ent;

			/* caching 
			only done once in the entire life of software */
			if (!getBit_boneIndexCached(characterModel)) {
				
				boneInfo_t CHE_boneInfo;
				CHE_boneInfo.head			= TF2_functions::lookUpBone(pEnt, "bip_head");
				CHE_boneInfo.leftShoulder	= TF2_functions::lookUpBone(pEnt, "bip_collar_L");
				CHE_boneInfo.rightShoulder	= TF2_functions::lookUpBone(pEnt, "bip_collar_R");
				CHE_boneInfo.leftFoot		= TF2_functions::lookUpBone(pEnt, "bip_foot_L");
				CHE_boneInfo.rightFoot		= TF2_functions::lookUpBone(pEnt, "bip_foot_R");
				CHE_boneInfo.chest			= TF2_functions::lookUpBone(pEnt, "bip_spine_3");
				CHE_boneInfo.pelvis			= TF2_functions::lookUpBone(pEnt, "bip_pelvis");

				setBit_boneIndexCached(characterModel);

				switch (characterModel)
				{
				case TF_SCOUT:
					scoutBone = CHE_boneInfo;
					break;
				case TF_SNIPER:
					sniperBone = CHE_boneInfo;
					break;
				case TF_SOLDIER:
					soldierBone = CHE_boneInfo;
					break;
				case TF_DEMOMAN:
					demomanBone = CHE_boneInfo;
					break;
				case TF_MEDIC:
					medicBone = CHE_boneInfo;
					break;
				case TF_HEAVY:
					heavyBone = CHE_boneInfo;
					break;
				case TF_PYRO:
					pyroBone = CHE_boneInfo;
					break;
				case TF_SPY:
					spyBone = CHE_boneInfo;
					break;
				case TF_ENGINEER:
					engiBone = CHE_boneInfo;
					break;
				default:
					#ifdef _DEBUG
					cons.Log(FG_RED, "BONE MAMANGER", "Failed bone caching");
					#endif
					break;
				}

				#ifdef _DEBUG
				cons.Log(FG_GREEN, "BONE MANAGER", "Cached bone information for model : %d", characterModel);
				#endif 

			}
			
			/* returning pointer to required bone structs */
			switch (characterModel)
			{
			case TF_SCOUT:
				return &scoutBone;
				break;
			case TF_SNIPER:
				return &sniperBone;
				break;
			case TF_SOLDIER:
				return &soldierBone;
				break;
			case TF_DEMOMAN:
				return &demomanBone;
				break;
			case TF_MEDIC:
				return &medicBone;
				break;
			case TF_HEAVY:
				return &heavyBone;
				break;
			case TF_PYRO:
				return &pyroBone;
				break;
			case TF_SPY:
				return &spyBone;
				break;
			case TF_ENGINEER:
				return &engiBone;
				break;
			default:
				#ifdef _DEBUG
				cons.Log(FG_RED, "BONE MAMANGER", "Failed to find character model");
				#endif
				break;
			}
		}
	};
	inline boneManager_t boneManager;

	class IDManager_t {
	
	public:
		IDclass_t getID(I_client_entity* ent) {
			
			// entity clas name
			std::string name = std::string(ent->GetClientNetworkable()->GetClientClass()->m_pNetworkName);
			
			// class ID
			int ID = ent->GetClientNetworkable()->GetClientClass()->m_ClassID;
			
			auto iterator = CHE_mapID.find(name);
			if (iterator != CHE_mapID.end()) { // if it is stored in the map
				return iterator->second; // return class ID
			}

			// if not stored then store it
			IDclass_t TEMPclassID = NOT_DEFINED;
			// any PLAYER
			if (name == "CTFPlayer") {
				TEMPclassID = PLAYER;
				#ifdef _DEBUG
				cons.Log(FG_GREEN, "ID Manager", "Cached class ID for : %s", name.c_str());
				#endif
			}
			// engi -> sentery gun
			else if (name == "CObjectSentrygun") {
				TEMPclassID = SENTRY_GUN;
				#ifdef _DEBUG
				cons.Log(FG_GREEN, "ID Manager", "Cached class ID for : %s", name.c_str());
				#endif
			}
			// engi -> dispenser
			else if (name == "CObjectDispenser") {
				TEMPclassID = DISPENSER;
				#ifdef _DEBUG
				cons.Log(FG_GREEN, "ID Manager", "Cached class ID for : %s", name.c_str());
				#endif
			}
			// engi -> Teleported
			else if (name == "CObjectTeleporter") {
				TEMPclassID = TELEPORTER;
				#ifdef _DEBUG
				cons.Log(FG_GREEN, "ID Manager", "Cached class ID for : %s", name.c_str());
				#endif
			}
			// TODO : CHECK THIS 
			else if (name == "CTFAmmoPack") {
				TEMPclassID = AMMO_PACK;
				#ifdef _DEBUG
				cons.Log(FG_GREEN, "ID Manager", "Cached class ID for : %s", name.c_str());
				#endif
			}
			// Payload cart, if more than one in a match, that means we are not playing PAYLOAD
			else if (name == "CFuncTrackTrain") {
				TEMPclassID = PAYLOAD;
				#ifdef _DEBUG
				cons.Log(FG_GREEN, "ID Manager", "Cached class ID for : %s", name.c_str());
				#endif
			}
			// Flag in capture the Flag
			else if (name == "CCaptureFlag") {
				TEMPclassID = TF_ITEM;
				#ifdef _DEBUG
				cons.Log(FG_GREEN, "ID Manager", "Cached class ID for : %s", name.c_str());
				#endif
			}
			else if (name == "CTFPlayerResource") {
				TEMPclassID = ENT_RESOURCE_MANAGER;
				#ifdef _DEBUG
				cons.Log(FG_GREEN, "ID Manager", "Cached class ID for : %s", name.c_str());
				#endif
			}
			// WEAPONS
			else if (name == "CBaseCombatWeapon") {
				TEMPclassID = WEAPON;
				#ifdef _DEBUG
				cons.Log(FG_GREEN, "ID Manager", "Cached class ID for : %s", name.c_str());
				#endif
			}

			// storing and returning ID
			if (TEMPclassID != NOT_DEFINED) CHE_mapID[name] = TEMPclassID;
			return CHE_mapID[name];
		}
	private:
		std::unordered_map<std::string, IDclass_t> CHE_mapID;
	};
	inline IDManager_t IDManager;

	/* converts world cordinates to screen cordinates, useful for ESP and other rendering stuff 
	if returns FALSE, screen cordinates are not on the screen,
	if returns TRUE, screen cordinated are valid and on the screen. */
	int world_to_screen(const vec& worldPos, vec2& screenPos, const view_matrix* viewMatrix);

	float getFOV(qangle& viewAngles, qangle& targetAngles);
	float FOVToRadius(float fovDegrees, float gameFOV, int screenWidth);

	/* returns view angles for the target world coordinates */
	qangle worldToViewangles(const vec& localPosition, const vec& targetPosition);

	vec projAimbotCalculations(vec& projectileStartPos, vec& targetPosition, vec& entVel, bool onGround);

	/* Finds the distance from localplayer crosshair to the target angles */
	inline float disFromCrosshair(const qangle& local_angles, const qangle& target_angles)
	{
		return (sqrt(pow(local_angles.pitch - target_angles.pitch, 2) + pow(local_angles.yaw - target_angles.yaw, 2)));
	}

	/* return distanance of a vec2 from the center of the screen. */
	float vec_dis_from_screen_center(const vec2& target_pos);

	/* keeps view angles in bound to prevent unneccesary bullshit */
	inline void verify_angles(qangle& angles)
	{
		/* fixing pitch */
		if (angles.pitch > 89.0f) angles.pitch = 89.0f;
		else if (angles.pitch < -89.0f) angles.pitch = -89.0f;

		/* fixing roll */
		if (angles.roll != 0.0f) angles.roll = 0.0f;
	}
}

enum cur_window
{
	QUOTE_WINDOW = 0,
	AIMBOT_WINDOW,
	ANTIAIM_WINDOW,
	WORLD_VISUALS_WINDOW,
	PLAYER_VISUALS_WINDOW,
	VIEW_VISUALS_WINDOW,
	MISCELLANEOUS_WINDOW,
	SKIN_CHANGER_WINDOW,
	CONFIG_WINDOW,
	SETTING_WINDOW
};

/*this holds imformation about the target process*/
namespace global
{
	extern vec2 window_size;

	extern const char* target_window_name;
	extern const char* target_proc_name;
	extern HWND target_hwnd;

	/* thread specific information */
	inline bool entities_popullated = false;
};

/* Handles to all game modules */
namespace handle
{
	extern uintptr_t client_dll;
	extern uintptr_t engine_dll;

	bool initialize();
};

/*This holds information about the textures and images used in 
the software*/
namespace resource
{
	/* logo , i didn't use */
	extern unsigned char logo_data[8581];
	extern raw_image_data logo;

	/* icons */
	extern unsigned char aimbot[577];
	extern raw_image_data aimbot_data;

	extern unsigned char folder[309];
	extern raw_image_data folder_data;

	extern unsigned char left_wing[1132];
	extern raw_image_data left_wing_data;

	extern unsigned char planet[500];
	extern raw_image_data planet_data;

	extern unsigned char player[491];
	extern raw_image_data player_data;

	extern unsigned char right_wing[1165];
	extern raw_image_data right_wing_data;

	extern unsigned char setting[621];
	extern raw_image_data setting_data;

	extern unsigned char stars[420];
	extern raw_image_data stars_data;

	extern unsigned char view[573];
	extern raw_image_data view_data;

	extern unsigned char misc_data[239];
	extern raw_image_data misc;

	extern unsigned char antiaim_data[382];
	extern raw_image_data anti_aim;

	/* fonts */
	extern unsigned char agency_FB[49624];
	extern raw_image_data agencyFB;

	extern unsigned char roboto_data[168488];
	extern raw_image_data roboto;

	extern unsigned char background_data[587353];
	extern raw_image_data background;

	extern unsigned char adobe_clean_bold_data[483456];
	extern raw_image_data adobe_clean_bold;

	extern unsigned char haas_black_data[100220];
	extern raw_image_data hass_black;

	extern unsigned char kabel_data[17460];
	extern raw_image_data kabel;

	extern unsigned char adobe_clean_light_data[486828];
	extern raw_image_data adobe_clean_light;
};

/* This holds the final data about the texture
and only this struct is to be used with ImGui texture rendering
EndScene function.*/
struct texture_data
{
	/* default constructor*/
	texture_data()
		:texture(nullptr), name("NULL"), image_width(0), image_height(0),
		rendering_image_height(0), rendering_image_width(0), aspect_ratio(0.0f),
		scalling_factor(1.0f)
	{}

	/*normal constructor */
	texture_data(
		void* texture_pntr = nullptr ,
		const char* texture_namme = "NOT SPECIFIED",
		int height = 0,
		int width = 0,
		float ratio = 0.0f,
		float scalling_ratio = 1.0f) :
		texture(texture_pntr),
		name(texture_namme),
		image_height(height),
		image_width(width),
		aspect_ratio(ratio),
		scalling_factor(scalling_ratio) {}

	void* texture;
	const char* name;
	int image_height;
	int image_width;
	
	/* WIDHT / HEIGHT, multiply height with this to get new width*/
	float aspect_ratio = (float)image_width / (float)image_height;

	/*scalling the image down*/
	float scalling_factor;

	/* final image resolutions to use*/
	int rendering_image_height = image_height * scalling_factor;
	int rendering_image_width = image_width * scalling_factor;

	/* function to update the final rendering resoulutiong cause 
	structs do not re-calculate the values after initialization*/
	inline void update_res()
	{
		if (scalling_factor != 1.0f)
		{
			rendering_image_height = (int)((float)image_height * scalling_factor);
			rendering_image_width = (int)((float)image_width * scalling_factor);
		}
		else
		{
			rendering_image_height = image_height;
			rendering_image_width = image_width;
		}
	}
};


/* This holds all the captures interfaces so I can use them across multiple files */
namespace interface_tf2
{
	extern IBaseClientDLL*			base_client;
	extern I_client_entity_list*	entity_list;
	extern IVEngineClient013*		engine;
	extern I_engine_client_replay*	engine_replay;
	extern IEngineTrace*			pEngineTrace;
	extern IVDebugOverlay*			pDebugOverlay;
	extern void*					IPanel;
};