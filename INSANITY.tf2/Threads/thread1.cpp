#include "thread1.h"

#ifdef _DEBUG
	Console_System cons(FG_CYAN, BOLD, BG_BLACK);
#endif // _DEBUG
	Utility util;


void execute_thread1(HINSTANCE instance)
{
	// Initializing MinHook and Console_System & utility
	MH_Initialize();
	#ifdef _DEBUG
	cons.CreateNewConsole();
	cons.DoIntroduction();
	cons.DoDevider();
	cons.FastLog("MinHook initialized");
	#endif

	/* initializing module handles */
	if (!handle::initialize())
	{
		#ifdef _DEBUG
		cons.Log("[ error ] Failed to get moudle handles for one or all modules", FG_RED);
		#endif
	}
	#ifdef _DEBUG
	cons.Log("Successfully initialized module handles", FG_GREEN);
	#endif

	/* NetVars initializing */
	if (!initialize_netvars())
	{
		#ifdef _DEBUG
		cons.Log("[ error ] NetVar failed", FG_RED);
		#endif
	}
	#ifdef _DEBUG
	cons.Log("Successfully initialized NetVar", FG_GREEN);
	#endif

	/* hooking and enabling hooks */
	MH_CreateHook((LPVOID*)get_endscene(), (LPVOID)directX::H_endscene, (LPVOID*)&directX::O_endscene); //End scene hook
	MH_EnableHook(MH_ALL_HOOKS); //enabling all hooks
	winproc::hook_winproc(); // Hooking WinProc

	/* main cheat loop */
	while (!directX::UI::UI_has_been_shutdown)
	{
		std::this_thread::sleep_for(std::chrono::milliseconds(100));
	}

	//waiting until ImGui has been shutdown properly, in case of some errors
	while (!directX::UI::UI_has_been_shutdown)
	{
		#ifdef _DEBUG
		cons.Log("Waiting for ImGui to shutdown properly", FG_YELLOW);
		#endif // _DEBUG
		std::this_thread::sleep_for(std::chrono::milliseconds(50));
	}
	MH_DisableHook(MH_ALL_HOOKS); // disabling hooks
	MH_Uninitialize(); // Uninitializing Minhook

	#ifdef _DEBUG
	cons.Log("Removed all hooks", FG_RED);
	cons.FreeConsoleInstance();
	#endif

	FreeLibraryAndExitThread(instance, 0);
}

bool initialize_netvars()
{
	int error_code;
	IBaseClientDLL* base_client = (IBaseClientDLL*)util.GetInterface("VClient017", "client.dll", &error_code);

	/* returning if failed to get interface */
	if (error_code) return false;

	ClientClass* all_class_linked_list = base_client->GetAllClasses();
	if (all_class_linked_list == nullptr || all_class_linked_list->m_pNext == nullptr) return false;

	printf("In the clear\n");

	int Index = 0, OldProp = 0;
	while (all_class_linked_list->m_pNext != NULL)
	{
		Index++;
		int PropCount = all_class_linked_list->m_pRecvTable->m_nProps;
		printf("Index : %d, Table name : %s, Property Count : %d\n", Index, all_class_linked_list->m_pRecvTable->m_pNetTableName, PropCount);

		//Looping through props
		for (int i = 1; i < PropCount; i++) //Starting from 1 avoids the repeating Base class in start of every table.
		{
			const auto Prop = &all_class_linked_list->m_pRecvTable->m_pProps[i];

			if (!Prop) {
				continue;
			}

			offsets::netvar_map[Prop->m_pVarName] = Prop->GetOffset();
			//std::cout << Prop->m_pVarName << " -> 0x" << std::hex << Prop->GetOffset() << std::endl;
		}
		all_class_linked_list = all_class_linked_list->m_pNext;
	}

	return true;
}