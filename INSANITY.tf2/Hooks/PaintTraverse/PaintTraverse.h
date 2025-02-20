#pragma once
#include <cstdint>

namespace hook {
	namespace paintTraverse {
		typedef int64_t(__fastcall* T_paintTraverse)(void*, int64_t, uint8_t, uint8_t);
		inline T_paintTraverse O_paintTraverse = nullptr;
		int64_t __fastcall H_paintTraverse(void* pVTable, int64_t PANEL, uint8_t idk1, uint8_t idk2);
	}
}