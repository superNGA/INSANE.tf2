#include "Utility.h"

//-----------------------------------------------------------
// PUBLIC function definitions
//-----------------------------------------------------------
uintptr_t Utility::FindPattern(const char* Signature, const char* MoudleName)
{
	//Checking if the Module is valid...
	uintptr_t Base = (uintptr_t)GetModuleHandle(MoudleName);
	if (!Base)
		return 0;

	//checking if console is allocated
	bool IsConsoleAllocated = GetConsoleWindow() != NULL;

	//Formating the signature...
	std::string Mask;
	std::vector<BYTE> reinterpretedSignature = StringToByte(Signature, Mask);

	//Getting the module size, Not getting the module size will result in reading ristricted regions causing a crash...
	uintptr_t ModuleSize;
	if (!GetMoudleInfo(MoudleName, ModuleSize) && IsConsoleAllocated)
		printf("Failed to get Module size!\n");
	
	//Printing Debug information only if CONSOLE is allocated
	if (IsConsoleAllocated)
	{
		printf("Signature     : %s\n", Signature); // <- Printing original signature
		
		//Printing Formated Signature
		printf("Formated Sig. : ");
		for (BYTE x : reinterpretedSignature) printf("%02X ", x);
		printf("\n");
		for (char x : Mask) printf("%c ", x); // <- Printing Mask
		printf("\n Base Adrs -> 0x%p\n MoudleSize -> 0x%ld\n", Base, ModuleSize); // <- printing Base Adrs and Module size...
	}

	//Returning the Siganture address...
	return MemoryScanner(reinterpretedSignature, Mask, ModuleSize, Base);
}

void* Utility::GetInterface(const char* InterfaceName, const char* MouduleName, int* ReturnCode)
{
	T_CreateInterface* CreateInterface = (T_CreateInterface*)GetProcAddress(GetModuleHandle(MouduleName), CREATE_INTERFACE); //Pointer to CreateInterface Function

	void* InterfacePntr = CreateInterface(InterfaceName, ReturnCode);

	return InterfacePntr;
}

void** GetVirtualTable(void* InterfacePntr)
{
	void** VirtualTable = *(void***)InterfacePntr;
	return VirtualTable;
}

//-----------------------------------------------------------
// PRIVATE function definitions -> these are just helper function for public functions
//-----------------------------------------------------------
int Utility::AdjustASCII(char Input)
{
	char InputFormatted = toupper(Input);
	if (InputFormatted - '0' > 9)
		return InputFormatted - '7';
	else
		return InputFormatted - '0';
}

std::vector<BYTE> Utility::StringToByte(const char* Signature, std::string& Mask)
{
	std::vector<BYTE> ByteArray;
	int SignatureLength = strlen(Signature);

	BYTE CacheBYTE = 0x00;
	int IndexSig = 0;
	while (Signature[IndexSig] != '\0')
	{
		CacheBYTE = 0x00;//resetting cache byte

		if (Signature[IndexSig] == '?')
		{
			ByteArray.push_back(WildCard); //Adding wildcard to Bytearray;
			Mask.push_back('?'); //adding wildcard to mask string
			IndexSig++;
		}
		else
		{
			CacheBYTE = (AdjustASCII(Signature[IndexSig]) * 16) + AdjustASCII(Signature[IndexSig + 1]);
			ByteArray.push_back(CacheBYTE);
			Mask.push_back('x');
			IndexSig += 2;
		}

		if (Signature[IndexSig] == '\0')
			break;

		IndexSig++;
	}

	return ByteArray;
}

bool Utility::GetMoudleInfo(const char* MouduleName,uintptr_t& ModuleSize)
{
	//Size_t can be used to represent size of something in bytes, it is just another form of integer.
	HMODULE ModuleHandle = GetModuleHandle(MouduleName);
	if (!ModuleHandle)
		return false;

	// Access NT headers to get module size
	auto dosHeader = reinterpret_cast<IMAGE_DOS_HEADER*>(ModuleHandle);
	auto ntHeaders = reinterpret_cast<IMAGE_NT_HEADERS*>(
		reinterpret_cast<uintptr_t>(ModuleHandle) + dosHeader->e_lfanew);

	ModuleSize = ntHeaders->OptionalHeader.SizeOfImage; //Storing the module size

	return true;
}

uintptr_t Utility::MemoryScanner(std::vector<BYTE> Signature, std::string Mask, uintptr_t ModuleSize, uintptr_t BaseAdrs)
{
	uintptr_t RunTimeAdrs = 0;
	int SignatureSize = Signature.size();
	for (int i = 0; i < ModuleSize - Signature.size() + 1; i++)
	{
		if (Signature[0] == *reinterpret_cast<BYTE*>(BaseAdrs + i) && Mask[0] != '?') //caught the first matching BYTE
		{
			bool TrueMatchFound = true; // TrueMatchFound is set to true by default and if it remains true till the end, return the current adrs...

			//Matching the rest of the BYTEs with signature
			for (int x = 1; x < SignatureSize; x++)
			{
				if (Mask[x] != '?' && Signature[x] != *reinterpret_cast<BYTE*>(BaseAdrs + i + x)) //if Mask Xth index is not ? and signature not matches, then not a true match!
				{
					TrueMatchFound = false;
					break;
				}
			}
			//If match found, returnig Run Time adrs...
			if (TrueMatchFound)
			{
				return BaseAdrs + i;
			}
		}
	}

	return RunTimeAdrs;
}
