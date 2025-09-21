// UTILITY
#include "../../Utility/Hook Handler/Hook_t.h"
#include "../../Utility/ConsoleLogging.h"
#include "../../Features/Graphics Engine V2/Graphics.h"
#include "../../Features/Graphics Engine/DirectX Handler/DirectXHandler.h"

// SDK
#include "../../SDK/Entity Manager/entityManager.h"
#include "../../SDK/TF object manager/TFOjectManager.h"


//void hook::frame_stage_notify::hook_frame_stage_notify(void* p_vtable, client_frame_stage frame_stage)
MAKE_HOOK(FrameStateNotify, "48 83 EC ? 89 15", __stdcall, CLIENT_DLL, void, void* pVTable, client_frame_stage frameStage)
{

	Hook::FrameStateNotify::O_FrameStateNotify(pVTable, frameStage);

	static int16_t frameCounter = 0; // counts frames elapsed

	switch (frameStage)
	{
	case FRAME_NET_UPDATE_START:
		break;
	
	case FRAME_NET_UPDATE_POSTDATAUPDATE_END:
		break;

	case FRAME_NET_UPDATE_POSTDATAUPDATE_START:
		//Features::antiAim.StoreAABones();
		break;

	case FRAME_NET_UPDATE_END:
		F::graphics.CaptureW2SMatrix();
		F::directxHandler.SetW2SMatrix();

		if (frameCounter == 2) 
		{
			entityManager.UpdateLocalPlayer();
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