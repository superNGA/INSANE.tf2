#include "EndScene.h"

inline void directX::DRAW_RECTANGLE(D3DCOLOR color)
{
	D3DRECT rect = { 100, 100, 200, 200 };
	directX::device->Clear(1, &rect, D3DCLEAR_TARGET, color, 0, 0);
}

HRESULT directX::H_endscene(LPDIRECT3DDEVICE9 P_DEVICE)
{
	if (!device)
	{
		device = P_DEVICE;
	}

	DRAW_RECTANGLE(D3DCOLOR_ARGB(255, 255, 255, 255));

	return O_endscene(P_DEVICE);
}