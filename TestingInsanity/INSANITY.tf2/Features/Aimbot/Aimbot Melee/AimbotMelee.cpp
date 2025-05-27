#include "AimbotMelee.h"

#include "../AimbotHelper.h"

// SDK
#include "../../../SDK/class/Source Entity.h"
#include "../../../SDK/class/BaseWeapon.h"
#include "../../../SDK/class/IVDebugOverlay.h"
#include "../../../SDK/class/FileWeaponInfo.h"
#include "../../../SDK/class/CCollisionProperty.h"
#include "../../../SDK/class/Basic Structures.h"
#include "../../../SDK/class/CUserCmd.h"
#include "../../../SDK/class/IEngineTrace.h"
#include "../../../SDK/TF object manager/TFOjectManager.h"
#include "../../../Extra/math.h"

// NOTES
/*
-> DoSwingTraceInternal : determines what this swing hit.
-> But what do we AIM at? head? torso? What? 
-> We know that "MELEE USES PROJECTILE HITBOX" so its a big ass box, which doesn't rotate.
-> the HitBox needs to be colliding with the "HULL" for that hit to count. 

--> Lets try calculating the hull size, and try to draw it using IVDebugOverlay? How does that sound? Should be that hard.
--> After this, we can try to figure out "What to aim at?"
--> Then at last, the prediction part.

-> We will need to do some maths to find out the closest point on the enemy that could take 
*/

constexpr float SWING_RANGE_MULTIPLIER = 1.2f;

//=========================================================================
//                     PUBLIC METHODS
//=========================================================================
void AimbotMelee_t::Run(BaseEntity* pLocalPlayer, baseWeapon* pActiveWeapon, CUserCmd* pCmd, bool* pSendPacket)
{
    I::IDebugOverlay->ClearAllOverlays();

    // Drawing Melee Range Circle
    if (TempFeatureHelper::MeleeRange_Circle.IsActive() == true)
        _DrawMeleeSwingRadius(pLocalPlayer, pActiveWeapon);

    // Drawing EyePos ( Bullet origin )
    if (TempFeatureHelper::MeleeEyePos.IsActive() == true)
        _DrawEyePos(pLocalPlayer, pActiveWeapon);

    // Drawing Hull ( Effective Range )
    if (TempFeatureHelper::MeleeRange_HULL.IsActive() == true)
        _DrawMeleeHull(pLocalPlayer, pActiveWeapon, pCmd);
}


//=========================================================================
//                     PRIVATE METHODS
//=========================================================================
float AimbotMelee_t::_GetSwingRange(BaseEntity* pLocalPlayer, baseWeapon* pActiveWeapon)
{
    float flSwingRange = 0.0f;

    if (pLocalPlayer->InCond(TF_COND_SHIELD_CHARGE) == true)
    {
        flSwingRange = 128.0f;
    }
    else
    {
        int iIsSword = 0;
        pActiveWeapon->CALL_ATRIB_HOOK_INT(iIsSword, "is_a_sword");
        
        if (iIsSword == 0)
            flSwingRange = 48.0f;
        else
            flSwingRange = 72.0f;
    }

    return flSwingRange;
}

bool AimbotMelee_t::_ShouldSwing(const BaseEntity* pLocalPlayer, const float flSwingRange)
{
    const auto& aimbotTargets = FeatureObj::aimbotHelper.GetAimbotTargetData();

    auto ShouldSwingForThisGroup = [&](const std::vector<BaseEntity*>& vecTargets)->bool
        {
            for (BaseEntity* pEnt : vecTargets)
            {
                auto* pCollidable            = static_cast<CCollisionProperty*>(pEnt->GetCollideable());
                const vec& vWorldSpaceCenter = *pCollidable->WorldSpaceCenter();   // <- WARNING : CRASHING RIGHT HERE !!!

                if (vWorldSpaceCenter.DistTo(pLocalPlayer->getLocalEyePos()) < flSwingRange)
                    return true;
            }

            return false;
        };

    return
        ShouldSwingForThisGroup(aimbotTargets.m_vecEnemyPlayers)   ||
        ShouldSwingForThisGroup(aimbotTargets.m_vecEnemyBuildings) ||
        ShouldSwingForThisGroup(aimbotTargets.m_vecEnemySentry)    ||
        ShouldSwingForThisGroup(aimbotTargets.m_vecEnemyProjectiles);
}

