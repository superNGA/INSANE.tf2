#pragma once
#include "Basic Structures.h"
#include "dworldLight_t.h"


#define MAXLOCALLIGHTS	     4
#define	MAX_LIGHTSTYLES	     64
#define MAX_LIGHTSTYLE_BITS	 MAX_LIGHTSTYLES
#define MAX_LIGHTSTYLE_BYTES ((MAX_LIGHTSTYLE_BITS + 7) / 8 )


enum LightCacheFlags_t : unsigned int
{
    LIGHTCACHEFLAGS_STATIC      = 0x1,
    LIGHTCACHEFLAGS_DYNAMIC     = 0x2,
    LIGHTCACHEFLAGS_LIGHTSTYLE  = 0x4,
    LIGHTCACHEFLAGS_ALLOWFAST   = 0x8,
};


///////////////////////////////////////////////////////////////////////////
struct LightingStateInfo_t
{
    float	m_pIllum[MAXLOCALLIGHTS];
    bool 	m_LightingStateHasSkylight;
};
///////////////////////////////////////////////////////////////////////////


///////////////////////////////////////////////////////////////////////////
struct LightingState_t
{
    vec		            r_boxcolor[6];		// ambient, and lights that aren't in locallight[]
    int			        numlights;
    dworldlight_t*      locallight[MAXLOCALLIGHTS];
};
///////////////////////////////////////////////////////////////////////////


///////////////////////////////////////////////////////////////////////////
class CBaseLightCache : public LightingStateInfo_t
{
public:
    LightingState_t	    m_StaticLightingState;
    LightingState_t     m_LightStyleLightingState;
    int				    m_LastFrameUpdated_LightStyles;
    LightingState_t     m_DynamicLightingState;
    int				    m_LastFrameUpdated_DynamicLighting;
    int	                m_LightingFlags;
    int                 leaf;
    unsigned char	    m_pLightstyles[MAX_LIGHTSTYLE_BYTES];
    vec 			    m_LightingOrigin;
    void*               m_pEnvCubemapTexture; // is a ITexture*
};
///////////////////////////////////////////////////////////////////////////