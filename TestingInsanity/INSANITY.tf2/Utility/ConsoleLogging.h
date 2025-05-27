#pragma once
//#include "../Libraries/Console System/Console_System.h"
#include "../Libraries/Console System V2/ConsoleSystem.h"
//extern Console_System cons;

#ifdef _DEBUG

#define EXPAND(X) X

#define INITIALIZE_CONSOLE()    EXPAND(CONS_INITIALIZE())
#define UNINITIALIZE_CONSOLE()  EXPAND(CONS_UNINITIALIZE())
#define WIN_LOG(msg, ...)       EXPAND(CONS_FASTLOG(FG_GREEN, msg, ##__VA_ARGS__))
#define FAIL_LOG(msg, ...)      EXPAND(CONS_FASTLOG(FG_RED,   msg, ##__VA_ARGS__))
#define LOG(msg, ...)           EXPAND(CONS_FASTLOG(FG_CYAN,  msg, ##__VA_ARGS__))
#define LOG_VEC3(Vector)        EXPAND(CONS_FASTLOG_FLOAT_ARR_CUSTOM(Vector, sizeof(float) * 3))
#define LOG_VEC2(Vector)        EXPAND(CONS_FASTLOG_FLOAT_ARR_CUSTOM(Vector, sizeof(float) * 2))

#else 

#define WIN_LOG(msg, ...)       (void)0
#define FAIL_LOG(msg, ...)      (void)0
#define LOG(msg, ...)           (void)0
#define INITIALIZE_CONSOLE()    (void)0
#define UNINITIALIZE_CONSOLE()  (void)0
#define LOG_VEC3(Vector)        (void)0
#define LOG_VEC2(Vector)        (void)0

#endif // _DEBUG