//=========================================================================
//                      BONE INDEX MANAGER
//=========================================================================
// by      : INSANE
// created : 25/02/2025
// 
// purpose : Caches bone indexs by comparing their names, since this 
// might change with some major update.

#include "../TF object manager/TFOjectManager.h"
#include "../../GlobalVars.h"
#include "boneManager.h"

//=========================================================================
//                     PUBLIC METHODS
//=========================================================================


//=========================================================================
// boneInfo_t* getBone(entInfo_t* ent, player_class characterModel)
//=========================================================================
/**
* caches and returns pointer to cached bone indexs
*
* @param ent : pointer to entInfo_t object for desired entity
* @param characterModel : class this entity is playing
*/
boneInfo_t* boneManager_t::getBone(entInfo_t* ent, player_class characterModel)
{
	if (ent->classID != PLAYER) {
		return &nonPlayerEntities;
	}

	void* pEnt = (void*)ent->p_ent;

	/* caching
	only done once in the entire life of software */
	if (!getBit_boneIndexCached(characterModel)) {

		boneInfo_t CHE_boneInfo;
		CHE_boneInfo.head			= tfObject.lookUpBones(pEnt, "bip_head");
		CHE_boneInfo.leftShoulder	= tfObject.lookUpBones(pEnt, "bip_collar_L");
		CHE_boneInfo.rightShoulder	= tfObject.lookUpBones(pEnt, "bip_collar_R");
		CHE_boneInfo.leftFoot		= tfObject.lookUpBones(pEnt, "bip_foot_L");
		CHE_boneInfo.rightFoot		= tfObject.lookUpBones(pEnt, "bip_foot_R");
		CHE_boneInfo.chest			= tfObject.lookUpBones(pEnt, "bip_spine_3");
		CHE_boneInfo.pelvis			= tfObject.lookUpBones(pEnt, "bip_pelvis");

		setBit_boneIndexCached(characterModel);

		switch (characterModel)
		{
		case TF_SCOUT:
			scoutBone = CHE_boneInfo;
			break;
		case TF_SNIPER:
			sniperBone = CHE_boneInfo;
			break;
		case TF_SOLDIER:
			soldierBone = CHE_boneInfo;
			break;
		case TF_DEMOMAN:
			demomanBone = CHE_boneInfo;
			break;
		case TF_MEDIC:
			medicBone = CHE_boneInfo;
			break;
		case TF_HEAVY:
			heavyBone = CHE_boneInfo;
			break;
		case TF_PYRO:
			pyroBone = CHE_boneInfo;
			break;
		case TF_SPY:
			spyBone = CHE_boneInfo;
			break;
		case TF_ENGINEER:
			engiBone = CHE_boneInfo;
			break;
		default:
			#ifdef _DEBUG
			cons.Log(FG_RED, "BONE MAMANGER", "Failed bone caching");
			#endif
			break;
		}

		#ifdef _DEBUG
		cons.Log(FG_GREEN, "BONE MANAGER", "Cached bone information for model : %d", characterModel);
		#endif 

	}

	/* returning pointer to required bone structs */
	switch (characterModel)
	{
	case TF_SCOUT:
		return &scoutBone;
		break;
	case TF_SNIPER:
		return &sniperBone;
		break;
	case TF_SOLDIER:
		return &soldierBone;
		break;
	case TF_DEMOMAN:
		return &demomanBone;
		break;
	case TF_MEDIC:
		return &medicBone;
		break;
	case TF_HEAVY:
		return &heavyBone;
		break;
	case TF_PYRO:
		return &pyroBone;
		break;
	case TF_SPY:
		return &spyBone;
		break;
	case TF_ENGINEER:
		return &engiBone;
		break;
	default:
		#ifdef _DEBUG
		cons.Log(FG_RED, "BONE MAMANGER", "Failed to find character model");
		#endif
		break;
	}
}