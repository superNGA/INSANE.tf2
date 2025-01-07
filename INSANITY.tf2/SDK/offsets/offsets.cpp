#include "offsets.h"

namespace offsets
{
	bool netvar_initialized = false;
	bool failed_netvar_retrival = false;
	
	T_map* netvar_map = new T_map;
};


uintptr_t offsets::get_netvar(const char* name)
{
	if (!offsets::netvar_initialized || (*offsets::netvar_map).empty())
	{
		#ifdef _DEBUG
		cons.Log("NetVar map not initialized yet", FG_RED);
		#endif
		return 0;
	}

	return (*offsets::netvar_map)[name];
}


bool offsets::initialize()
{
	if (offsets::netvar_initialized) return true;

	/* getting VClient017 interface */
	int error_code;
	IBaseClientDLL* base_client = (IBaseClientDLL*)util.GetInterface("VClient017", "client.dll", &error_code);

	/* returning if failed to get interface */
	if (error_code) return false;

	/* getting client class and error checking */
	ClientClass* all_class_linked_list = base_client->GetAllClasses();
	if (all_class_linked_list == nullptr || all_class_linked_list->m_pNext == nullptr) return false;

	/* looping througth and retriving netvars */
	int Index = 0, OldProp = 0;
	while (all_class_linked_list->m_pNext != NULL)
	{
		Index++;
		int PropCount = all_class_linked_list->m_pRecvTable->m_nProps;

		//Looping through props
		for (int i = 1; i < PropCount; i++) //Starting from 1 avoids the repeating Base class in start of every table.
		{
			const auto Prop = &all_class_linked_list->m_pRecvTable->m_pProps[i];
			if (!Prop) continue;

			(*netvar_map)[Prop->m_pVarName] = Prop->GetOffset();
		}
		all_class_linked_list = all_class_linked_list->m_pNext;
	}

	#ifdef _DEBUG
	cons.Log("Finished processing NetVars", FG_GREEN);
	#endif // _DEBUG

	offsets::netvar_initialized = true;
	return true;
}