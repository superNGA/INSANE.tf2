//=========================================================================
//                      CONSOLE SYSTEM V2
//=========================================================================
// by      : INSANE
// created : 07/03/2025
// 
// purpose : easier and better console output / printing.
//-------------------------------------------------------------------------

#pragma once

#ifdef _DEBUG
#define ENABLE_CONSOLE 1 // set to 0 to disable console
#else
#define ENABLE_CONSOLE 0
#endif // DEBUG


//======================= COLOR MACROS =======================
// text colors
enum consTextClr_t
{
    FG_INVALID = -1,
    FG_BLACK = 30,
    FG_RED = 31,
    FG_GREEN = 32,
    FG_YELLOW = 33,
    FG_BLUE = 34,
    FG_MAGENTA = 35,
    FG_CYAN = 36,
    FG_WHITE = 37
};

//Background color
enum consBgClr_t
{
    BG_BLACK = 40,
    BG_RED = 41,
    BG_GREEN = 42,
    BG_YELLOW = 43,
    BG_BLUE = 44,
    BG_MAGENTA = 45,
    BG_CYAN = 46,
    BG_WHITE = 47
};

//Text format
enum consTextFormat_t
{
    NORMAL = 0,
    BOLD = 1,
    UNDERLINE = 4,
};

//======================= FN MACROS =======================
#define CONS_INITIALIZE()   CONS.inititalize()
#define CONS_UNINITIALIZE() CONS.uninitialize()
#define DRAW_DEVIDER()      CONS.drawDevider()

// error & sucess logs
#define CONS_LOG_ERROR(errorMessage) CONS.Log(FG_RED, BG_BLACK, BOLD, __FUNCTION__, false, errorMessage)
#define CONS_LOG_SUCCESS(Message) CONS.Log(FG_GREEN, BG_BLACK, BOLD, __FUNCTION__, false, Message)

// text logs
#define CONS_FASTLOG(textColor, logMessage, ...) CONS.Log(textColor, BG_BLACK, BOLD, __FUNCTION__, false, logMessage, ##__VA_ARGS__)
#define CONS_FASTLOGWT(textColor, logMessage, ...) CONS.Log(textColor, BG_BLACK, BOLD, __FUNCTION__, true, logMessage, ##__VA_ARGS__)
#define CONS_LOG(textColor, BGColor, textFormat, logmessage, ...) CONS.Log(textColor, BGColor, textFormat, __FUNCTION__, false, logmessage, ##__VA_ARGS__)
#define CONS_LOGWT(textColor, BGColor, textFormat, logmessage, ...) CONS.Log(textColor, BGColor, textFormat, __FUNCTION__, true, logmessage, ##__VA_ARGS__)

// float arrays / struct logs
#define CONS_FASTLOG_FLOAT_ARR_CUSTOM(pFloatArr, iSize) CONS.LogFloatArr(FG_CYAN, BG_BLACK, BOLD, __FUNCTION__, false, #pFloatArr, (void*)&pFloatArr, iSize)
#define CONS_FASTLOG_FLOAT_ARR(pFloatArr) CONS.LogFloatArr(FG_CYAN, BG_BLACK, BOLD, __FUNCTION__, false, #pFloatArr, (void*)&pFloatArr, sizeof(pFloatArr))
#define CONS_FASTLOG_FLOAT_ARRWT(pFloatArr) CONS.LogFloatArr(FG_CYAN, BG_BLACK, BOLD, __FUNCTION__, true, #pFloatArr, (void*)&pFloatArr, sizeof(pFloatArr))
#define CONS_LOG_FLOAT_ARR(textColor, BGColor, textFormat, pFloatArr) CONS.LogFloatArr(textColor, BGColor, textFormat, __FUNCTION__, false, #pFloatArr, (void*)&pFloatArr, sizeof(pFloatArr))
#define CONS_LOG_FLOAT_ARRWT(textColor, BGColor, textFormat, pFloatArr) CONS.LogFloatArr(textColor, BGColor, textFormat, __FUNCTION__, true, #pFloatArr, (void*)&pFloatArr, sizeof(pFloatArr))


#include <iostream>
#include <string>
#include <vector>

class ConsoleSystem_t
{
public:
    ConsoleSystem_t();
    bool inititalize(consTextClr_t introClr = FG_INVALID, bool bDoIntro = true);
    void uninitialize();

    void drawDevider();
    void Log(consTextClr_t textColor, consBgClr_t BGColor, consTextFormat_t format, const char* tag, bool showTimeInstead, const char* logMessage...);
    void LogFloatArr(consTextClr_t textColor, consBgClr_t BGColor, consTextFormat_t format, const char* tag, bool showTimeInstead, const char* varName, void* pFloatArr, uint32_t size);

    // character used for printing devider
    const char* deviderCharacter = "=";
    
private:
    bool _isConsoleAllocated();
    bool _allocateConsole();
    void _printIntro(consTextClr_t introClr);
    int _getConsoleWidth();
    bool _isPrinting = false;
    bool _isInitialized = false;

    std::vector<std::string> _intro;
};
extern ConsoleSystem_t CONS;