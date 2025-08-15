#pragma once

#include "Basic Structures.h"
#include "ISurface.h"
#include "../../Utility/ConsoleLogging.h"
#include "../../Utility/Signature Handler/signatures.h"

class Panel;

MAKE_SIG(VGui_Panel_MakeReadyToUse, "48 89 5C 24 ? 48 89 74 24 ? 57 48 83 EC ? 48 8B D9 E8 ? ? ? ? 48 8B 3D ? ? ? ? 48 8B CB 48 8B 03 48 8B 37 FF 10 41 B0",
    CLIENT_DLL, void, Panel*)

enum PanelFlags_t
{
    MARKED_FOR_DELETION                     = 0x0001,
    NEEDS_REPAINT                           = 0x0002,
    PAINT_BORDER_ENABLED                    = 0x0004,
    PAINT_BACKGROUND_ENABLED                = 0x0008,
    PAINT_ENABLED                           = 0x0010,
    POST_CHILD_PAINT_ENABLED                = 0x0020,
    AUTODELETE_ENABLED                      = 0x0040,
    NEEDS_LAYOUT                            = 0x0080,
    NEEDS_SCHEME_UPDATE                     = 0x0100,
    NEEDS_DEFAULT_SETTINGS_APPLIED          = 0x0200,
    IN_PERFORM_LAYOUT                       = 0x0800,
    IS_PROPORTIONAL                         = 0x1000,
    TRIPLE_PRESS_ALLOWED                    = 0x2000,
    DRAG_REQUIRES_PANEL_EXIT                = 0x4000,
    IS_MOUSE_DISABLED_FOR_THIS_PANEL_ONLY   = 0x8000,
    ALL_FLAGS                               = 0xFFFF,
};

class Panel
{
public:
    virtual vgui::VPANEL GetVPanel() const = 0;

    void MakeReadyToUse()
    {
        Sig::VGui_Panel_MakeReadyToUse(this);
    }
};