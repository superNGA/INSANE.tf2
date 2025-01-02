#include "EndScene.h"

//stb image
#define STB_IMAGE_IMPLEMENTATION // required for this library, I dont know what it does
#include "../../External Libraries/stb image/stb_image.h"

void directX::initialize_image_texture()
{
    /* logo */
    textures::logo.texture = load_texture_from_image_data(resource::logo, textures::logo);
    textures::logo.update_res();

    /* icons */
    textures::aimbot.texture = load_texture_from_image_data(resource::aimbot_data, textures::aimbot);
    textures::folder.texture = load_texture_from_image_data(resource::folder_data, textures::folder);
    textures::left_wing.texture = load_texture_from_image_data(resource::left_wing_data, textures::left_wing);
    textures::right_wing.texture = load_texture_from_image_data(resource::right_wing_data, textures::right_wing);
    textures::planet.texture = load_texture_from_image_data(resource::planet_data, textures::planet);
    textures::player.texture = load_texture_from_image_data(resource::player_data, textures::player);
    textures::setting.texture = load_texture_from_image_data(resource::setting_data, textures::setting);
    textures::stars.texture = load_texture_from_image_data(resource::stars_data, textures::stars);
    textures::view.texture = load_texture_from_image_data(resource::view_data, textures::view);

    /* background*/
    textures::background = load_texture_from_image_data(resource::background, textures::background);
}


void* directX::load_texture_from_image_data(raw_image_data& image_data, texture_data& texture_object)
{
    unsigned char* decoded_data = nullptr;
    int channels = 0;

    decoded_data = stbi_load_from_memory((stbi_uc*)image_data.image_bytes, image_data.image_bytearray_size, &texture_object.image_width, &texture_object.image_height, &channels, STBI_rgb_alpha);
    if (!decoded_data)
    {
        #ifdef _DEBUG
        cons.Log("E R R O R", FG_RED);
        printf("Load from memory failed : %s | resolution : %d x %d | size -> %zd\n", texture_object.name, texture_object.image_height, texture_object.image_width, image_data.image_bytearray_size);
        #endif
        return nullptr;
    }

    /*creating the texture*/
    IDirect3DTexture9* texture = nullptr;
    HRESULT hr = device->CreateTexture(
        texture_object.image_width,
        texture_object.image_height,
        1,
        D3DUSAGE_DYNAMIC,
        D3DFMT_A8R8G8B8,
        D3DPOOL_DEFAULT,
        &texture,
        nullptr
    );

    if (FAILED(hr)) {
        #ifdef _DEBUG
        cons.Log("Failed to create DirectX9 texture", FG_RED);
        #endif // _DEBUG

        stbi_image_free(decoded_data);
        return nullptr;
    }

    // Lock the texture to copy the pixel data
    D3DLOCKED_RECT rect;
    texture->LockRect(0, &rect, nullptr, D3DLOCK_DISCARD);
    unsigned char* dest = (unsigned char*)rect.pBits;
    unsigned char* src = decoded_data;
    for (int y = 0; y < texture_object.image_height; ++y) {
        memcpy(dest, src, texture_object.image_width * 4); // Copy one row
        dest += rect.Pitch;  // Move to the next row in the texture
        src += texture_object.image_width * 4; // Move to the next row in the source data
    }

    texture->UnlockRect(0);

    // Free the decoded data
    stbi_image_free(decoded_data);

    #ifdef _DEBUG
    printf("TEXTURE NAME : %s | ", texture_object.name);
    printf("RES. : %d x %d | ", texture_object.image_width, texture_object.image_height);
    printf("ARRAY SIZE : %zd | ", image_data.image_bytearray_size);
    printf("CHANNELS : %d\n", channels);
    #endif // _DEBUG


    return (void*)texture;
}


