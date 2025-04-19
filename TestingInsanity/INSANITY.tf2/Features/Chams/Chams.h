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
#include "../../Hooks/DrawModelExecute/DrawModelExecute.h"

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
    bool        _ApplyChams(DrawModelState_t* pModelState, IMaterial* pChamMaterial, ChamSetting_t& pChamConfig);
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

    const char* szMat03 = R"(VertexLitGeneric
    {
        "$basetexture" "vgui/white"
        "$ignorez" "0"
        "$envmap" "env_cubemap"
        "$alpha" "0.5" // 0.0 = fully invisible, 1.0 = fully solid
        "$phong" "1"
        "$phongexponent" "10"
        "$phongboost" "2"
    }
)";
};
extern Chams_t chams;