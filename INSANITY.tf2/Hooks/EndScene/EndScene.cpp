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
        const char* heading = "insanity";

        bool done_styling = false;

        ImVec4 left_menu(0, 1, 3, 210);
        ImVec4 rigth_menu(10, 10, 10, 255);

        ImVec2 padding_001(5.0f, 5.0f);
        ImVec2 standard_button_size(200.0f, 40.0f);
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
    };

    namespace fonts
    {
        ImFont* roboto = nullptr;
        ImFont* agency_FB = nullptr;
        ImFont* adobe_clean_bold = nullptr;
        bool fonts_initialized = false;
    };

    ImGuiContext* context = nullptr;
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
        return O_endscene(P_DEVICE);
    }
    #pragma endregion

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
        IM_COL32(2, 2, 2, 240),  
        10.0f,                   
        ImDrawFlags_RoundCornersBottomLeft | ImDrawFlags_RoundCornersTopLeft);

    /* right side menu rectangle */
    draw_list->AddRectFilled(
        ImVec2(window_pos.x + 200.0f, window_pos.y),
        ImVec2(window_pos.x + UI::width_window, window_pos.y + UI::height_window),
        IM_COL32(10, 10, 10, 255),
        10.0f,
        ImDrawFlags_RoundCornersTopRight | ImDrawFlags_RoundCornersBottomRight);

    /*making sections*/
    //ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(1, 1, 1, 1));
    ImGui::BeginChild("sections", ImVec2(200.0f, UI::height_window), true);

    ImGui::PushFont(fonts::agency_FB);

    asthetic::top_spacing(12);

    /*cursor pos*/
    ImVec2 cursor_pos = ImGui::GetCursorScreenPos(); // Get current cursor position
    /* creating button*/
    if (ImGui::Button("##aimbot", UI::standard_button_size)) {
        std::cout << "Aimbot button clicked!" << std::endl;
    }
    asthetic::create_button_with_image(cursor_pos, textures::aimbot, "aimbot", true);

    /*cursor pos*/
    cursor_pos = ImGui::GetCursorScreenPos(); // Get current cursor position
    /* creating button*/
    if (ImGui::Button("##antiaim", UI::standard_button_size)) {
        std::cout << "anti aim button clicked!" << std::endl;
    }
    asthetic::create_button_with_image(cursor_pos, textures::stars, "anti-aim", true);
    asthetic::top_spacing(2);

    /*cursor pos*/
    cursor_pos = ImGui::GetCursorScreenPos(); // Get current cursor 
    /* creating button*/
    if (ImGui::Button("##world", UI::standard_button_size)) {
        std::cout << "world button clicked!" << std::endl;
    }
    asthetic::create_button_with_image(cursor_pos, textures::planet, "world", true);

    /*cursor pos*/
    cursor_pos = ImGui::GetCursorScreenPos(); // Get current cursor position
    /* creating button*/
    if (ImGui::Button("##player", UI::standard_button_size)) {
        std::cout << "player button clicked!" << std::endl;
    }
    asthetic::create_button_with_image(cursor_pos, textures::player, "player", true);

    /*cursor pos*/
    cursor_pos = ImGui::GetCursorScreenPos(); // Get current cursor position
    /* creating button*/
    if (ImGui::Button("##view", UI::standard_button_size)) {
        std::cout << "view button clicked!" << std::endl;
    }
    asthetic::create_button_with_image(cursor_pos, textures::view, "view", true);
    asthetic::top_spacing(2);

    /*cursor pos*/
    cursor_pos = ImGui::GetCursorScreenPos(); // Get current cursor position
    /* creating button*/
    if (ImGui::Button("##misc", UI::standard_button_size)) {
        std::cout << "misc button clicked!" << std::endl;
    }
    asthetic::create_button_with_image(cursor_pos, textures::stars, "misc", true);

    /*cursor pos*/
    cursor_pos = ImGui::GetCursorScreenPos(); // Get current cursor position
    /* creating button*/
    if (ImGui::Button("##skins", UI::standard_button_size)) {
        std::cout << "skins button clicked!" << std::endl;
    }
    asthetic::create_button_with_image(cursor_pos, textures::left_wing, "skins", true);


    ImGui::PopFont();
    ImGui::EndChild();
        
    ImGui::SameLine();
    //ImGui::PopStyleColor();
    //ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0,0,0, 1));
    ImGui::BeginChild("features", ImVec2(UI::width_window - 200.0f, UI::height_window), true);

    ImGui::SetCursorPosX((ImGui::GetContentRegionAvail().x - UI::top_text_width) / 2);
    ImGui::Text("insanity");

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


