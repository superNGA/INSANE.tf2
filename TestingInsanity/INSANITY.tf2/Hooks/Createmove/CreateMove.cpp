#include "CreateMove.h"
#include "../../SDK/Entity Manager/entityManager.h"
#include "../../SDK/FN index Manager/FN index manager.h"

hook::createmove::template_createmove hook::createmove::original_createmove = nullptr;
bool hook::createmove::hooked_createmove(int64_t a1, int64_t a2, CUserCmd* cmd)
{
	bool result = original_createmove(a1, a2, cmd);

	if (!cmd || !cmd->command_number || entityManager.initialized.load() == false) {
		return result;
	}

	//int64_t randomSeed = tfObject.MD5_PseudoRandom(cmd->command_number) & 0x7FFFFFFF;

	BaseEntity* pLocalPlayer	= entityManager.getLocalPlayer();
	baseWeapon* pActiveWeapon	= pLocalPlayer->getActiveWeapon();

	//int index = g_FNindexManager.getFnIndex(FN_name_t::FN_IS_ATTACK_CRIT, (void*)pActiveWeapon);
	//typedef bool(__fastcall* T_isCrit)(void*);
	//bool output = ((T_isCrit)g_FNindexManager.getFnAdrs(FN_name_t::FN_IS_ATTACK_CRIT, (void*)pActiveWeapon))(pActiveWeapon);

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

	return result;
}