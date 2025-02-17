#pragma once
#include <cstdint>
#include "../../SDK/class/viewSetup.h"

namespace hook {
	namespace overrideView {
		typedef int64_t(__fastcall* T_overrideView)(void*, CViewSetup*);
		extern T_overrideView O_overrideView;
		int64_t __fastcall H_overrideView(void* VT_IClientMode, CViewSetup* pViewSetup);
	}
}