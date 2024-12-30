#pragma once
#include <d3d9.h>
#include <d3dx9.h>

//global vars
#include "../../GlobalVars.h"

//ImGui
#include "../../External Libraries/ImGui/imgui.h"
#include "../../External Libraries/ImGui/imgui_impl_dx9.h"
#include "../../External Libraries/ImGui/imgui_impl_win32.h"

// console system
#ifdef _DEBUG
#include "../../Libraries/Console System/Console_System.h"
extern Console_System cons;
#endif // _DEBUG

//stb image
//#define STB_IMAGE_IMPLEMENTATION // required for this library, I dont know what it does
#include "../../External Libraries/stb image/stb_image.h"

namespace directX
{
	/*These prevent my wonderfull cheat from crashing*/
	namespace UI
	{
		extern int height_window;
		extern int width_window;

		extern bool UI_initialized_DX9;
		extern bool shutdown_UI;
		extern bool UI_has_been_shutdown;
		extern bool UI_visble;
		extern bool WIN32_initialized;
	};

	/* This will be popullated initialize_image_texture funtion :)*/
	namespace textures
	{
		extern texture_data logo;
		extern bool are_textures_initialized;
	};

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
};