#include "WorldColorModulation.h"

// SDK
#include "../../SDK/class/IMaterial.h"
#include "../../SDK/class/IMaterialSystem.h"
#include "../../SDK/class/LightDesc_t.h"



///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
WorldColorModulation_t::WorldColorModulation_t()
{
    m_lastClr.Set(255, 255, 255, 255);
}


///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
void WorldColorModulation_t::Run()
{
    RGBA_t desiredClr = Features::Materials::World::World_ColorMod.GetData().GetAsBytes();

    if (desiredClr == m_lastClr)
        return;

    // Iterate all the mateirals loaded and modulate color & alpha for all of them.
    for (unsigned short hMat = I::iMaterialSystem->FirstMaterial(); hMat != I::iMaterialSystem->InvalidMaterial(); hMat = I::iMaterialSystem->NextMaterial(hMat))
    {
        IMaterial* pMat = I::iMaterialSystem->GetMaterial(hMat);

        if (strcmp(pMat->GetTextureGroupName(), TEXTURE_GROUP_WORLD) == 0)
        {
            Vec4 clr = desiredClr.GetAsVec4();
            pMat->ColorModulate(clr.x, clr.y, clr.z);
            pMat->AlphaModulate(clr.w);
        }
    }

    m_lastClr = desiredClr;
}


///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
void WorldColorModulation_t::GetWorldColorModulation(Vec4* pClrOut) const
{
    *pClrOut = Features::Materials::World::World_ColorMod.GetData().GetAsBytes().GetAsVec4();
}


///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
vec* WorldColorModulation_t::GetAmbientLight()
{
    m_vAmbientLights[AmbientLight_t::LIGHT_BACK]   = Features::Materials::World::World_StaticPropLightBack.GetData().GetAsBytes().GetAsVec4().XYZ();
    m_vAmbientLights[AmbientLight_t::LIGHT_FRONT]  = Features::Materials::World::World_StaticPropLightFront.GetData().GetAsBytes().GetAsVec4().XYZ();
    m_vAmbientLights[AmbientLight_t::LIGHT_TOP]    = Features::Materials::World::World_StaticPropLightTop.GetData().GetAsBytes().GetAsVec4().XYZ();
    m_vAmbientLights[AmbientLight_t::LIGHT_BOTTON] = Features::Materials::World::World_StaticPropLightBottom.GetData().GetAsBytes().GetAsVec4().XYZ();
    m_vAmbientLights[AmbientLight_t::LIGHT_LEFT]   = Features::Materials::World::World_StaticPropLightLeft.GetData().GetAsBytes().GetAsVec4().XYZ();
    m_vAmbientLights[AmbientLight_t::LIGHT_RIGHT]  = Features::Materials::World::World_StaticPropLightRight.GetData().GetAsBytes().GetAsVec4().XYZ();

    return m_vAmbientLights;
}
