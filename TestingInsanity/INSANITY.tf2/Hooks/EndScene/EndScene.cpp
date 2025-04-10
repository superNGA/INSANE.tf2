#pragma once
#define _CRT_SECURE_NO_WARNINGS
#include <format>
#include "EndScene.h"
#include "cheatFeaturesRendering.h"
#include "Cheat windows/cheat_window.h"
#include "../../Features/NoSpread/NoSpread.h"


/*================ Giving namespace variables default value here ========================*/
namespace directX {
    namespace UI
    {
        cur_window section_index = QUOTE_WINDOW;

        int height_window = 575;
        int width_window = 950;

        bool UI_initialized_DX9 = false;
        bool shutdown_UI = false;
        bool UI_has_been_shutdown = false;
        bool UI_visble = true;
        bool WIN32_initialized = false;

        int top_text_width = 0;
        bool static_calc_done = false;
        const char* heading = "I      N      S      A      N      E";

        bool done_styling = false;

        ImVec4 left_menu(0, 1, 3, 210);
        ImVec4 rigth_menu(10, 10, 10, 255);

        ImVec2 padding_001(5.0f, 5.0f);
        ImVec2 standard_button_size(200.0f, 40.0f);
        ImVec2 close_button_size(40.0f, 40.0f);

        float max_BG_opacity = 150.0f;
        float cur_BG_alpha = 255.0f;

        ImVec2 content_start_point(0.0f, 0.0f);

        bool animation_done = false;
        const char* end_message = "bye! :)";
        ImVec2 end_meassage_size(0.0f, 0.0f);
        float shutdown_anim_duration = 500.0f;
        std::chrono::time_point<std::chrono::high_resolution_clock> shutdown_anim_start_time = std::chrono::high_resolution_clock::now();
        bool time_noted = false;
        ImFont* shutdown_anim_font = nullptr; // <- intialized in load_all_fonts() cause initalizing it here will just fill current values, which are all nullptrs :(
    };

    namespace textures
    {
        bool are_textures_initialized = false;

        texture_data logo(nullptr, "Logo", 0, 0, 1,0.2f);
        texture_data aimbot(nullptr, "aimbot scope");
        texture_data folder(nullptr, "config folder");
        texture_data left_wing(nullptr, "left wing image");
        texture_data right_wing(nullptr, "right wing image");
        texture_data planet(nullptr, "planet");
        texture_data player(nullptr, "player");
        texture_data setting(nullptr, "setting");
        texture_data stars(nullptr, "stars");
        texture_data view(nullptr, "view");
        texture_data misc(nullptr, "miscellaneous");
        texture_data antiaim(nullptr, "anti aim");

        /* background */
        texture_data background(nullptr, "background image");
    };

    namespace fonts
    {
        ImFont* roboto = nullptr;
        ImFont* agency_FB = nullptr;
        ImFont* agency_FB_small = nullptr;
        ImFont* adobe_clean_bold = nullptr;
        ImFont* haas_black = nullptr;
        ImFont* kabel = nullptr;
        ImFont* adobe_clean_light = nullptr;

        bool fonts_initialized = false;
    };

    namespace colours
    {
        ImVec4 grey(40.0f/255.0f, 40.0f / 255.0f, 40.0f / 255.0f, 200.0f/255.0f);

        ImColor white(255, 255, 255, 255);
        ImColor main_menu(7, 7, 7, (BYTE)(UI::cur_BG_alpha * 2.0f));
        ImColor side_menu(2, 2, 2, (BYTE)(UI::cur_BG_alpha * 1.8f));
    };

    ImGuiIO* IO = nullptr;
    ImGuiContext* context = nullptr;

    bool time_refreshed = false;
    std::chrono::time_point<std::chrono::high_resolution_clock> menu_visible_time = std::chrono::high_resolution_clock::now();
};

/*========================= Initializing functions here =======================================*/

