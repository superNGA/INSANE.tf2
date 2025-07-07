#pragma once
#include "../../Utility/Interface Handler/Interface.h"

class IVModelRender
{
public:
    // Dummy class, need it to use MAKE_INTERFACE_MACRO
};

MAKE_INTERFACE_VERSION(iVModelRender, "VEngineModel016", IVModelRender, ENGINE_DLL)