#include "GlobalVars.h"

namespace global
{
	vec2 window_size;
	const char* target_window_name		= "Team Fortress 2 - Direct3D 9 - 64 Bit";
	const char* target_proc_name		= "tf win64.exe";
	HWND target_hwnd					= nullptr;
};

namespace handle
{
	uintptr_t client_dll;
	uintptr_t engine_dll;
};

namespace resource
{
	/* logo */
	raw_image_data logo					(logo_data, sizeof(logo_data));

	/* icons */
	raw_image_data aimbot_data			(aimbot, sizeof(aimbot));
	raw_image_data folder_data			(folder, sizeof(folder));
	raw_image_data left_wing_data		(left_wing, sizeof(left_wing));
	raw_image_data planet_data			(planet, sizeof(planet));
	raw_image_data player_data			(player, sizeof(player));
	raw_image_data setting_data			(setting, sizeof(setting));
	raw_image_data right_wing_data		(right_wing, sizeof(right_wing));
	raw_image_data stars_data			(stars, sizeof(stars));
	raw_image_data view_data			(view, sizeof(view));
	raw_image_data misc					(misc_data, sizeof(misc_data));
	raw_image_data anti_aim				(antiaim_data, sizeof(antiaim_data));

	/* backgound */
	raw_image_data background			(background_data, sizeof(background_data));
	
	/* Fonts */
	raw_image_data agencyFB				(agency_FB, sizeof(agency_FB));
	raw_image_data roboto				(roboto_data, sizeof(roboto_data));
	raw_image_data adobe_clean_bold		(adobe_clean_bold_data, sizeof(adobe_clean_bold_data));
	raw_image_data hass_black			(haas_black_data, sizeof(haas_black_data));
	raw_image_data kabel				(kabel_data, sizeof(kabel_data)); // <- this is the super thick one
	raw_image_data adobe_clean_light	(adobe_clean_light_data, sizeof(adobe_clean_light_data));
}

bool handle::initialize()
{
	client_dll = reinterpret_cast<uintptr_t>(GetModuleHandle("client.dll"));
	engine_dll = reinterpret_cast<uintptr_t>(GetModuleHandle("engine.dll"));

	if (!client_dll || !engine_dll)
	{
		return false;
	}

	return true;	
}

namespace interface_tf2
{
	IBaseClientDLL* base_client				= nullptr;
	I_client_entity_list* entity_list		= nullptr;
	IVEngineClient013* engine				= nullptr;
	I_engine_client_replay* engine_replay	= nullptr;
};

bool entities::world_to_screen(const vec& worldPos, vec2& screen_pos, const view_matrix* matrix) {
	// Matrix-vector multiplication
	float w = matrix->m[3][0] * worldPos.x + matrix->m[3][1] * worldPos.y + matrix->m[3][2] * worldPos.z + matrix->m[3][3];
	if (w < 0.001f) { // Behind the camera
		return false;
	}

	float x = matrix->m[0][0] * worldPos.x + matrix->m[0][1] * worldPos.y + matrix->m[0][2] * worldPos.z + matrix->m[0][3];
	float y = matrix->m[1][0] * worldPos.x + matrix->m[1][1] * worldPos.y + matrix->m[1][2] * worldPos.z + matrix->m[1][3];

	// Normalize coordinates
	float invW = 1.0f / w;
	x *= invW;
	y *= invW;

	// Convert to screen space
	screen_pos.x = (global::window_size.x / 2.0f) + (x * global::window_size.x / 2.0f);
	screen_pos.y = (global::window_size.y / 2.0f) - (y * global::window_size.y / 2.0f); // Y-axis is inverted in screen space

	if ((screen_pos.x < 0.0f || screen_pos.x > global::window_size.x) || (screen_pos.y < 0.0f || screen_pos.y > global::window_size.y)) return false;
	
	return true;
}