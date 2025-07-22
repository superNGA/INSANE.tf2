#pragma once
#include "../../Utility/Interface Handler/Interface.h"
#include "Basic Structures.h"

struct model_t;

struct mstudiobbox_t
{
	int					bone;
	int					group;				// intersection group
	vec					bbmin;				// bounding box
	vec					bbmax;
	int					szhitboxnameindex;	// offset to the name of the hitbox.
	int					unused[8]; // This is actually important.

	const char* pszHitboxName() const
	{
		if (szhitboxnameindex == 0)
			return "";

		return ((const char*)this) + szhitboxnameindex;
	}
};

struct mstudiohitboxset_t
{
	int					sznameindex;
	inline char* const	pszName(void) const { return ((char*)this) + sznameindex; }
	int					numhitboxes;
	int					hitboxindex;
	inline mstudiobbox_t* pHitbox(int i) const { return (mstudiobbox_t*)(((byte*)this) + hitboxindex) + i; };
};


struct StudioHdr_t
{
	int					id;
	int					version;

	int					checksum;		// this has to be the same in the phy and vtx files to load!

	//inline const char* pszName(void) const { if (studiohdr2index && pStudioHdr2()->pszName()) return pStudioHdr2()->pszName(); else return name; }
	char				name[64];
	int					length;


	vec				eyeposition;	// ideal eye position

	vec				illumposition;	// illumination center

	vec				hull_min;		// ideal movement hull size
	vec				hull_max;

	vec				view_bbmin;		// clipping bounding box
	vec				view_bbmax;

	int					flags;

	int					numbones;			// bones
	int					boneindex;
	//inline mstudiobone_t* pBone(int i) const { Assert(i >= 0 && i < numbones); return (mstudiobone_t*)(((byte*)this) + boneindex) + i; };
	//int					RemapSeqBone(int iSequence, int iLocalBone) const;	// maps local sequence bone to global bone
	//int					RemapAnimBone(int iAnim, int iLocalBone) const;		// maps local animations bone to global bone

	int					numbonecontrollers;		// bone controllers
	int					bonecontrollerindex;
	//inline mstudiobonecontroller_t* pBonecontroller(int i) const { Assert(i >= 0 && i < numbonecontrollers); return (mstudiobonecontroller_t*)(((byte*)this) + bonecontrollerindex) + i; };

	int					numhitboxsets;
	int					hitboxsetindex;

	// Look up hitbox set by index
	mstudiohitboxset_t* pHitboxSet(int i) const
	{
		return (mstudiohitboxset_t*)(((byte*)this) + hitboxsetindex) + i;
	};

	// Calls through to hitbox to determine size of specified set
	inline void* pHitbox(int i, int set) const
	{
		mstudiohitboxset_t const* s = pHitboxSet(set);
		if (!s)
			return NULL;

		return s->pHitbox(i);
	};
};

class IVModelInfo
{
public:
	virtual							~IVModelInfo(void) {}

	// Returns model_t* pointer for a model given a precached or dynamic model index.
	virtual const model_t * GetModel(int modelindex) = 0;

	// Returns index of model by name for precached or known dynamic models.
	// Does not adjust reference count for dynamic models.
	virtual int						GetModelIndex(const char* name) const = 0;

	// Returns name of model
	virtual const char* GetModelName(const model_t* model) const = 0;
	virtual void* GetVCollide(const model_t* model) = 0;
	virtual void* GetVCollide(int modelindex) = 0;
	virtual void					GetModelBounds(const model_t* model, vec& mins, vec& maxs) const = 0;
	virtual	void					GetModelRenderBounds(const model_t* model, vec& mins, vec& maxs) const = 0;
	virtual int						GetModelFrameCount(const model_t* model) const = 0;
	virtual int						GetModelType(const model_t* model) const = 0;
	virtual void* GetModelExtraData(const model_t* model) = 0;
	virtual bool					ModelHasMaterialProxy(const model_t* model) const = 0;
	virtual bool					IsTranslucent(model_t const* model) const = 0;
	virtual bool					IsTranslucentTwoPass(const model_t* model) const = 0;
	virtual void					RecomputeTranslucency(const model_t* model, int nSkin, int nBody, void /*IClientRenderable*/* pClientRenderable, float fInstanceAlphaModulate = 1.0f) = 0;
	virtual int						GetModelMaterialCount(const model_t* model) const = 0;
	virtual void					GetModelMaterials(const model_t* model, int count, IMaterial** ppMaterial) = 0;
	virtual bool					IsModelVertexLit(const model_t* model) const = 0;
	virtual const char* GetModelKeyValueText(const model_t* model) = 0;
	virtual bool					GetModelKeyValue(const model_t* model, int& buf) = 0; // supports keyvalue blocks in submodels
	virtual float					GetModelRadius(const model_t* model) = 0;

	virtual const void* FindModel(const void* pStudioHdr, void** cache, const char* modelname) const = 0;
	virtual const void* FindModel(void* cache) const = 0;
	virtual	void* GetVirtualModel(const void* pStudioHdr) const = 0;
	virtual byte* GetAnimBlock(const void* pStudioHdr, int iBlock) const = 0;

	// Available on client only!!!
	virtual void					GetModelMaterialColorAndLighting(const model_t* model, vec const& origin,
										qangle const& angles, void* pTrace,
										vec& lighting, vec& matColor) = 0;
	virtual void					GetIlluminationPoint(const model_t* model, void* pRenderable, vec const& origin,
										qangle const& angles, vec* pLightingCenter) = 0;

	virtual int						GetModelContents(int modelIndex) = 0;
	virtual StudioHdr_t*			GetStudiomodel(const model_t* mod) = 0;
	virtual int						GetModelSpriteWidth(const model_t* model) const = 0;
	virtual int						GetModelSpriteHeight(const model_t* model) const = 0;

	// Sets/gets a map-specified fade range (client only)
	virtual void					SetLevelScreenFadeRange(float flMinSize, float flMaxSize) = 0;
	virtual void					GetLevelScreenFadeRange(float* pMinArea, float* pMaxArea) const = 0;

	// Sets/gets a map-specified per-view fade range (client only)
	virtual void					SetViewScreenFadeRange(float flMinSize, float flMaxSize) = 0;

	// Computes fade alpha based on distance fade + screen fade (client only)
	virtual unsigned char			ComputeLevelScreenFade(const vec& vecAbsOrigin, float flRadius, float flFadeScale) const = 0;
	virtual unsigned char			ComputeViewScreenFade(const vec& vecAbsOrigin, float flRadius, float flFadeScale) const = 0;

	// both client and server
	virtual int						GetAutoplayList(const void* pStudioHdr, unsigned short** pAutoplayList) const = 0;

	// Gets a virtual terrain collision model (creates if necessary)
	// NOTE: This may return NULL if the terrain model cannot be virtualized
	virtual void* GetCollideForVirtualTerrain(int index) = 0;

	virtual bool					IsUsingFBTexture(const model_t* model, int nSkin, int nBody, void /*IClientRenderable*/* pClientRenderable) const = 0;
};

MAKE_INTERFACE_VERSION(iVModelInfo, "VModelInfoClient006", IVModelInfo, ENGINE_DLL)