#define _CRT_SECURE_NO_WARNINGS
#include <format>
#include "EndScene.h"
#include "cheatFeaturesRendering.h"
#include "Cheat windows/cheat_window.h"

#include "../../Features/Graphics Engine/Graphics Engine/GraphicsEngine.h"
#include "../../Features/ImGui/InfoWindow/InfoWindow_t.h"
#include "../../Features/ImGui/Menu/Menu.h"
#include "../../Utility/Insane Profiler/InsaneProfiler.h"


HRESULT directX::H_BeginScene(LPDIRECT3DDEVICE9 P_DEVICE)
{
    return directX::O_BeginScene(P_DEVICE);
}


/*================ Giving namespace variables default value here ========================*/
namespace directX {
    namespace UI
    {
        //cur_window section_index = QUOTE_WINDOW;

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

    //namespace textures
    //{
    //    bool are_textures_initialized = false;

    //    texture_data logo(nullptr, "Logo", 0, 0, 1,0.2f);
    //    texture_data aimbot(nullptr, "aimbot scope");
    //    texture_data folder(nullptr, "config folder");
    //    texture_data left_wing(nullptr, "left wing image");
    //    texture_data right_wing(nullptr, "right wing image");
    //    texture_data planet(nullptr, "planet");
    //    texture_data player(nullptr, "player");
    //    texture_data setting(nullptr, "setting");
    //    texture_data stars(nullptr, "stars");
    //    texture_data view(nullptr, "view");
    //    texture_data misc(nullptr, "miscellaneous");
    //    texture_data antiaim(nullptr, "anti aim");

    //    /* background */
    //    texture_data background(nullptr, "background image");
    //};

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
    /*if (!textures::are_textures_initialized)
    {
        initialize_image_texture();
        textures::are_textures_initialized = true;

        #ifdef _DEBUG
        WIN_LOG(" All textures initialized ");
        #endif
    }*/

    /* Initializing fonts */
    if (!fonts::fonts_initialized && UI::UI_initialized_DX9 && UI::WIN32_initialized)
    {
        load_all_fonts();
    }

    /* skipping all if already ended */
    if (UI::UI_has_been_shutdown) return O_endscene(P_DEVICE);

    /* Starting ImGui new frame*/
    ImGuiIO& io = ImGui::GetIO();
    io.DisplaySize = ImVec2(1920.0f, 1080.0f); // Replace with actual screen resolution
    ImGui_ImplWin32_NewFrame();
    ImGui_ImplDX9_NewFrame();
    ImGui::NewFrame();

    F::graphicsEngine.Run(P_DEVICE);
    Render::InfoWindow.Draw();
    Render::uiMenu.Draw();
    insaneProfiler.Render();

    /* Frame end */
    ImGui::EndFrame();
    ImGui::Render();
    ImGui_ImplDX9_RenderDrawData(ImGui::GetDrawData());

    if (UI::UI_visble)
    {
        /* managing shutdone once the animation is done */
        if (UI::shutdown_UI && !UI::UI_has_been_shutdown)
        {
            shutdown_imgui();
        }
    }

    /* calling original function */
	auto result = O_endscene(P_DEVICE);

    return result;
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