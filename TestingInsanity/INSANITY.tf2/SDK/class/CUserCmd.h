#pragma once
#include "Basic Structures.h"

constexpr float MAX_MOVE_USERCMD = 450.0f;

enum CommandButtons : int
{
    IN_ATTACK         = (1 << 0),
    IN_JUMP           = (1 << 1),
    IN_DUCK           = (1 << 2),
    IN_FORWARD        = (1 << 3),
    IN_BACK           = (1 << 4),
    IN_USE            = (1 << 5),
    IN_CANCEL         = (1 << 6),
    IN_LEFT           = (1 << 7),
    IN_RIGHT          = (1 << 8),
    IN_MOVELEFT       = (1 << 9),
    IN_MOVERIGHT      = (1 << 10),
    IN_SECOND_ATTACK  = (1 << 11),
    IN_RUN            = (1 << 12),
    IN_RELOAD         = (1 << 13),
    IN_LEFT_ALT       = (1 << 14),
    IN_RIGHT_ALT      = (1 << 15),
    IN_SCORE          = (1 << 16),
    IN_SPEED          = (1 << 17),
    IN_WALK           = (1 << 18),
    IN_ZOOM           = (1 << 19),
    IN_FIRST_WEAPON   = (1 << 20),
    IN_SECOND_WEAPON  = (1 << 21),
    IN_BULLRUSH       = (1 << 22),
    IN_FIRST_GRENADE  = (1 << 23),
    IN_SECOND_GRENADE = (1 << 24),
    IN_MIDDLE_ATTACK  = (1 << 25)
};

class CUserCmd {
public:
    CUserCmd()
    {
        VirtualTable     = nullptr;
        command_number   = 0;
        tick_count       = 0;
        viewangles.Init();
        forwardmove      = 0.0f;
        sidemove         = 0.0f;
        upmove           = 0.0f;
        buttons          = 0;
        impulse          = 0;
        weaponselect     = 0;
        weaponsubtype    = 0;
        random_seed      = 0;
        mousedx          = 0;
        mousedy          = 0;
        hasbeenpredicted = false;
    }

    void*   VirtualTable;       // This is just padding shit. No real use     
    int     command_number;     // Sequence number of command
    int     tick_count;         // Tick when this command was created
    qangle  viewangles;         // Aim direction (pitch, yaw, roll)
    float	forwardmove;
    float	sidemove;
    float	upmove;
    int		buttons;            // Correct till here
    BYTE    impulse;
    //char    padding1[3];
    int		weaponselect;       // Changes only when I add change weapon and stays at 0 for the rest of the ticks
    int		weaponsubtype;      // Always 0, maybe cause I don't have skins ?
    //char    Padding[3];         // 3 Bytes worth of padding to align Random seed and rest of the data.
    int		random_seed;
    short	mousedx;            // Mouse movement in X | This is 0.0f at all times, doesn't change if I shoot or move mouse
    short	mousedy;            // Mouse movement in Y | This is 0.0f at all times, doesn't change if I shoot or move mouse
    bool    hasbeenpredicted;   // Predicted at least once (client-side only)
};