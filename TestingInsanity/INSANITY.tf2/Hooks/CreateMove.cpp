#include <intrin.h>

//======================= Internal stuff =======================
#include "../SDK/FN index Manager/FN index manager.h"
#include "../Extra/math.h"
#include "../Utility/Signature Handler/signatures.h"
#include "../Utility/Hook Handler/Hook_t.h"
#include "../Utility/ConsoleLogging.h"
#include "../Utility/Profiler/Profiler.h"
#include "../Utility/ClassIDHandler/ClassIDHandler.h"
#include "../Utility/CVar Handler/CVarHandler.h"
#include "../SDK/TF object manager/TFOjectManager.h"
#include "../Features/Graphics Engine/Graphics Engine/GraphicsEngine.h"

//======================= Features =======================
#include "../Features/Movement/Movement.h"
#include "../Features/NoSpread/NoSpreadV2.h" // VENGENCE !
#include "../Features/NoSpread/NoSpread.h"
#include "../Features/TickManip/TickManipHelper.h"
#include "../Features/CritHack/CritHack.h"
#include "../Features/Aimbot/AimbotHelper.h"
#include "../Features/MovementSimulation/MovementSimulation.h"
#include "../Features/Tick Shifting/TickShifting.h"
#include "../Features/Entity Iterator/EntityIterator.h"
#include "../Features/ESP/ESPV2.h"

//======================= SDK =======================
#include "../SDK/class/CUserCmd.h"
#include "../SDK/class/BaseWeapon.h"
#include "../SDK/class/Source Entity.h"
#include "../SDK/class/CommonFns.h"
#include "../SDK/class/IVEngineClient.h"
#include "../SDK/class/IVDebugOverlay.h"
#include "../SDK/class/ISurface.h"
#include "../SDK/class/IEngineTrace.h"
#include "../SDK/class/IMaterialSystem.h"
#include "../SDK/class/IMaterial.h"


/*
I'm INSANE motha fuka, not a bitch ass niga like yourself. shut the fuck up. I will burn this fucking world down
just to watch this fuckign world burn down. You don't dare put your pathetic name in the same line with my name. 
*/

/*
* Priority TODO : 
        * Anti-Aim ( legit and rage )
        * Anti-Aim chams.
        * Decide on how the chocked ticks would be tracked.
        * Projectiles.
* 
* TODO's : 
        * Crit Hack ( rapid fire and reliable )
        * Projectile aimbot. ( absolute one )
        * Spash bot ( good & efficient one )
        * Melee aimbot ( very good one, shouldn't miss even once )
        * Anti Aim ( a good one again )
        * Resolver ( I don't even know how to get started on this shit. )
        * Enemy index arrow.
        * Radar.
        * custom sky box
        * world lighting.
        * Fake lag ( With absolute chocked tick tracking )
        * 
        * and so on...
*/


MAKE_HOOK(CreateMove, "40 53 48 83 EC ? 0F 29 74 24 ? 49 8B D8", __fastcall, CLIENT_DLL, bool,
    int64_t a1, int64_t a2, CUserCmd* pCmd)
{
    bool result = Hook::CreateMove::O_CreateMove(a1, a2, pCmd);

    if ( pCmd == nullptr || pCmd->command_number == 0)
        return result;

    // Getting Local Player
    BaseEntity* pLocalPlayer = I::IClientEntityList->GetClientEntity(I::iEngine->GetLocalPlayer());
    if (pLocalPlayer == nullptr)
        return result;

    // Getting Active Weapon
    baseWeapon* pActiveWeapon = pLocalPlayer->getActiveWeapon();
    if (pActiveWeapon == nullptr)
        return result;

    // are we alive ?
    if (pLocalPlayer->m_lifeState() != lifeState_t::LIFE_ALIVE)
        return result;

    // --> bSendPacket <--
    /*
    We moved 0x128 bytes down in the stack frame to find the bSendPacket 
    that was pushed onto the stack by IBaseClientDLL Createmove
    */
    uintptr_t pStackFrameStart_ClientModeShared_Createmove = (uintptr_t)_AddressOfReturnAddress();
    pStackFrameStart_ClientModeShared_Createmove += 0x8; // compensating for the adrs of start adrs
    
    // CInput createmove's stack
    pStackFrameStart_ClientModeShared_Createmove += 0xB0; // 0xB0 bytes for local variables allocated on stack
    pStackFrameStart_ClientModeShared_Createmove += 0x8;  // r15
    pStackFrameStart_ClientModeShared_Createmove += 0x8;  // r14
    pStackFrameStart_ClientModeShared_Createmove += 0x8;  // r12
    pStackFrameStart_ClientModeShared_Createmove += 0x8;  // rdi
    pStackFrameStart_ClientModeShared_Createmove += 0x8;  // rbp
    
    // IBaseClientDLL Createmove
    pStackFrameStart_ClientModeShared_Createmove += 0x8;  // adrs of return adrs
    pStackFrameStart_ClientModeShared_Createmove += 0x40; // 0x40 bytes for local variables allocated on stack

    bool* bSendPacket = reinterpret_cast<bool*>(pStackFrameStart_ClientModeShared_Createmove);


    // Me features, me pride-n-joy :)
    PROFILER_START_SCOPE_NAMED("CreateMove");
    {
        F::entityIterator.Run (pLocalPlayer, pActiveWeapon, pCmd);
        F::tickManipHelper.Run(pLocalPlayer, pActiveWeapon, pCmd, bSendPacket, &result);
        F::aimbotHelper.Run   (pLocalPlayer, pActiveWeapon, pCmd, &result);
        F::critHack.RunV2     (pLocalPlayer, pActiveWeapon, pCmd);
        F::noSpreadV2.Run     (pLocalPlayer, pActiveWeapon, pCmd, &result, bSendPacket);
        F::movement.Run       (pLocalPlayer, pActiveWeapon, pCmd, result);
        F::tickShifter.Run    (pLocalPlayer, pActiveWeapon, pCmd, bSendPacket);
        F::esp.RunCreateMove();
    }
    PROFILER_END_SCOPE_NAMED("CreateMove");


    return result;
}
