//=========================================================================
//                      CONSOLE SYSTEM V2
//=========================================================================
// by      : INSANE
// created : 07/03/2025
// 
// purpose : easier and better console output / printing.
//-------------------------------------------------------------------------

#define _CRT_SECURE_NO_WARNINGS
#include "ConsoleSystem.h"
ConsoleSystem_t cons;

#include <Windows.h>
#include <chrono>
#include <thread>

#if ENABLE_CONSOLE == 1
#define ASSERT_TEXTCLR(clr) if(clr > FG_WHITE || clr < FG_BLACK) clr = FG_WHITE
#endif // ENABLE_CONSOLE

//=========================================================================
// ConsoleSystem_t::ConsoleSystem_t()
//=========================================================================
/**
* contructor, fills in the intro text
*/
//-------------------------------------------------------------------------
ConsoleSystem_t::ConsoleSystem_t()
{
    _intro.clear();
    
    // make sure the intro is all lined up. else can cause trouble printing
    _intro.push_back(std::string(" 8888888     888b    888   .d8888b.         888       888b    888   8888888888    \n"));
    _intro.push_back(std::string("   888       8888b   888  d88P  Y88b       8888       8888b   888   888           \n"));
    _intro.push_back(std::string("   888       88888b  888  Y88b.           88P88       88888b  888   888           \n"));
    _intro.push_back(std::string("   888       888Y88b 888   \"Y888b.       88P 88       888Y88b 888   8888888      \n"));
    _intro.push_back(std::string("   MMM       M  `MN. MMM    `YMMNq.     ,M  `MM       M  `MN. MMM   MMMMmmMM      \n"));
    _intro.push_back(std::string("   MMM       M   `MM.MMM  .     `MM     AbmmmqMA      M   `MM.MMM   MMMM   Y  ,   \n"));
    _intro.push_back(std::string("   MMM       M     YMMMM  Mb     dM    A'     VML     M     YMMMM   MMMM     ,M   \n"));
    _intro.push_back(std::string(".JMMMMML.  .JML.    YMMM  P\"Ybmmd\"   .AMA.   .AMMA. .JML.    YMMM .JMMmmmmmmMMM \n"));
}

//=========================================================================
//                     PUBLIC METHODS
//=========================================================================


//=========================================================================
// bool ConsoleSystem_t::inititalize()
//=========================================================================
/**
* handles console allocation and intro
*/
//-------------------------------------------------------------------------
bool ConsoleSystem_t::inititalize(consTextClr_t introClr, bool bDoIntro)
{
    if (_isInitialized == true)
        return true;

    // if console not already allocated
    if (_isConsoleAllocated() == false)
    {
        // allocated a console
        if (_allocateConsole() == false)
            return false; // failed console allocation
    }

    if (introClr == consTextClr_t::FG_INVALID)
    {
        time_t NowTime = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
        tm* LocalTime = localtime(&NowTime);
        float second = (float)LocalTime->tm_sec;
        introClr = (consTextClr_t)(int)((second / 7.5f) + 30.0f); // I am scared to cast it straight to enum :(
        ASSERT_TEXTCLR(introClr);
    }

    _printIntro(introClr);
    drawDevider();

    _isInitialized = true;

    return true;
}


//=========================================================================
// void ConsoleSystem_t::uninitialize()
//=========================================================================
/**
* free allcoated console
*/
//-------------------------------------------------------------------------
void ConsoleSystem_t::uninitialize()
{
    if(_isConsoleAllocated() == true)
        FreeConsole();
}


//=========================================================================
// void ConsoleSystem_t::drawDevider()
//=========================================================================
/**
* draws a line of characters along the whole width of console
*/
//-------------------------------------------------------------------------
void ConsoleSystem_t::drawDevider()
{
    // skipping if no console allocated
    if (_isConsoleAllocated() == false)
        return;

    uint32_t consoleWidth = _getConsoleWidth();
    for (int i = 0; i < consoleWidth; i++)
    {
        printf(deviderCharacter);
    }
    printf("\n");
}


//=========================================================================
// void ConsoleSystem_t::Log(consTextClr_t textColor, consBgClr_t BGColor, consTextFormat_t format, const char* tag, bool showTimeInstead, const char* logMessage...)
//=========================================================================
/**
* prints text
*
* @param textColor : color of text
* @param BGColor : Back ground color for text
* @param format : choose btw bold, normal and underline
* @param tag : name of the function
* @param showTimeinstead : print system time instead of caller name
* @param logMessage : message to print
*/
//-------------------------------------------------------------------------
void ConsoleSystem_t::Log(consTextClr_t textColor, consBgClr_t BGColor, consTextFormat_t format, const char* tag, bool showTimeInstead, const char* logMessage...)
{
    if (_isInitialized == false)
        return;

    while (_isPrinting == true)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(100)); // sleeping for 0.1 seconds, when currently printing something
    }
    _isPrinting = true;

    // printing TAG 
    if (showTimeInstead == true)
    {
        time_t NowTime = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
        tm* LocalTime = localtime(&NowTime);

        printf("[ %d:%d:%d %s ] ", LocalTime->tm_hour > 12 ? LocalTime->tm_hour - 12 : LocalTime->tm_hour, LocalTime->tm_min, LocalTime->tm_sec, LocalTime->tm_hour > 12 ? "PM" : "AM");
    }
    else
    {
        printf("[ %s ] ", tag);
    }
    
    // printing MESSAGE
    va_list args;
    va_start(args, logMessage);
    printf("\033[%d;%d;%dm", format, textColor, BGColor); // Set text color
    vprintf(logMessage, args); // Format the LogMessage with the arguments
    printf("\033[0m\n"); // Reset text formatting
    va_end(args);

    _isPrinting = false;
}


