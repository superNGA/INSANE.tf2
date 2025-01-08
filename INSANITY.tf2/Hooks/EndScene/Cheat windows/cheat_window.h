#pragma once
#include "../EndScene.h" // <- this file also includes the global.h which include config.h

namespace cheat_window
{
	namespace quotes
	{
		extern ImFont* quote_font;
		extern ImVec2 quote_size;

		/* defines your quotes here */
		extern const char* quote1;
	};

	inline void draw_quote_window()
	{
		ImDrawList* draw_list = ImGui::GetWindowDrawList();

		draw_list->AddText(
			ImVec2(directX::UI::content_start_point.x + (ImGui::GetContentRegionAvail().x - quotes::quote_size.x) / 2, directX::UI::content_start_point.y + (ImGui::GetContentRegionAvail().y - quotes::quote_size.y) / 2),
			directX::colours::white,
			quotes::quote1
		);
	}

	inline void draw_miscellaneous_window()
	{
		ImGui::PushFont(directX::fonts::agency_FB_small);

		ImGui::Checkbox("bhop", &config::miscellaneous::bhop);

		ImGui::PopFont();
	}
};