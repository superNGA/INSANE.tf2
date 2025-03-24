// ! THIS HOOK IS REMOVED, NOT IN USE !

//=========================================================================
//                      PROCESS MOVEMENT HOOK
//=========================================================================
// by      : INSANE
// created : 17/03/2025
// 
// purpose : This hook is primarly used to store the pMove poitner
//    as it looks like this can help us create the airMove feature.
//-------------------------------------------------------------------------
#include "ProcessMovement.h"
#include <algorithm>
#include "../../SDK/TF object manager/TFOjectManager.h"
#include "../../SDK/Entity Manager/entityManager.h"

hook::processMovement::T_processMovement hook::processMovement::O_processMovement = nullptr;
void __fastcall hook::processMovement::H_processMovement(void* pVTable, void* pPlayer, CMoveData* pMove)
{
    // storing local players move data
    BaseEntity* pLocalPlayer = entityManager.getLocalPlayer();
    if ((void*)pLocalPlayer == nullptr || pPlayer != (void*)pLocalPlayer)
    {
        O_processMovement(pVTable, pPlayer, pMove);
        return;
    }

    tfObject.pMove.store(pMove);
    O_processMovement(pVTable, pPlayer, pMove);
}