//=========================================================================
// void ConsoleSystem_t::LogFloatArr(
//              consTextClr_t textColor, consBgClr_t BGColor,
//              consTextFormat_t format, const char* tag,
//              bool showTimeInstead, const char* varName,
//              void* pFloatArr, uint32_t size)
//=========================================================================
/**
* prints float array or struct, usefull for physics float vectors
*
* @param textColor : color of text
* @param BGColor : backGround color for text
* @param format : choose btw bold, normal & underline
* @param tag : caller name
* @param showTimeInstead : print system time instead of caller name
* @param varName : name of varaible we are printing
* @param pFloatArr : pointer to variable we are printing
* @param size : sizeof(var)
*/
//-------------------------------------------------------------------------
void ConsoleSystem_t::LogFloatArr(
    consTextClr_t textColor, consBgClr_t BGColor, 
    consTextFormat_t format, const char* tag, 
    bool showTimeInstead, const char* varName, 
    void* pFloatArr, uint32_t size)
{
    if (_isInitialized == false)
        return;

    while (_isPrinting == true)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(100)); // sleeping for 0.1 seconds, when currently printing something
    }
    _isPrinting = true;

    // printing TAG 
    if (showTimeInstead == true)
    {
        time_t NowTime = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
        tm* LocalTime = localtime(&NowTime);

        printf("[ %d:%d:%d %s ] ", LocalTime->tm_hour > 12 ? LocalTime->tm_hour - 12 : LocalTime->tm_hour, LocalTime->tm_min, LocalTime->tm_sec, LocalTime->tm_hour > 12 ? "PM" : "AM");
    }
    else
    {
        printf("[ %s ] ", tag);
    }

    printf("\033[%d;%d;%dm", format, textColor, BGColor); // Set text color
    printf("%s : ", varName);
    printf("\033[0m"); // Reset text formatting

    uint32_t nElements = size / 4;
    for (uint32_t i = 0; i < nElements; i++)
    {
        float curFloat = *(float*)((uintptr_t)pFloatArr + (0x4 * i));
        printf("%.4f ", curFloat);
    }
    printf("\n");

    _isPrinting = false;
}


//=========================================================================
//                     PRIVATE METHODS
//=========================================================================

//=========================================================================
// bool ConsoleSystem_t::_isConsoleAllocated()
//=========================================================================
/**
* checks if console is allocated
*/
//-------------------------------------------------------------------------
bool ConsoleSystem_t::_isConsoleAllocated()
{
    return (GetConsoleWindow() != INVALID_HANDLE_VALUE && GetConsoleWindow() != NULL);
}


//=========================================================================
// bool ConsoleSystem_t::_allocateConsole()
//=========================================================================
/**
* allocates a console
*/
//-------------------------------------------------------------------------
bool ConsoleSystem_t::_allocateConsole()
{
    if (AllocConsole() == true)
    {
        FILE* file;
        freopen_s(&file, "CONOUT$", "w", stdout);
        freopen_s(&file, "CONOUT$", "w", stderr);

        //Enabling colors or some shit
        HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
        DWORD dwMode = 0;
        GetConsoleMode(hOut, &dwMode);
        dwMode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
        SetConsoleMode(hOut, dwMode);

        return true;
    }

	return false;
}


//=========================================================================
// void ConsoleSystem_t::_printIntro()
//=========================================================================
/**
* prints intro in the center of the console
*/
//-------------------------------------------------------------------------
void ConsoleSystem_t::_printIntro(consTextClr_t introClr)
{
    int consoleWidth = _getConsoleWidth();
    bool canFitIntroInConsole = true;

    // Formatting for intro text
    printf("\033[%d;%d;%dm", consTextFormat_t::BOLD, introClr, consBgClr_t::BG_BLACK);

    // if intro text is greater than the console width, then just print normaly
    if (_intro[0].length() >= consoleWidth)
    {
        for (std::string& x : _intro)
            std::cout << x;
    }

    // if intro can be fitted in the center
    int nSpacesBeforeIntro = (consoleWidth - _intro[0].length()) / 2;
    for (std::string& x : _intro)
    {
        // printing spaces
        for (int i = 0; i < nSpacesBeforeIntro; i++)
        {
            std::cout << " ";
        }
        std::cout << x; // printing intro line
    }

    printf("\033[0m"); // Reset text formatting
}


//=========================================================================
// int ConsoleSystem_t::_getConsoleDimension()
//=========================================================================
/**
* get the windows conslole dimensions
*/
//-------------------------------------------------------------------------
int ConsoleSystem_t::_getConsoleWidth()
{
    CONSOLE_SCREEN_BUFFER_INFO ConsoleInfo;
    if (GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &ConsoleInfo) == FALSE)
        return 0;

    return ConsoleInfo.srWindow.Right - ConsoleInfo.srWindow.Left + 1;
}