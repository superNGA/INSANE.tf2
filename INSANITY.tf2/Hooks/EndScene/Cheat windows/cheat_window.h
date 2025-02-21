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
		ImGui::Checkbox("auto rocket jump", &config::miscellaneous::rocket_jump);
		ImGui::Checkbox("Third person", &config::miscellaneous::third_person);

		ImGui::PopFont();
	}

	inline void draw_aimbot_window()
	{
		ImGui::Checkbox("GLOBAL", &config::aimbot::global);
		ImGui::SliderFloat("FOV", &config::aimbot::FOV, 0.0f, 180.0f, "%.2f");
		ImGui::Checkbox("PROJ. AIMBOT", &config::aimbot::projectile_aimbot);
		ImGui::Checkbox("Future position indicator", &config::aimbot::future_pos_helper);
		ImGui::Checkbox("Auto shoot", &config::aimbot::autoShoot);
	}

	inline void draw_player_visual_window()
	{
		ImGui::Checkbox("ESP", &config::visuals::ESP);
		ImGui::Checkbox("Health bar", &config::visuals::healthBar);
		ImGui::Checkbox("Skip disguised spy", &config::visuals::skipDisguisedSpy);
		ImGui::Checkbox("Skip cloaked spy", &config::visuals::skipCloackedSpy);
		ImGui::Checkbox("Name", &config::visuals::playerName);
	}

	inline void draw_view_visual_window() 
	{
		ImGui::SliderFloat("FOV", &config::view::FOV, 0.0f, 180.0f, "%.2f");
		ImGui::Checkbox("Remove sniper scope overlay",	&config::view::RemoveSniperScopeOverlay);
		ImGui::Checkbox("Remove sniper charge overlay", &config::view::RemoveSniperChargeOverlay);
		ImGui::Checkbox("draw model in thirdperson",	&config::view::alwaysRenderInThirdPerson);
		ImGui::Checkbox("keep viewModel while scopped",	&config::view::alwaysDrawViewModel);
	}
};