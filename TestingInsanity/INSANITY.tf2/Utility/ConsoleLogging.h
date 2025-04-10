#pragma once
#include "../Libraries/Console System/Console_System.h"
extern Console_System cons;

#ifdef _DEBUG

#define WIN_LOG(msg, ...)   cons.Log(FG_GREEN, __FUNCTION__, msg, ##__VA_ARGS__)
#define FAIL_LOG(msg, ...)  cons.Log(FG_RED  , __FUNCTION__, msg, ##__VA_ARGS__)
#define LOG(msg, ...)       cons.Log(FG_CYAN , __FUNCTION__, msg, ##__VA_ARGS__)

#endif // _DEBUG