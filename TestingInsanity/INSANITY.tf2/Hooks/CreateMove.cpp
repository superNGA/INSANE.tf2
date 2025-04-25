#include <intrin.h>

//======================= Internal stuff =======================
#include "../SDK/Entity Manager/entityManager.h"
#include "../SDK/FN index Manager/FN index manager.h"
#include "../Extra/math.h"
#include "../Utility/signatures.h"
#include "../Utility/Hook_t.h"

//======================= Features =======================
#include "../Features/Movement/Movement.h"
#include "../Features/NoSpread/NoSpread.h"
#include "../Features/Anti Aim/AntiAim.h"
#include "../Features/Fake Lag/FakeLag.h"

//======================= SDK =======================
#include "../SDK/class/CUserCmd.h"
#include "../SDK/class/BaseWeapon.h"


// Althoght inlining this function shouldn't cause any problems and since this functoin also dones't have any variables
// it should work fine
//__forceinline bool* GetbSendPacket(uint32_t iOffset)
//{
//	return reinterpret_cast<bool*>((uintptr_t)_AddressOfReturnAddress + iOffset);
//}


MAKE_HOOK(CreateMove, "40 53 48 83 EC ? 0F 29 74 24 ? 49 8B D8", __fastcall, CLIENT_DLL, bool,
	int64_t a1, int64_t a2, CUserCmd* cmd)
{
	bool result = Hook::CreateMove::O_CreateMove(a1, a2, cmd);

	if (!cmd || !cmd->command_number || entityManager.initialized.load() == false) {
		return result;
	}

	BaseEntity* pLocalPlayer = entityManager.getLocalPlayer();
	baseWeapon* pActiveWeapon = pLocalPlayer->getActiveWeapon();

	if (pLocalPlayer == nullptr || pActiveWeapon == nullptr)
		return result;

	// --> bSendPacket <--
	// Trying to get bSendPacket the proper way :) // 0x128 bytes moved
	uintptr_t pStackFrameStart_ClientModeShared_Createmove = (uintptr_t)_AddressOfReturnAddress();
	pStackFrameStart_ClientModeShared_Createmove += 0x8; // compensating for the adrs of start adrs
	
	// CInput createmove's stack
	pStackFrameStart_ClientModeShared_Createmove += 0xB0; // 0xB0 bytes for local variables allocated on stack
	pStackFrameStart_ClientModeShared_Createmove += 0x8;  // r15
	pStackFrameStart_ClientModeShared_Createmove += 0x8;  // r14
	pStackFrameStart_ClientModeShared_Createmove += 0x8;  // r12
	pStackFrameStart_ClientModeShared_Createmove += 0x8;  // rdi
	pStackFrameStart_ClientModeShared_Createmove += 0x8;  // rbp
	
	// IBaseClientDLL Createmove
	pStackFrameStart_ClientModeShared_Createmove += 0x8;  // adrs of return adrs
	pStackFrameStart_ClientModeShared_Createmove += 0x40; // 0x40 bytes for local variables allocated on stack

	bool* bSendPacket = reinterpret_cast<bool*>(pStackFrameStart_ClientModeShared_Createmove);

	static uint8_t bit_flags = 0;
	
	Features::movement.Run(cmd, result);

	Features::fakeLag.Run(bSendPacket, cmd);

	Features::antiAim.Run(cmd, result, bSendPacket);

	Features::noSpread.Run(cmd, result); // incomplete, not working

	return result;
}