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
	IEngineTrace* pEngineTrace				= nullptr;
	IVDebugOverlay* pDebugOverlay			= nullptr;
};

namespace TF2_functions
{
	T_lookUpBone lookUpBone = nullptr;
}

int entities::world_to_screen(const vec& worldPos, vec2& screen_pos, const view_matrix* matrix) {
	// Matrix-vector multiplication to get homogeneous coordinates (x, y, z, w)
	float w = matrix->m[3][0] * worldPos.x + matrix->m[3][1] * worldPos.y + matrix->m[3][2] * worldPos.z + matrix->m[3][3];

	// Check if the point is behind the camera (very small w value means behind the camera)
	if (w < 0.001f) {
		return 0;
	}

	// Perform the transformation for the x and y coordinates
	float x = matrix->m[0][0] * worldPos.x + matrix->m[0][1] * worldPos.y + matrix->m[0][2] * worldPos.z + matrix->m[0][3];
	float y = matrix->m[1][0] * worldPos.x + matrix->m[1][1] * worldPos.y + matrix->m[1][2] * worldPos.z + matrix->m[1][3];

	// Normalize by dividing by w to perform the perspective divide
	float invW = 1.0f / w;
	x *= invW;
	y *= invW;

	// Convert to screen space by mapping from normalized device coordinates to screen coordinates
	screen_pos.x = (global::window_size.x / 2.0f) + (x * global::window_size.x / 2.0f);
	screen_pos.y = (global::window_size.y / 2.0f) - (y * global::window_size.y / 2.0f); // Inverted Y-axis

	// Check if the screen position is within the window bounds
	if (screen_pos.x < 0.0f || screen_pos.x > global::window_size.x || screen_pos.y < 0.0f || screen_pos.y > global::window_size.y) {
		return 0;
	}

	return 1;
}

float entities::getFOV(qangle& viewAngles, qangle& targetAngles) {
	qangle delta = targetAngles - viewAngles;

	// Normalize angles
	delta.yaw = std::fmod(delta.yaw + 180.0f, 360.0f) - 180.0f;
	delta.pitch = std::fmod(delta.pitch + 180.0f, 360.0f) - 180.0f;

	return std::sqrt(delta.pitch * delta.pitch + delta.yaw * delta.yaw);
}


/* HIT-SCAN aimbot calculations */
qangle entities::worldToViewangles(const vec& localPosition, const vec& targetPosition) {
	vec delta = targetPosition - localPosition;

	float hypotenuse = std::sqrt(delta.x * delta.x + delta.y * delta.y);
	qangle angles;

	angles.pitch = -std::atan2(delta.z, hypotenuse) * (180.0f / M_PI);
	angles.yaw = std::atan2(delta.y, delta.x) * (180.0f / M_PI);

	if (angles.yaw < 0.0f) {
		angles.yaw += 360.0f;
	}

	entities::verify_angles(angles); // keeping angles in bound

	return angles;
}

/* PROJECTILE aimbot calculations */
vec entities::projAimbotCalculations(vec& projectileStartPos ,vec& targetPosition, vec& entVel, bool onGround) {

	float projectileVel = 1980.0f; // todo : make this dynamic 
	
	// Convert viewangles (degrees) to radians
	qangle viewAngles = entities::local::viewAngles.load();
	viewAngles.pitch = -viewAngles.pitch * M_PI / 180; // Pitch is inverted in-game
	viewAngles.yaw = viewAngles.yaw * M_PI / 180;

	// Calculate projectile velocity in different directions
	vec vecProjectileVel;
	vecProjectileVel.z = projectileVel * sin(viewAngles.pitch);
	vecProjectileVel.x = projectileVel * cos(viewAngles.pitch) * cos(viewAngles.yaw);
	vecProjectileVel.y = projectileVel * cos(viewAngles.pitch) * sin(viewAngles.yaw);

	// Calculating relative velocity
	vec relativeVel;
	relativeVel.x = vecProjectileVel.x - entVel.x;
	relativeVel.y = vecProjectileVel.y - entVel.y;
	relativeVel.z = vecProjectileVel.z - entVel.z;

	// calculating distance & time
	float distance = (projectileStartPos - targetPosition).mag(); // this will be + at all times
	float time = distance / projectileVel;

	// final position calculation
	vec targetFuturePos;
	targetFuturePos.x = targetPosition.x + entVel.x * time; // <- X axis
	targetFuturePos.y = targetPosition.y + entVel.y * time; // <- Y axis
	onGround ?
		targetFuturePos.z = targetPosition.z + entVel.z * time : // normal position calculation if on ground
		targetFuturePos.z = targetPosition.z + (entVel.z * time) + (0.5f * -TF2_GRAVITY * time * time); // else consider gravity

	return targetFuturePos;
}

float entities::vec_dis_from_screen_center(const vec2& target_pos)
{
	return (sqrt(pow(target_pos.x - global::window_size.x / 2, 2) + pow(target_pos.y - global::window_size.y / 2, 2)));
}