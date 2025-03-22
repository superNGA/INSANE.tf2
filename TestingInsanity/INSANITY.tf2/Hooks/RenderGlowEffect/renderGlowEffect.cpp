#include "renderGlowEffect.h"

hook::renderGlowEffect::T_renderGlowEffect hook::renderGlowEffect::O_renderGlowEffect = nullptr;
int64_t hook::renderGlowEffect::H_renderGlowEffect(glowManager* pTF_glowManager, int64_t pViewSetup, int64_t somethingIDK) {

	static bool updatedGlowManager = false;
	
	if (!updatedGlowManager) {

		TF_objects::pGlowManager = pTF_glowManager;
		#ifdef _DEBUG
		cons.Log(FG_GREEN, "GLOW MANAGER", "Updated glow manager adrs : %p", (uintptr_t)pTF_glowManager);
		#endif
		updatedGlowManager = true;
	}

	return hook::renderGlowEffect::O_renderGlowEffect(pTF_glowManager, pViewSetup, somethingIDK);
}