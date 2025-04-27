#pragma once
#include "../EndScene.h" // <- this file also includes the global.h which include config.h
#include "../../../Features/features.h"

inline void RenderFeaturesInVec(const std::vector<IFeature_t*>& vecAntiAimFeatures)
{
	uint32_t iLastWidgetId = 0;
	for (auto* pFeature : vecAntiAimFeatures)
	{
		// SPACING
		ImGui::Dummy(ImVec2(0.0f, 5.0f));
		if (pFeature->m_iRenderID != iLastWidgetId)
			ImGui::Dummy(ImVec2(0.0f, 15.0f));

		// RENDRING WIDGET
		switch (pFeature->m_eFeatureType)
		{
		case FeatureType_t::FEATURE_BOOL:
		{
			auto feature = static_cast<Feature_t<bool>*>(pFeature);
			ImGui::Checkbox(feature->m_szPath.c_str(), &feature->m_data);
		}
		break;

		case FeatureType_t::FEATURE_INT:
		{
			auto feature = static_cast<Feature_t<IN_IntegerSlider>*>(pFeature);
			ImGui::SliderInt(feature->m_szPath.c_str(), &feature->m_data.m_data, feature->m_data.m_min, feature->m_data.m_max);
		}
		break;

		case FeatureType_t::FEATURE_FLOAT:
		{
			auto feature = static_cast<Feature_t<IN_FloatSlider>*>(pFeature);
			ImGui::SliderFloat(feature->m_szPath.c_str(), &feature->m_data.m_data, feature->m_data.m_min, feature->m_data.m_max);
		}
		break;

		case FeatureType_t::FEATURE_COLOR:
		{
			auto feature = static_cast<Feature_t<IN_Color>*>(pFeature);
			ImGui::ColorEdit4(feature->m_szPath.c_str(), &feature->m_data.r, ImGuiColorEditFlags_AlphaBar);
		}
		break;

		default:
			break;
		}

		iLastWidgetId = pFeature->m_iRenderID;
	}
}

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

		//ImGui::SliderFloat("No Spread offset", &config.miscConfig.flNoSpreadOffset, -1.0f, 1.0f, "%.6f");
		ImGui::SliderInt("No Spread offset", &config.miscConfig.iServerTimeOffset,-10, 10, "%d");

		ImGui::PopFont();
	}

	inline void draw_aimbot_window()
	{
		ImGui::Checkbox("GLOBAL",						&config.aimbotConfig.global);
		ImGui::SliderFloat("FOV",						&config.aimbotConfig.FOV, 0.0f, 180.0f, "%.2f");
		ImGui::Checkbox("PROJ. AIMBOT",					&config.aimbotConfig.projectile_aimbot);
		ImGui::Checkbox("Future position indicator",	&config.aimbotConfig.future_pos_helper);
		ImGui::Checkbox("Auto shoot",					&config.aimbotConfig.autoShoot);

		ImGui::Checkbox("No Spread",					&config.aimbotConfig.bNoSpread);
	}

	// Make something proper in final release, but for now it should do.
	inline void DrawChamButtons(ChamSetting_t& chamConfig, std::string text)
	{
		ImGui::Text(text.c_str());
		ImGui::Checkbox((text + "IgnoreZ").c_str(), &chamConfig.bIgnorez);
		ImGui::Checkbox((text + "toggle").c_str(), &chamConfig.bChams);
		ImGui::ColorEdit4((text + "color").c_str(), &chamConfig.clrChams.r, ImGuiColorEditFlags_AlphaBar);
	}

	inline void draw_player_visual_window()
	{
		ImGui::Checkbox("ESP",							&config.visualConfig.ESP);
		ImGui::Checkbox("Health bar",					&config.visualConfig.healthBar);
		ImGui::Checkbox("Skip disguised spy",			&config.visualConfig.skipDisguisedSpy);
		ImGui::Checkbox("Skip cloaked spy",				&config.visualConfig.skipCloackedSpy);
		ImGui::Checkbox("Name",							&config.visualConfig.playerName);

		DrawChamButtons(config.visualConfig.ChamEnemyPlayer,		"Enemy player");
		DrawChamButtons(config.visualConfig.ChamFriendlyPlayer,		"Friendly player");

		DrawChamButtons(config.visualConfig.ChamEnemySentry,		"Enemy Sentry");
		DrawChamButtons(config.visualConfig.ChamFriendlySentry,		"Friendly sentry");

		DrawChamButtons(config.visualConfig.ChamEnemyDispenser,		"Enemy dispenser");
		DrawChamButtons(config.visualConfig.ChamFriendlyDispenser,	"Friendly dispenser");

		DrawChamButtons(config.visualConfig.ChamEnemyTeleporter,	"Enemy teleporter");
		DrawChamButtons(config.visualConfig.ChamFriendlyTeleporter, "Friendly Teleporter");

		DrawChamButtons(config.visualConfig.ChamDroppedAmmoPack,    "dropped ammo pack");
		DrawChamButtons(config.visualConfig.ChamMedkit,			    "Med kit");
		DrawChamButtons(config.visualConfig.ChamAnimAmmoPack,	    "Animation ammo pack");
		DrawChamButtons(config.visualConfig.ChamTFItem,			    "tf item");
																    
		DrawChamButtons(config.visualConfig.ChamEnemyProjectile,    "Enemy projectile");
		DrawChamButtons(config.visualConfig.ChamFriendlyProjectile, "Friendly projectle");

		DrawChamButtons(config.visualConfig.ChamViewModel,			"view model");
		DrawChamButtons(config.visualConfig.ChamDroppedWeapon,		"Dropped weapon");
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

	inline void drawAntiAimWindow()
	{
		auto& vecAntiAimFeatures = allFeatures.m_umBaseFeatures["antiaim"]->m_vecChildFeature;
		RenderFeaturesInVec(vecAntiAimFeatures);
		
		auto& vecFakeLagFeatures= allFeatures.m_umBaseFeatures["FakeLag"]->m_vecChildFeature;
		RenderFeaturesInVec(vecFakeLagFeatures);
		
		auto& vecCritHackFeatures = allFeatures.m_umBaseFeatures["CritHack"]->m_vecChildFeature;
		RenderFeaturesInVec(vecCritHackFeatures);
	}
};