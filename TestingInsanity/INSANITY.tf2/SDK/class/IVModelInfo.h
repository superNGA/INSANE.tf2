#pragma once
#include "../../Utility/Interface.h"

struct model_t;

class IVModelInfo
{
public:
	virtual					~IVModelInfo(void) {}
	virtual const model_t* GetModel(int modelindex) = 0;
	virtual int				GetModelIndex(const char* name) const = 0; // <- maybe this can help us get model index, and allow us to 
	virtual const char* GetModelName(const model_t* model) const = 0;
};

MAKE_INTERFACE_VERSION(iVModelInfo, "VModelInfoClient006", IVModelInfo, ENGINE_DLL)