HRESULT directX::H_endscene(LPDIRECT3DDEVICE9 P_DEVICE)
{
	if (!device)
	{
		device = P_DEVICE;
	}

    /* Doing the backend stuff */
    if (!UI::UI_initialized_DX9 || !UI::WIN32_initialized)
    {
        initialize_backends();
    }

    /* Initializing textures */
    if (!textures::are_textures_initialized)
    {
        initialize_image_texture();
        textures::are_textures_initialized = true;

        #ifdef _DEBUG
        cons.Log(" All textures initialized ", FG_GREEN);
        #endif
    }

    /* Initializing fonts */
    if (!fonts::fonts_initialized && UI::UI_initialized_DX9 && UI::WIN32_initialized)
    {
        load_all_fonts();
    }

    /* skipping all if already ended */
    if (UI::UI_has_been_shutdown) return O_endscene(P_DEVICE);

    /* Starting ImGui new frame*/
    ImGuiIO& io = ImGui::GetIO();
    global::window_size.x = ImGui::GetMainViewport()->Size.x;
    global::window_size.y = ImGui::GetMainViewport()->Size.y;
    io.DisplaySize = ImVec2(1920.0f, 1080.0f); // Replace with actual screen resolution
    ImGui_ImplWin32_NewFrame();
    ImGui_ImplDX9_NewFrame();
    ImGui::NewFrame();

    /* Calculating text size here */
    if (!UI::static_calc_done)
    {
        do_static_calc();

        #ifdef _DEBUG
        cons.Log("done Static calculations", FG_GREEN);
        #endif // _DEBUG
    }

    /* doing menu styling */
    if (!UI::done_styling)
    {
        make_it_pretty();
    }

    /* RENDERING CHEAT FEATURES */
    //ImDrawList* draw_list = ImGui::GetBackgroundDrawList();
    //draw_list->PushClipRectFullScreen(); // if you don't do this then it isn't visible  
    //draw_list->PopClipRect();

    // PERFORMANCE TRACKING
    ImGui::SetNextWindowPos(ImVec2(0.0f, 0.0f), ImGuiCond_Always);
    ImGui::Begin("Performance", nullptr, 
        ImGuiWindowFlags_NoMove | ImGuiWindowFlags_AlwaysAutoResize |
        ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse);
    ImGui::Text("DME exec.time              : %.4f\n", Timer::flDMETimeInMs.load());
    ImGui::Text("EST. server time           : %.6f\n", noSpread.m_flCurServerEngineTime.load());
    ImGui::Text("server time storage status : %s\n", noSpread.m_bStoredServerTime.load() ? "Stored :)" : "Not stored :(");
    ImGui::End();
    //ImGui::PopFont();


    if (UI::UI_visble)
    {
        /* refreshing time */
        if (!time_refreshed)
        {
            /* refeshing global time vars. */
            menu_visible_time = std::chrono::high_resolution_clock::now();
            time_refreshed = true;

            /* globally storing/refreshing Imgui io */
            IO = &ImGui::GetIO();

            #ifdef _DEBUG
            cons.Log("refreshed start time", FG_GREEN);
            #endif
        }

        /* making new default font */
        ImGui::PushFont(fonts::agency_FB);

        /* Drawing background window & clock, fade-in mechanism is also done here*/
        draw_background();

        /* Initializing window*/
        ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.0f, 0.0f, 0.0f, 0.0f));
        ImGui::SetNextWindowSize(ImVec2(UI::width_window, UI::height_window));
        ImGui::Begin("INSANITY", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize);

        /* getting essential measurments */
        ImVec2 window_pos = ImGui::GetWindowPos();
        ImDrawList* draw_list = ImGui::GetWindowDrawList();

        /* left side menu rectangle */
        draw_list->AddRectFilled(
            window_pos,
            ImVec2(window_pos.x + 200.0f, window_pos.y + UI::height_window),
            colours::side_menu,
            10.0f,
            ImDrawFlags_RoundCornersBottomLeft | ImDrawFlags_RoundCornersTopLeft);

        /* right side menu rectangle */
        draw_list->AddRectFilled(
            ImVec2(window_pos.x + 200.0f, window_pos.y),
            ImVec2(window_pos.x + UI::width_window, window_pos.y + UI::height_window),
            colours::main_menu,
            10.0f,
            ImDrawFlags_RoundCornersTopRight | ImDrawFlags_RoundCornersBottomRight);

        /*child : SIDE MENU*/
        ImGui::BeginChild("sections", ImVec2(200.0f, UI::height_window), true);

        ImGui::PushFont(fonts::roboto);

        /*spacing*/
        asthetic::top_spacing(12);
        asthetic::left_spacing();

        /*text : Aim assist*/
        ImGui::PushFont(fonts::agency_FB_small);
        ImGui::PushStyleColor(ImGuiCol_Text, colours::grey);
        ImGui::Text("Aim assit");
        ImGui::PopStyleColor();
        ImGui::PopFont();

        /*button : AIMBOT*/
        ImVec2 cursor_pos = ImGui::GetCursorScreenPos();
        if (ImGui::Button("##aimbot", UI::standard_button_size)) UI::section_index = AIMBOT_WINDOW;
        asthetic::create_button_with_image(cursor_pos, textures::aimbot, "AIMBOT", false);

        /*button : AITI-AIM*/
        cursor_pos = ImGui::GetCursorScreenPos();
        if (ImGui::Button("##antiaim", UI::standard_button_size)) UI::section_index = ANTIAIM_WINDOW;
        asthetic::create_button_with_image(cursor_pos, textures::antiaim, "ANTI-AIM", false);

        /* spacing */
        asthetic::top_spacing(2);
        asthetic::left_spacing();

        /*text : Visuals*/
        ImGui::PushFont(fonts::agency_FB_small);
        ImGui::PushStyleColor(ImGuiCol_Text, colours::grey);
        ImGui::Text("Visuals");
        ImGui::PopStyleColor();
        ImGui::PopFont();

        /*button : WORLD VISUALS*/
        cursor_pos = ImGui::GetCursorScreenPos();
        if (ImGui::Button("##world", UI::standard_button_size)) UI::section_index = WORLD_VISUALS_WINDOW;
        asthetic::create_button_with_image(cursor_pos, textures::planet, "WORLD", false);

        /*button : PLAYER VISUALS*/
        cursor_pos = ImGui::GetCursorScreenPos();
        if (ImGui::Button("##player", UI::standard_button_size)) UI::section_index = PLAYER_VISUALS_WINDOW;
        asthetic::create_button_with_image(cursor_pos, textures::player, "PLAYER", false);

        /*button : VIEW -> VISUALS*/
        cursor_pos = ImGui::GetCursorScreenPos();
        if (ImGui::Button("##view", UI::standard_button_size)) UI::section_index = VIEW_VISUALS_WINDOW;
        asthetic::create_button_with_image(cursor_pos, textures::view, "VIEW", false);

        /* spacing */
        asthetic::top_spacing(2);
        asthetic::left_spacing();

        /*text : Miscellaneous*/
        ImGui::PushFont(fonts::agency_FB_small);
        ImGui::PushStyleColor(ImGuiCol_Text, colours::grey);
        ImGui::Text("Miscellaneous");
        ImGui::PopStyleColor();
        ImGui::PopFont();

        /*button : MISCELLANEOUS*/
        cursor_pos = ImGui::GetCursorScreenPos();
        if (ImGui::Button("##misc", UI::standard_button_size)) UI::section_index = MISCELLANEOUS_WINDOW;
        asthetic::create_button_with_image(cursor_pos, textures::misc, "MISC", false);

        /*button : SKIN CHANGER*/
        cursor_pos = ImGui::GetCursorScreenPos();
        if (ImGui::Button("##skins", UI::standard_button_size)) UI::section_index = SKIN_CHANGER_WINDOW;
        asthetic::create_button_with_image(cursor_pos, textures::stars, "SKINS", false);

        /*buttons : CONFIG*/
        ImGui::SetCursorScreenPos(ImVec2(window_pos.x, window_pos.y + UI::height_window - UI::standard_button_size.y));
        cursor_pos = ImGui::GetCursorScreenPos();
        if (ImGui::Button("##config", ImVec2(UI::standard_button_size.x / 2, UI::standard_button_size.y))) UI::section_index = CONFIG_WINDOW;
        asthetic::create_half_button(cursor_pos, textures::folder);

        /*button : SETTING*/
        ImGui::SameLine();
        cursor_pos = ImGui::GetCursorScreenPos();
        if (ImGui::Button("##setting", ImVec2(UI::standard_button_size.x / 2, UI::standard_button_size.y))) UI::section_index = SETTING_WINDOW;
        asthetic::create_half_button(cursor_pos, textures::setting);

        ImGui::PopFont();
        ImGui::EndChild();
        ImGui::SameLine();

        /*child : MAIN MENU*/
        ImGui::BeginChild("features", ImVec2(UI::width_window - 200.0f, UI::height_window), false, ImGuiWindowFlags_AlwaysVerticalScrollbar);

        /*spacing*/
        asthetic::top_spacing();

        /*text : I      N      S      A      N      E*/
        //ImGui::SetCursorPosX((ImGui::GetContentRegionAvail().x - UI::top_text_width) / 2);
        ImGui::PushFont(fonts::haas_black);
        ImGui::Text(UI::heading);
        ImGui::PopFont();
        ImGui::SameLine();

        /*button : CLOSE*/
        //ImGui::SetCursorScreenPos(ImVec2(window_pos.x + UI::width_window - UI::close_button_size.x, window_pos.y + 0.0f));
        ImGui::PushFont(fonts::adobe_clean_bold);
        if (ImGui::Button("x", UI::close_button_size)) UI::shutdown_UI = true;
        ImGui::PopFont();

        /*RENDERING CHEAT FEATURE WINDOWS HERE*/
        //UI::content_start_point = ImGui::GetCursorScreenPos();
        //draw_list->AddRect(cursor_pos, ImVec2(cursor_pos.x + ImGui::GetContentRegionAvail().x, cursor_pos.y + ImGui::GetContentRegionAvail().y), ImColor(255, 255, 255, 255));

        ImGui::PushFont(directX::fonts::agency_FB_small); // <- change this to change UI font :)
        switch (UI::section_index)
        {
        case QUOTE_WINDOW:
            ImGui::PushFont(fonts::adobe_clean_bold);
            cheat_window::draw_quote_window();
            ImGui::PopFont();
            break;
        case MISCELLANEOUS_WINDOW:
            asthetic::top_spacing(2);
            cheat_window::draw_miscellaneous_window();
            break;
        case PLAYER_VISUALS_WINDOW:
            cheat_window::draw_player_visual_window();
            break;
        case AIMBOT_WINDOW:
            cheat_window::draw_aimbot_window();
            break;
        case VIEW_VISUALS_WINDOW:
            cheat_window::draw_view_visual_window();
            break;
        case CONFIG_WINDOW:
            cheat_window::draw_config_window();
            break;
        default: break;
        }
        ImGui::PopFont();

        ImGui::EndChild();
        ImGui::PopStyleColor();

        /*Poping fonts*/
        ImGui::PopFont();

        /* managing shutdown animation */
        if (UI::shutdown_UI && !UI::animation_done)
        {
            if (!UI::time_noted)
            {
                UI::shutdown_anim_start_time = std::chrono::high_resolution_clock::now();
                UI::time_noted = true;
            }

            ImDrawList* bg_draw_list = ImGui::GetForegroundDrawList();
            bg_draw_list->AddRectFilled(ImVec2(0.0f, 0.0f), io.DisplaySize, ImColor(0, 0, 0, 255));

            ImGui::PushFont(UI::shutdown_anim_font);
            #ifdef _DEBUG
            if (!UI::shutdown_anim_font)
            {
                printf("font is null\n");
            }
            #endif
            bg_draw_list->AddText(ImVec2((io.DisplaySize.x - UI::end_meassage_size.x) / 2.0f, (io.DisplaySize.y - UI::end_meassage_size.y) / 2.0f), ImColor(255, 255, 255, 255), UI::end_message);
            ImGui::PopFont();

            if (std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - UI::shutdown_anim_start_time).count() >= UI::shutdown_anim_duration)
            {
                UI::animation_done = true;

                #ifdef _DEBUG
                cons.Log(FG_GREEN, "termination", "Shutdown Animation Done");
                #endif
            }
        }

        /* ending main UI window */
        ImGui::End();
    }
    else
    {
        time_refreshed = false;
    }

    /* Frame end */
    ImGui::EndFrame();
    ImGui::Render();
    ImGui_ImplDX9_RenderDrawData(ImGui::GetDrawData());

    if (UI::UI_visble)
    {
        /* managing shutdone once the animation is done */
        if (UI::shutdown_UI && !UI::UI_has_been_shutdown && UI::animation_done)
        {
            shutdown_imgui();
        }
    }

    /* calling original function */
	return O_endscene(P_DEVICE);
}


