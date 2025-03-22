#include "CreateMove.h"
#include "../../SDK/Entity Manager/entityManager.h"

hook::createmove::template_createmove hook::createmove::original_createmove = nullptr;
bool hook::createmove::hooked_createmove(int64_t a1, int64_t a2, CUserCmd* cmd)
{
	bool result = original_createmove(a1, a2, cmd);

	if (!cmd || !cmd->command_number || entityManager.initialized.load() == false) {
		return result;
	}

	int64_t randomSeed = tfObject.MD5_PseudoRandom(cmd->command_number) & 0x7FFFFFFF;
	//printf("random seed : %lld\n", randomSeed);

	BaseEntity* pLocalPlayer	= entityManager.getLocalPlayer();
	baseWeapon* pActiveWeapon	= pLocalPlayer->getActiveWeapon();
	
	float critChance = *(float*)((uintptr_t)pActiveWeapon + 0x1080);
	printf("crit Chance : %.2f\n", critChance);

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