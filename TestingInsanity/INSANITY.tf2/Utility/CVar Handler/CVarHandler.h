#pragma once

#include <vector>

#include "../../Features/FeatureHandler.h"

struct CVarHelper_t;

/////////////////////////////////////////////////////////////
class CVarHandler_t
{
public:
    void InitializeAllCVars();
    void RegisterCVar(CVarHelper_t* pCVar);
    inline void InvalidateCVars() { m_bInitialzed = false; }

private:
    std::vector<CVarHelper_t*> m_vecRegisteredCVars = {};

    bool m_bInitialzed = false;
};
DECLARE_FEATURE_OBJECT(cVarHandler, CVarHandler_t)
/////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////
struct CVarHelper_t
{
    CVarHelper_t(const char* szCVarName, bool bInt, void* pDestination)
    {
        m_bIsInt = bInt;
        m_pDestination = pDestination;
        m_szCVarName = szCVarName;

        F::cVarHandler.RegisterCVar(this);
    }

    void* m_pDestination = nullptr;
    const char* m_szCVarName = nullptr;
    bool        m_bIsInt = true;
};
/////////////////////////////////////////////////////////////


#define REGISTER_CVAR_INT(szCVarName)\
namespace CVars{      inline int           szCVarName = 0;          }\
namespace CVarHelper{ inline CVarHelper_t szCVarName(#szCVarName, true, reinterpret_cast<void*>(&CVars::szCVarName)); }

#define REGISTER_CVAR_FLOAT(szCVarName)\
namespace CVars{      inline float           szCVarName = 0;          }\
namespace CVarHelper{ inline CVarHelper_t szCVarName(#szCVarName, false, reinterpret_cast<void*>(&CVars::szCVarName)); }


REGISTER_CVAR_FLOAT(sv_gravity)
REGISTER_CVAR_FLOAT(tf_weapon_criticals_bucket_bottom)
REGISTER_CVAR_FLOAT(tf_weapon_criticals_bucket_cap)
REGISTER_CVAR_FLOAT(tf_weapon_criticals_bucket_default)
REGISTER_CVAR_FLOAT(sv_airaccelerate)

REGISTER_CVAR_FLOAT(cl_interp)
REGISTER_CVAR_FLOAT(cl_interp_ratio)
REGISTER_CVAR_INT(cl_updaterate)
REGISTER_CVAR_INT(sv_maxusrcmdprocessticks)

REGISTER_CVAR_INT(cl_flipviewmodels)
