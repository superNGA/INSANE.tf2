/*
This thread will be center for retreiving, updating & filtering entities
such as local player, active weapon, enimies.
might also do projectile aimbot caculations and other stuff */

#pragma once
#define _CRT_SECURE_NO_WARNINGS
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

#include "../SDK/FN index manager.h"
#include "../SDK/class/BaseWeapon.h"

extern local_netvars netvar;

void execute_thread2(HINSTANCE instance);

vec proj_aimbot_calc(vec ent_pos, vec ent_vel, bool on_ground, qangle viewangles);

void decide_bone_id();

/* tells if we are holding a PROJECTILE weapon or a HIT-SCAN weapon 
TRUE -> projectile
FALSE -> HIT-SCAN*/
bool getWeaponType(int16_t weaponID);

/* simple hashing function, uses a string, and choice of character ( or any static number related to 
the entity ). Might need to work more on this in future. Using string straight away with map was not 
working. */
inline int hashString(const char* entUserName, int8_t entCharacterChoice) {
	int16_t nameLen = strlen(entUserName);
	int32_t hashOutput = 0;
	for (int i = 0; i < nameLen; i++) {
		hashOutput += (i + 1 + entCharacterChoice) * (int)entUserName[i];
	}
	return hashOutput;
}