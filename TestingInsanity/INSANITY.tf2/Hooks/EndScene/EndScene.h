#pragma once
#define _CRT_SECURE_NO_WARNINGS
#include <chrono>
#include <d3d9.h>
#include <d3dx9.h>

//ImGui
#include "../../External Libraries/ImGui/imgui.h"
#include "../../External Libraries/ImGui/imgui_impl_dx9.h"
#include "../../External Libraries/ImGui/imgui_impl_win32.h"

//stb image
#include "../../External Libraries/stb image/stb_image.h"

namespace directX
{
    /*These prevent my wonderfull cheat from crashing*/
    namespace UI
    {
        extern bool UI_initialized_DX9;
        extern bool shutdown_UI;
        extern bool UI_has_been_shutdown;
        extern bool UI_visble;
        extern bool WIN32_initialized;
    };

    /* ImGui vars global */
    extern ImGuiIO* IO;
    extern ImGuiContext* context;
    
    // EndScene Hook stuff...
    inline LPDIRECT3DDEVICE9 device = nullptr;

    // IDirect3DDevice9::EndScene
    typedef HRESULT(APIENTRY* T_endscene)(LPDIRECT3DDEVICE9);
    inline T_endscene O_endscene = nullptr;
    HRESULT H_endscene(LPDIRECT3DDEVICE9 P_DEVICE);

    // IDirect3DDevice9::Present
    typedef HRESULT(APIENTRY* T_Present)(LPDIRECT3DDEVICE9, void*, void*, void*, void*);
    inline T_Present O_present = nullptr;
    HRESULT H_present(LPDIRECT3DDEVICE9 pDevice, void* a1, void* a2, void* a3, void* a4);

    // IDirect3DDevice9::StretchRect
    typedef HRESULT(APIENTRY* T_StretchRect)(LPDIRECT3DDEVICE9, void*, void*, void*, void*, int);
    inline T_StretchRect O_stretchRect = nullptr;
    HRESULT H_StretchRect(LPDIRECT3DDEVICE9 pDevice, IDirect3DSurface9* pSrcSurface, void* pSrcRect, IDirect3DSurface9* pDestSurface, void* pDestRect, int StretchRectType);

    void initialize_backends();
    void shutdown_imgui();
};

inline bool IsMenuOpen() { return directX::UI::UI_visble; }