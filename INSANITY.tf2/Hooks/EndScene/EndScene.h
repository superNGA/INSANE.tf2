#pragma once
#include <d3d9.h>
#include <d3dx9.h>

namespace directX
{
	typedef HRESULT(APIENTRY* T_endscene)(LPDIRECT3DDEVICE9);
	inline T_endscene O_endscene = nullptr;
	inline LPDIRECT3DDEVICE9 device = nullptr;

	HRESULT H_endscene(LPDIRECT3DDEVICE9 P_DEVICE);
	inline void DRAW_RECTANGLE(D3DCOLOR color);
};