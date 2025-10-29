#pragma once


///////////////////////////////////////////////////////////////////////////
// Damage multiplier for each hitbox / hitgroup. These might not be very accurate, but its accurate enough.
constexpr float DMG_HITGROUP_HEAD	  = 3.00f;
constexpr float DMG_HITGROUP_CHEST	  = 1.00f;
constexpr float DMG_HITGROUP_STOMACH  = 1.25f;
constexpr float DMG_HITGROUP_LEFTARM  = 1.00f;
constexpr float DMG_HITGROUP_RIGHTARM = 1.00f;
constexpr float DMG_HITGROUP_LEFTLEG  = 0.75f;
constexpr float DMG_HITGROUP_RIGHTLEG = 0.75f;
constexpr float DMG_HITGROUP_GEAR     = 1.00f;
///////////////////////////////////////////////////////////////////////////


///////////////////////////////////////////////////////////////////////////
// Hitbox index for each player entities. ( same for all clases )
enum HitboxPlayer_t
{
    HitboxPlayer_Head = 0,

    // Torso...
    HitboxPlayer_Hip,
    HitboxPlayer_SpineLower,
    HitboxPlayer_SpineMiddle,
    HitboxPlayer_SpineUpper,
    HitboxPlayer_SpineTop,

    // Left Arm...
    HitboxPlayer_LeftUpperArm,
    HitboxPlayer_LeftForearm,
    HitboxPlayer_LeftHand,

    // Right Arm...
    HitboxPlayer_RightUpperArm,
    HitboxPlayer_RightForearm,
    HitboxPlayer_RightHand,

    // Left Leg...
    HitboxPlayer_LeftUpperLeg,
    HitboxPlayer_LeftLowerLeg,
    HitboxPlayer_LeftFoot,

    // Right Leg...
    HitboxPlayer_RightUpperLeg,
    HitboxPlayer_RightLowerLeg,
    HitboxPlayer_RightFoot,

    HitboxPlayer_Count

    // Only for scout & pyro
    //HitboxPlayer_Backpack
};
///////////////////////////////////////////////////////////////////////////