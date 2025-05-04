#include "offsets.h"
//#include "../TF object manager/TFOjectManager.h"
#include "../class/I_BaseEntityDLL.h"

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
	offsets::fill_local_netvars(temp_map, table->m_pNetTableName);
}


bool offsets::initialize()
{
	if (offsets::netvar_initialized) return true;

	/* create map */
	T_map netvar_map;

	/* getting interface */
	int error_code;
	auto base_client = I::IBaseClient;

	/* iterating through all client classes */
	ClientClass* client_class = base_client->GetAllClasses();
	while (client_class) // <- this iterated throught the base classes
	{
		/* get the base tables and iterate through them */
		RecvTable* base_table = client_class->m_pRecvTable;
		if (!base_table) {
			#ifdef _DEBUG
			cons.Log("Bad base table", FG_RED);
			#endif	
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

		offsets::fill_local_netvars(netvar_map, base_table->m_pNetTableName);

		/* getting next table */
		client_class = client_class->m_pNext;
	}

	#ifdef _DEBUG
	cons.Log("Finished processing NetVars", FG_GREEN);
	#endif

	/* loading desired netvars in local varibles and freeing map */
	offsets::fill_local_netvars(netvar_map, "BASE_TABLE");

	#ifdef _DEBUG
	cons.Log("Loaded NetVars", FG_GREEN);
	#endif

	offsets::netvar_initialized = true;
	return true;
}


void offsets::fill_local_netvars(T_map& map, const char* table_name)
{
	/* matching table and var name*/
	if (!strcmp(table_name, "DT_TFPlayerClassShared")	&& map["m_iClass"])					netvar.m_iClass					= map["m_iClass"];
	if (!strcmp(table_name, "DT_EconEntity")			&& map["m_AttributeManager"])		netvar.m_AttributeManager		= map["m_AttributeManager"];
	if (!strcmp(table_name, "DT_AttributeContainer")	&& map["m_Item"])					netvar.m_Item					= map["m_Item"];
	if (!strcmp(table_name, "DT_ScriptCreatedItem")		&& map["m_iItemDefinitionIndex"])	netvar.m_iItemDefinitionIndex	= map["m_iItemDefinitionIndex"];
	if (!strcmp(table_name, "DT_TFPlayer")				&& map["m_PlayerClass"])			netvar.m_PlayerClass			= map["m_PlayerClass"];
	if (!strcmp(table_name, "DT_BasePlayer")			&& map["m_iHealth"])				netvar.m_iHealth				= map["m_iHealth"];
	if (!strcmp(table_name, "DT_BasePlayer")			&& map["m_lifeState"])				netvar.m_lifeState				= map["m_lifeState"];
	if (!strcmp(table_name, "DT_BaseEntity")			&& map["m_iTeamNum"])				netvar.m_iTeamNum				= map["m_iTeamNum"];
	if (!strcmp(table_name, "DT_LocalPlayerExclusive")	&& map["m_vecViewOffset[2]"])		netvar.m_vecViewOffset			= map["m_vecViewOffset[2]"];
	if (!strcmp(table_name, "DT_LocalPlayerExclusive")	&& map["m_vecVelocity[0]"])			netvar.m_velocity				= map["m_vecVelocity[0]"];
	if (!strcmp(table_name, "DT_TFPlayerResource")		&& map["m_iMaxHealth"])				netvar.m_iMaxHealth				= map["m_iMaxHealth"];
	if (!strcmp(table_name, "DT_TFPlayer")				&& map["m_Shared"])					netvar.m_Shared					= map["m_Shared"];
	if (!strcmp(table_name, "DT_TFPlayerShared")		&& map["m_nPlayerCond"])			netvar.m_nPlayerCond			= map["m_nPlayerCond"];
	if (!strcmp(table_name, "DT_TFWeaponKnife")			&& map["m_bReadyToBackstab"])		netvar.m_bReadyToBackstab		= map["m_bReadyToBackstab"];
	
	if (!strcmp(table_name, "DT_BaseAnimating")			&& map["m_flPoseParameter"])		netvar.m_flPoseParameter		= map["m_flPoseParameter"];
	if (!strcmp(table_name, "DT_BaseAnimating")			&& map["m_nSequence"])				netvar.m_nSequence				= map["m_nSequence"];
	if (!strcmp(table_name, "DT_ServerAnimationData")	&& map["m_flCycle"])				netvar.m_flCycle				= map["m_flCycle"];
	
	if (!strcmp(table_name, "DT_LocalPlayerExclusive")	&& map["m_nTickBase"])				netvar.m_nTickBase				= map["m_nTickBase"];
	
	if (!strcmp(table_name, "DT_TFPlayerShared")		&& map["m_iCritMult"])				netvar.m_iCritMult				= map["m_iCritMult"];
	if (!strcmp(table_name, "DT_LocalTFWeaponData")		&& map["m_flObservedCritChance"])	netvar.m_flObservedCritChance	= map["m_flObservedCritChance"];
	
	if (!strcmp(table_name, "DT_TFPlayerSharedLocal")	&& map["m_RoundScoreData"])			netvar.m_RoundScoreData			= map["m_RoundScoreData"];
	if (!strcmp(table_name, "DT_LocalActiveWeaponData")	&& map["m_flNextPrimaryAttack"])	netvar.m_flNextPrimaryAttack	= map["m_flNextPrimaryAttack"];

	/* matching var name */
	if (map["m_fFlags"])			netvar.m_fFlags			= map["m_fFlags"];
	if (map["m_nForceTauntCam"])	netvar.m_nForceTauntCam = map["m_nForceTauntCam"];
	if (map["m_iReloadMode"])		netvar.m_iReloadMode	= map["m_iReloadMode"];
	if (map["m_hActiveWeapon"])		netvar.m_hActiveWeapon	= map["m_hActiveWeapon"];
	if (map["m_bGlowEnabled"])		netvar.m_bGlowEnabled	= map["m_bGlowEnabled"];
	if (map["m_hItem"])				netvar.m_hItem			= map["m_hItem"];
}