void AimbotMelee_t::_DrawMeleeHull(BaseEntity* pLocalPlayer, baseWeapon* pActiveWeapon, CUserCmd* pCmd)
{
    static vec vMeleeHullMinBase(-18.0f, -18.0f, -18.0f);
    static vec vMeleeHullMaxBase( 18.0f,  18.0f,  18.0f);

    float flBoundScale = 1.0f;
    pActiveWeapon->CALL_ATRIB_HOOK_FLOAT(flBoundScale, "melee_bounds_multiplier");

    // Compensating for Bound Scale
    vec vMeleeHullMin = vMeleeHullMinBase * flBoundScale;
    vec vMeleeHullMax = vMeleeHullMaxBase * flBoundScale;

    // Compensating for Model Scale
    float flSwingRange = _GetSwingRange(pLocalPlayer, pActiveWeapon);
    float flModelScale = pLocalPlayer->GetModelScale();
    if (flModelScale > 1.0f)
    {
        flSwingRange  *= flModelScale;
        vMeleeHullMin *= flModelScale;
        vMeleeHullMax *= flModelScale;
    }

    // Adding Atribute Melee range
    pActiveWeapon->CALL_ATRIB_HOOK_FLOAT(flSwingRange, "melee_range_multiplier");
    
#define DEBUG_HULL_SIZE false
#if (DEBUG_HULL_SIZE == true)
    printf("Mins : %.2f %.2f %.2f\n", vMeleeHullMin.x, vMeleeHullMin.y, vMeleeHullMin.z);
    printf("vMeleeHullMaxBase : %.2f %.2f %.2f\n", vMeleeHullMinBase.x, vMeleeHullMinBase.y, vMeleeHullMinBase.z);
    printf("Maxs : %.2f %.2f %.2f\n", vMeleeHullMax.x, vMeleeHullMax.y, vMeleeHullMax.z);
    printf("vMeleeHullMaxBase : %.2f %.2f %.2f\n", vMeleeHullMaxBase.x, vMeleeHullMaxBase.y, vMeleeHullMaxBase.z);
    printf("ModelScale : %.2f | BoundScale : %.2f\n", flModelScale, flBoundScale);
#endif

    vec vForward;
    Maths::AngleVectors(pCmd->viewangles, &vForward);
    const vec vSwingStart = pLocalPlayer->getLocalEyePos();
    const vec vSwingEnd   = vSwingStart + (vForward * flSwingRange);

    bool bDidHullHit = [&]()->bool
        {
            trace_t trace;
            i_trace_filter filter(pLocalPlayer->GetCollideable()->GetEntityHandle()); // This is a make shift filter, make a proper one ( or maybe not? IDK )

            I::EngineTrace->UTIL_TraceHull(
                vSwingStart, vSwingEnd,         // Swing Start & End 
                vMeleeHullMin, vMeleeHullMax,   // Hull Mins & Max
                MASK_SOLID, &filter, &trace     // Filters n shit
            );

            return trace.did_hit();
        }();

    // Drawing Hull
    I::IDebugOverlay->AddSweptBoxOverlay(
        vSwingStart, vSwingEnd,         // Swing Start & End
        vMeleeHullMin, vMeleeHullMax,   // Hull Mins and Maxes
        pCmd->viewangles,               // Orientation
        bDidHullHit ? 0 : 255, bDidHullHit ? 255 : 0, 0, 50, 10.0f); // Color, Alpha & duration
}

void AimbotMelee_t::_DrawMeleeSwingRadius(BaseEntity* pLocalPlayer, baseWeapon* pActiveWeapon)
{
    I::IDebugOverlay->AddCircleOverlay(
        pLocalPlayer->GetAbsOrigin(),                                         // Circles base
        vec(0.0f, 0.0f, 1.0f),                                                // points straight up!
        _GetSwingRange(pLocalPlayer, pActiveWeapon) * SWING_RANGE_MULTIPLIER, // This is not the real range.
        20,                 // Segments / Smoothness
        255, 255, 0, 255,   // Color
        false,
        10.0f,
        DEG2RAD(pLocalPlayer->GetAbsAngles().yaw));
}

void AimbotMelee_t::_DrawEyePos(BaseEntity* pLocalPlayer, baseWeapon* pActiveWeapon)
{
    vec BOX_SIZE_EYE_POS(5.0f, 5.0f, 5.0f);

    I::IDebugOverlay->AddBoxOverlay(
        pLocalPlayer->getLocalEyePos(), // <- Eye Pos
        BOX_SIZE_EYE_POS,               // Box mins
        BOX_SIZE_EYE_POS * -1.0f,       // Box maxs
        pLocalPlayer->GetAbsAngles(),   // Box's orientation
        255, 255, 255, 50, 10.0f);       // Color, alpha and duration
}