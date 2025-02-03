#pragma once
#include <cstdint>
#include "../../GlobalVars.h"
#include "../../Libraries/Console System/Console_System.h"

extern Console_System cons;
namespace hook
{
	namespace renderGlowEffect
	{
		typedef int64_t(__fastcall* T_renderGlowEffect)(void*, int64_t, int64_t);
		extern T_renderGlowEffect O_renderGlowEffect;
		int64_t H_renderGlowEffect(glowManager* pTF_glowManager, int64_t pViewSetup, int64_t somethingIDK);
	}
}