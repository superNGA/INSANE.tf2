//=========================================================================
//                      NO SPREAD
//=========================================================================
// by      : INSANE
// created : 14/04/2025
// 
// purpose : Removes random bullet inaccuracy.
//-------------------------------------------------------------------------
#pragma once
#include "../features.h"

class cmd;

class NoSpread_t
{
public:
    void Run(CUserCmd* cmd, bool& result);

private:

};

ADD_FEATURE(noSpread, NoSpread_t);