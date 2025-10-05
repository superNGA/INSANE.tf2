#pragma once

#include <chrono>
#include <vector>
#include "../../FeatureHandler.h"

class BaseEntity;

///////////////////////////////////////////////////////////////////////////
class PlayerListV2_t
{
public:
    PlayerListV2_t();

    void Draw();
    void SetVisible(bool bVisible);

private:
    bool m_bVisible = true;

    void _DrawList(float x, float y, float flWidth, const std::vector<BaseEntity*>* vecPlayers, bool bGrowUpwards);
    void _DrawEntSetting(BaseEntity* pEnt, std::string szPopupId);

    std::chrono::high_resolution_clock::time_point m_lastResetTime;
    float m_flAnimation = 0.0f;
};


DECLARE_CUSTOM_OBJECT(playerListV2, PlayerListV2_t, Render)
