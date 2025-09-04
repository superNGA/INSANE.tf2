#pragma once

#include "../../FeatureHandler.h"


class PlayerList_t
{
public:
    void Draw();
};

DECLARE_CUSTOM_OBJECT(playerList, PlayerList_t, Render)