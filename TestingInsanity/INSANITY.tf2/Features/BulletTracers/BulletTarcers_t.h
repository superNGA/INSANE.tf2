#pragma once
#include "../features.h"

// this is a fucking half working make shift bullet tracer. 
// I shall make a fucking gem of a bullet tracer logic, but once the no spread
// is done.

class BulletTarcers_t
{
public:
    const char* Run(void* pWeapon); 

private:
    bool _ShouldRun(void* pWeapon);

};

ADD_FEATURE(bulletTracers, BulletTarcers_t);