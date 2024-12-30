#pragma once
#include <d3d9.h>
#include <d3dx9.h>

//global vars
#include "../../GlobalVars.h"

//ImGui
#include "../../External Libraries/ImGui/imgui.h"
#include "../../External Libraries/ImGui/imgui_impl_dx9.h"
#include "../../External Libraries/ImGui/imgui_impl_win32.h"


#ifdef _DEBUG
// console system
#include "../../Libraries/Console System/Console_System.h"
extern Console_System cons;
#endif // _DEBUG

namespace directX
{
	namespace UI
	{
		extern bool UI_initialized_DX9;
		extern bool shutdown_UI;
		extern bool UI_has_been_shutdown;
		extern bool UI_visble;
		extern bool WIN32_initialized;

		inline void draw_UI();
	};

	typedef HRESULT(APIENTRY* T_endscene)(LPDIRECT3DDEVICE9);
	inline T_endscene O_endscene = nullptr;
	inline LPDIRECT3DDEVICE9 device = nullptr;

	HRESULT H_endscene(LPDIRECT3DDEVICE9 P_DEVICE);
};