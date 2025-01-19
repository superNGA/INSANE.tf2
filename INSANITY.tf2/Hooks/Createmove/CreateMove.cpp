#include "CreateMove.h"

hook::createmove::template_createmove hook::createmove::original_createmove = nullptr;
bool hook::createmove::hooked_createmove(int64_t a1, int64_t a2, CUserCmd* cmd)
{
	bool result = original_createmove(a1, a2, cmd);

	if (!cmd || !cmd->command_number) {
		return result;
	}

	static uint8_t bit_flags = 0;
	
	/*BHOP*/
	feature::bhop(cmd, bit_flags);
	feature::rocket_jump(cmd, result);
	feature::third_person();

	/* testing */
	cmd->viewangles = entities::target::best_angle;
	result = false;

	return result;
}