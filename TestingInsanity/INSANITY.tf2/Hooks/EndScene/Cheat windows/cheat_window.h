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

		//======================= Player Chams =======================
		ImGui::Checkbox("Friendly Player Chams player",		&config.visualConfig.bPlayerChamsFriendly);
		ImGui::Checkbox("enemy Chams player",				&config.visualConfig.bPlayerChamsEnemy);
		ImGui::Checkbox("IgnoreZ Friendly player",			&config.visualConfig.ignorezFriendlyPlayer);
		ImGui::Checkbox("IgnoreZ enemy player",				&config.visualConfig.ignorezEnemyPlayer);
		ImGui::ColorEdit4("team mates cham color",			&config.visualConfig.clrFriendlyPlayerChams.r, ImGuiColorEditFlags_AlphaBar);
		ImGui::ColorEdit4("enemy cham color",				&config.visualConfig.clrEnemyPlayerCham.r, ImGuiColorEditFlags_AlphaBar);

		//======================= Sentry Chams =======================
		ImGui::Checkbox("IgnoreZ sentry Enemy",				&config.visualConfig.ignorezEnemySentry);
		ImGui::Checkbox("Sentry Chams Enemy",				&config.visualConfig.bSentryEnemy);
		ImGui::ColorEdit4("Sentry cham color Enemy",		&config.visualConfig.clrSentryEnemy.r, ImGuiColorEditFlags_AlphaBar);

		ImGui::Checkbox("IgnoreZ sentry Friendly",			&config.visualConfig.ignorezFriendlySentry);
		ImGui::Checkbox("Sentry Chams Friendly",			&config.visualConfig.bSentryFriendly);
		ImGui::ColorEdit4("Sentry cham color Friendly",		&config.visualConfig.clrSentryFriendly.r, ImGuiColorEditFlags_AlphaBar);

		//======================= Teleporter Chams =======================
		ImGui::Checkbox("IgnoreZ teleporter Enemy",			&config.visualConfig.ignorezTeleporterEnemy);
		ImGui::Checkbox("Teleporter Chams Enemy",			&config.visualConfig.bTeleporterEnemy);
		ImGui::ColorEdit4("Teleporter cham color Enemy",	&config.visualConfig.clrTeleporterEnemy.r, ImGuiColorEditFlags_AlphaBar);

		ImGui::Checkbox("IgnoreZ teleporter Friendly",		&config.visualConfig.ignorezTeleporterFriendly);
		ImGui::Checkbox("Teleporter Chams Friendly",		&config.visualConfig.bTeleporterFriendly);
		ImGui::ColorEdit4("Teleporter cham color Friendly",	&config.visualConfig.clrTeleporterFriendly.r, ImGuiColorEditFlags_AlphaBar);

		//======================= Dispenser Chams =======================
		ImGui::Checkbox("IgnoreZ dispenser Enemy",			&config.visualConfig.ignorezDispenserEnemy);
		ImGui::Checkbox("Dispenser Chams Enemy",			&config.visualConfig.bDispenserEnemy);
		ImGui::ColorEdit4("Dispenser cham color Enemy",		&config.visualConfig.clrDispenserEnemy.r, ImGuiColorEditFlags_AlphaBar);

		ImGui::Checkbox("IgnoreZ dispenser Friendly",		&config.visualConfig.ignorezDispenserFirendly);
		ImGui::Checkbox("Dispenser Chams Friendly",			&config.visualConfig.bDispenserFirendly);
		ImGui::ColorEdit4("Dispenser cham color Friendly",	&config.visualConfig.clrDispenserFriendly.r, ImGuiColorEditFlags_AlphaBar);

		ImGui::Checkbox("IgnoreZ animating Ammo Pack",		&config.visualConfig.ignorezDropAmmoPack);
		ImGui::Checkbox("Animating Ammo Pack chams",		&config.visualConfig.bAnimAmmoPack);
		ImGui::ColorEdit4("Animating Ammo Pack cham color",	&config.visualConfig.clrAnimAmmoPackChams.r, ImGuiColorEditFlags_AlphaBar);

		ImGui::Checkbox("IgnoreZ Medkit",					&config.visualConfig.ignorezMedkit);
		ImGui::Checkbox("Medkit Chams",						&config.visualConfig.bMedkit);
		ImGui::ColorEdit4("Medkit cham color",				&config.visualConfig.clrMedkit.r, ImGuiColorEditFlags_AlphaBar);

		ImGui::Checkbox("IgnoreZ ammo Pack",				&config.visualConfig.ignorezAnimAmmoPack);
		ImGui::Checkbox("AmmoPack Chams",					&config.visualConfig.bDropAmmoPackChams);
		ImGui::ColorEdit4("AmmoPack cham color",			&config.visualConfig.clrDropAmmoPackChams.r, ImGuiColorEditFlags_AlphaBar);

		ImGui::Checkbox("IgnoreZ tf item",					&config.visualConfig.ignorezTfItem);
		ImGui::Checkbox("intelligence Chams",				&config.visualConfig.bTfItemChams);
		ImGui::ColorEdit4("intelligence cham color",		&config.visualConfig.clrTfItemCham.r, ImGuiColorEditFlags_AlphaBar);

		//======================= Projectile Chams =======================
		ImGui::Checkbox("IgnoreZ projectile Enemy",			&config.visualConfig.ignorezProjectilesEnemy);
		ImGui::Checkbox("projectile Chams Enemy",			&config.visualConfig.bProjectileEnemy);
		ImGui::ColorEdit4("projectile cham color Enemy",	&config.visualConfig.clrProjectileEnemy.r, ImGuiColorEditFlags_AlphaBar);

		ImGui::Checkbox("IgnoreZ projectile Friendly",		&config.visualConfig.ignorezProjectileFriendly);
		ImGui::Checkbox("projectile Chams Friendly",		&config.visualConfig.bProjectileFriendly);
		ImGui::ColorEdit4("projectile cham color Friendly",	&config.visualConfig.clrProjectileFirendly.r, ImGuiColorEditFlags_AlphaBar);

		ImGui::Checkbox("IgnoreZ viewmodel",				&config.visualConfig.ignorezViewModel);
		ImGui::Checkbox("view model chams",					&config.visualConfig.bViewModelChams);
		ImGui::ColorEdit4("view model chams clr",			&config.visualConfig.clrViewModelChams.r, ImGuiColorEditFlags_AlphaBar);
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