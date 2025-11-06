#include "../Utility/Hook Handler/Hook_t.h"
#include "../Utility/ConsoleLogging.h"

#include "../SDK/class/BaseEntity.h"
#include "../SDK/class/viewSetup.h"

#include "../Features/FeatureHandler.h"
#include "../Features/Aimbot/AimbotHelper.h"
#include "../Features/Entity Iterator/EntityIterator.h"



///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
MAKE_HOOK(CViewRender_RenderView, "48 8B C4 44 89 48 ? 44 89 40 ? 48 89 50 ? 48 89 48 ? 55 53", __fastcall, CLIENT_DLL, int64_t,
	void* pVTable, CViewSetup* pViewSetup, int iClearFlag, int iWhatToDraw)
{
	const LocalPlayerInfo_t& localPlayerInfo = F::entityIterator.GetLocalPlayerInfo();


	// FOV...
	float flDesiredFOV = localPlayerInfo.m_iClass == TF_SNIPER && (localPlayerInfo.m_iCond & TF_COND_ZOOMED) == true ?
		Features::Misc::View::View_ScopedInFOV.GetData().m_flVal :
		Features::Misc::View::View_FOV.GetData().m_flVal;

	pViewSetup->fov = flDesiredFOV;
	F::aimbotHelper.NotifyGameFOV(pViewSetup->fov);


	// Z-Near & Far...
	if(Features::Misc::View::View_zNear.GetData().m_flVal > 0.0f)
	{
		pViewSetup->zNear = Features::Misc::View::View_zNear.GetData().m_flVal;
	}
	if (Features::Misc::View::View_zFar.GetData().m_flVal > 0.0f)
	{
		pViewSetup->zFar = Features::Misc::View::View_zFar.GetData().m_flVal;
	}


	// View Model's Z-Near & Far...
	if (Features::Misc::View::View_ViewModelZNear.GetData().m_flVal > 0.0f)
	{
		pViewSetup->zNearViewmodel = Features::Misc::View::View_ViewModelZNear.GetData().m_flVal;
	}
	if (Features::Misc::View::View_ViewModelZFar.GetData().m_flVal > 0.0f)
	{
		pViewSetup->zFarViewmodel = Features::Misc::View::View_ViewModelZFar.GetData().m_flVal;
	}


	// View model FOV...
	pViewSetup->fovViewmodel = Features::Misc::View::View_ViewModelFOV.GetData().m_flVal;


	// Draw view model ? 
	if(Features::Misc::View::View_AlwaysDrawViewModel.IsActive() == true)
	{
		iWhatToDraw |= RenderViewInfo_t::RENDERVIEW_DRAWVIEWMODEL;
	}


	// No HUD? 
	if(Features::Misc::View::View_NoHud.IsActive() == true)
	{
		iWhatToDraw &= ~(RenderViewInfo_t::RENDERVIEW_DRAWHUD);
	}


	return Hook::CViewRender_RenderView::O_CViewRender_RenderView(pVTable, pViewSetup, iClearFlag, iWhatToDraw);
}