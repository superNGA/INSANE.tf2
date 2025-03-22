#pragma once
#include <cstdint>

#include "../../SDK/offsets/offsets.h"
#include "../../SDK/class/CUserCmd.h"
#include "../../Features/features.h"

extern local_netvars netvar;

namespace hook
{
	namespace createmove
	{
		const uint8_t spacebar_state = 1 << 0;

		typedef bool(__fastcall* template_createmove)(int64_t, int64_t, CUserCmd*);
		extern template_createmove original_createmove;
		bool hooked_createmove(int64_t a1, int64_t a2, CUserCmd* cmd);
	}
}