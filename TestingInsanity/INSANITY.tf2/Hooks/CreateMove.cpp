#include <intrin.h>

//======================= Internal stuff =======================
#include "../SDK/Entity Manager/entityManager.h"
#include "../SDK/FN index Manager/FN index manager.h"
#include "../Extra/math.h"
#include "../Utility/signatures.h"
#include "../Utility/Hook_t.h"
#include "../Utility/ConsoleLogging.h"

//======================= Features =======================
#include "../Features/Movement/Movement.h"
#include "../Features/NoSpread/NoSpread.h"
#include "../Features/Anti Aim/AntiAim.h"
#include "../Features/Fake Lag/FakeLag.h"
#include "../Features/CritHack/CritHack.h"

//======================= SDK =======================
#include "../SDK/class/CUserCmd.h"
#include "../SDK/class/BaseWeapon.h"
#include "../SDK/class/Source Entity.h"


MAKE_HOOK(CreateMove, "40 53 48 83 EC ? 0F 29 74 24 ? 49 8B D8", __fastcall, CLIENT_DLL, bool,
	int64_t a1, int64_t a2, CUserCmd* cmd)
{
	bool result = Hook::CreateMove::O_CreateMove(a1, a2, cmd);

	if ( cmd == nullptr || cmd->command_number == 0)
		return result;

	// Getting Local Player
	BaseEntity* pLocalPlayer = I::IClientEntityList->GetClientEntity(I::iEngine->GetLocalPlayer());
	if (pLocalPlayer == nullptr)
		return result;

	// Getting Active Weapon
	baseWeapon* pActiveWeapon = pLocalPlayer->getActiveWeapon();
	if (pActiveWeapon == nullptr)
		return result;

	// are we alive ?
	if (pLocalPlayer->getLifeState() != lifeState_t::LIFE_ALIVE)
		return result;

	// --> bSendPacket <--
	/*
	We moved 0x128 bytes down in the stack frame to find the bSendPacket 
	that was pushed onto the stack by IBaseClientDLL Createmove
	*/
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
	
	Features::movement.Run(cmd, result, pLocalPlayer, pActiveWeapon);

	Features::fakeLag.Run(bSendPacket, cmd);

	Features::antiAim.Run(cmd, result, bSendPacket, pLocalPlayer);

	Features::noSpread.Run(cmd, result); // incomplete, not working

	Features::critHack.RunV2(cmd, pLocalPlayer, pActiveWeapon);

	return result;
}