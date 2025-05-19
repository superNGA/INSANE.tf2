#pragma once
#include "../FeatureHandler.h"

// this is a fucking half working make shift bullet tracer. 
// I shall make a fucking gem of a bullet tracer logic, but once the no spread
// is done.

class BulletTarcers_t
{
public:
    const char* Run(void* pWeapon, const char* szOriginalOutput);

private:
    bool _ShouldRun(void* pWeapon);

};

DECLARE_FEATURE_OBJECT(bulletTracers, BulletTarcers_t);