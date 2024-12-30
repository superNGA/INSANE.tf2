#pragma once
#include <Windows.h>

struct raw_image_data
{
	raw_image_data(unsigned char* bytes, size_t size)
		:image_bytes(bytes), image_bytearray_size(size) {}

	unsigned char* image_bytes;
	size_t image_bytearray_size;
};

/*this holds imformation about the target process*/
namespace global
{
	extern const char* target_window_name;
	extern const char* target_proc_name;
	extern HWND target_hwnd;
};

/*This holds information about the textures and images used in 
the software*/
namespace resource
{
	extern unsigned char logo_data[8581];
	extern raw_image_data logo;
};

struct texture_data
{
	void* texture;
	int image_height;
	int image_width;
	const char* name;
};

