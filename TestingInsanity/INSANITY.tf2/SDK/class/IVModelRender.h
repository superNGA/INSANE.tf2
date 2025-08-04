#pragma once
#include "../../Utility/Interface Handler/Interface.h"

class IVModelRender
{
public:
	virtual int		DrawModel() = 0;

	// This causes a material to be used when rendering the model instead 
	// of the materials the model was compiled with
	virtual void	ForcedMaterialOverride() = 0;

	virtual void	SetViewTarget() = 0;

	// Creates, destroys instance data to be associated with the model
	virtual void CreateInstance() = 0;
	virtual void DestroyInstance() = 0;

	// Associates a particular lighting condition with a model instance handle.
	// FIXME: This feature currently only works for static props. To make it work for entities, etc.,
	// we must clean up the lightcache handles as the model instances are removed.
	// At the moment, since only the static prop manager uses this, it cleans up all LightCacheHandles 
	// at level shutdown.
	virtual void SetStaticLighting() = 0;
	virtual int GetStaticLighting() = 0;

	// moves an existing InstanceHandle to a nex Renderable to keep decals etc. Models must be the same
	virtual bool ChangeInstance() = 0;

	// Creates a decal on a model instance by doing a planar projection
	// along the ray. The material is the decal material, the radius is the
	// radius of the decal to create.
	virtual void AddDecal() = 0;

	// Removes all the decals on a model instance
	virtual void RemoveAllDecals() = 0;

	// Remove all decals from all models
	virtual void RemoveAllDecalsFromAllModels() = 0;

	// Shadow rendering, DrawModelShadowSetup returns the address of the bone-to-world array, NULL in case of error
	virtual void* DrawModelShadowSetup() = 0;
	virtual void DrawModelShadow() = 0;

	// This gets called when overbright, etc gets changed to recompute static prop lighting.
	virtual bool RecomputeStaticLighting() = 0;

	virtual void ReleaseAllStaticPropColorData(void) = 0;
	virtual void RestoreAllStaticPropColorData(void) = 0;

	// Extended version of drawmodel
	virtual int	DrawModelEx() = 0;

	virtual int	DrawModelExStaticProp() = 0;

	virtual bool DrawModelSetup() = 0;
	virtual void DrawModelExecute() = 0;

	// Sets up lighting context for a point in space
	virtual void SetupLighting() = 0;

	// doesn't support any debug visualization modes or other model options, but draws static props in the
	// fastest way possible
	virtual int DrawStaticPropArrayFast() = 0;

	// Allow client to override lighting state
	virtual void SuppressEngineLighting(bool bSuppress) = 0;

	virtual void SetupColorMeshes(int nTotalVerts) = 0;

	virtual void AddColoredDecal() = 0;

	virtual void GetMaterialOverride() = 0;
};

MAKE_INTERFACE_VERSION(iVModelRender, "VEngineModel016", IVModelRender, ENGINE_DLL)