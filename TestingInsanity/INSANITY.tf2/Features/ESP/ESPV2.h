#pragma once

#include <unordered_map>
#include "../FeatureHandler.h"


class BaseEntity;
class IDrawObj_t;

///////////////////////////////////////////////////////////////////////////
class ESPV2_t
{
public:
    void Run(BaseEntity* pLocalPlayer);
    void Free();

private:
    bool m_bMapCleared = true;
    std::unordered_map<BaseEntity*, IDrawObj_t*> m_vecEntToDrawObj = {};

};
///////////////////////////////////////////////////////////////////////////

DECLARE_FEATURE_OBJECT(espV2, ESPV2_t)