#include "Console_System.h"

Console_System::Console_System()
{

}

Console_System::Console_System(int8_t TextColor, int8_t Format, int8_t BackgroundColor)
	:D_TextColor(TextColor), D_Format(Format), D_BackgroundColor(BackgroundColor)
{

}

bool Console_System::CreateNewConsole()
{
	if (AllocConsole())
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

bool Console_System::FreeConsoleInstance()
{
	if (FreeConsole())
	{
		return true;
	}

	return false;
}

void Console_System::DoDevider()
{
	int ConsoleWidth;
	CONSOLE_SCREEN_BUFFER_INFO ConsoleInfo;
	if (GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &ConsoleInfo))
	{
		ConsoleWidth = ConsoleInfo.srWindow.Right - ConsoleInfo.srWindow.Left + 1;
		for (int i = 0; i < ConsoleWidth; i++)
		{
			printf("_");
		}
		printf("\n");
	}
	else
	{
		Log("Failed to print devider", FG_RED, BOLD, BG_BLACK);
	}

	return;
}

void Console_System::DoIntroduction()
{
	printf("\033[%d;%d;%dm", BOLD, FG_CYAN, BG_BLACK); //setting up config
	
	//may change this as per your liking
	//this pattern in perfect don't change spacing and tryna be a smart ass
	printf(" 8888888     888b    888   .d8888b.         888       888b    888   8888888888    \n");
	printf("   888       8888b   888  d88P  Y88b       8888       8888b   888   888           \n");
	printf("   888       88888b  888  Y88b.           88P88       88888b  888   888           \n");
	printf("   888       888Y88b 888   \"Y888b.       88P 88       888Y88b 888   8888888       \n");
	printf("   MMM       M  `MN. MMM    `YMMNq.     ,M  `MM       M  `MN. MMM   MMMMmmMM      \n");
	printf("   MMM       M   `MM.MMM  .     `MM     AbmmmqMA      M   `MM.MMM   MMMM   Y  ,   \n");
	printf("   MMM       M     YMMMM  Mb     dM    A'     VML     M     YMMMM   MMMM     ,M   \n");
	printf(".JMMMMML.  .JML.    YMMM  P\"Ybmmd\"   .AMA.   .AMMA. .JML.    YMMM .JMMmmmmmmMMM   \n");

	printf("\033[0m\n"); //reseting printing config
	return;
}

void Console_System::Log(const char* LogMessage, int8_t TextColor, int8_t Format , int8_t BackgroundColor )
{
	time_t NowTime = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
	tm* LocalTime = localtime(&NowTime);

	printf("[ %d:%d:%d %s ] ", LocalTime->tm_hour > 12 ? LocalTime->tm_hour - 12 : LocalTime->tm_hour, LocalTime->tm_min, LocalTime->tm_sec, LocalTime->tm_hour > 12 ? "PM" : "AM");

	printf("\033[%d;%d;%dm", Format, TextColor, BackgroundColor); //Setting up printing config
	printf("%s\033[0m\n", LogMessage); //printing text
}

void Console_System::Log(int8_t TextColor, const char* tagText, const char* LogMessage...)
{
	time_t NowTime = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
	tm* LocalTime = localtime(&NowTime);

	if (!strcmp(tagText, "CS_NO")) {
		printf("[ %d:%d:%d %s ] ", LocalTime->tm_hour > 12 ? LocalTime->tm_hour - 12 : LocalTime->tm_hour, LocalTime->tm_min, LocalTime->tm_sec, LocalTime->tm_hour > 12 ? "PM" : "AM");
	}
	else{
		printf("[ %s ] ", tagText);
	}

	// Process the variadic arguments
	va_list args;
	va_start(args, LogMessage);

	// Print with formatting
	printf("\033[%d;%d;%dm", BOLD, TextColor, BG_BLACK); // Set text color
	vprintf(LogMessage, args); // Format the LogMessage with the arguments
	printf("\033[0m\n"); // Reset text formatting

	va_end(args);
}