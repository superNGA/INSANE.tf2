#pragma once
#include "../../GlobalVars.h"
#include "EntityProcessing.h"

namespace hook
{
	namespace frame_stage_notify
	{
		typedef void(__fastcall* template_frame_stage_notify)(void*, client_frame_stage);
		extern template_frame_stage_notify original_frame_stage_notify;
		void hook_frame_stage_notify(void* p_vtable, client_frame_stage frame_stage);
	}
}