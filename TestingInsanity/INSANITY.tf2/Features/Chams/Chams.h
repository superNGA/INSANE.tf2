//=========================================================================
//                      CHAMS MANAGMENT SYSTEM
//=========================================================================
// by      : INSANE
// created : 24/03/2025
// 
// purpose : Handles chams :)
//-------------------------------------------------------------------------
#pragma once
#include <unordered_map>
#include <string>
#include "../../SDK/class/Basic Structures.h"
#include "../../SDK/class/IMaterial.h"
#include "../FeatureHandler.h"

/*
* DONE :
* -> use CreateMaterial logic instead of yanking material out of a file
* -> put chams on the player's items like guns and secondaries.
* -> FIX MENU !!! , so we can actually use the fucking features!
* -> allow different color / materials chams on active weapons and on-ground weapons
* -> allow different color / materials chams on Medkits and Ammo boxes
* -> allow chams with and without materials
* -> team and enemy diff in all chams
* -> custom mats for Arms and Active weapon
* ( this feature doesn't seem to cause much performance issue. just using 0.5 ms approax)
* -> Getting some CRITICAL performance issues. Fix & get to the cause!
* -> make a proper material creation logic.
* -> FREE all materials when terminating.
*/

/*
* TODO : 
* FINAL TASKS : ( move to no-spread or something like that after this )
* -> make a argument structure for _CreateMaterial Fn & Find a method to showCase / preview 
*    a Material.
* 
* -> Imcorporate the ConsoleSystem V2 in software.
* -> deal with dropped weapon chams, maybe remove or give user control. likely causing
*    performance complications.
* -> Make something that could take in properties and create a trully custom material
*    accoring to that. Don't hardCode specs in the _CreateMaterial Fn.
* -> Make some logic to let user create and save materials with a very wide variety
*    of options. ( or maybe we can do that later when more important features are done ?)
* -> get & Add some cool material options.
* -> Create custom Material at run time and preview ?? How does that sound.
* 
* -> soetimes translucent ignoreZ ? fix that 
* -> document content well / cleanUp this bullshit and make it " propa' "
* 
* -> software menu show mouse without using escape menu.
* -> Look into world material modulation, and put that rijin mateiral if you can.
*/

#define MAX_MATERIAL_NAME_SIZE 32
#define MAX_PROP_NAME 32
struct Material_t
{
    IMaterial* pMaterial                    = nullptr;
    KeyValues* pKV                          = nullptr;
    char szMatName[MAX_MATERIAL_NAME_SIZE]  = "NOT DEFINED";
};


union data_t
{
    int32_t iData;
    float flData;
    char szData[MAX_PROP_NAME];
    TFclr_t clrData;
}; 
struct MatProp_t
{
    MatProp_t() : szPropName(""), dataType(TYPE_NONE) {}
    char szPropName[MAX_PROP_NAME];
    data_t data;
    types_t dataType;
};

class Chams_t
{
public:
    int64_t     Run(void* pVTable, DrawModelState_t* modelState, ModelRenderInfo_t* renderInfo, matrix3x4_t* boneMatrix);
    bool        FreeAllMaterial();

private:
    bool        _ApplyChams(DrawModelState_t* pModelState, IMaterial* pChamMaterial, bool bIgnoreZ, bool bChams, const float* pChamClrs, const float flAlpha);
    bool        _IsAmmoPack(uint32_t iHash);
    bool        _IsMedKit(uint32_t iHash);

    TFclr_t     _GetClrFromString(std::string szColorString);
    std::string _GetMaterialType(const char* szMaterialVMT);
    types_t     _GetMatPropDataType(data_t & dataOut, std::string szData);
    bool        _GetMaterialPropVector(std::vector<MatProp_t>& vecMatPropOut, const char* szMaterialVMT);

    bool        _CreateMaterial(std::string szMatName, const char* szMaterialVMT);
    bool        _DeleteMaterial(std::string szMatName);
    
    // this stores all custom made materials, reason for using a map is so I can 
    // scale is easily in future.
    std::unordered_map<std::string, Material_t*> UM_materials;

//=========================================================================
//                     MATERIALS ( FOR NOW )
//=========================================================================
    const char* szMat01 = R"(UnlitGeneric
    {
        "$ignorez" "1"
    })";
    
    const char* szMat02 = R"(VertexLitGeneric
    {
        "$basetexture" "vgui/white"
        "$envmap" "env_cubemap"
        "$envmaptint" "[1 1 1]"
        "$ignorez" "1"
        "$envmapfresnel" "1"
        "$phong" "1"
        "$phongexponent" "20"
        "$phongboost" "2"
    })";

    const char* szMat03 = R"(UnlitGeneric
    {
        "$basetexture" "vgui/white"
        "$ignorez" "1"
        "$envmap" "env_cubemap"
        "$alpha" "0.5"
        "$phong" "1"
        "$phongexponent" "10"
        "$phongboost" "2"
    }
)";
};
extern Chams_t chams;

