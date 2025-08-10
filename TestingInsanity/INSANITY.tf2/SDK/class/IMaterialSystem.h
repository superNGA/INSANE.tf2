#pragma once

#include "IMatRenderContext.h"

#include "../../Utility/Interface Handler/Interface.h"
#include "../../Utility/Signature Handler/signatures.h"


// Functions
MAKE_SIG(CMaterialSystem_CreateNamedRenderTargetTextureEx, "48 89 5C 24 ? 48 89 6C 24 ? 48 89 74 24 ? 57 41 56 41 57 48 83 EC ? 8B 9C 24", MATERIALSYSTEM_DLL, void*,
	void*, const char*, int, int, int, int, int, int, int)

MAKE_SIG(CMaterialSystem_GetRenderContext, "48 89 5C 24 ? 48 89 74 24 ? 57 48 83 EC ? 48 8B F9 48 81 C1", MATERIALSYSTEM_DLL, IMatRenderContext*,
	void*)

MAKE_SIG(CMaterialSystem_ClearBuffer, "48 89 5C 24 ? 48 89 6C 24 ? 48 89 74 24 ? 57 48 83 EC ? 48 8B D9 41 0F B6 F9", MATERIALSYSTEM_DLL, void*,
	void*, bool, bool, bool)

MAKE_SIG(CMaterialSystem_GetBackBufferFormat, "48 8B 0D ? ? ? ? 48 8B 01 48 FF 60 ? CC CC 48 8B 0D ? ? ? ? 48 8B 01 48 FF 60 ? CC CC 48 83 EC", MATERIALSYSTEM_DLL, int64_t, int)

//																		    Return, this,  name,		type,        complain, flags
MAKE_SIG(CMateiralSystem_FindTexture, "40 55 57 41 55", MATERIALSYSTEM_DLL, void*,	void*, const char*, const char*, bool,     int)


class IMaterialSystem
{
public:
	inline void* ClearBuffer(bool bClearColor, bool bClearDepth, bool bClearStencil = false) 
	{ 
		return Sig::CMaterialSystem_ClearBuffer(this, bClearColor, bClearDepth, bClearStencil);
	}

	inline void* FindTexture(const char* szTextureName, const char* szType, bool bComplain = true, int iAdditionalFlags = 0)
	{
		return Sig::CMateiralSystem_FindTexture(this, szTextureName, szType, bComplain, iAdditionalFlags);
	}
	
	inline IMatRenderContext* GetRenderContext() { return Sig::CMaterialSystem_GetRenderContext(this); }
	inline int GetBackBufferFormat() { return static_cast<int>(((int64_t(*)())(Sig::CMaterialSystem_GetBackBufferFormat.m_ullAdrs))()); }

	inline void* CreateRenderTarget(
		int w,
		int h,
		int sizeMode,	// Controls how size is generated (and regenerated on video mode change).
		int format,
		int depth)
	{
		return Sig::CMaterialSystem_CreateNamedRenderTargetTextureEx(this,
			NULL,
			w, h,
			sizeMode, format, depth,
			/*TEXTUREFLAGS_CLAMPS | TEXTUREFLAGS_CLAMPT*/4 | 8, 0);
	}

	inline void* CreateRenderTarget(
		const char* szTargetName,
		int w,
		int h,
		int sizeMode,	// Controls how size is generated (and regenerated on video mode change).
		int format,
		int depth)
	{
		return Sig::CMaterialSystem_CreateNamedRenderTargetTextureEx(this,
			szTargetName,
			w, h,
			sizeMode, format, depth,
			/*TEXTUREFLAGS_CLAMPS | TEXTUREFLAGS_CLAMPT*/4 | 8, 0);
	}
};

MAKE_INTERFACE_VERSION(iMaterialSystem, "VMaterialSystem082", IMaterialSystem, MATERIALSYSTEM_DLL)