#include "EndScene.h"

/*================ Giving namespace variables default value here ========================*/
namespace directX {
    namespace UI
    {
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

        float max_BG_opacity = 150.0f;
        float cur_BG_alpha = 255.0f;
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
        bool fonts_initialized = false;
    };

    namespace colours
    {
        ImVec4 grey(20.0f/255.0f, 20.0f / 255.0f, 20.0f / 255.0f, 200.0f/255.0f);
        ImColor main_menu(7, 7, 7, 255);
        ImColor side_menu(2, 2, 2, 240);
    };

    ImGuiContext* context = nullptr;
    bool time_refreshed = false;
    std::chrono::time_point<std::chrono::high_resolution_clock> menu_visible_time = std::chrono::high_resolution_clock::now();
};

/*========================= Initializing functions here =======================================*/

HRESULT directX::H_endscene(LPDIRECT3DDEVICE9 P_DEVICE)
{
    #pragma region INITIALIZATION
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

    //skipping rendering if menu not visible
    if (!UI::UI_visble || UI::UI_has_been_shutdown)
    {
        time_refreshed = false;
        return O_endscene(P_DEVICE);
    }
    #pragma endregion

    /* refreshing time */
    if (!time_refreshed)
    {
        menu_visible_time = std::chrono::high_resolution_clock::now();
        time_refreshed = true;

        #ifdef _DEBUG
        cons.Log("refreshed start time", FG_GREEN);
        #endif
    }

    /* Starting ImGui new frame*/
    ImGuiIO& io = ImGui::GetIO();
    io.DisplaySize = ImVec2(1920.0f, 1080.0f); // Replace with actual screen resolution
    ImGui_ImplDX9_NewFrame();
    ImGui::NewFrame();

    /* Imgui maths */
    if (!UI::static_calc_done)
    {
        do_static_calc();

        #ifdef _DEBUG
        cons.Log("done Static calculations", FG_GREEN);
        #endif // _DEBUG
    }

    /* doing styling */
    if (!UI::done_styling)
    {
        make_it_pretty();
    }
    
    /*Pushing font*/
    ImGui::PushFont(fonts::agency_FB);

    /* Drawing background window*/
    draw_background();

    /* Initializing window*/
    ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.0f, 0.0f, 0.0f, 0.0f));
    ImGui::SetNextWindowSize(ImVec2(UI::width_window, UI::height_window));
    //ImGui::SetNextWindowSize(ImVec2(UI::width_window, UI::height_window));
    ImGui::SetNextWindowBgAlpha(UI::cur_BG_alpha * 2);
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

    /*making sections*/
    //ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(1, 1, 1, 1));
    ImGui::BeginChild("sections", ImVec2(200.0f, UI::height_window), true);

    ImGui::PushFont(fonts::roboto);

    asthetic::top_spacing(12);

    asthetic::left_spacing();
    ImGui::PushFont(fonts::agency_FB_small);
    ImGui::PushStyleColor(ImGuiCol_Text, colours::grey);
    ImGui::Text("Aim assit");
    ImGui::PopStyleColor();
    ImGui::PopFont();
    /*cursor pos*/
    ImVec2 cursor_pos = ImGui::GetCursorScreenPos(); // Get current cursor position
    /* creating button*/
    if (ImGui::Button("##aimbot", UI::standard_button_size)) {
        std::cout << "Aimbot button clicked!" << std::endl;
    }
    asthetic::create_button_with_image(cursor_pos, textures::aimbot, "AIMBOT", false);

    /*cursor pos*/
    cursor_pos = ImGui::GetCursorScreenPos(); // Get current cursor position
    /* creating button*/
    if (ImGui::Button("##antiaim", UI::standard_button_size)) {
        std::cout << "anti aim button clicked!" << std::endl;
    }
    asthetic::create_button_with_image(cursor_pos, textures::stars, "ANTI-AIM", false);
    asthetic::top_spacing(2);

    asthetic::left_spacing();
    ImGui::PushFont(fonts::agency_FB_small);
    ImGui::PushStyleColor(ImGuiCol_Text, colours::grey);
    ImGui::Text("Visuals");
    ImGui::PopStyleColor();
    ImGui::PopFont();
    /*cursor pos*/
    cursor_pos = ImGui::GetCursorScreenPos(); // Get current cursor 
    /* creating button*/
    if (ImGui::Button("##world", UI::standard_button_size)) {
        std::cout << "world button clicked!" << std::endl;
    }
    asthetic::create_button_with_image(cursor_pos, textures::planet, "WORLD", false);

    /*cursor pos*/
    cursor_pos = ImGui::GetCursorScreenPos(); // Get current cursor position
    /* creating button*/
    if (ImGui::Button("##player", UI::standard_button_size)) {
        std::cout << "player button clicked!" << std::endl;
    }
    asthetic::create_button_with_image(cursor_pos, textures::player, "PLAYER", false);

    /*cursor pos*/
    cursor_pos = ImGui::GetCursorScreenPos(); // Get current cursor position
    /* creating button*/
    if (ImGui::Button("##view", UI::standard_button_size)) {
        std::cout << "view button clicked!" << std::endl;
    }
    asthetic::create_button_with_image(cursor_pos, textures::view, "VIEW", false);
    asthetic::top_spacing(2);

    asthetic::left_spacing();
    ImGui::PushFont(fonts::agency_FB_small);
    ImGui::PushStyleColor(ImGuiCol_Text, colours::grey);
    ImGui::Text("Miscellaneous");
    ImGui::PopStyleColor();
    ImGui::PopFont();
    /*cursor pos*/
    cursor_pos = ImGui::GetCursorScreenPos(); // Get current cursor position
    /* creating button*/
    if (ImGui::Button("##misc", UI::standard_button_size)) {
        std::cout << "misc button clicked!" << std::endl;
    }
    asthetic::create_button_with_image(cursor_pos, textures::stars, "MISC", false);

    /*cursor pos*/
    cursor_pos = ImGui::GetCursorScreenPos(); // Get current cursor position
    /* creating button*/
    if (ImGui::Button("##skins", UI::standard_button_size)) {
        std::cout << "skins button clicked!" << std::endl;
    }
    asthetic::create_button_with_image(cursor_pos, textures::left_wing, "SKINS", false);


    ImGui::PopFont();
    ImGui::EndChild();
        
    ImGui::SameLine();
    //ImGui::PopStyleColor();
    //ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0,0,0, 1));
    ImGui::BeginChild("features", ImVec2(UI::width_window - 200.0f, UI::height_window), false);

    ImGui::SetCursorPosX((ImGui::GetContentRegionAvail().x - UI::top_text_width) / 2);
    ImGui::PushFont(fonts::haas_black);
    ImGui::Text(UI::heading);
    ImGui::PopFont();

    /* testing all images */
    asthetic::top_spacing(2);

    ImGui::EndChild();
    ImGui::PopStyleColor();

    /*Poping fonts*/
    ImGui::PopFont();

    /* Ending ImGui */
    ImGui::End();

    /* this pop style belongs to main window styling */
    //ImGui::PopStyleColor();

    ImGui::EndFrame();
    ImGui::Render();

    ImGui_ImplDX9_RenderDrawData(ImGui::GetDrawData());

    /*Shuting down ImGui on command*/
    if (UI::shutdown_UI && !UI::UI_has_been_shutdown)
    {
        shutdown_imgui();
    }

	return O_endscene(P_DEVICE);
}


void directX::draw_background()
{
    ImDrawList* draw_list = ImGui::GetBackgroundDrawList();
    
    /* getting time difference in milliseconds*/
    UI::cur_BG_alpha = sqrt((std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - menu_visible_time).count())/5.0f);
    UI::cur_BG_alpha = UI::cur_BG_alpha * 12;
    if (UI::cur_BG_alpha > UI::max_BG_opacity) {
        UI::cur_BG_alpha = UI::max_BG_opacity;
    }

    draw_list->AddImage((ImTextureID)textures::background.texture, ImVec2(0, 0), ImVec2(1920, 1080), ImVec2(0,0), ImVec2(1,1), IM_COL32(255, 255, 255, UI::cur_BG_alpha));
}