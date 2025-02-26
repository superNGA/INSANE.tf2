//=========================================================================
//                      BONE INDEX MANAGER
//=========================================================================
// by      : INSANE
// created : 25/02/2025
// 
// purpose : Caches bone indexs by comparing their names, since this 
// might change with some major update.

#pragma once
#include "../entInfo_t.h"

class boneManager_t
{
public:
	boneInfo_t* getBone(entInfo_t* ent, player_class characterModel);

private:
	/* this is a 16-bit bit field, each booleans/bit holds whether that character models bones
	are cached or not. and if they are not cached, they will be aquired and stored. */
	int16_t BF_boneIndexCached = 0;

	/* used to toggle BF_boneIndexCached's bits */
	void setBit_boneIndexCached(player_class characterModel) {
		BF_boneIndexCached |= (1 << characterModel);
	}
	void clearBit_boneIndexCached(player_class characterModel) {
		BF_boneIndexCached &= ~(1 << characterModel);
	}
	bool getBit_boneIndexCached(player_class characterModel) {
		return BF_boneIndexCached & (1 << characterModel);
	}

	// bone indexs will be stored here after cached & won't be changed once made
	boneInfo_t scoutBone;
	boneInfo_t sniperBone;
	boneInfo_t soldierBone;
	boneInfo_t demomanBone;

	boneInfo_t medicBone;
	boneInfo_t heavyBone;
	boneInfo_t pyroBone;
	boneInfo_t engiBone;

	boneInfo_t spyBone;
	boneInfo_t nonPlayerEntities;
};
inline boneManager_t boneManager;