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

/*
"What would have INSANE done in this situation?" -> Thats what I am gonna do!
*/

/*
I am confused. very confused. I don't know what to do! Some part says make new feature, some part says, 
Fix current bugs and faults. Some part says make loader, some says improve UI. I don't understand what to do.
Maybe if I make a fixed plan, then I will see a clear way.
Lets try that out.
-> first I shall make difficult features, get them half working!
-> then go back and work on UI.
-> then complete each feature completly & add them to the UI along side.
-> when cheat is done, then we can make the loader.

How does that sound? nice? well it is quite actually. But it requires some grit to stick to a plan.
and grit is often flushed down the toilet by weak people like me ( yes, I am talking about nofap ).
I lack grit, I get attracted towards shinny things too much, I see new shit, I go for new shit leaving
old one in dirt. But that won't cut it. I must stick to this plan and I think I won't bother giving
myself deadlines, cause this is the experimental phase, and then the real cheat dev, mannual labor phase
comes.

- INSANE
*/

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
	
	Features::movement.Run(cmd, result);

	Features::fakeLag.Run(bSendPacket, cmd);

	Features::antiAim.Run(cmd, result, bSendPacket);

	Features::noSpread.Run(cmd, result); // incomplete, not working

	Features::critHack.Run(cmd, pActiveWeapon, pLocalPlayer);

	return result;
}