DEFINE_TAB(Chams, 5)
DEFINE_SECTION(Player, "Chams", 1)

DEFINE_FEATURE(Enemy_IgnoreZ,                   bool,           Player,   Chams, 1, false)
DEFINE_FEATURE(Enemy_Chams,                     bool,           Player,   Chams, 2, false,                    FeatureFlag_SupportKeyBind | FeatureFlag_HoldOnlyKeyBind)
DEFINE_FEATURE(EnemyChams_Color,                ColorData_t,    Player,   Chams, 3, ColorData_t(),            FeatureFlag_SupportKeyBind | FeatureFlag_HoldOnlyKeyBind)                                                                                                                 
DEFINE_FEATURE(Friendly_IgnoreZ,                bool,           Player,   Chams, 4, false)                    
DEFINE_FEATURE(Friendly_Chams,                  bool,           Player,   Chams, 5, false,                    FeatureFlag_SupportKeyBind | FeatureFlag_HoldOnlyKeyBind)
DEFINE_FEATURE(FriendlyChams_Color,             ColorData_t,    Player,   Chams, 6, ColorData_t(),            FeatureFlag_SupportKeyBind | FeatureFlag_HoldOnlyKeyBind)

DEFINE_SECTION(Building, "Chams", 2)

// SENTRY
DEFINE_FEATURE(EnemySentry_IgnoreZ,             bool,           Building,     Chams, 1, false)
DEFINE_FEATURE(EnemySentry_Chams,               bool,           Building,     Chams, 2, false,                FeatureFlag_SupportKeyBind | FeatureFlag_HoldOnlyKeyBind)
DEFINE_FEATURE(EnemySentryChams_Color,          ColorData_t,    Building,     Chams, 3, ColorData_t(),        FeatureFlag_SupportKeyBind | FeatureFlag_HoldOnlyKeyBind)
                                                                 
DEFINE_FEATURE(FriendlySentry_IgnoreZ,          bool,           Building,     Chams, 4, false)
DEFINE_FEATURE(FriendlySentry_Chams,            bool,           Building,     Chams, 5, false,                FeatureFlag_SupportKeyBind | FeatureFlag_HoldOnlyKeyBind)
DEFINE_FEATURE(FriendlySentryChams_Color,       ColorData_t,    Building,     Chams, 6, ColorData_t(),        FeatureFlag_SupportKeyBind | FeatureFlag_HoldOnlyKeyBind)
                                                                 
// DISPENSER                                                     
DEFINE_FEATURE(EnemyDispenser_IgnoreZ,          bool,           Building,     Chams, 7, false)
DEFINE_FEATURE(EnemyDispenser_Chams,            bool,           Building,     Chams, 8, false,                FeatureFlag_SupportKeyBind | FeatureFlag_HoldOnlyKeyBind)
DEFINE_FEATURE(EnemyDispenserChams_Color,       ColorData_t,    Building,     Chams, 9, ColorData_t(),        FeatureFlag_SupportKeyBind | FeatureFlag_HoldOnlyKeyBind)
                                                                 
DEFINE_FEATURE(FriendlyDispenser_IgnoreZ,       bool,           Building,     Chams, 10, false)
DEFINE_FEATURE(FriendlyDispenser_Chams,         bool,           Building,     Chams, 11, false,               FeatureFlag_SupportKeyBind | FeatureFlag_HoldOnlyKeyBind)
DEFINE_FEATURE(FriendlyDispenserChams_Color,    ColorData_t,    Building,     Chams, 12, ColorData_t(),       FeatureFlag_SupportKeyBind | FeatureFlag_HoldOnlyKeyBind)
                                                             
// TELEPORTER                                                
DEFINE_FEATURE(EnemyTeleporter_IgnoreZ,         bool,           Building,     Chams, 13, false)
DEFINE_FEATURE(EnemyTeleporter_Chams,           bool,           Building,     Chams, 14, false,               FeatureFlag_SupportKeyBind | FeatureFlag_HoldOnlyKeyBind)
DEFINE_FEATURE(EnemyTeleporterChams_Color,      ColorData_t,    Building,     Chams, 15, ColorData_t(),       FeatureFlag_SupportKeyBind | FeatureFlag_HoldOnlyKeyBind)
                                                                 
DEFINE_FEATURE(FriendlyTeleporter_IgnoreZ,      bool,           Building,     Chams, 16, false)
DEFINE_FEATURE(FriendlyTeleporter_Chams,        bool,           Building,     Chams, 17, false,               FeatureFlag_SupportKeyBind | FeatureFlag_HoldOnlyKeyBind)
DEFINE_FEATURE(FriendlyTeleporterChams_Color,   ColorData_t,    Building,     Chams, 18, ColorData_t(),       FeatureFlag_SupportKeyBind | FeatureFlag_HoldOnlyKeyBind)

