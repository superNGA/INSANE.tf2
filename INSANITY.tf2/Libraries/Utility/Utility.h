#pragma once
#include <Windows.h>
#include <vector>
#include <cstdint>
#include <iostream>

#define WildCard 0xFF
#define CREATE_INTERFACE "CreateInterface"

/* used by input_util functions */
#define HELD_DOWN	(1<<15)
#define DONE_ONCE	(1<<0)
#define STATE		(1<<1)

//Template for Get interface function
typedef void* (__cdecl T_CreateInterface)(const char* InterfaceName, int* ReturnCode);

class Utility
{
public:
	/**Returns 0 if signature not found or invalid address
	@param Signature must be a string of IDA format
	@param Double check the module name, else cause error*/
	uintptr_t FindPattern(const char* Signature, const char* MoudleName);

	/* works with starting adrs 
	@param signature to search
	@param starting adrs for the search*/
	bool FindPattern(const char* Signature, uintptr_t startAdrs);

	/**Gets the interface pointer from source CreateInterface export function
	*@param InterfaceName -> Name of the interface.
	*@param MoudleName	  -> Name of the game module.
	*@param ReturnCode    -> 0 means successfully retrived interface, else error.
	*/
	void* GetInterface(const char* InterfaceName, const char* MouduleName, int* ReturnCode);

	/**Gets the Virtual Table from Interface Pointer
	Do prefer casting interface to class, but it is also ok :)*/
	void** GetVirtualTable(void* InterfacePntr);
private:
	/**Function tuns char to hex.
	@param Input -> character accpet its hex value in decimal.*/
	int AdjustASCII(char Input);

	/**@param Mask -> Recieve mask via refrence*/
	std::vector<BYTE> StringToByte(const char* Signature, std::string& Mask);

	/**Return Moudle size
	@param ModuleName
	@param base address will be 0 for invalid module names
	@param Module size is a uintptr_t not a size_t!*/
	bool GetMoudleInfo(const char* ModuleName, uintptr_t& ModuleSize);

	/**Return 0 if nothing found!
	Not using Viurtual Protect cause if it is gonna find it, it will find it anyways.*/
	uintptr_t MemoryScanner( std::vector<BYTE> Signature, std::string Mask, uintptr_t ModuleSize, uintptr_t BaseAdrs);

	/* works with start adrs, supposed to use for scanning in small regions. i.e. with the findPattern overload
	@param the fucking signature to scan for
	@param the fucking mask to use
	@param the fucking starting adrs */
	bool MemoryScanner(std::vector<BYTE> Signature, std::string Mask, uintptr_t startAdrs);
};


/* input detection / altering functions here */
namespace input_util
{
	/* retuns key state in different fashions 
	@param if toggle -> TRUE , output is toggles when key pressed
	@param if toggle -> FALSE, output true only when key pressed */
	inline bool key_detect(const BYTE key, const bool toggle)
	{
		static BYTE key_state_data = 0;
		switch (toggle)
		{
		case true:
			if (GetAsyncKeyState(key) & HELD_DOWN)
			{
				if (!(key_state_data & DONE_ONCE))
				{
					key_state_data ^= (STATE | DONE_ONCE); // Toggling state bit & setting done once bit to true
					return key_state_data & STATE; //returning state bit
				}
			}
			else
			{
				key_state_data &= ~DONE_ONCE; // setting done once bit to false
				return key_state_data & STATE; // returning state bit
			}
			break;
		default:
			if (GetAsyncKeyState(key) & HELD_DOWN) return true;
			return false;
		}
	}
}