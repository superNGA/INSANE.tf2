#pragma once
#include "../SDK/class/CUserCmd.h"
#include "config.h"

extern local_netvars netvar;

#define SPACEBAR_STATE (1<<0)

namespace feature
{
	/*This bhop feature might seem badly constructed but this allows for double jumping when 
	playing as scout.*/
	inline void bhop(CUserCmd* cmd, uint8_t& bits)
	{
		if (!config::miscellaneous::bhop) return;

		int32_t flag = *(int32_t*)(netvar.local_player + netvar.m_fFlags);

		if (!GetAsyncKeyState(VK_SPACE))
		{
			/* if in air and NOT holding space bar */
			if (!(flag & (1 << 0))) {
				bits &= ~SPACEBAR_STATE;
				printf("AIR | NO SPACE\n");
			}
			return;
		}

		if (flag & (1 << 0))
		{
			cmd->buttons |= IN_JUMP;
			printf("ground | NO SPACE\n");
		}
		else if (!(bits & SPACEBAR_STATE))
		{
			bits |= SPACEBAR_STATE;
			cmd->buttons |= IN_JUMP;
			printf("AIR | SPACE | DOUBLE JUMP\n");
		}
		else
		{
			cmd->buttons &= ~(IN_JUMP);
			printf("AIR | SPACE\n");
		}
	}


};