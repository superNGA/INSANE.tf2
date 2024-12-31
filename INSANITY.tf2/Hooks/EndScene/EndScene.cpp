#include "EndScene.h"

//stb image
#define STB_IMAGE_IMPLEMENTATION // required for this library, I dont know what it does
#include "../../External Libraries/stb image/stb_image.h"

/*================ Giving namespace variables default value here ========================*/
namespace directX {
    namespace UI
    {
        int height_window = 600;
        int width_window = 475;

        bool UI_initialized_DX9 = false;
        bool shutdown_UI = false;
        bool UI_has_been_shutdown = false;
        bool UI_visble = true;
        bool WIN32_initialized = false;

        int top_text_width = 0;
        bool static_calc_done = false;
        const char* heading = "insanity";
    };

    namespace textures
    {
        bool are_textures_initialized = false;

        texture_data logo(nullptr, "Logo", 0, 0, 1,0.2f);
    };

    namespace fonts
    {
        ImFont* roboto = nullptr;
        ImFont* agency_FB = nullptr;
        bool fonts_initialized = false;
    };

    ImGuiContext* context = nullptr;
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

    //skipping rendering if menu not visible
    if (!UI::UI_visble || UI::UI_has_been_shutdown)
    {
        return O_endscene(P_DEVICE);
    }

    /* Starting ImGui new frame*/
    ImGuiIO& io = ImGui::GetIO();
    io.DisplaySize = ImVec2(1920.0f, 1080.0f); // Replace with actual screen resolution
    ImGui_ImplDX9_NewFrame();
    ImGui::NewFrame();

    /*the sheer fact that I have to make something like this
    tells that my IQ is questionable, but I do this before initialization
    it causes crash*/
    if (!UI::static_calc_done)
    {
        do_static_calc();

        #ifdef _DEBUG
        cons.Log("done Static calculations", FG_GREEN);
        #endif // _DEBUG
    }

    /*Pushing font*/
    ImGui::PushFont(fonts::agency_FB);

    /* Initializing window*/
    ImGui::SetNextWindowSize(ImVec2(UI::width_window, UI::height_window)); // update the window size at the end ( if needed )
    ImGui::Begin("INSANITY", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiTableColumnFlags_NoResize);

    /* adding logo */
    //ImVec2 logo_size(textures::logo.rendering_image_width , textures::logo.rendering_image_height);
    //ImGui::Image((ImTextureID)textures::logo.texture, logo_size); // Rendering image
    ImGui::SetCursorPosX((UI::width_window - UI::top_text_width) / 2);
    ImGui::Text(UI::heading);

    ImGui::Separator();

    /*Poping fonts*/
    ImGui::PopFont();

    /* Ending ImGui */
    ImGui::End();
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


void directX::initialize_image_texture()
{
    /* logo */
    textures::logo.texture = load_texture_from_image_data(resource::logo, textures::logo);
    textures::logo.update_res();
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
    memcpy(rect.pBits, decoded_data, texture_object.image_width * texture_object.image_height * 4); // RGBA = 4 bytes per pixel
    texture->UnlockRect(0);

    // Free the decoded data
    stbi_image_free(decoded_data);

    #ifdef _DEBUG
    printf("Successfuly loaded texture : %s\n", texture_object.name);
    printf("size : %d x %d\n", texture_object.image_width, texture_object.image_height);
    printf("byte array size : %lld\n", image_data.image_bytearray_size);
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
    //fonts::roboto = io.Fonts->AddFontFromMemoryTTF(resource::roboto_data, resource::roboto.image_bytearray_size, 20.0f, &font_config);
    fonts::agency_FB = io.Fonts->AddFontFromMemoryTTF(resource::agency_FB, resource::agencyFB.image_bytearray_size, 50.0f, &font_config);

    #ifdef _DEBUG
    if (!fonts::agency_FB)
    {
        cons.Log("Failed to load font", FG_RED);
    }
    else
    {
        cons.Log("Font data is valid", FG_GREEN);
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
    UI::top_text_width = ImGui::CalcTextSize(UI::heading).x;

    UI::static_calc_done = true;
}