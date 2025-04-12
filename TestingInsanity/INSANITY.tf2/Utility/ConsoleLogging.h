#pragma once
#include "../Libraries/Console System/Console_System.h"
extern Console_System cons;

#ifdef _DEBUG

#define WIN_LOG(msg, ...)   cons.Log(FG_GREEN, __FUNCTION__, msg, ##__VA_ARGS__)
#define FAIL_LOG(msg, ...)  cons.Log(FG_RED  , __FUNCTION__, msg, ##__VA_ARGS__)
#define LOG(msg, ...)       cons.Log(FG_CYAN , __FUNCTION__, msg, ##__VA_ARGS__)

#else 

#define WIN_LOG(msg, ...)   (void)0
#define FAIL_LOG(msg, ...)  (void)0
#define LOG(msg, ...)       (void)0

#endif // _DEBUG