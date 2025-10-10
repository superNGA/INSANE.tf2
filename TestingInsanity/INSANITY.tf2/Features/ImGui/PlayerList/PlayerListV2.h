#pragma once

#include <vector>
#include "../../FeatureHandler.h"
#include "../AnimationHandler.h"

class BaseEntity;

///////////////////////////////////////////////////////////////////////////
class PlayerListV2_t
{
public:
    PlayerListV2_t() : m_playerListAnim() {}

    void Draw();
    void SetVisible(bool bVisible);

private:
    bool m_bVisible = true;

    void _DrawList(float x, float y, float flWidth, const std::vector<BaseEntity*>* vecPlayers, bool bGrowUpwards);
    void _DrawEntSetting(BaseEntity* pEnt, std::string szPopupId);

    AnimationHandler_t m_playerListAnim;
};


DECLARE_CUSTOM_OBJECT(playerListV2, PlayerListV2_t, Render)
