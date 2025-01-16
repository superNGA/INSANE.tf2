#include "offsets.h"

local_netvars netvar;

namespace offsets
{
	bool netvar_initialized = false;
	bool failed_netvar_retrival = false;
};


void recurse_maxxxing(RecvTable* table)
{
	T_map temp_map;
	for (int i = 0; i < table->m_nProps; i++)
	{
		RecvProp* prop = &table->m_pProps[i];//getting each prop
		if (!prop) continue;//only skips the current prop, but if you return it skips rest of the props

		if (prop->child_table)recurse_maxxxing(prop->child_table);	
		temp_map[prop->m_pVarName] = prop->m_Offset;
	}
	offsets::fill_local_netvars(temp_map);
}


bool offsets::initialize()
{
	if (offsets::netvar_initialized) return true;

	/* create map */
	T_map netvar_map;

	/* getting interface */
	int error_code;
	interface_tf2::base_client = (IBaseClientDLL*)util.GetInterface("VClient017", "client.dll", &error_code);

	/* iterating through all client classes */
	ClientClass* client_class = interface_tf2::base_client->GetAllClasses();
	while (client_class) // <- this iterated throught the base classes
	{
		/* get the base tables and iterate through them */
		RecvTable* base_table = client_class->m_pRecvTable;
		if (!base_table) {
			cons.Log("Bad base table", FG_RED);
			continue;
		}

		/* iterating props & checking child tables */
		for (int i = 0; i < base_table->m_nProps; i++)
		{
			RecvProp* prop = &base_table->m_pProps[i];	//getting each prop
			if (!prop) continue;					//only skips the current prop, but if you return it skips rest of the props
			netvar_map[prop->m_pVarName] = prop->m_Offset;
			if (prop->child_table) recurse_maxxxing(prop->child_table);
		}

		/* getting next table */
		client_class = client_class->m_pNext;
	}

	#ifdef _DEBUG
	cons.Log("Finished processing NetVars", FG_GREEN);
	#endif

	/* loading desired netvars in local varibles and freeing map */
	offsets::fill_local_netvars(netvar_map);

	#ifdef _DEBUG
	cons.Log("Loaded NetVars", FG_GREEN);
	#endif

	offsets::netvar_initialized = true;
	return true;
}


void offsets::fill_local_netvars(T_map& map)
{
	if (map["m_fFlags"])			netvar.m_fFlags = map["m_fFlags"];
	if (map["m_nForceTauntCam"])	netvar.m_nForceTauntCam = map["m_nForceTauntCam"];
	if (map["m_iReloadMode"])		netvar.m_iReloadMode = map["m_iReloadMode"];
	if (map["m_hActiveWeapon"])		netvar.m_hActiveWeapon = map["m_hActiveWeapon"];
}