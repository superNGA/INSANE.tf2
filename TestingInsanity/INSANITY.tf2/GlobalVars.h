#pragma once
#include <Windows.h>
#include <atomic>
#include <mutex>
#include <string>
#include <unordered_map>
#include <vector>

/* game classes */
#include "SDK/class/I_BaseEntityDLL.h"
#include "SDK/class/Source Entity.h"
#include "SDK/class/IVEngineClient.h"
#include "SDK/class/I_EngineClientReplay.h"
#include "SDK/class/GlowManager.h"
#include "SDK/class/IVDebugOverlay.h"

// Forward Declares
class baseWeapon;

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
	// inline IEngineTrace* pEngineTrace = nullptr;
}

/* this will hold information about when threads are started & teminated */
namespace thread_termination_status
{
	inline bool thread1 = false;

	inline bool thread1_primed = false;
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
	extern Vec2		   window_size;

	extern const char* target_window_name;
	extern const char* target_proc_name;
	extern HWND		   target_hwnd;
};

// TODO : Who the fuck wrote this dog shit. Fix this!!! a 5 year old can write 
//			something better than this. This is international crime my nigga!
//			Worse than terrorism!
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