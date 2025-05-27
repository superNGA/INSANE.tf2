#pragma once
#define _CRT_SECURE_NO_WARNINGS
#include <chrono>
#include <d3d9.h>
#include <d3dx9.h>

//global vars
#include "../../GlobalVars.h"

//ImGui
#include "../../External Libraries/ImGui/imgui.h"
#include "../../External Libraries/ImGui/imgui_impl_dx9.h"
#include "../../External Libraries/ImGui/imgui_impl_win32.h"

/*extra*/
#include "../../Extra/spacing.h"


//stb image
#include "../../External Libraries/stb image/stb_image.h"

namespace directX
{
	/*These prevent my wonderfull cheat from crashing*/
	namespace UI
	{
		/* this is the important one, this stores the window the user wants to see.
		the default is 0, where I plan to show quotes :)*/
		extern cur_window section_index;

		extern int	height_window;
		extern int	width_window;

		extern bool UI_initialized_DX9;
		extern bool shutdown_UI;
		extern bool UI_has_been_shutdown;
		extern bool UI_visble;
		extern bool WIN32_initialized;

		extern int	top_text_width;
		extern bool static_calc_done;
		
		extern bool done_styling;
		extern const char* heading;

		extern ImVec4 left_menu;
		extern ImVec4 right_menu;

		extern ImVec2 padding_001;
		extern ImVec2 standard_button_size;
		extern ImVec2 close_button_size;

		extern float max_BG_opacity;
		extern float cur_BG_alpha;

		/*this is the start point of the all & any main menu content */
		extern ImVec2 content_start_point;

		/* shutdown animation vars */
		extern bool animation_done;
		extern const char* end_message;
		extern ImVec2 end_meassage_size;
		extern float shutdown_anim_duration;
		extern std::chrono::time_point<std::chrono::high_resolution_clock> shutdown_anim_start_time;
		extern bool time_noted;
		extern ImFont* shutdown_anim_font;
	};

	/* This will be popullated initialize_image_texture funtion :)*/
	namespace textures
	{
		/* logo */
		extern texture_data logo;
		extern bool are_textures_initialized;

		/* icons */
		extern texture_data aimbot;
		extern texture_data folder;
		extern texture_data left_wing;
		extern texture_data right_wing;
		extern texture_data planet;
		extern texture_data player;
		extern texture_data setting;
		extern texture_data stars;
		extern texture_data view;
		extern texture_data misc;
		extern texture_data antiaim;

		/* texture */
		extern texture_data background;
	};

	/* information about fonts and their initializtaions status*/
	namespace fonts
	{
		extern ImFont* roboto;
		extern ImFont* agency_FB;
		extern ImFont* agency_FB_small;
		extern ImFont* adobe_clean_bold;
		extern ImFont* haas_black;
		extern ImFont* kabel;
		extern ImFont* adobe_clean_light;

		extern bool fonts_initialized;
	};

	/* this shall hold some basics color definitions, which I might let the user 
	control later*/
	namespace colours
	{
		extern ImVec4 grey;
		extern ImColor white;
		extern ImColor main_menu;
		extern ImColor side_menu;
	};

	/* ImGui vars global */
	extern ImGuiIO* IO;
	extern ImGuiContext* context;

	/* time vars. (just used in fade-in mechanism)*/
	extern bool time_refreshed;
	extern std::chrono::time_point<std::chrono::high_resolution_clock> menu_visible_time;

	/*Function template for EndScene*/
	typedef HRESULT(APIENTRY* T_endscene)(LPDIRECT3DDEVICE9);
	
	inline T_endscene O_endscene = nullptr;
	inline LPDIRECT3DDEVICE9 device = nullptr;

	/*This is the hook function, all ImGui stuff will happen here*/
	HRESULT H_endscene(LPDIRECT3DDEVICE9 P_DEVICE); 

	/*This function will initialize all the image texture which will
	be used during the life time of the software
	to be called only once*/
	void initialize_image_texture();

	/*This function will turn image data into a DirectX 9 texture,
	which ImGui can use directly.*/
	void* load_texture_from_image_data(raw_image_data& image_data, texture_data& texture_object);

	/*does the ImGui DirectX9 backend implementation & 
	ImGui WIN32 backend implementation*/
	void initialize_backends();

	/*Shuts down all the back ends of ImGui*/
	void shutdown_imgui();

	/*Loads all the fonts into memory and fills the ImFont pointers
	so the fonts can be pushed and popped easily*/
	void load_all_fonts();

	/*does stupid maths cuase ImGui is a bitch I did to improve
	performance could have been avoided if I was smart.
	NOTE : Only do this after all fonts are loaded */
	void do_static_calc();

	/* does the ImGui styling and shit*/
	void make_it_pretty();

	/* drawing background window sperately do reduce clutter*/
	inline void draw_background();
};

inline bool IsMenuOpen() { return directX::UI::UI_visble; }