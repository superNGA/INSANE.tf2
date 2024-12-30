#include "EndScene.h"

//stb image
#define STB_IMAGE_IMPLEMENTATION // required for this library, I dont know what it does
#include "../../External Libraries/stb image/stb_image.h"

/*================ Giving namespace variables default value here ========================*/
namespace directX {
    namespace UI
    {
        int height_window = 700;
        int width_window = 400;

        bool UI_initialized_DX9 = false;
        bool shutdown_UI = false;
        bool UI_has_been_shutdown = false;
        bool UI_visble = true;
        bool WIN32_initialized = false;
    };

    namespace textures
    {
        bool are_textures_initialized = false;

        texture_data logo = { nullptr, 0,0 ,"logo text" };
    };
};


/*========================= Initializing functions here =======================================*/

HRESULT directX::H_endscene(LPDIRECT3DDEVICE9 P_DEVICE)
{
	if (!device)
	{
		device = P_DEVICE;
	}

    // Initializin DX9 imgui
    static ImGuiContext* context;
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

    // Initializing textures
    if (!textures::are_textures_initialized)
    {
        initialize_image_texture();
        textures::are_textures_initialized = true;
    }

    //skipping rendering if menu not visible
    if (!UI::UI_visble || UI::UI_has_been_shutdown)
    {
        return O_endscene(P_DEVICE);
    }

    ImGuiIO& io = ImGui::GetIO();
    io.DisplaySize = ImVec2(1920.0f, 1080.0f); // Replace with actual screen resolution
    ImGui_ImplDX9_NewFrame();
    ImGui::NewFrame();

    ImGui::SetNextWindowSize(ImVec2(UI::height_window, UI::width_window));
    ImGui::Begin("INSANITY", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse);

    ImVec2 logo_size(200, (int)((200.0f / (float)textures::logo.image_height) * (float)textures::logo.image_width));
    ImGui::Image((ImTextureID)textures::logo.texture, logo_size);
    ImGui::Text("%d", (int)((700.0f / (float)textures::logo.image_height) * (float)textures::logo.image_width));
    ImGui::End();

    ImGui::EndFrame();
    ImGui::Render();
    ImGui_ImplDX9_RenderDrawData(ImGui::GetDrawData());

    // Shuting down ImGui backends
    if (UI::shutdown_UI && !UI::UI_has_been_shutdown)
    {

        #ifdef _DEBUG
        if (ImGui::GetCurrentContext() != context)
        {
            cons.Log("[ Error ] current context is null before destroying it", FG_RED);  
        }
        #endif
        if(UI::UI_initialized_DX9) ImGui_ImplDX9_Shutdown();
        if(UI::WIN32_initialized) ImGui_ImplWin32_Shutdown();
        ImGui::DestroyContext();
        UI::shutdown_UI = false;
        UI::UI_has_been_shutdown = true;
        
        #ifdef _DEBUG
        cons.Log("ImGui has been shutdown", FG_RED);
        #endif // _DEBUG

    }

	return O_endscene(P_DEVICE);
}


void directX::initialize_image_texture()
{
    textures::logo.texture = load_texture_from_image_data(resource::logo, textures::logo);
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
    printf("Successfuly loaded tecture : %s\n", texture_object.name);
    printf("size : %d x %d\n", texture_object.image_width, texture_object.image_height);
    printf("byte array size : %lld\n", image_data.image_bytearray_size);
    #endif // _DEBUG


    return (void*)texture;
}