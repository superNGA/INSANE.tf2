#include "FrameStageNotify.h"
#include "../../SDK/Entity Manager/entityManager.h"
#include "../../SDK/TF object manager/TFOjectManager.h"

hook::frame_stage_notify::template_frame_stage_notify hook::frame_stage_notify::original_frame_stage_notify = nullptr;
void hook::frame_stage_notify::hook_frame_stage_notify(void* p_vtable, client_frame_stage frame_stage)
{
	original_frame_stage_notify(p_vtable, frame_stage);
	static int16_t frameCounter = 0; // counts frames elapsed

	switch (frame_stage)
	{
	case FRAME_NET_UPDATE_POSTDATAUPDATE_END:
		break;

	case FRAME_NET_UPDATE_END:
		if (frameCounter == 2) 
		{
			uint64_t startTime = __rdtsc();
			entityManager.processEntities();
			uint64_t endTime = __rdtsc();

			//======================= performance tracking =======================
			//float deltaTimeinMs		= (endTime - startTime) * (1e6 / 2.9e9);
			//float timePerFrameinMs	= deltaTimeinMs / 3.0f;
			//float timePerFrameinSec	= timePerFrameinMs / 1e6;
			//uint16_t fps			= 1.0f / tfObject.pGlobalVar->absoluteframetime;
			//uint16_t estFps			= 1.0f / (tfObject.pGlobalVar->absoluteframetime - timePerFrameinSec);
			
			//std::cout << "entityManager.processEntities() time : " << deltaTimeinMs;
			//std::cout << " | FPS : " << fps << " , EST. FPS : " << estFps << " | frame drop : " << estFps - fps <<'\n';

			frameCounter = 0; // reseting frame counter
		}
		frameCounter++;
		break;

	case FRAME_RENDER_START:
		break;

	case FRAME_RENDER_END: 
		break;
	default: 
		break;
	}
}