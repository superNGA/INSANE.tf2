#pragma once

#include "IMatRenderContext.h"

#include "../../Utility/Interface Handler/Interface.h"
#include "../../Utility/Signature Handler/signatures.h"

class ITexture;
class KeyValues;
class IMaterial;
class IMaterialSystem;

// Functions
MAKE_SIG(CMaterialSystem_CreateNamedRenderTargetTextureEx, "48 89 5C 24 ? 48 89 6C 24 ? 48 89 74 24 ? 57 41 56 41 57 48 83 EC ? 8B 9C 24",                   MATERIALSYSTEM_DLL, ITexture*, void*, const char*, int, int, int, int, int, int, int)
MAKE_SIG(CMaterialSystem_GetRenderContext,                 "48 89 5C 24 ? 48 89 74 24 ? 57 48 83 EC ? 48 8B F9 48 81 C1",                                    MATERIALSYSTEM_DLL, IMatRenderContext*, void*)
MAKE_SIG(CMaterialSystem_ClearBuffer,                      "48 89 5C 24 ? 48 89 6C 24 ? 48 89 74 24 ? 57 48 83 EC ? 48 8B D9 41 0F B6 F9",                   MATERIALSYSTEM_DLL, void*, void*, bool, bool, bool)
MAKE_SIG(CMaterialSystem_GetBackBufferFormat,              "48 8B 0D ? ? ? ? 48 8B 01 48 FF 60 ? CC CC 48 8B 0D ? ? ? ? 48 8B 01 48 FF 60 ? CC CC 48 83 EC", MATERIALSYSTEM_DLL, int64_t, int)


MAKE_SIG(CMateiralSystem_FindTexture,    "40 55 57 41 55",                       MATERIALSYSTEM_DLL, void*,	     void*,            const char*, const char*, bool,     int)
MAKE_SIG(CMaterialSystem_FindMaterial,   "48 83 EC ? 48 8B 44 24 ? 4C 8B 11",    MATERIALSYSTEM_DLL, IMaterial*, void*,            const char*, const char*, bool, const char*)
MAKE_SIG(CMaterialSystem_CreateMaterial, "48 89 5C 24 ? 57 48 83 EC ? 48 8B C2", MATERIALSYSTEM_DLL, IMaterial*, IMaterialSystem*, const char*, KeyValues*)
MAKE_SIG(CMaterialSystem_FirstMaterial,  "0F B7 81 ? ? ? ? 4C 8B C9",            MATERIALSYSTEM_DLL, unsigned short, void*)
MAKE_SIG(CMaterialSystem_NextMaterial,   "48 81 C1 ? ? ? ? E9 ? ? ? ? CC CC CC CC 89 15", MATERIALSYSTEM_DLL, unsigned short, void*, unsigned short)
MAKE_SIG(CMaterialSystem_GetMaterial,    "0F B7 C2 48 8D 14 40",                 MATERIALSYSTEM_DLL, IMaterial*, void*, unsigned short)

class IMaterialSystem
{
public:
    ///////////////////////////////////////////////////////////////////////////
    inline void* ClearBuffer(bool bClearColor, bool bClearDepth, bool bClearStencil = false) 
    { 
        return Sig::CMaterialSystem_ClearBuffer(this, bClearColor, bClearDepth, bClearStencil);
    }


    ///////////////////////////////////////////////////////////////////////////
    inline void* FindTexture(const char* szTextureName, const char* szType, bool bComplain = true, int iAdditionalFlags = 0)
    {
        return Sig::CMateiralSystem_FindTexture(this, szTextureName, szType, bComplain, iAdditionalFlags);
    }
    

    ///////////////////////////////////////////////////////////////////////////
    inline IMatRenderContext* GetRenderContext() { return Sig::CMaterialSystem_GetRenderContext(this); }


    ///////////////////////////////////////////////////////////////////////////
    inline void* CreateRenderTarget( int w, int h, int sizeMode, int format, int depth)
    {
        return 
            Sig::CMaterialSystem_CreateNamedRenderTargetTextureEx(this, NULL, w, h, sizeMode, format, depth, 4 | 8, 0);
    }


    ///////////////////////////////////////////////////////////////////////////
    inline ITexture* CreateRenderTarget(const char* szTargetName, int w, int h, int sizeMode, int format, int depth)
    {
        return 
            Sig::CMaterialSystem_CreateNamedRenderTargetTextureEx(this, szTargetName, w, h, sizeMode, format, depth, 4 | 8, 0);
    }


    ///////////////////////////////////////////////////////////////////////////
    // Find Materials
    inline IMaterial* FindMaterial(const char* szMaterialName, const char* szTextureGroupName, bool bComplain, const char* szCommonPrefix)
    {
        return Sig::CMaterialSystem_FindMaterial(this, szMaterialName, szTextureGroupName, bComplain, szCommonPrefix);
    }


    ///////////////////////////////////////////////////////////////////////////
    inline IMaterial* CreateMaterial(const char* szMaterialName, KeyValues* pKV)
    {
        return Sig::CMaterialSystem_CreateMaterial(this, szMaterialName, pKV);
    }


    ///////////////////////////////////////////////////////////////////////////
    inline unsigned short FirstMaterial()
    {
        return Sig::CMaterialSystem_FirstMaterial(this);
    }


    ///////////////////////////////////////////////////////////////////////////
    inline unsigned short NextMaterial(unsigned short hMaterial)
    {
        return Sig::CMaterialSystem_NextMaterial(this, hMaterial);
    }


    ///////////////////////////////////////////////////////////////////////////
    inline unsigned short InvalidMaterial()
    {
        return 0xFFFF;
    }


    ///////////////////////////////////////////////////////////////////////////
    inline IMaterial* GetMaterial(unsigned short hMaterial)
    {
        return Sig::CMaterialSystem_GetMaterial(this, hMaterial);
    }
};

MAKE_INTERFACE_VERSION(iMaterialSystem, "VMaterialSystem082", IMaterialSystem, MATERIALSYSTEM_DLL)