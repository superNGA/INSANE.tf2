//=========================================================================
//                      No Spread
//=========================================================================
// by      : INSANE
// created : 05/04/2025
// 
// purpose : Removes spread (random weapon inaccuracy ) from weapon
//-------------------------------------------------------------------------
#pragma once
#include <cstdint>
class CUserCmd;

/* Conclusions : 
* -> weaponFileInfo_t seems to be something of very constant nature and looping through bullets
*		from weaponFIleINfo ain't necessary. avoid it.
* -> the bullet tracers tell me that I have absolute 0 spread and the bullet holes in the
*       walls and the damage output tells me that I have all the fucking spread. even in local
*       servers. now what the fuck is that?
*/

/* DONE :
* -> Aquire Seed
* -> hook the fire bullet Fn & compare your seed against it and see what's up
* -> GetRandomFloat seems to be working very nicely
* -> find a absolute no-spread removal logic, cause even if the seed was wrong,
*       view angles should be a little off and not just aiming at the fucking ground &
*       in the fucking air and shit.
* -> underStand what the AngleVectors fn is doing
* -> clamp the values
* -> look the cStd clam and gMod and make a proper efficient clamping logic.
* -> underStand what the VectorAngles function is doing
* -> for Heavy the Base spread seems to have some random-ness. get that after some progress.
*/

/* TODO :
* -> device a method to send in commands on the in-game console.
* -> send in the playerperf command & store / read it.
* -> get the seed and learn what the syncing logic even does in bigger softwares.
* -> fix the no-spread logic for minigun & make no spread work user cmd seed enabled.
* 
* -> maybe even aquire the in-game ping
* -> make a imformation window ( translucent ) and display some good imformation on it.
* 
* -> get server seed.
* 
* -> completely understand how game is calculating spread.
*/


class NoSpread_t
{
public:
    void run(CUserCmd* cmd, bool& result);

private:
	uint32_t _GetSeed(CUserCmd* cmd);
};
extern NoSpread_t noSpread;