void directX::draw_background()
{
    ImDrawList* draw_list = ImGui::GetBackgroundDrawList();
    
    /* getting time difference in milliseconds */
    UI::cur_BG_alpha = sqrt((std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - menu_visible_time).count())/5.0f);
    UI::cur_BG_alpha = UI::cur_BG_alpha * 12;
    if (UI::cur_BG_alpha > UI::max_BG_opacity) {
        UI::cur_BG_alpha = UI::max_BG_opacity;
    }

    time_t nowtime = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
    tm* system_time = localtime(&nowtime);

    ImGui::PushFont(fonts::kabel);

    /*making clock and making numbers of constant width*/
    std::string display_time_string =
        (system_time->tm_hour > 12 ? std::to_string(system_time->tm_hour - 12) : std::to_string(system_time->tm_hour)) + ":" +
        (system_time->tm_min < 10 ? "0" : "") + std::to_string(system_time->tm_min) + ":" +
        std::to_string(system_time->tm_sec) + " " +
        (system_time->tm_hour > 12 ? "PM" : "AM");

    const char* display_time = display_time_string.c_str();
    ImVec2 display_time_size = ImGui::CalcTextSize(display_time);

    draw_list->AddRectFilled(ImVec2(0, 0), ImVec2(1920, 1080), IM_COL32(0, 0, 0, UI::cur_BG_alpha));
    draw_list->AddText(ImVec2((IO->DisplaySize.x - display_time_size.x) / 2, 0.02f * IO->DisplaySize.y), IM_COL32(255, 255, 255, UI::cur_BG_alpha), display_time);

    ImGui::PopFont();
    /* This is the image background */
    //draw_list->AddImage((ImTextureID)textures::background.texture, ImVec2(0, 0), ImVec2(1920, 1080), ImVec2(0,0), ImVec2(1,1), IM_COL32(255, 255, 255, UI::cur_BG_alpha));
}