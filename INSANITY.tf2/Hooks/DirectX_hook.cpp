#include "DirectX_hook.h"

//global pointers
void* DIRECTX_DEVICE_VTABLE[119]; //devices virtual table
int WINDOW_HEIGHT, WINDOW_WIDTH;
HWND WINDOW = nullptr;
LPDIRECT3DDEVICE9 DEVICE = nullptr;

//==============================END SCENE HOOK=============================================
//typedef HRESULT(APIENTRY* T_ENDSCENE)(LPDIRECT3DDEVICE9);
//T_ENDSCENE O_ENDSCENE = nullptr;
//HRESULT H_ENDSCENE(LPDIRECT3DDEVICE9 P_DEVICE)
//{
//	//storing the device pointer globally
//	if (!DEVICE) {
//		DEVICE = P_DEVICE;
//	}
//
//	DRAW_RECTANGLE(D3DCOLOR_ARGB(255, 255, 255, 255));
//
//	printf("Hooked endscene\n");
//	return O_ENDSCENE(DEVICE);
//}

void* get_endscene()
{
	if (GET_DIRECTX_DEVICES_VTABLE(DIRECTX_DEVICE_VTABLE, sizeof(DIRECTX_DEVICE_VTABLE))) {
		return DIRECTX_DEVICE_VTABLE[42];
	}
}

bool GET_DIRECTX_DEVICES_VTABLE(void** P_TABLE, size_t SIZE)
{
	if (!P_TABLE) {
		return false;
	}

	//getting the directx interface
	IDirect3D9* DX_INTERFACE = Direct3DCreate9(D3D_SDK_VERSION);
	if (!DX_INTERFACE) {
		return false;
	}

	IDirect3DDevice9* DUMMY_DEVICE = nullptr;
	D3DPRESENT_PARAMETERS PAR = {};
	PAR.Windowed = false;
	PAR.SwapEffect = D3DSWAPEFFECT_DISCARD;
	PAR.hDeviceWindow = GET_PROCESS_WINDOW();

	HRESULT DUMMY_DEVICE_CREATED = DX_INTERFACE->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, PAR.hDeviceWindow, D3DCREATE_SOFTWARE_VERTEXPROCESSING, &PAR, &DUMMY_DEVICE);
	//if failed to create device, we try using windowed mode
	if (DUMMY_DEVICE_CREATED != S_OK) {
		PAR.Windowed = !PAR.Windowed;
		HRESULT DUMMY_DEVICE_CREATED = DX_INTERFACE->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, PAR.hDeviceWindow, D3DCREATE_SOFTWARE_VERTEXPROCESSING, &PAR, &DUMMY_DEVICE);
		//if still failed to create a device, we quit.
		if (DUMMY_DEVICE_CREATED != S_OK) {
			DX_INTERFACE->Release();
			return false;
		}
	}

	//getting the virtual table form the dummy device
	memcpy(P_TABLE, *(void***)DUMMY_DEVICE, SIZE);
	DUMMY_DEVICE->Release();
	return true;
}

HWND GET_PROCESS_WINDOW()
{
	WINDOW = nullptr;
	EnumWindows(CHECK_WINDOW, NULL); //This enumerates through all the windows and calls a function where we can do the checking :)

	/*By now we have our window*/
	RECT WINDOW_SIZE;
	GetWindowRect(WINDOW, &WINDOW_SIZE);
	WINDOW_HEIGHT = WINDOW_SIZE.bottom - WINDOW_SIZE.top;
	WINDOW_WIDTH = WINDOW_SIZE.left - WINDOW_SIZE.right;

	return WINDOW;
}

BOOL CALLBACK CHECK_WINDOW(HWND CURRENT_WINDOW, LPARAM LP)
{
	DWORD CURRENT_PROCESS_ID;
	GetWindowThreadProcessId(CURRENT_WINDOW, &CURRENT_PROCESS_ID); //gets the current windows process ID and campares it against the target windows process ID
	if (GetCurrentProcessId() != CURRENT_PROCESS_ID) {
		return TRUE;
	}

	WINDOW = CURRENT_WINDOW;
	return FALSE;
}

//void DRAW_RECTANGLE(D3DCOLOR color)
//{
//	D3DRECT rect = { 100, 100, 200, 200 };
//	DEVICE->Clear(1, &rect, D3DCLEAR_TARGET, color, 0, 0);
//}