#include "TraceRay.h"

hook::traceRay::T_traceRay hook::traceRay::O_traceRay = nullptr;
int64_t hook::traceRay::H_traceRay(IEngineTrace* pEngineTrace, ray_t& ray, unsigned int fMask, i_trace_filter* pTraceFilter, CGameTrace* pTrace) {

	return O_traceRay(pEngineTrace, ray, fMask, pTraceFilter, pTrace);
}