#pragma once
#define _CRT_SECURE_NO_WARNINGS


#include <Windows.h>
#include <stdio.h>
#include <iostream>
#include <chrono>
#include <ctime>

//Macros for TextColors
#define FG_BLACK	30
#define FG_RED		31
#define FG_GREEN	32
#define FG_YELLOW	33
#define FG_BLUE		34
#define FG_MAGENTA	35
#define FG_CYAN		36
#define FG_WHITE	37

//Background color macros
#define BG_BLACK	40
#define BG_RED		41
#define BG_GREEN	42
#define BG_YELLOW	43
#define BG_BLUE		44
#define BG_MAGENTA	45
#define BG_CYAN		46
#define BG_WHITE	47

//Text format marcros
#define NORMAL		0
#define BOLD		1
#define UNDERLINE	4

class Console_System
{
public:
	//Constructors
	Console_System();
	Console_System(int8_t TextColor, int8_t Format, int8_t BackgroundColor);

	//Creating and freeing Console
	/**
	* ALLocates a console
	* only use in case of DLL else won't work
	*/
	bool CreateNewConsole();
	/**
	* Frees console
	*/
	bool FreeConsoleInstance();

	//Other Functionality
	/**
	* prints a underscore line across the while console
	*/
	void DoDevider();
	/**
	* Prints out INSANE written in ascii art, changes this for advertising
	*/
	void DoIntroduction(); //types out creaters name in big letters
	/**
	* use the color sceme you passed in at the constructor else uses default colors
	* default colors : white text , back background 
	* FastLogMessage : Message to print
	*/
	template <typename T>
	void FastLog(T FastLogMessage) //fast meessage printing
	{
		time_t NowTime = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
		tm* LocalTime = localtime(&NowTime);
		//Printing time
		printf("[ %d:%d:%d %s ] ", LocalTime->tm_hour > 12 ? LocalTime->tm_hour - 12 : LocalTime->tm_hour, LocalTime->tm_min, LocalTime->tm_sec, LocalTime->tm_hour > 12 ? "PM" : "AM");
		printf("\033[%d;%d;%dm", D_Format, D_TextColor, D_BackgroundColor); //Setting up printing config
		std::cout << FastLogMessage;
		printf("\033[0m\n"); //resesting
	}
	/**
	Logs a message to the console with the specified attributes.
	LogMessage : The message to log.
	TextColor : The foreground color of the text.
	Format : The text format (e.g., bold or italic).
	BackgroundColor : The background color of the text.
	*/
	void Log(const char* LogMessage, int8_t TextColor = FG_WHITE, int8_t Format = NORMAL, int8_t BackgroundColor = BG_BLACK); // prints message

	/* prints text in color
	* @param tagText : CS_NO for priting system time instead of tagText
	* Default format			-> BOLD
	* Default BackGround color	-> BLACK*/
	void Log(int8_t TextColor, const char* tagText,const char* LogMessage...);

private:
	bool   _isPrinting			= false;
	int8_t D_TextColor			= FG_WHITE;
	int8_t D_BackgroundColor	= BG_BLACK;
	int8_t D_Format				= NORMAL;
};