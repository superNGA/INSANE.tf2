//#include "renderGlowEffect.h"
#include "../Utility/Hook_t.h"
#include "../Utility/ConsoleLogging.h"

#include "../SDK/TF object manager/TFOjectManager.h"
#include "../SDK/class/GlowManager.h"

MAKE_HOOK(RenderGlowEffect, "48 89 6C 24 ? 48 89 74 24 ? 57 48 83 EC ? 48 8B E9 41 8B F8 48 8B 0D", __stdcall, CLIENT_DLL, int64_t,
	glowManager* pTF_glowManager, int64_t pViewSetup, int64_t somethingIDK)
{
	static bool updatedGlowManager = false;
	
	if (!updatedGlowManager) {

		TF_objects::pGlowManager = pTF_glowManager;
		WIN_LOG("Updated glow manager adrs : %p", (uintptr_t)pTF_glowManager);
		updatedGlowManager = true;
	}

	return Hook::RenderGlowEffect::O_RenderGlowEffect(pTF_glowManager, pViewSetup, somethingIDK);
}