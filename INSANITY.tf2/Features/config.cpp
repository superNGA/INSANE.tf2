#include "config.h"

namespace config
{
	namespace aimbot
	{
		bool global				= false;
		float FOV				= 10.0f;
		bool projectile_aimbot	= false;
		bool future_pos_helper	= false;
		bool autoShoot			= false;
	}
	namespace visuals
	{
		bool ESP				= false;
		bool healthBar			= false;
		bool skipDisguisedSpy	= false;
		bool skipCloackedSpy	= false;
		bool playerName			= false;
	}
	namespace view 
	{
		bool RemoveSniperScopeOverlay	= false;
		bool RemoveSniperChargeOverlay	= false;
		bool alwaysRenderInThirdPerson	= false;
		bool alwaysDrawViewModel		= false;
	}
	namespace miscellaneous
	{
		bool bhop			= false;
		bool rocket_jump	= false;
		bool third_person	= false;
	}
};