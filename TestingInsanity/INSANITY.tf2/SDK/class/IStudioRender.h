#pragma once
#include "../../Utility/Interface Handler/Interface.h"

class IStudioRender
{
public:
    // I don't need anything from here, this is just a dummy class so I can add the MAKE_INTERFACE_VERSION macro.
    // this is part of my attempt of making my code base more scalable.
};

MAKE_INTERFACE_VERSION(iStudioRender, "VStudioRender025", IStudioRender, STUDIORENDER_DLL)