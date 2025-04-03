//=========================================================================
//                      TIMER
//=========================================================================
// by      : INSANE
// created : 16/03/2025
// 
// purpose : Calculates time using queryPerformanceCounter ( windows only )
//-------------------------------------------------------------------------
#pragma once
#include <stdio.h>
#include <Windows.h>
#include <time.h>

#define ENABLE_TIMER_LOG 1

typedef struct TimerData_t
{
    LARGE_INTEGER tmStart;
    LARGE_INTEGER tmEnd;
    LARGE_INTEGER cpuFrequency;

    double flTimeElapsedInMs;
}TimerData_t;

TimerData_t g_timerData;

inline void StartTimer()
{
    QueryPerformanceFrequency(&g_timerData.cpuFrequency);
    QueryPerformanceCounter(&g_timerData.tmStart);
}

inline float EndTimer()
{
    QueryPerformanceCounter(&g_timerData.tmEnd);
    g_timerData.flTimeElapsedInMs = (g_timerData.tmEnd.QuadPart - g_timerData.tmStart.QuadPart) * 1000.0f / g_timerData.cpuFrequency.QuadPart;
    return g_timerData.flTimeElapsedInMs;
}