void directX::initialize_backends()
{
    // Initializin DX9 imgui
    if (!UI::UI_initialized_DX9)
    {
        context = ImGui::CreateContext();
        ImGui::SetCurrentContext(context);

        ImGui_ImplDX9_Init(device);
        ImGui_ImplDX9_CreateDeviceObjects();
        ImGuiIO& io = ImGui::GetIO();
        io.ConfigFlags = ImGuiConfigFlags_NoMouseCursorChange;

        UI::UI_initialized_DX9 = true;

    #ifdef _DEBUG
        cons.Log("initialized ImGui DX9", FG_GREEN);
    #endif // _DEBUG
    }

    // initializing WIN32 imgui
    if (!UI::WIN32_initialized)
    {
        if (global::target_hwnd != nullptr)
        {
            ImGui_ImplWin32_Init(global::target_hwnd);
            UI::WIN32_initialized = true;

            #ifdef _DEBUG
            cons.Log("initialized ImGui WIN32", FG_GREEN);
            #endif // _DEBUG
        }
        #ifdef _DEBUG
        else
        {
            cons.Log("No window handle yet", FG_YELLOW);
        }
        #endif // _DEBUG
    }

}


void directX::shutdown_imgui()
{
    // Shuting down ImGui backends
    #ifdef _DEBUG
    if (ImGui::GetCurrentContext() != context)
    {
        cons.Log("[ Error ] current context is null before destroying it", FG_RED);
    }
    #endif

    /*Shuting down backends*/
    if (UI::UI_initialized_DX9) ImGui_ImplDX9_Shutdown();
    if (UI::WIN32_initialized) ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();
    UI::shutdown_UI = false;
    UI::UI_has_been_shutdown = true;
    #ifdef _DEBUG
    cons.Log("ImGui has been shutdown", FG_RED);
    #endif // _DEBUG
}


void directX::load_all_fonts()
{
    /* withtout this ImGui won't load the font and everything will fuck up */
    ImFontConfig font_config;
    font_config.FontDataOwnedByAtlas = false;

    /* Now loading font into memory */
    ImGuiIO& io = ImGui::GetIO();

    fonts::roboto = io.Fonts->AddFontFromMemoryTTF(resource::roboto_data, resource::roboto.image_bytearray_size, 20.0f, &font_config);
    fonts::agency_FB = io.Fonts->AddFontFromMemoryTTF(resource::agency_FB, resource::agencyFB.image_bytearray_size, 50.0f, &font_config);
    fonts::agency_FB_small = io.Fonts->AddFontFromMemoryTTF(resource::agency_FB, resource::agencyFB.image_bytearray_size, 20.0f, &font_config);
    fonts::adobe_clean_bold = io.Fonts->AddFontFromMemoryTTF(resource::adobe_clean_bold_data, resource::adobe_clean_bold.image_bytearray_size, 30.0f, &font_config);
    fonts::haas_black = io.Fonts->AddFontFromMemoryTTF(resource::haas_black_data, resource::hass_black.image_bytearray_size, 40.0f, &font_config);

    #ifdef _DEBUG
    if (!fonts::agency_FB || !fonts::roboto || !fonts::adobe_clean_bold)
    {
        cons.Log("one or all of the font's data are fucked up", FG_RED);
    }
    else
    {
        cons.Log("All fonts have been verified", FG_GREEN);
    }
    #endif // _DEBUG

    fonts::fonts_initialized = true;

    ImGui_ImplDX9_InvalidateDeviceObjects();
    ImGui_ImplDX9_CreateDeviceObjects();

    #ifdef _DEBUG
    cons.Log("All fonts loaded", FG_GREEN);
    #endif
}


void directX::do_static_calc()
{
    ImGui::PushFont(fonts::haas_black);
    UI::top_text_width = ImGui::CalcTextSize(UI::heading).x;
    ImGui::PopFont();

    UI::static_calc_done = true;
}


void directX::make_it_pretty()
{
    ImGuiStyle& style = ImGui::GetStyle();

    /*do styling here and dont use magic numbers, use variables
    we will be giving user the accent color control*/
    style.WindowRounding = 10.0f;
    style.WindowPadding = ImVec2(0.0f, 0.0f);
    style.ItemSpacing = ImVec2(0.0f, 0.0f);
    style.WindowBorderSize = 0.0f;

    style.ChildRounding = 10.0f;
    style.ChildBorderSize = 0.0f;

    style.Colors[ImGuiCol_Button] = ImVec4(0.0f, 0.0f, 0.0f, 0.0f);
    style.Colors[ImGuiCol_ButtonHovered] = ImVec4(0.92f, 0.18f, 0.29f, 0.86f);

    UI::done_styling = true;

    #ifdef _DEBUG
    cons.Log("Done Styling menu", FG_GREEN);
    #endif // _DEBUG
}