DEFINE_SECTION(Projectile, "Chams", 3)

DEFINE_FEATURE(EnemyProjectile_IgnoreZ,         bool,           Projectile,   Chams, 1, false)
DEFINE_FEATURE(EnemyProjectile_Chams,           bool,           Projectile,   Chams, 2, false,                  FeatureFlag_SupportKeyBind | FeatureFlag_HoldOnlyKeyBind)
DEFINE_FEATURE(EnemyProjectileChams_Color,      ColorData_t,    Projectile,   Chams, 3, ColorData_t(),          FeatureFlag_SupportKeyBind | FeatureFlag_HoldOnlyKeyBind)
                                                                 
DEFINE_FEATURE(FriendlyProjectile_IgnoreZ,      bool,           Projectile,   Chams, 4, false)
DEFINE_FEATURE(FriendlyProjectile_Chams,        bool,           Projectile,   Chams, 5, false,                  FeatureFlag_SupportKeyBind | FeatureFlag_HoldOnlyKeyBind)
DEFINE_FEATURE(FriendlyProjectileChams_Color,   ColorData_t,    Projectile,   Chams, 6, ColorData_t(),          FeatureFlag_SupportKeyBind | FeatureFlag_HoldOnlyKeyBind)

DEFINE_SECTION(Misc, "Chams", 4)

DEFINE_FEATURE(DroppedAmmoPack_IgnoreZ,         bool,           Misc,         Chams, 1, false)
DEFINE_FEATURE(DroppedAmmoPack_Chams,           bool,           Misc,         Chams, 2, false,                     FeatureFlag_SupportKeyBind | FeatureFlag_HoldOnlyKeyBind)
DEFINE_FEATURE(DroppedAmmoPackChams_Color,      ColorData_t,    Misc,         Chams, 3, ColorData_t(),             FeatureFlag_SupportKeyBind | FeatureFlag_HoldOnlyKeyBind)
                                                                 
DEFINE_FEATURE(AmmoPack_IgnoreZ,                bool,           Misc,         Chams, 4, false)
DEFINE_FEATURE(AmmoPack_Chams,                  bool,           Misc,         Chams, 5, false,                     FeatureFlag_SupportKeyBind | FeatureFlag_HoldOnlyKeyBind)
DEFINE_FEATURE(AmmoPackChams_Color,             ColorData_t,    Misc,         Chams, 6, ColorData_t(),             FeatureFlag_SupportKeyBind | FeatureFlag_HoldOnlyKeyBind)
                                                                 
DEFINE_FEATURE(Medkit_IgnoreZ,                  bool,           Misc,         Chams, 7, false)
DEFINE_FEATURE(Medkit_Chams,                    bool,           Misc,         Chams, 8, false,                     FeatureFlag_SupportKeyBind | FeatureFlag_HoldOnlyKeyBind)
DEFINE_FEATURE(MedkitChams_Color,               ColorData_t,    Misc,         Chams, 9, ColorData_t(),             FeatureFlag_SupportKeyBind | FeatureFlag_HoldOnlyKeyBind)
                                                                 
DEFINE_FEATURE(TFItem_IgnoreZ,                  bool,           Misc,         Chams, 10, false)
DEFINE_FEATURE(TFItem_Chams,                    bool,           Misc,         Chams, 11, false,                    FeatureFlag_SupportKeyBind | FeatureFlag_HoldOnlyKeyBind)
DEFINE_FEATURE(TFItemChams_Color,               ColorData_t,    Misc,         Chams, 12, ColorData_t(),            FeatureFlag_SupportKeyBind | FeatureFlag_HoldOnlyKeyBind)
                                                                 
DEFINE_FEATURE(ViewModel_IgnoreZ,               bool,           Misc,         Chams, 13, false)
DEFINE_FEATURE(ViewModel_Chams,                 bool,           Misc,         Chams, 14, false,                    FeatureFlag_SupportKeyBind | FeatureFlag_HoldOnlyKeyBind)
DEFINE_FEATURE(ViewModelChams_Color,            ColorData_t,    Misc,         Chams, 15, ColorData_t(),            FeatureFlag_SupportKeyBind | FeatureFlag_HoldOnlyKeyBind)
                                                                 
DEFINE_FEATURE(DroppedWeapon_IgnoreZ,           bool,           Misc,         Chams, 16, false)
DEFINE_FEATURE(DroppedWeapon_Chams,             bool,           Misc,         Chams, 17, false,                    FeatureFlag_SupportKeyBind | FeatureFlag_HoldOnlyKeyBind)
DEFINE_FEATURE(DroppedWeaponChams_Color,        ColorData_t,    Misc,         Chams, 18, ColorData_t(),            FeatureFlag_SupportKeyBind | FeatureFlag_HoldOnlyKeyBind)