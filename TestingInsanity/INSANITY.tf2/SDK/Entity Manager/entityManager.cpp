#include "entityManager.h"

// SDK
#include "../class/Source Entity.h"
#include "../class/IVEngineClient.h"

void EntityManager_t::UpdateLocalPlayer()
{
    auto* pLocalPlayer = I::IClientEntityList->GetClientEntity(I::iEngine->GetLocalPlayer());
    m_pLocalPlayer.store(pLocalPlayer);
}

BaseEntity* EntityManager_t::GetLocalPlayer()
{
    return m_pLocalPlayer.load();
}