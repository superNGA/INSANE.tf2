/*
This thread will be center for retreiving, updating & filtering entities
such as local player, active weapon, enimies.
might also do projectile aimbot caculations and other stuff */

#pragma once
#include <Windows.h>
#include <thread> 

/* global vars */
#include "../GlobalVars.h"

/* handle netvars */
#include "../SDK/offsets/offsets.h"

/* directX EndScene hook, includes necessary information about UI status */
#include "../Hooks/EndScene/EndScene.h"

/* Console_System object made in main.cpp */
#ifdef _DEBUG
	#include "../Libraries/Console System/Console_System.h"
	extern Console_System cons;
#endif

extern local_netvars netvar;

void execute_thread2(HINSTANCE instance);

vec proj_aimbot_calc(vec ent_pos, vec ent_vel, bool on_ground, qangle viewangles);

void decide_bone_id();