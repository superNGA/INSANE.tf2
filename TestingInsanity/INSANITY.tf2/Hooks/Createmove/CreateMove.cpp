#include "CreateMove.h"
#include "../../SDK/Entity Manager/entityManager.h"
#include "../../SDK/FN index Manager/FN index manager.h"
#include "../../Extra/math.h"
#include "../../Features/NoSpread/NoSpread.h"
#include "../../Features/Anti Aim/AntiAim.h"
#include "../../Utility/signatures.h"
#include "../../Utility/Hook_t.h"

hook::createmove::template_createmove hook::createmove::original_createmove = nullptr;
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

	static uint8_t bit_flags = 0;
	
	/* MISCELLANEOUS */
	feature::bhop(cmd, bit_flags);
	feature::rocket_jump(cmd, result);
	feature::third_person();
	feature::autoBackStab(cmd, pLocalPlayer, pActiveWeapon);

	feature::airMove(cmd, result, pLocalPlayer); // <-- this one is not done

	/* AIMBOT */
	feature::aimbot(cmd, result);

	/* No spread */
	Features::noSpread.Run(cmd, result); // incomplete, not working

	Features::antiAim.Run(cmd, result);

	return result;
}