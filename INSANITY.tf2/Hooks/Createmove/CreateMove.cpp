#include "CreateMove.h"
#include "../../SDK/Entity Manager/entityManager.h"

hook::createmove::template_createmove hook::createmove::original_createmove = nullptr;
bool hook::createmove::hooked_createmove(int64_t a1, int64_t a2, CUserCmd* cmd)
{
	bool result = original_createmove(a1, a2, cmd);

	if (!cmd || !cmd->command_number || entityManager.initialized.load() == false) {
		return result;
	}

	static uint8_t bit_flags = 0;
	
	/* MISCELLANEOUS */
	feature::bhop(cmd, bit_flags);
	feature::rocket_jump(cmd, result);
	feature::third_person();

	/* AIMBOT */
	feature::aimbot(cmd, result);

	return result;
}