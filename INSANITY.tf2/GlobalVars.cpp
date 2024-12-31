#include "GlobalVars.h"

namespace global
{
	const char* target_window_name = "Team Fortress 2 - Direct3D 9 - 64 Bit";
	const char* target_proc_name = "tf win64.exe";
	HWND target_hwnd = nullptr;
};

namespace resource
{
	/* textures */
	raw_image_data logo(logo_data, sizeof(logo_data));
	
	/* Fonts */
	raw_image_data agencyFB(agency_FB, sizeof(agency_FB));
	raw_image_data roboto(roboto_data, sizeof(roboto_data));
}