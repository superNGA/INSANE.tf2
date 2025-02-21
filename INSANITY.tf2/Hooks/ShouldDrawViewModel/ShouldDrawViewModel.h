#pragma once

namespace hook {
	namespace shouldDrawViewModel {
		typedef bool(__fastcall* T_shouldDrawViewModel)();
		extern T_shouldDrawViewModel O_shouldDrawViewModel;
		bool __fastcall H_shouldDrawViewModel();
	}
}