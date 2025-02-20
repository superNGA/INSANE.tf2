#pragma once
#include "../SDK/class/Basic Structures.h"

namespace config
{
	namespace aimbot
	{
		extern bool global;
		extern float FOV;
		extern bool projectile_aimbot;
		extern bool future_pos_helper;
	}
	namespace visuals
	{
		extern bool ESP;
		extern bool healthBar;
		extern bool skipDisguisedSpy;
		extern bool skipCloackedSpy;
		extern bool playerName;
	}
	namespace view
	{
		inline float FOV = 90.0f; // TODO : make it keep the default, and only change when user wants
		extern bool RemoveSniperScopeOverlay;
		extern bool RemoveSniperChargeOverlay;
	}
	namespace miscellaneous
	{
		extern bool bhop;
		extern bool rocket_jump;
		extern bool third_person;
	}
};