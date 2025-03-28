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

		ImGui::Checkbox("bhop",							&config.miscConfig.bhop);
		ImGui::Checkbox("auto rocket jump",				&config.miscConfig.rocket_jump);
		ImGui::Checkbox("Third person",					&config.miscConfig.third_person);
		ImGui::Checkbox("Auto BackStab",				&config.miscConfig.autoBackStab);
		ImGui::Checkbox("Air Move ( in-complete )",		&config.miscConfig.airMove);

		ImGui::PopFont();
	}

	inline void draw_aimbot_window()
	{
		ImGui::Checkbox("GLOBAL",						&config.aimbotConfig.global);
		ImGui::SliderFloat("FOV",						&config.aimbotConfig.FOV, 0.0f, 180.0f, "%.2f");
		ImGui::Checkbox("PROJ. AIMBOT",					&config.aimbotConfig.projectile_aimbot);
		ImGui::Checkbox("Future position indicator",	&config.aimbotConfig.future_pos_helper);
		ImGui::Checkbox("Auto shoot",					&config.aimbotConfig.autoShoot);
	}

	inline void draw_player_visual_window()
	{
		ImGui::Checkbox("ESP",							&config.visualConfig.ESP);
		ImGui::Checkbox("Health bar",					&config.visualConfig.healthBar);
		ImGui::Checkbox("Skip disguised spy",			&config.visualConfig.skipDisguisedSpy);
		ImGui::Checkbox("Skip cloaked spy",				&config.visualConfig.skipCloackedSpy);
		ImGui::Checkbox("Name",							&config.visualConfig.playerName);

		ImGui::Checkbox("Player Chams",					&config.visualConfig.playerChams);
		ImGui::ColorEdit4("team mates cham color",		&config.visualConfig.clrFriendlyPlayerCham.r, ImGuiColorEditFlags_AlphaBar);
		ImGui::ColorEdit4("enemy cham color",			&config.visualConfig.clrEnemyPlayerCham.r, ImGuiColorEditFlags_AlphaBar);

		ImGui::Checkbox("Sentry Chams",					&config.visualConfig.sentryChams);
		ImGui::ColorEdit4("Sentry cham color",			&config.visualConfig.clrSentryCham.r, ImGuiColorEditFlags_AlphaBar);

		ImGui::Checkbox("Teleporter Chams",				&config.visualConfig.teleporterChams);
		ImGui::ColorEdit4("Teleporter cham color",		&config.visualConfig.clrTeleporterCham.r, ImGuiColorEditFlags_AlphaBar);

		ImGui::Checkbox("Dispenser Chams",				&config.visualConfig.dispenserChams);
		ImGui::ColorEdit4("Dispenser cham color",		&config.visualConfig.clrDispenserCham.r, ImGuiColorEditFlags_AlphaBar);

		ImGui::Checkbox("BaseAnimating Chams",			&config.visualConfig.baseAnimating);
		ImGui::ColorEdit4("BaseAnimating cham color",	&config.visualConfig.clrBaseAnimatingCham.r, ImGuiColorEditFlags_AlphaBar);

		ImGui::Checkbox("AmmoPack Chams",				&config.visualConfig.ammoPackChams);
		ImGui::ColorEdit4("AmmoPack cham color",		&config.visualConfig.clrAmmoPackCham.r, ImGuiColorEditFlags_AlphaBar);

		ImGui::Checkbox("intelligence Chams",			&config.visualConfig.tfItemChams);
		ImGui::ColorEdit4("intelligence cham color",	&config.visualConfig.clrTfItemCham.r, ImGuiColorEditFlags_AlphaBar);
	}

	inline void draw_view_visual_window() 
	{
		ImGui::SliderFloat("FOV", &config.viewConfig.FOV, 0.0f, 180.0f, "%.2f");
		ImGui::Checkbox("Remove sniper scope overlay",	&config.viewConfig.RemoveSniperScopeOverlay);
		ImGui::Checkbox("Remove sniper charge overlay", &config.viewConfig.RemoveSniperChargeOverlay);
		ImGui::Checkbox("draw model in thirdperson",	&config.viewConfig.alwaysRenderInThirdPerson);
		ImGui::Checkbox("keep viewModel while scopped",	&config.viewConfig.alwaysDrawViewModel);
	}

	inline void draw_config_window()
	{
		std::string testFile = "testConfig";
		if (ImGui::Button("create file")) {
			g_configManager.createFile(testFile);
		}
		ImGui::SameLine();
		if (ImGui::Button("load config")) {
			g_configManager.loadConfigFromFile(testFile, config);
		}
		ImGui::SameLine();
		if (ImGui::Button("save config")) {
			g_configManager.saveConfigToFile(testFile, config);
		}
		ImGui::SameLine();
		if (ImGui::Button("Check file")) {
			g_configManager.isFileSigned(testFile);
		}

		// file selection menu
		g_configManager.update_vecAllConfigFiles();
		if (ImGui::BeginCombo("select config", g_configManager.vec_allConfigFiles[g_configManager.activeConfigIndex].c_str())) {
			for (int i = 0; i < g_configManager.vec_allConfigFiles.size(); i++) {
				
				// if this file is getting selected, then save its index
				bool isSelected = (i == g_configManager.activeConfigIndex);
				if (ImGui::Selectable(g_configManager.vec_allConfigFiles[i].c_str(), &isSelected)) {
					g_configManager.activeConfigIndex = i;
				}
				
				// if current item is the selected one
				if (g_configManager.activeConfigIndex == i) {
					ImGui::SetItemDefaultFocus();
				}
			}

			ImGui::EndCombo();
		}
	}
};