#pragma once
#include <cstdint>
#include "../../SDK/class/Basic Structures.h"
#include "../../SDK/class/Source Entity.h"


struct studioloddata_t
{
	void*					m_pMeshData; // there are studiohwdata_t.m_NumStudioMeshes of these.
	float					m_SwitchPoint;

	int						numMaterials;
	IMaterial**				ppMaterials; /* will have studiohdr_t.numtextures elements allocated */

	int*					pMaterialFlags; /* will have studiohdr_t.numtextures elements allocated */
	int*					m_pHWMorphDecalBoneRemap;
	int						m_nDecalBoneCount;
};

struct studiohwdata_t
{
	int						m_RootLOD;	// calced and clamped, nonzero for lod culling
	int						m_NumLODs;
	studioloddata_t*		m_pLODs;
	int						m_NumStudioMeshes;
};


struct DrawModelState_t
{
	void*					m_pStudioHdr;
	studiohwdata_t*			m_pStudioHWData;
	I_client_renderable*	m_pRenderable;
	const void*				m_pModelToWorld;
	int						m_decals;
	int						m_drawFlags;
	int						m_lod;
};


struct ModelRenderInfo_t
{
	vec						origin;
	qangle					angles;
	I_client_renderable*	pRenderable;
	const model_t*			pModel;
	const matrix3x4_t*		pModelToWorld;
	const matrix3x4_t*		pLightingOffset;
	const vec*				pLightingOrigin;
	int						flags;
	int						entity_index;
	int						skin;
	int						body;
	int						hitboxset;
	int16_t					instance;
};


namespace hook
{
    namespace DME
    {
		typedef int64_t(__fastcall* T_DME)(void*, DrawModelState_t*, ModelRenderInfo_t*, matrix3x4_t*);
		extern T_DME O_DME;
		int64_t __fastcall H_DME(void* pVTable, DrawModelState_t* modelState, ModelRenderInfo_t* renderInfo, matrix3x4_t* boneMatrix);
    }
}