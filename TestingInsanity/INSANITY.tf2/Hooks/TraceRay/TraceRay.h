#pragma once
#include"../../GlobalVars.h"
#include "../../SDK/class/IEngineTrace.h"

namespace hook
{
	namespace traceRay
	{
		typedef int64_t(__fastcall* T_traceRay)(IEngineTrace*, ray_t&, unsigned int, i_trace_filter*, CGameTrace*);
		extern T_traceRay O_traceRay;

		int64_t H_traceRay(IEngineTrace* pEngineTrace, ray_t& ray, unsigned int fMask, i_trace_filter* pTraceFilter, CGameTrace* pTrace);
	}
}