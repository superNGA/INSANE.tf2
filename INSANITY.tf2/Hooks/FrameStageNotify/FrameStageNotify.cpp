#include "FrameStageNotify.h"

hook::frame_stage_notify::template_frame_stage_notify hook::frame_stage_notify::original_frame_stage_notify = nullptr;
void hook::frame_stage_notify::hook_frame_stage_notify(void* p_vtable, client_frame_stage frame_stage)
{
	original_frame_stage_notify(p_vtable, frame_stage);
	static int16_t frameCounter = 0; // counts frames elapsed

	switch (frame_stage)
	{
	case FRAME_NET_UPDATE_END: 
		break;
	case FRAME_RENDER_START:

		if (frameCounter == 2) { // calling every third frame
			processEntities();
			frameCounter = 0; // reseting frame counter
		}

		frameCounter++;
		break;
	case FRAME_RENDER_END: 
		break;
	default: 
		break;
	}
}