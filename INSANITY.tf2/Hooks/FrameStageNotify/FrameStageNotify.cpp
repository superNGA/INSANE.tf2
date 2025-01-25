#include "FrameStageNotify.h"

hook::frame_stage_notify::template_frame_stage_notify hook::frame_stage_notify::original_frame_stage_notify = nullptr;
void hook::frame_stage_notify::hook_frame_stage_notify(void* p_vtable, client_frame_stage frame_stage)
{
	original_frame_stage_notify(p_vtable, frame_stage);

	switch (frame_stage)
	{
	case FRAME_START:
		break;
	case FRAME_NET_UPDATE_START:
		break;
	case FRAME_NET_UPDATE_POSTDATAUPDATE_START:
		break;
	case FRAME_NET_UPDATE_POSTDATAUPDATE_END:
		break;
	case FRAME_NET_UPDATE_END:
		break;
	case FRAME_RENDER_START:
		entities::matrix_world_to_screen.store(interface_tf2::engine->WorldToScreenMatrix());
		break;
	case FRAME_RENDER_END:
		break;
	default:
		break;
	}
}