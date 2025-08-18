#include "MaterialGen.h"

#include "../ModelPreview/ModelPreview.h"

// SDK
#include "../../SDK/class/IVEngineClient.h"
#include "../../SDK/class/BaseEntity.h"

// UTILITY
#include "../../Utility/ConsoleLogging.h"

// External Dependencies
#include "../../External Libraries/ImGui/imgui.h"


void MaterialGen_t::Run()
{
    if (Features::MaterialGen::MaterialGen::Enable.IsActive() == false)
        return;

    // clr
    F::modelPreview.SetRenderViewClr(40, 40, 40, 255);
    F::modelPreview.SetPanelClr(22, 22, 22, 255);

    // position
    F::modelPreview.SetPanelPos(0, 0);
    int iWidth = 0, iHeight = 0; I::iEngine->GetScreenSize(iWidth, iHeight);
    F::modelPreview.SetRenderViewPos(static_cast<int>((2.0f / 3.0f) * static_cast<float>(iWidth)), 0);

    // size
    F::modelPreview.SetPanelSize(iHeight, iWidth);
    F::modelPreview.SetRenderViewSize(iHeight, static_cast<int>((1.0f / 3.0f) * static_cast<float>(iWidth)));

    vec vCamera(
        Features::MaterialGen::MaterialGen::X.GetData().m_flVal,
        Features::MaterialGen::MaterialGen::Y.GetData().m_flVal,
        Features::MaterialGen::MaterialGen::Z.GetData().m_flVal
    );
    //auto* pEnt = F::modelPreview.GetModelEntity();
    //*reinterpret_cast<vec*>(reinterpret_cast<uintptr_t>(pEnt) + Netvars::DT_BaseEntity::m_vecOrigin - (12ll * 4ll)) = vCamera;

    F::modelPreview.SetCameraPos(vCamera);
}


/*
TODO : 
-> Make model move.
-> Make model rotate.
-> Alter lighting.
-> Stop Console from opening while material gen is enabled.
-> Position object mathematically or give slider.
*/